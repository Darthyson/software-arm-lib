/**
 * @file serial.cpp
 * @brief LPC11xx Serial port driver
 *
 * @author Stefan Taferner <stefan.taferner@gmx.at> Copyright (c) 2014
 * @author HoRa Copyright (c) March 2021
 * @author Darthyson <darth@maptrack.de> Copyright (c) 2021
 *
 * @bug No known bugs.
 *
 * @note Interrupt priority set to lowest level in order to avoid conflicts
 *       with the time critical knx bus interrupt source value for high
 *       speed baud rates for debugging of bus timing
 *
 * @par
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 3 as
 * published by the Free Software Foundation.
 */

#include <sblib/serial.h>
#include <sblib/digital_pin.h>
#include <sblib/interrupt.h>
#include <sblib/platform.h>

#define LSR_RDR  0x01   //!> UART line status: receive data ready bit: RBR holds an unread character

#define LSR_THRE 0x20   //!> UART line status: the transmitter hold register (THR) is empty
#define LSR_TEMT 0x40   //!> UART line status: transmitter empty (THR and TSR are empty)
#define LSR_RXFE 0x80   //!> UART line status: error in RX FIFO
#define UART_IE_RBR 0x01    //!> UART read-buffer-ready interrupt
#define UART_IE_THRE 0x02   //!> UART transmit-hold-register-empty interrupt

constexpr uint32_t LINE_STATUS_ERROR_MASK = LSR_OE | LSR_PE | LSR_FE | LSR_BI; //!< Mask for all line status error bits

Serial::Serial(const int rxPin, const int txPin) :
    enabled_(false),
    errorCallback(nullptr),
    errorCallbackContext(nullptr)
{
    setRxPin(rxPin);
    setTxPin(txPin);
}

void Serial::setRxPin(const int rxPin)
{
    if (enabled())
    {
        end();
    }
    pinMode(rxPin, SERIAL_RXD);
}

void Serial::setTxPin(const int txPin)
{
    if (enabled())
    {
        end();
    }
    pinMode(txPin, SERIAL_TXD);
}

void Serial::begin(const int baudRate, const SerialConfig config)
{
    disableInterrupt(UART_IRQn);

    LPC_SYSCON->SYSAHBCLKCTRL |= 1 << 12; // Enable UART clock
    LPC_SYSCON->UARTCLKDIV = 1;           // divided by 1

    LPC_UART->LCR = 0x80 | config;

    unsigned int val = SystemCoreClock * LPC_SYSCON->SYSAHBCLKDIV /
                       LPC_SYSCON->UARTCLKDIV / 16 / baudRate;

    if (baudRate == 460800) //FIXME works only with SystemCoreClock=48000000 ?
    {
        val = 5;
        LPC_UART->FDR = (0x00a3); //DIVADDVAL = 3, MULVAL = 10
    }
    else if (baudRate == 576000) //FIXME works only with SystemCoreClock=48000000 ?
    {
        val = 3;
        LPC_UART->FDR = (0x00fb); //DIVADDVAL = 11, MULVAL = 15
    }

    LPC_UART->DLM = val / 256;
    LPC_UART->DLL = val % 256;

    LPC_UART->LCR = static_cast<int>(config); // Configure data bits, parity, stop bits
    LPC_UART->FCR = 0x07;         // Enable and reset TX and RX FIFO.
    LPC_UART->MCR = 0;            // Disable modem controls (DTR, DSR, RTS, CTS)
    LPC_UART->IER |= UART_IE_RBR; // Enable RX/TX interrupts

    // Ensure a clean start, no data in either TX or RX FIFO
    clearBuffers();
    flush();

    // Drop data from the RX FIFO
    while (LPC_UART->LSR & LSR_RDR)
        val = LPC_UART->RBR;

    //added by Hora in order to provide the highest interrupt level to the bus timer of the lib
    NVIC_SetPriority(UART_IRQn, 3);

    enableInterrupt(UART_IRQn);
    enabled_ = true;
}

void Serial::end()
{
    flush();
    disableInterrupt(UART_IRQn);
    LPC_SYSCON->SYSAHBCLKCTRL &= ~(1 << 12); // Disable UART clock
    enabled_ = false;
}

uint32_t Serial::write(byte ch)
{
    if (!enabled_)
    {
        return 0;
    }

#if defined(SERIAL_WRITE_DIRECT) && !defined(IAP_EMULATION)
    // wait until the transmitter hold register is free
    while (!(LPC_UART->LSR & LSR_THRE))
        ;
    LPC_UART->THR = ch;
    return 1;
#endif

    if (writeEmpty() && (LPC_UART->LSR & LSR_THRE))
    {
        // Transmitter hold register and write buffer are empty -> directly send
        LPC_UART->THR = ch;
        LPC_UART->IER |= UART_IE_THRE;
        return 1;
    }

    // Wait until the output buffer has space
    while (writeFull())
        ;

    pushWrite(ch);
    LPC_UART->IER |= UART_IE_THRE;

#ifdef IAP_EMULATION
    ///\todo This is ok, but better would be to write into a temp-file and then check its content?
    // Simulate for unit tests, that the byte was sent
    LPC_UART->LSR |= LSR_THRE; // Set line status register to transmitter hold register empty
    UART_IRQHandler();
#endif

    return 1;
}

void Serial::flush()
{
    if (!enabled_)
    {
        return;
    }
#ifdef SERIAL_WRITE_DIRECT
    while ((LPC_UART->LSR & (LSR_THRE | LSR_TEMT)) != (LSR_THRE | LSR_TEMT))
        ;
#else
    while (!writeEmpty())
            ;
#endif
}

int Serial::read()
{
    if (!enabled_)
    {
        return -1;
    }

    const bool isReadFull = readFull();
    const int ch = BufferedStream::read();

    if (isReadFull && (LPC_UART->LSR & LSR_RDR))
    {
        disableInterrupt(UART_IRQn);
        interruptHandler();
        enableInterrupt(UART_IRQn);
    }

    return ch;
}

void Serial::interruptHandler()
{
    //FIXME check if this is save to activate
    /*
    if (!enabled_)
    {
        return;
    }
    */

    // Rx interrupt handling
    uint32_t lineStatusRegister;
    while ((lineStatusRegister = LPC_UART->LSR) & LSR_RDR)
    {
        if (lineStatusRegister & LINE_STATUS_ERROR_MASK)
        {
            handleLineError(lineStatusRegister);
            if (lineStatusRegister & LSR_BI)
            {
                LPC_UART->RBR; // Discard the null byte generated by the break condition
                continue;
            }
        }

        if (!readFull())
        {
            pushRead(LPC_UART->RBR);
        }
        else
        {
            LPC_UART->RBR; // if the readBuffer is full, empty UART
        }
    }

    // Tx interrupt handling
    if (LPC_UART->LSR & LSR_THRE)
    {
        if (writeEmpty())
        {
            LPC_UART->IER &= ~UART_IE_THRE;
        }
        else
        {
            LPC_UART->THR = popWrite();
        }
    }
}

void Serial::setErrorCallback(SerialErrorCallback callback, void* context)
{
    errorCallback = callback;
    errorCallbackContext = context;
}

void Serial::handleLineError(const uint32_t lineStatus) const
{
    if (errorCallback)
    {
        errorCallback(lineStatus & LINE_STATUS_ERROR_MASK, errorCallbackContext);
    }
}
