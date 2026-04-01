/**
 * @brief LPC11xx Serial port driver register bit definitions
 * 
 * @details This header defines the relevant UART register bits and masks for the LPC11xx.
 *
 * @author Stefan Taferner <stefan.taferner@gmx.at> Copyright (c) 2014
 * @author HoRa Copyright (c) March 2021
 * @author Darthyson <darth@maptrack.de> Copyright (c) 2026
 *
 * @note This header only contains register bit definitions and does not define the actual UART register addresses
 * 
 * @par
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 3 as
 * published by the Free Software Foundation.
 */
#ifndef SBLIB_SERIAL_REGISTERS_H_
#define SBLIB_SERIAL_REGISTERS_H_


#include <cstdint>

constexpr uint32_t UART_CLOCK_ENABLE = 1 << 12; //!< Bit in SYSAHBCLKCTRL to enable clock for UART

/**
 * @brief UART Receiver Buffer Register (RBR), read-only, DLAB=0
 *
 * Holds the next byte received by the UART.
 * @note UM10398 LPC11xx User Manual, section 13.5.1 p.201
 */
enum UART_RBR : uint32_t
{
    RBR = 0xff //!< The received byte in the RBR register
};

/**
 * @brief UART Transmitter Holding Register (THR), write-only, DLAB=0
 *
 * Holds the next byte to be transmitted by the UART.
 * @note UM10398 LPC11xx User Manual, section 13.5.2 p.201
 */
enum UART_THR : uint32_t
{
    THR = 0xff //!< The byte to be transmitted in the THR register
};

/**
 * @brief UART Divisor Latch registers DLL (LSB), read-write, DLAB=1
 *
 * Holds the lower 8 bit of the divisor latch value for setting the baud rate.
 * @note UM10398 LPC11xx User Manual, section 13.5.3 p.201
 */
enum UART_DLL : uint32_t
{
    DLL = 0xff //!< The lower 8 bit of the divisor latch value in the DLL register
};

/**
 * @brief UART Divisor Latch registers DLM (MSB), read-write, DLAB=1
 *
 * Holds the higher 8 bit of the divisor latch value for setting the baud rate.
 * @note UM10398 LPC11xx User Manual, section 13.5.3 p.201
 */
enum UART_DLM : uint32_t
{
    DLM = 0xff //!< The higher 8 bit of the divisor latch value in the DLM register
};

/**
 * @brief UART Interrupt Enable Register (IER), (read/write), DLAB=0
 *
 * Controls which UART interrupts are enabled. Each bit enables a specific interrupt source,
 * such as receive data available or transmitter holding register empty.
 * @note UM10398 LPC11xx User Manual, section 13.5.4 p.202
 */
enum UART_IER : uint32_t
{
    IER_RBRIE = 1 << 0, //!< Receive data available interrupt
    IER_THRIE = 1 << 1, //!< Transmit-hold-register empty interrupt
    IER_RXLIE = 1 << 2, //!< Rx line status interrupt
    IER_ABOEINTEN = 1 << 8, //!< Auto-baud end interrupt
    IER_ABTOINTEN = 1 << 9  //!< Auto-baud time-out interrupt
};

/**
 * @brief UART Interrupt Identification Register (IIR), read-only
 *
 * Holds the interrupt status.
 * @note UM10398 LPC11xx User Manual, section 13.5.5 p.203
 */
enum UART_IIR : uint32_t
{
    /**
     * @brief Interrupt status bit.
     * Indicates whether an interrupt is pending.
     * This bit is active low: 0 means at least one interrupt is pending,
     *                         1 means no interrupt is pending.
     */
    IIR_INTSTATUS = 1 << 0,

    /**
     * @brief Receive Line Status (RLS) bit.
     * @note Interrupt reset by reading the Line Status Register (LSR).
     */
    IIR_INTID_RLS = 3 << 1,

    /**
     * @brief Receive Data Available (RDA) bit.
     * @note Interrupt reset by reading the Receiver Buffer Register (RBR) or FIFO drops below trigger level.
     */
    IIR_INTID_RDA = 2 << 1,

    /**
     * @brief Character Time-out Indicator (CTI) bit.
     * @note Interrupt reset by reading the Receiver Buffer Register (RBR).
     */
    IIR_INTID_CTI = 6 << 1,

    /**
     * @brief Transmitter hold register is empty (THRE).
     * @note Interrupt reset by reading the Interrupt Identification Register (IIR) or writing to the Transmitter Holding Register (THR).
     */
    IIR_INTID_THRE = 1 << 1,

    /**
     * @brief Modem Interrupt bit.
     * @note Interrupt reset by reading the Modem Status Register (MSR).
     */
    IIR_INTID_MODEM = 0 << 1,

    /**
     * @brief Mask for the interrupt ID bits in the UART IIR register.
     */
    IIR_INTID_MASK = 7 << 1,

    /**
     * @brief FIFO enable bits.
     */
    IIR_FIFOENABLE_MASK = 3 << 6,

    /**
     * @brief End of auto-baud interrupt
     */
    IIR_ABEOINT = 1 << 8,

    /**
     * @brief Auto-baud time-out interrupt
     */
    IIR_ABTOINT = 1 << 9
};

/**
 * @brief UART FIFO Control Register (FCR), write-only
 *
 * Controls the FIFO settings of the UART, including enabling/disabling FIFOs,
 * resetting FIFOs, and setting the RX trigger level.
 * @note UM10398 LPC11xx User Manual, section 13.5.6 p.205
 */
enum UART_FCR : uint32_t
{
    FCR_FIFOEN    = 1 << 0, //!< FIFO Enable, 0 = FIFOs disabled, 1 = Tx/Rx FIFOs enabled
    FCR_RXFIFORES = 1 << 1, //!< Rx FIFO Reset, 0 = no impact, 1 = clear all bytes in Rx FIFO
    FCR_TXFIFORES = 1 << 2, //!< Tx FIFO Reset, 0 = no impact, 1 = clear all bytes in Tx FIFO
    FCR_RXTL_0    = 0 << 6, //!< Rx Trigger Level 0 (1 character)
    FCR_RXTL_1    = 1 << 6, //!< Rx Trigger Level 1 (4 characters)
    FCR_RXTL_2    = 2 << 6, //!< Rx Trigger Level 2 (8 characters)
    FCR_RXTL_3    = 3 << 6, //!< Rx Trigger Level 3 (14 characters)
};

/**
 * @brief UART Line Control Register (LCR) bits, (read/write)
 *
 * Controls the data format of the UART communication, including word length, stop bits, parity, and break control. * 
 * @note UM10398 LPC11xx User Manual, section 13.5.7 p.206
 */
enum UART_LCR : uint32_t
{
    LCR_WLS_MASK = 3 << 0, //!< Word length select mask
    LCR_WLS_5    = 0 << 0, //!< Word length select: 5 bits
    LCR_WLS_6    = 1 << 0, //!< Word length select: 6 bits
    LCR_WLS_7    = 2 << 0, //!< Word length select: 7 bits
    LCR_WLS_8    = 3 << 0, //!< Word length select: 8 bits
    LCR_SBS_1    = 0 << 2, //!< Stopbit select: 1 stop bit
    LCR_SBS_2    = 1 << 2, //!< Stopbit select: 2 stop bits, or 1.5 stop bits if word length is 5 bits
    LCR_PE       = 1 << 3, //!< Parity enable
    LCR_PS_MASK  = 3 << 4, //!< Parity select mask
    LCR_PS_ODD   = 0 << 4, //!< Parity select: Odd parity
    LCR_PS_EVEN  = 1 << 4, //!< Parity select: Even parity
    LCR_PS_FORCED1 = 2 << 4, //!< Parity select: Force parity bit to 1
    LCR_PS_FORCED0 = 3 << 4, //!< Parity select: Force parity bit to 0
    LCR_BC       = 1 << 6, //!< Break control
    LCR_DLAB     = 1 << 7  //!< Divisor latch access bit
};

/**
 * @brief UART Modem Control Register (MCR) bits, (read/write)
 *
 * Controls the modem control signals of the UART, such as DTR, RTS, loopback mode, and flow control.
 * @note UM10398 LPC11xx User Manual, section 13.5.8 p.207
 */
enum UART_MCR : uint32_t
{
    MCR_NONE  = 0,        //!< No modem control
    MCR_DTRC  = 1 << 0, //!< Data Terminal Ready (DTR) control
    MCR_RTSC  = 1 << 1, //!< Request to Send (RTS) control
    MCR_LMS   = 1 << 4, //!< Loopback Mode Select
    MCR_RTSEN = 1 << 6, //!< RTS flow control
    MCR_CTSEN = 1 << 7  //!< CTS flow control
};

/**
 * @brief UART Line Status Register (LSR), read-only
 *
 * Indicates the current status of the UART line, including errors and FIFO status.
 * @warning Reading the LSR clears certain error bits (OE, PE, FE, BI), so it should only be read in the interrupt handler.
 * @note UM10398 LPC11xx User Manual, section 13.5.9 p.210-211
 */
enum UART_LSR : uint32_t
{
    LSR_RDR  = 1 << 0, //!< The receiver buffer register (RBR) holds an unread character
    LSR_OE   = 1 << 1, //!< Overrun error, reading the LSR clears this bit!
    LSR_PE   = 1 << 2, //!< Parity error, reading the LSR clears this bit!
    LSR_FE   = 1 << 3, //!< Framing error, reading the LSR clears this bit!
    LSR_BI   = 1 << 4, //!< Break indication, reading the LSR clears this bit!
    LSR_THRE = 1 << 5, //!< The transmitter hold register (THR) is empty
    LSR_TEMT = 1 << 6, //!< Transmitter hold and shift registers are empty (THR and TSR)
    LSR_RXFE = 1 << 7, //!< Error in Rx FIFO (PE, FE or BI), reading the LSR clears this bit!
    LSR_RX_ERROR_MASK = LSR_OE | LSR_PE | LSR_FE | LSR_BI //!< Error in Rx FIFO
};

/**
 * @brief UART Modem Status Register (MSR), read-only
 *
 * Holds the status information about the modem control lines, such as CTS, DSR, RI and DCD.
 * @note UM10398 LPC11xx User Manual, section 13.5.10 p.212
 */
enum UART_MSR : uint32_t
{
    MSR_DCTS = 1 << 0, //!< Delta Clear To Send (CTS). CTS state changed since last read of MSR
    MSR_DDSR = 1 << 1, //!< Delta Data Set Ready (DSR). DSR state changed since last read of MSR
    MSR_TERI = 1 << 2, //!< Trailing Edge Ring Indicator (RI). Low to high transition on RI since last read of MSR
    MSR_DDCD = 1 << 3, //!< Delta Data Carrier Detect (DCD). DCD state changed since last read of MSR
    MSR_CTS  = 1 << 4, //!< Complement of Clear To Send signal (~CTS)
    MSR_DSR  = 1 << 5, //!< Complement of Data Set Ready signal (~DSR)
    MSR_RI   = 1 << 6, //!< Complement of Ring Indicator signal (~RI)
    MSR_DCD  = 1 << 7  //!< Complement of Data Carrier Detect signal (~DCD)
};

/**
 * @brief UART Scratch pad register (SCR), read-write
 *
 * The SCR register is not used by the UART hardware and can be used for any purpose by software.
 * @note UM10398 LPC11xx User Manual, section 13.5.11 p.212
 * @warning This register is hardware specific and should not be used for portable code.
 */
enum UART_SCR : uint32_t
{
    SCR = 0xffffffff //!< The entire 32 bit SCR register can be used for any purpose
};

/**
 * @brief UART Auto-baud Control Register (ACR), read-write
 *
 * Controls the auto-baud feature of the UART, which can automatically detect the baud rate of incoming data.
 * @note UM10398 LPC11xx User Manual, section 13.5.12 p.213
 * @warning UNTESTED. Check implementation if needed in the future.
 */
enum UART_ACR : uint32_t
{
    ACR_START       = 1 << 0, //!< Auto-baud start control bit. Writing 1 starts auto-baud, cleared by hardware when auto-baud is complete.
    ACR_MODE        = 1 << 1, //!< Auto-baud mode: 0 = mode 0, 1 = mode 1
    ACR_AUTORESTART = 1 << 2, //!< Auto-baud restart enable
    ACR_ABEOINTCLR  = 1 << 8, //!< Auto-baud end of operation interrupt clear (write only)
    ACR_ABTOINTCLR  = 1 << 9, //!< Auto-baud time-out interrupt clear (write only)
};

/**
 * @brief UART Fractional Divider Register (FDR), read-write
 *
 * Controls the fractional divider for the UART baud rate generator.
 * @note UM10398 LPC11xx User Manual, section 13.5.15 p.216
 */
enum UART_FDR : uint32_t
{
    FDR_DIVADDVAL_MASK = 0x0f, //!< Mask for the DIVADDVAL bits in the FDR register
    FDR_MULVAL_MASK = 0xf0     //!< Mask for the MULVAL bits in the FDR register
};

/**
 * @def UART_TER
 * @brief UART Transmit Enable Register (TER), read-write
 *
 * Controls whether the UART transmitter is enabled or disabled.
 * @note UM10398 LPC11xx User Manual, section 13.5.16 p.219
 * @warning Software should not use this register directly, as the transmitter is enabled/disabled by the UART auto flow control when needed.
 */

/**
 * @brief UART RS485 Control Register (RS485CTRL), read-write
 *
 * Controls the RS-485 mode of the UART.
 * @note UM10398 LPC11xx User Manual, section 13.5.17 p.220
 * @warning UNTESTED. Check implementation if needed in the future.
 */
enum UART_RS485CTRL : uint32_t
{
    NMMEN = 1 << 0, //!< Normal Multidrop Mode Enable. When set, the UART operates in normal multidrop mode.
    RXDIS = 1 << 1, //!< Receiver Disable. When set, the UART receiver is disabled.
    AADEN = 1 << 2, //!< Auto Address Detect Enable. When set, auto address detect is enabled.
    SEL   = 1 << 3, //!< Select direction control pin. When set, DTR is used as the direction control pin, otherwise RTS is used.
    DCTRL = 1 << 4, //!< Auto Direction Control. When set, the UART controls the direction of data flow.
    OINV  = 1 << 5, //!< Polarity Control. When set, the direction control pin will be high, if the transmitter has data to send.
};

/**
 * @brief UART RS-485 Address Match Register (RS485ADRMATCH), read-write
 *
 * Contains the address match value for the RS-485 mode.
 * @note UM10398 LPC11xx User Manual, section 13.5.18 p.221
 * @warning UNTESTED. Check implementation if needed in the future.
 */
enum UART_RS485ADRMATCH : uint32_t
{
    ADRMATCH_MASK = 0xff //!< Mask for the address match value in the RS485ADRMATCH register
};

/**
 * @brief UART RS-485 Delay Register (RS485DLY), read-write
 *
 * Contains the delay value for the RS-485 mode.
 * @note UM10398 LPC11xx User Manual, section 13.5.19 p.221
 * @warning UNTESTED. Check implementation if needed in the future.
 */
enum UART_RS485DLY : uint32_t
{
     /**
      * @brief Mask for the delay value in the RS485DLY register
      * 
      * The delay value is specified in periods of the baud clock.
      * It can be used to add a delay between the end of transmission and
      * the de-assertion of the direction control pin in RS-485 mode.
      */
    DLY_MASK = 0xff
};

#endif /* SBLIB_SERIAL_REGISTERS_H_ */
