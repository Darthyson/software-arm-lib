/**
 * @brief LPC11xx Serial port driver
 *
 * @author Stefan Taferner <stefan.taferner@gmx.at> Copyright (c) 2014
 * @author HoRa Copyright (c) March 2021
 * @author Darthyson <darth@maptrack.de> Copyright (c) 2026
 *
 * @note Interrupt priority set to the lowest level (3) in order to avoid conflicts
 *       with the time critical KNX bus interrupt source value for high
 *       speed baud rates for debugging of bus timing
 *
 * @par
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 3 as
 * published by the Free Software Foundation.
 */

#include "sblib/serial.h"
#include "sblib/serial_registers.h"
#include "sblib/digital_pin.h"
#include "sblib/interrupt.h"
#include "sblib/platform.h"


constexpr uint8_t TxFifoSize = 16; //!< The Tx FIFO size of the HW. If no FIFOs are used, this must be changed to 1.

constexpr uint32_t InitialEnabledUARTInterrupts = IER_RBRIE; //!< Initially enable only Rx interrupt
constexpr uint32_t EnableTransmitInterrupt = InitialEnabledUARTInterrupts | IER_THRIE;
constexpr uint32_t DisableTransmitInterrupt = InitialEnabledUARTInterrupts & ~IER_THRIE;

Serial::Serial(const uint32_t rxPin, const uint32_t txPin) :
    enabled_(false),
    receiveBuffer(nullptr),
    transmitBuffer(nullptr),
    errorCallback(nullptr),
    errorCallbackContext(nullptr)
{
    setRxPin(rxPin);
    setTxPin(txPin);
}

Serial::~Serial()
{
    end();
    Stream::~Stream();
}

void Serial::allocateBuffers(const RingBuffer::Size receiveBufferSize, const RingBuffer::Size transmitBufferSize)
{
    delete receiveBuffer;
    receiveBuffer = new RingBuffer(receiveBufferSize);
    delete transmitBuffer;
    transmitBuffer = new RingBuffer(transmitBufferSize);
}

void Serial::deallocateBuffers()
{
    delete receiveBuffer;
    receiveBuffer = nullptr;
    delete transmitBuffer;
    transmitBuffer = nullptr;
}

// ReSharper disable once CppMemberFunctionMayBeConst
void Serial::clearBuffers()
{
    disableInterrupt(UART_IRQn);
    if (receiveBuffer != nullptr)
    {
        receiveBuffer->clear();
    }

    if (transmitBuffer != nullptr)
    {
        transmitBuffer->clear();
    }
    enableInterrupt(UART_IRQn);
}

int16_t Serial::peek()
{
    if (receiveBuffer == nullptr)
    {
        return -1; // receiveBuffer was probably deallocated in end()
    }

    return receiveBuffer->peek();
}

uint32_t Serial::available()
{
    if (receiveBuffer == nullptr)
    {
        return 0; // receiveBuffer was probably deallocated in end()
    }

    return receiveBuffer->available();
}

void Serial::setRxPin(const uint32_t rxPin)
{
    if (enabled())
    {
        end();
    }
    // Enable also hysteresis for better noise immunity on the input pin
    pinMode(rxPin, SERIAL_RXD | HYSTERESIS);
}

void Serial::setTxPin(const uint32_t txPin)
{
    if (enabled())
    {
        end();
    }
    pinMode(txPin, SERIAL_TXD);
}

void Serial::begin(const uint32_t baudRate, const SerialConfig config, const RxTriggerLevel rxTriggerLevel,
               const RingBuffer::Size receiveBufferSize, const RingBuffer::Size transmitBufferSize)
{
    disableInterrupt(UART_IRQn);

    // Enable UART clock, GPIO pins must have been already configured (see UM10398 13.2 p.198)!
    LPC_SYSCON->SYSAHBCLKCTRL |= UART_CLOCK_ENABLE;
    LPC_SYSCON->UARTCLKDIV = 1; // divided by 1

    LPC_UART->LCR = config; // Configure data bits, parity, stop bits

    uint32_t divisorLatchValue;
    // Restrictions for the fractional divider (UM10398 13.5.15 p. 216):
    // 1 <= MULVAL <= 15
    // 0 <= DIVADDVAL < MULVAL
    switch (baudRate) // pre-set val and FDR only works for system clock 48MHz
    {
        case 460800:
            divisorLatchValue = 5;
            LPC_UART->FDR = 0xa3; //MULVAL = 10, DIVADDVAL = 3
            break;
        case 576000:
            divisorLatchValue = 3;
            LPC_UART->FDR = 0xfb; //MULVAL = 15, DIVADDVAL = 11
            break;
        case 661765:
            divisorLatchValue = 4;
            LPC_UART->FDR = 0xf2; //MULVAL = 15, DIVADDVAL = 2
            break;
        case 921600:
            divisorLatchValue = 3;
            LPC_UART->FDR = 0xc1; //MULVAL = 12, DIVADDVAL = 1
            break;
        default:
            // 2400, 4800, 1,000,000, 1,500,000 baud match perfectly with 48MHz system clock
            // Other lower baud rates 9600 - 230400 should be within the error margin
            // For unsupported baud rates, disable the fractional divider to avoid incorrect baud rates
            divisorLatchValue = SystemCoreClock * LPC_SYSCON->SYSAHBCLKDIV /
                                     LPC_SYSCON->UARTCLKDIV / 16 / baudRate;
            LPC_UART->FDR = 0x10; //DIVADDVAL = 0, MULVAL = 1 (fractional divider disabled)
            break;
    }

    LPC_UART->LCR |= LCR_DLAB; // Set DLAB to access divisor latches DLM/DLL
    LPC_UART->DLM = divisorLatchValue / 256;
    LPC_UART->DLL = divisorLatchValue % 256;
    LPC_UART->LCR &= ~LCR_DLAB; // Clear DLAB to switch back to RBR/THR and IER access

    uint32_t triggerLevelValue;
    switch (rxTriggerLevel)
    {
        case RxTriggerLevel::CHAR_1:
            triggerLevelValue = FCR_RXTL_0;
            break;
        case RxTriggerLevel::CHAR_4:
            triggerLevelValue = FCR_RXTL_1;
            break;
        case RxTriggerLevel::CHAR_8:
            triggerLevelValue = FCR_RXTL_2;
            break;
        case RxTriggerLevel::CHAR_14:
            triggerLevelValue = FCR_RXTL_3;
            break;
        default:
            triggerLevelValue = FCR_RXTL_0; // Default to 1 character
    }

    // Set Rx FIFO triggerLevel and enable and reset Tx and Rx FIFOs
    // If no FIFOs are used, the value of `TxFifoSize` must be changed to 1!
    LPC_UART->FCR = triggerLevelValue | FCR_FIFOEN | FCR_RXFIFORES | FCR_TXFIFORES;
    LPC_UART->MCR = MCR_NONE;   // Disable modem controls (DTR, DSR, RTS, CTS)

    LPC_UART->IER = InitialEnabledUARTInterrupts;

    allocateBuffers(receiveBufferSize, transmitBufferSize);

    // Drop data from the Rx FIFO
    while (LPC_UART->LSR & LSR_RDR)
    {
        divisorLatchValue = LPC_UART->RBR;
    }

    //added by Hora in order to provide the highest interrupt level to the bus timer of the lib
    NVIC_SetPriority(UART_IRQn, 3);

    LPC_UART->IIR; // Read IIR to clear any pending UART interrupt
    clearPendingInterrupt(UART_IRQn); // Clear any stale pending interrupt
    enableInterrupt(UART_IRQn);

#ifdef IAP_EMULATION
    // Only for unit testing: initialize the IIR
    testInitIIR();
#endif

    enabled_ = true;
}

void Serial::begin(const uint32_t baudRate)
{
    if (enabled())
    {
        end();
    }
    begin(baudRate, SERIAL_8N1);
}

void Serial::end()
{
    // Wait for write buffer to drain into UART Tx FIFO
    flush();

#ifdef IAP_EMULATION
    LPC_UART->LSR |= LSR_TEMT; // simulate empty transmit shift register for unit tests
#endif

    while (!(LPC_UART->LSR & LSR_TEMT))
    {
        // ReSharper disable once CppRedundantEmptyStatement
        ; // Wait for Tx transmitter shift register to be empty, before killing the clock
    }

    disableInterrupt(UART_IRQn);
    LPC_SYSCON->SYSAHBCLKCTRL &= ~UART_CLOCK_ENABLE; // Disable UART clock

    deallocateBuffers();
    enabled_ = false;
}

uint32_t Serial::write(const uint8_t ch)
{
    if (transmitBuffer == nullptr)
    {
        return 0; // transmitBuffer was probably deallocated in end()
    }
    writeTotal++;

    while(!transmitBuffer->push(ch))
    {
        // ReSharper disable once CppRedundantEmptyStatement
        ;
    }

    if (LPC_UART->IER == InitialEnabledUARTInterrupts)
    {
        // Tx is disabled
        setPendingInterrupt(UART_IRQn);
        writeDirect++;
    }
    else
    {
        writeQueued++;
    }
    LPC_UART->IER = EnableTransmitInterrupt; // Make sure Rx and Tx interrupt are enabled;

#ifdef IAP_EMULATION
    // for unit testing only
    // Simulate that the THRE interrupt is triggered after the byte was "sent" and the THR is empty again
    testSimulateByteSent(ch);
#endif
    return 1;
}

uint32_t Serial::write(const uint8_t* data, const uint32_t count)
{
    if (transmitBuffer == nullptr || count == 0)
    {
        return 0;
    }

    uint32_t written = 0;
    for (uint32_t i = 0; i < count; i++)
    {
        while (!transmitBuffer->push(data[i]))
        {
            // ReSharper disable once CppRedundantEmptyStatement
            ;
        }
        written++;

#ifdef IAP_EMULATION
        // for unit testing only
        // Simulate that the THRE interrupt is triggered after the byte was "sent" and the THR is empty again
        testSimulateByteSent(data[i]);
#endif
    }

    if (LPC_UART->IER == InitialEnabledUARTInterrupts)
    {
        // Tx is disabled
        setPendingInterrupt(UART_IRQn);
        writeDirect += written;
    }
    else
    {
        writeQueued+= written;
    }
    LPC_UART->IER = EnableTransmitInterrupt; // Make sure Rx and Tx interrupt are enabled;
    return written;
}

void Serial::flush()
{
    if (!enabled_)
    {
        return;
    }

    while (!transmitBuffer->empty())
    {
        // ReSharper disable once CppRedundantEmptyStatement
        ; // Wait until all bytes in the SW transmit buffer have been written into the UART Tx FIFO
    }
}

int16_t Serial::read()
{
    if (receiveBuffer == nullptr)
    {
        return -1; // receiveBuffer was probably deallocated in end()
    }

    const int16_t ch = receiveBuffer->pop();

    if (ch != -1) readCounter++;
    return ch;
}

void Serial::uartInterruptHandler()
{
    isrEntries++;
    // Tx part
    if (LPC_UART->LSR & LSR_THRE)
    {
        // If the transmitter hold register (THRE) is empty,
        // the whole Tx FIFO is also empty
        // See: https://community.nxp.com/t5/LPCXpresso-IDE/can-t-get-uart-tx-fifo-to-work/td-p/560098

        // Fill the HW Tx FIFO in one go to reduce interrupt frequency
        for (uint8_t i = 0; i < TxFifoSize; i++)
        {
            const int16_t nextByteToSend = transmitBuffer->pop();
            if (nextByteToSend < 0)
            {
                // Nothing to send, so disable Tx interrupt
                LPC_UART->IER = InitialEnabledUARTInterrupts;
                break;
            }

            LPC_UART->THR = nextByteToSend;
            transmitCounter++;
        }
    }

    // Rx part
    uint8_t lineStatusRegister = LPC_UART->LSR; // Read line status
    while (lineStatusRegister & LSR_RDR)
    {
        const uint8_t errorFlags = lineStatusRegister & LSR_RX_ERROR_MASK;
        const uint8_t receivedByte = LPC_UART->RBR; // Read the actual Rx-byte
        if (errorFlags)
        {
            // current byte has an error --> call the error callback if set
            handleLineError(errorFlags, receivedByte);
        }
        receiveCounter++;
        // Put the byte in the receive buffer even if it has an error
        if (!receiveBuffer->push(receivedByte))
        {
            receiveDropped++; // drop the byte in receiver holding register
        }
        lineStatusRegister = LPC_UART->LSR;
    }
}

void Serial::uartNewInterruptHandler()
{
    isrEntries++;
    // Read IIR (Interrupt Identification Register)
    const uint32_t iir = LPC_UART->IIR;

    if (iir & IIR_INTSTATUS) // INTSTATUS is active low: bit0=1 means no interrupt pending
    {
        isrFakePendings++;
        const uint32_t lsr = LPC_UART->LSR;
        if (!(lsr & LSR_THRE))
        {
            //fatalError(); ///\todo remove on release
        }
        else
        {
            // Check if there is more data to send
            const int16_t nextByteToSend = transmitBuffer->pop();
            if (nextByteToSend > -1)
            {
                LPC_UART->THR = nextByteToSend;
                transmitCounter++;
            }
            else
            {
                fatalError(); ///\todo remove on release
            }
        }
    }

    // Read IIR in a loop until there are no more pending interrupts
    // E.g. multiple bytes in Rx FIFO or a Tx byte was sent and the THRE interrupt is still pending
    if (!(iir & IIR_INTSTATUS)) // INTSTATUS is active low: bit0=1 means no interrupt pending
    {
        isrRealPendings++;
        switch (iir & IIR_INTID_MASK)
        {
             // Line status/error interrupt
             // Reset by reading the Line Status Register (LSR)
            case IIR_INTID_RLS:
            {
                fatalError(); // not implemented
                break;
            }

            // Rx data available or trigger level reached in FIFO
            // Reset by reading the Receiver Buffer Register (RBR) or FIFO drops below trigger level.
            case IIR_INTID_RDA:
                isrCounterRDA++;
                [[fallthrough]]; // Intentional fall-through to CTI: both cases require draining all bytes from the Rx FIFO.

            // Character timeout interrupt, indicates that the Rx FIFO has not received a new byte within
            // a certain time after the last byte was received.
            // Reset by reading the Receiver Buffer Register (RBR)
            case IIR_INTID_CTI:
            {
                isrCounterCTI++;
                uint32_t lineStatusRegister = LPC_UART->LSR;
                while(lineStatusRegister & LSR_RDR)
                {
                    const uint8_t receivedByte = LPC_UART->RBR;
                    if (const uint8_t errorFlags = lineStatusRegister & LSR_RX_ERROR_MASK)
                    {
                        // current byte has an error, so call the error callback if set
                        handleLineError(errorFlags, receivedByte);
                    }
                    if (!receiveBuffer->push(receivedByte)) // Put the byte in the receive buffer even if it has an error, so the caller can decide what to do with it (e.g. discard or return with an error flag)
                    {
                        receiveDropped++;
                    }
                    receiveCounter++;
                    lineStatusRegister = LPC_UART->LSR;
                }
                break;
            }

            // Transmitter Holding Register Empty (THRE) interrupt
            // Indicates that the UART is ready to accept a new byte for transmission
            // Reset by reading the Interrupt Identification Register (IIR)
            //          or writing to the Transmitter Holding Register (THR)
            case IIR_INTID_THRE:
            {
                // Check if there is more data to send
                const int16_t nextByteToSend = transmitBuffer->pop();
                if (nextByteToSend > -1)
                {
                    LPC_UART->THR = nextByteToSend;
                    transmitCounter++;
                }
                else
                {
                    //LPC_UART->IER &= ~IER_THRIE; // No more data to send, disable Tx interrupt
                }
                // for unit testing only, simulates that the THRE interrupt was cleared by the ISR
#               ifdef IAP_EMULATION
                    testClearInterrupt(IIR_INTID_THRE, &LPC_UART->THR, 0, &LPC_UART->LSR, LSR_THRE, false);
#               endif
                break;
            }

            // Modem interrupt, e.g. CTS, DSR, RI or DCD change
            // Reset by reading the Modem Status Register (MSR)
            case IIR_INTID_MODEM:
                LPC_UART->MSR;
                fatalError(); ///todo delete on release
                break;

            // We should never get here, because all possible INTID values are handled above.
            // Auto-baud interrupts (IIR_ABEOINT/IIR_ABTOINT) are not in IIR_INTID_MASK
            // and need separate implementation.
            default:
                fatalError();
                break;
        }
    }
}

void Serial::setErrorCallback(const SerialErrorCallback callback, void* context)
{
    errorCallback = callback;
    errorCallbackContext = context;
}

void Serial::handleLineError(const uint8_t errorFlags, const uint8_t faultyByte) const
{
    if (errorCallback)
    {
        errorCallback(errorFlags, faultyByte, errorCallbackContext);
    }
}

/********************************************************************************************/
/* All methods below are only for unit testing and should never be used in production code. */
/********************************************************************************************/
#ifdef IAP_EMULATION
void Serial::testSimulateByteSent([[maybe_unused]]const uint8_t sentByte)
{

    testSetInterruptPending(IIR_INTID_THRE, &LPC_UART->THR, 0, &LPC_UART->LSR, LSR_THRE, true);
    sentBytesBuffer.push_back(sentByte);
}

void Serial::testInitIIR()
{
    // Initialize IIR to no interrupt pending, so that the interrupt handler does not get stuck in the IIR loop
    LPC_UART->IIR = 0;
    LPC_UART->IIR |= IIR_INTSTATUS; // INTSTATUS is active low: bit0=1 means no interrupt pending
}

void Serial::testSimulateRegisterValue(volatile uint32_t * registerToSet, const uint32_t value)
{
    if (registerToSet == nullptr)
    {
        return;
    }

    // Set the specified UART register to the specified value
    *registerToSet = value;
}

void Serial::testSimulateRegisterBitsChange(volatile uint32_t * registerToChange, const uint32_t bitMask,
        const bool setBits)
{
    if (registerToChange == nullptr)
    {
        return;
    }

    if (setBits)
    {
        // Set the specified bits in the UART register
        *registerToChange |= bitMask;
    }
    else
    {
        // Clear the specified bits in the UART register
        *registerToChange &= ~bitMask;
    }
}

void Serial::testChangeInterruptPending(const uint32_t interruptID, const bool setPending,
        volatile uint32_t * registerToSet, const uint32_t valueToSet,
        volatile uint32_t * registerToChange, const uint8_t bitMaskToSet, const bool setBits)
{
    testSimulateRegisterValue(registerToSet, valueToSet);
    testSimulateRegisterBitsChange(registerToChange, bitMaskToSet, setBits);

    // Note: INTSTATUS is active low: bit0=0 means interrupt pending
    //                                bit0=1 means no interrupt pending

    if (setPending)
    {
        // Set interrupt identification register to interrupt pending
        LPC_UART->IIR |= interruptID;
    }
    else
    {
        // Set interrupt identification register to no interrupt pending
        LPC_UART->IIR &= ~interruptID;
    }

    if ((LPC_UART->IIR & IIR_INTID_MASK) != 0)
    {
         // Set at least one interrupt is still pending
        LPC_UART->IIR &= ~IIR_INTSTATUS;
    }
    else
    {
        // No interrupts are pending
        LPC_UART->IIR |= IIR_INTSTATUS;
    }

    // Call the interrupt handler if at least one interrupt is still pending
    if (!(LPC_UART->IIR & IIR_INTSTATUS))
    {
        UART_IRQHandler();
    }
}

void Serial::testClearInterrupt(const uint32_t interruptID, volatile uint32_t * registerToSet,
         const uint32_t valueToSet, volatile uint32_t * registerToChange, const uint8_t bitMaskToChange,
         const bool setBits)
{
    testChangeInterruptPending(interruptID, false, registerToSet, valueToSet, registerToChange,
        bitMaskToChange, setBits);
}

void Serial::testSetInterruptPending(const uint32_t interruptID, volatile uint32_t * registerToSet,
         const uint32_t valueToSet, volatile uint32_t * registerToChange, const uint8_t bitMaskToChange,
         const bool setBits)
{
    testChangeInterruptPending(interruptID, true, registerToSet, valueToSet, registerToChange, bitMaskToChange, setBits);
}

std::vector<uint8_t> Serial::sentBytesBuffer;
const std::vector<uint8_t>& Serial::testGetSentBytes()
{
    return sentBytesBuffer;
}

void Serial::testClearSentBytes()
{
    sentBytesBuffer.clear();
}
#endif
