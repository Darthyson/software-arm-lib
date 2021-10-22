/**
 * @file serial.h
 * @brief LPC11xx Serial port driver
 *
 * @author Stefan Taferner <stefan.taferner@gmx.at> Copyright (c) 2014
 * @author HoRa  Copyright (c) March 2021
 * @author Darthyson <darth@maptrack.de> Copyright (c) 2021
 *
 * @note default serial Tx and Rx-pin definitions moved to config.h
 *
 * @bug No known bugs.
 *
 * @par
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 3 as
 * published by the Free Software Foundation.
 *
 * @warning This is just a testing warning
 */
#ifndef sblib_serial_h
#define sblib_serial_h

#include <sblib/buffered_stream.h>
#include <sblib/interrupt.h>
#include <sblib/config.h>

/** @defgroup SERIAL_11XX CHIP: LPC11xx Serial port driver
 * @ingroup CHIP_11XX_Drivers
 * @{
 */

#if !defined(SERIAL_TX_PIN) || !defined(SERIAL_RX_PIN)
#   if defined (__LPC11XX__)
#       define SERIAL_TX_PIN PIO1_7     //!> default serial Tx-Pin PIO1.7 (LPC11xx)
#       define SERIAL_RX_PIN PIO1_6     //!> default serial Tx-Pin PIO1.6 (LPC11xx)
#   elif defined (__LPC11UXX__)
#       define SERIAL_TX_PIN PIO0_19    //!> default serial Tx-Pin PIO0.19 (LPC11Uxx)
#       define SERIAL_RX_PIN PIO0_18    //!> default serial Tx-Pin PIO0.18 (LPC11Uxx)
#   endif
#endif

class Serial;

/**
 * The serial port, also known as UART.
 * This serial port uses PIO1_6 for RXD and PIO1_7 for TXD.
 */
extern Serial serial;


/**
 * The configuration for opening the serial port.
 */
enum SerialConfig
{
    SERIAL_5N1 = 0x00,  //!< 5 data bits, no parity, 1 stop bit
    SERIAL_6N1 = 0x01,  //!< 6 data bits, no parity, 1 stop bit
    SERIAL_7N1 = 0x02,  //!< 7 data bits, no parity, 1 stop bit
    SERIAL_8N1 = 0x03,  //!< 8 data bits, no parity, 1 stop bit
    SERIAL_5N2 = 0x04,  //!< 5 data bits, no parity, 2 stop bits
    SERIAL_6N2 = 0x05,  //!< 6 data bits, no parity, 2 stop bits
    SERIAL_7N2 = 0x06,  //!< 7 data bits, no parity, 2 stop bits
    SERIAL_8N2 = 0x07,  //!< 8 data bits, no parity, 2 stop bits
    SERIAL_5O1 = 0x08,  //!< 5 data bits, odd parity, 1 stop bit
    SERIAL_6O1 = 0x09,  //!< 6 data bits, odd parity, 1 stop bit
    SERIAL_7O1 = 0x0a,  //!< 7 data bits, odd parity, 1 stop bit
    SERIAL_8O1 = 0x0b,  //!< 8 data bits, odd parity, 1 stop bit
    SERIAL_5O2 = 0x0c,  //!< 5 data bits, odd parity, 2 stop bits
    SERIAL_6O2 = 0x0d,  //!< 6 data bits, odd parity, 2 stop bits
    SERIAL_7O2 = 0x0e,  //!< 7 data bits, odd parity, 2 stop bits
    SERIAL_8O2 = 0x0f,  //!< 8 data bits, odd parity, 2 stop bits
    SERIAL_5E1 = 0x18,  //!< 5 data bits, even parity, 1 stop bit
    SERIAL_6E1 = 0x19,  //!< 6 data bits, even parity, 1 stop bit
    SERIAL_7E1 = 0x1a,  //!< 7 data bits, even parity, 1 stop bit
    SERIAL_8E1 = 0x1b,  //!< 8 data bits, even parity, 1 stop bit
    SERIAL_5E2 = 0x1c,  //!< 5 data bits, even parity, 2 stop bits
    SERIAL_6E2 = 0x1d,  //!< 6 data bits, even parity, 2 stop bits
    SERIAL_7E2 = 0x1e,  //!< 7 data bits, even parity, 2 stop bits
    SERIAL_8E2 = 0x1f   //!< 8 data bits, even parity, 2 stop bits
};

extern "C" void UART_IRQHandler();


/**
 * Serial port access. All ARM processors have a serial port, also known as UART.
 */
class Serial: public BufferedStream
{
public:
    /**
     * Create a serial port access object.
     *
     * @param rxPin - the pin to use for RXD: PIO1_6, PIO2_7, PIO3_1, or PIO3_4
     * @param txPin - the pin to use for TXD: PIO1_7, PIO2_8, PIO3_0, or PIO3_5
     */
    Serial(int rxPin, int txPin);

    /**
     * Set rx pin for serial communication.
     *
     * @param rxPin - the pin to use for RXD: PIO1_6, PIO2_7, PIO3_1, or PIO3_4
     */
    void setRxPin(int rxPin);
    /**
     * Set tx pin for serial communication.
     *
     * @param txPin - the pin to use for TXD: PIO1_7, PIO2_8, PIO3_0, or PIO3_5
     */
    void setTxPin(int txPin);
    /**
     * Begin using the serial port with the specified baud rate and 8 data bits,
     * no parity bit, and 1 stop bit (SERIAL_8N1).
     *
     * @param baudRate - the baud rate: 9600, 19200, ...
     */
    void begin(int baudRate);

    /**
     * Begin using the serial port.
     *
     * @param baudRate - the baud rate: 9600, 19200, ...
     * @param config - the configuration for data bits, parity, stop bits, e.g. SERIAL_8N1
     */
    void begin(int baudRate, SerialConfig config);

    /**
     * End using the serial port.
     */
    void end();

    /**
     * Read a single byte.
     *
     * @return The read byte (0..255) or -1 if no byte was received.
     */
    virtual int read();

    // Pull in write(str) and write(buf, size) from Print
    using Print::write;

    /**
     * Write a single byte.
     *
     * @param ch - the byte to write.
     * @return 1 if the byte was written, 0 if not.
     */
    virtual int write(byte ch);

    /**
     * Wait until all bytes are written.
     */
    virtual void flush(void);

    /**
     * @brief Check if serial port enabled and available for transmission
     *
     * @return true if serial port is enabled, otherwise false
     */
    operator bool() const {return enabled_;}

    /**
     * @brief Check if serial port enabled and available for transmission
     *
     * @return true if serial port is enabled, otherwise false
     */
    bool enabled(void) {return enabled_;}

protected:
    // Allow the interrupt handler to call our protected methods
    friend void UART_IRQHandler();

    /**
     * Handle the serial interrupt.
     */
    void interruptHandler();

private:
    bool enabled_; //!> true if serial port is enabled, otherwise false

};


//
//  Inline functions
//
inline void Serial::begin(int baudRate)
{
    if (enabled())
    {
        end();
    }
    begin(baudRate, SERIAL_8N1);
}
/** @}*/
#endif /*sblib_serial_h*/
