/**
 * @brief LPC11xx Serial port driver
 *
 * @author Stefan Taferner <stefan.taferner@gmx.at> Copyright (c) 2014
 * @author HoRa Copyright (c) March 2021
 * @author Darthyson <darth@maptrack.de> Copyright (c) 2026
 *
 * @note default serial Tx and Rx-pin definitions moved to libconfig.h
 *
 * @par
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 3 as
 * published by the Free Software Foundation.
 */
#ifndef SBLIB_SERIAL_H_
#define SBLIB_SERIAL_H_

#include <sblib/buffered_stream.h>
#include <sblib/types.h>

/**
 * Callback for serial line error conditions.
 *
 * @param lineStatus The UART line status register (LSR) value containing the error flags.
 * @param context    User-provided context pointer.
 *
 * LSR error bits:
 * - @c LSR_OE  (0x02) Overrun error
 * - @c LSR_PE  (0x04) Parity error
 * - @c LSR_FE  (0x08) Framing error
 * - @c LSR_BI  (0x10) Break interrupt
 */
typedef void (*SerialErrorCallback)(uint32_t lineStatus, void* context);

constexpr uint8_t LSR_OE = 0x02; //!< Overrun error
constexpr uint8_t LSR_PE = 0x04; //!< Parity error
constexpr uint8_t LSR_FE = 0x08; //!< Framing error
constexpr uint8_t LSR_BI = 0x10; //!< Break interrupt

/** @defgroup SERIAL_11XX CHIP: LPC11xx Serial port driver
 * @ingroup CHIP_11XX_Drivers
 * @{
 */

class Serial;

/**
 * @brief The serial port, also known as UART.
 * This serial port uses PIO1_6 for RXD and PIO1_7 for TXD.
 */
extern Serial serial;


/**
 * @brief The configuration for opening the serial port.
 */
enum SerialConfig
{
    SERIAL_5N1 = 0x00, //!< 5 data bits, no parity, 1 stop bit
    SERIAL_6N1 = 0x01, //!< 6 data bits, no parity, 1 stop bit
    SERIAL_7N1 = 0x02, //!< 7 data bits, no parity, 1 stop bit
    SERIAL_8N1 = 0x03, //!< 8 data bits, no parity, 1 stop bit
    SERIAL_5N2 = 0x04, //!< 5 data bits, no parity, 2 stop bits
    SERIAL_6N2 = 0x05, //!< 6 data bits, no parity, 2 stop bits
    SERIAL_7N2 = 0x06, //!< 7 data bits, no parity, 2 stop bits
    SERIAL_8N2 = 0x07, //!< 8 data bits, no parity, 2 stop bits
    SERIAL_5O1 = 0x08, //!< 5 data bits, odd parity, 1 stop bit
    SERIAL_6O1 = 0x09, //!< 6 data bits, odd parity, 1 stop bit
    SERIAL_7O1 = 0x0a, //!< 7 data bits, odd parity, 1 stop bit
    SERIAL_8O1 = 0x0b, //!< 8 data bits, odd parity, 1 stop bit
    SERIAL_5O2 = 0x0c, //!< 5 data bits, odd parity, 2 stop bits
    SERIAL_6O2 = 0x0d, //!< 6 data bits, odd parity, 2 stop bits
    SERIAL_7O2 = 0x0e, //!< 7 data bits, odd parity, 2 stop bits
    SERIAL_8O2 = 0x0f, //!< 8 data bits, odd parity, 2 stop bits
    SERIAL_5E1 = 0x18, //!< 5 data bits, even parity, 1 stop bit
    SERIAL_6E1 = 0x19, //!< 6 data bits, even parity, 1 stop bit
    SERIAL_7E1 = 0x1a, //!< 7 data bits, even parity, 1 stop bit
    SERIAL_8E1 = 0x1b, //!< 8 data bits, even parity, 1 stop bit
    SERIAL_5E2 = 0x1c, //!< 5 data bits, even parity, 2 stop bits
    SERIAL_6E2 = 0x1d, //!< 6 data bits, even parity, 2 stop bits
    SERIAL_7E2 = 0x1e, //!< 7 data bits, even parity, 2 stop bits
    SERIAL_8E2 = 0x1f  //!< 8 data bits, even parity, 2 stop bits
};

extern "C" void UART_IRQHandler();


/**
 * @brief Serial port access. All ARM processors have a serial port, also known as UART.
 */
class Serial : public BufferedStream
{
public:
    /**
     * @brief Create a serial port access object.
     *
     * @param rxPin The pin to use for RXD: PIO1_6, PIO2_7, PIO3_1, or PIO3_4
     * @param txPin The pin to use for TXD: PIO1_7, PIO2_8, PIO3_0, or PIO3_5
     */
    Serial(int rxPin, int txPin);

    /**
     * @brief Set Rx pin for serial communication.
     *
     * @param rxPin The pin to use for RXD: PIO1_6, PIO2_7, PIO3_1, or PIO3_4
     */
    void setRxPin(int rxPin);

    /**
     * @brief Set Tx pin for serial communication.
     *
     * @param txPin The pin to use for TXD: PIO1_7, PIO2_8, PIO3_0, or PIO3_5
     */
    void setTxPin(int txPin);

    /**
     * @brief Begin using the serial port with the specified baud rate.
     *        - 8 data bits, no parity bit, 1 stop bit
     * @param baudRate The baud rate: 9600, 19200, ...
     */
    void begin(int baudRate);

    /**
     * @brief Begin using the serial port.
     *
     * @param baudRate The baud rate: 9600, 19200, ...
     * @param config   The configuration for data bits, parity, stop bits, e.g. SERIAL_8N1
     */
    void begin(int baudRate, SerialConfig config);

    /**
     * @brief End using the serial port.
     */
    void end();

    /**
     * @brief Read a single byte.
     *
     * @return The read byte (0..255) or -1 if no byte was received.
     */
    int16_t read() override;

    // Pull in write(str) and write(buf, size) from Print
    using Print::write;

    /**
     * @brief Write a single byte to the serial port.
     *
     * @param ch The byte to write.
     * @return 1 If the byte was written, 0 if not.
     *
     */
    uint32_t write(byte ch) override;

    /**
     * @brief Wait until all bytes are written.
     */
    void flush() override;

    /**
     * @brief Check if serial port enabled and available for transmission
     *
     * @return True if serial port is enabled, otherwise false
     */
    explicit operator bool() const { return enabled_; }

    /**
     * @brief Check if serial port enabled and available for transmission
     *
     * @return True if serial port is enabled, otherwise false
     */
    [[nodiscard]] bool enabled() const { return enabled_; }

    /**
     * @brief Set an optional callback for serial line error conditions.
     *
     * break (BI), framing error (FE), parity error (PE), overrun error (OE)
     *
     * @param callback Function to call on error, or nullptr to disable.
     *                 Receives the line status register value and the user context.
     * @param context  Optional user context pointer passed to the callback (default: nullptr).
     *
     * @code
     *      // Example using a lambda to call a member function:
     *      serial.setErrorCallback([](uint32_t lineStatus, void* ctx) {
     *          auto* self = static_cast<MyClass*>(ctx);
     *          if (lsr & LSR_BI) { self->onBreak(); }
     *          if (lsr & LSR_FE) { self->onFrameError(); }
     *          if (lsr & LSR_PE) { self->onParityError(); }
     *          if (lsr & LSR_OE) { self->onOverrunError(); }
     *      }, this);
     * @endcode
     * @warning The callback is called from interrupt context, so it should be kept short.
     */
    void setErrorCallback(SerialErrorCallback callback, void* context = nullptr);

protected:
    // Allow the UART interrupt handler to call our protected methods
    friend void UART_IRQHandler();

    /**
     * @brief Handle the serial interrupt.
     */
    void interruptHandler();

private:
    bool enabled_;               //!> True if serial port is enabled, otherwise false
    SerialErrorCallback errorCallback; //!> Optional callback for serial line errors
    void* errorCallbackContext;  //!> User context for error callback

    /**
     * Handle UART line errors detected in the line status register (LSR).
     *
     * Discards the 0x00 byte from the receive buffer on BREAK conditions
     * and invokes the user error callback (if set) with the relevant error flags.
     *
     * @param lineStatus The UART line status register value containing the error bits.
     */
    void handleLineError(uint32_t lineStatus)const;
};


//
//  Inline functions
//
inline void Serial::begin(const int baudRate)
{
    if (enabled())
    {
        end();
    }
    begin(baudRate, SERIAL_8N1);
}

/** @}*/
#endif /* SBLIB_SERIAL_H_ */
