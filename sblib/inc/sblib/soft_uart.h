/**
 * Software UART for LPC11xx
 *
 * Provides a software-based UART using GPIO pins and a hardware timer
 * for precise bit timing. Fixed configuration: 8 Databits, no Parity, 1 Stopbit (8N1).
 *
 * Rx uses a GPIO falling-edge interrupt to detect the start bit, then
 * the timer interrupt samples each data bit at the bit center.
 * Tx is driven entirely by the timer interrupt.
 *
 * @author Darthyson <darth@maptrack.de> Copyright (c) 2026
 *
 * @par
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 3 as
 * published by the Free Software Foundation.
 */
#ifndef SBLIB_SOFT_UART_H_
#define SBLIB_SOFT_UART_H_

#include "sblib/stream.h"
#include "sblib/ring_buffer.h"
#include "sblib/timer.h"

/**
 * @brief Software UART using GPIO bit-banging with hardware timer assistance.
 *
 * Uses one 16-bit timer (default: timer16_0) for precise bit timing.
 * Rx start-bit detection is done via GPIO falling-edge interrupt.
 * Internally uses match channels MAT2 (Tx) and MAT3 (Rx); MAT0 and MAT1 remain free.
 *
 * @par Timer restriction
 * timer16_1 is reserved by the Bus class and must NOT be used.
 * Use timer16_0, timer32_0 or timer32_1.
 *
 * @par GPIO pins that CAN NOT be used
 * | Pin      | Reason                                                |
 * |----------|-------------------------------------------------------|
 * | PIO0_0   | Hardware RESET, repurposing bricks the chip           |
 * | PIO0_4   | I2C open-drain pad (SCL), cannot drive push-pull high |
 * | PIO0_5   | I2C open-drain pad (SDA), cannot drive push-pull high |
 * | PIO0_10  | SWD clock (SWCLK), disables debugging                 |
 * | PIO1_3   | SWD data (SWDIO), disables debugging                  |
 * | PIO1_8   | PIN_EIB_RX, reserved by the Bus class                 |
 * | PIO1_9   | PIN_EIB_TX, reserved by the Bus class                 |
 *
 * Usage example:
 * @code
 *     #include <sblib/soft_uart.h>
 *
 *     // Use PIO0_3 as Rx and PIO0_6 as Tx with timer16_0
 *     SoftUART softSerial(PIO0_3, PIO0_6, timer16_0, SoftUART::BaudRate::Baud19200, SystemCoreClock);
 *
 *     // User must provide the timer and GPIO port ISR handlers e.g.:
 *     extern "C" void TIMER16_0_IRQHandler()
 *     {
 *         softUART.timerInterruptHandler();
 *     }
 *     extern "C" void PIOINT1_IRQHandler()
 *     {
 *         softUART.handleGpioInterrupt();
 *     }
 *
 *     void setup()
 *     {
 *         softSerial.begin();
 *         softSerial.println("Hello SoftUART!");
 *     }
 *
 *     void loop()
 *     {
 *         uint16_t byteReceived = softSerial.read();
 *         if (byteReceived >= 0x00)
 *         {
 *             softSerial.write(static_cast<uint8_t>(byteReceived)); // echo
 *         }
 *     }
 * @endcode
 */
class SoftUART : public Stream
{
public:
    /**
     * @brief Supported baud rates for SoftUART.
     */
    enum class BaudRate : uint32_t
    {
        Baud2400 = 2400,
        Baud4800 = 4800,
        Baud9600 = 9600,
        Baud19200 = 19200,
        Baud38400 = 38400,
        Baud57600 = 57600,
        Baud115200 = 115200,
        Baud230400 = 230400,
        Baud460800 = 460800,
        Baud576000 = 576000
    };

    static constexpr auto BUFFER_SIZE = RingBuffer::Size::bytes_64; //!< Size of the Rx and Tx ring buffers

    /**
     * @brief Create a SoftUART instance.
     * @param rxPin       GPIO pin for receiving data (any digital-capable pin).
     * @param txPin       GPIO pin for transmitting data (any digital-capable pin).
     * @param timer       Hardware timer to use for the bit timing (e.g. timer16_0).
     * @param baudRate    Baud rate to use.
     * @param systemClock System clock frequency in Hz, used to calculate bit timings. Typically, SystemCoreClock.
     */
    SoftUART(uint32_t rxPin, uint32_t txPin, Timer& timer, BaudRate baudRate, uint32_t systemClock);

    SoftUART() = delete;
    SoftUART(const SoftUART&) = delete;
    SoftUART& operator=(const SoftUART&) = delete;
    SoftUART(SoftUART&&) = delete;

    /**
     * @brief Initialize the software UART.
     * @details Configures GPIO pins, timer and enables interrupts.
     *          Must be called before any read/write operations.
     */
    void begin();

    /**
     * @brief Shut down the software UART.
     * @details Disables timer and GPIO interrupts and releases the timer.
     */
    void end();

    /**
     * @brief Read a single byte from the receiver buffer.
     * @return The received byte (0..255) or -1 if no data is available.
     */
    int16_t read() override;

    /**
     * @brief Peek at the next byte in the receiver buffer without removing it.
     * @return The next byte (0..255) or -1 if no data is available.
     */
    int16_t peek() override;

    /**
     * @return The number of bytes available in the receiver buffer.
     */
    uint32_t available() override;

    // Pull in write(str) and write(buf, size) from Print
    using Print::write;

    /**
     * @brief Write a single byte to the software UART.
     * @details The byte is queued and transmitted in the background by the timer interrupt.
     * @param ch The byte to write.
     * @return 1 if the byte was queued, 0 if not (port not enabled).
     */
    uint32_t write(uint8_t ch) override;

    /**
     * @brief Block until the transmit buffer has been fully sent.
     */
    void flush() override;

    /**
     * @return true if the software UART is enabled, false otherwise.
     */
    explicit operator bool() const { return enabled_; }

    /**
     * @return true if the software UART is enabled, false otherwise.
     */
    [[nodiscard]] bool enabled() const { return enabled_; }

    /**
     * @brief User must implement the correct timer ISR and call this method from it.
     * Example for timer16_0:
     * @code
     * extern "C" void TIMER16_0_IRQHandler() {
     *     instance::timerInterruptHandler();
     * }
     * @endcode
     */
    void timerInterruptHandler();

    /**
     * @brief Called from the GPIO port ISR when the Rx start bit is detected.
     * @warning Do NOT call directly.
     * @internal
     */
    void rxStartBitDetected();


    /**
     * @brief GPIO ISR dispatcher. Call this from your GPIO port ISR handler.
     *
     * Example for PIO1_x:
     * @code
     * extern "C" void PIOINT1_IRQHandler() {
     *     instance::handleGpioInterrupt();
     * }
     * @endcode
     */
    void handleGpioInterrupt();

protected:
    /**
     * @brief Compute bit period in timer ticks.
     *
     * Timer runs at SystemCoreClock, prescaler divides it so each tick equals 1 µs.
     * bitPeriod = 1_000_000 / baudRate  (in µs / ticks).
     *
     * @param baudRate   The desired baud rate.
     * @return Bit period in timer ticks (microseconds when prescaler gives 1 µs ticks).
     */
    static constexpr uint32_t computeBitPeriod(BaudRate baudRate);

    /**
     * @brief Compute half-bit period in timer ticks (for sampling at bit center).
     * @param baudRate   The desired baud rate.
     * @return Half-bit period in timer ticks.
     */
    static constexpr uint32_t computeHalfBitPeriod(BaudRate baudRate);

    /**
     * @brief Compute the timer prescaler value so that each timer tick equals 1 µs.
     *
     * @param systemClock  System clock frequency in Hz (e.g. 48000000).
     * @return Prescaler register value.
     */
    static constexpr uint32_t computePrescaler(uint32_t systemClock);

    /**
     * @brief SoftUART state machine states.
     */
    enum class UartState : uint8_t
    {
        Idle,      //!< No transmission in progress
        StartBit,  //!< Start bit (low)
        DataBits,  //!< Data bits 0..7
        StopBit    //!< Stop bit (high)
    };

    Timer& bitTimer_; //!< Reference to the hardware timer used for bit timings
    uint32_t timerMask_; //!< Bit mask for timer counter wrapping (0xFFFF for 16-bit, 0xFFFFFFFF for 32-bit)

    const uint32_t bitPeriodTicks_; //!< Bit period in timer ticks
    const uint32_t halfBitTicks_;   //!< Half-bit period in timer ticks (for sampling at bit center)
    const uint32_t prescaler_;      //!< Timer prescaler value (computed from system clock, each tick = 1 µs)

    volatile UartState txState_; //!< Current Tx state
    volatile UartState rxState_; //!< Current Rx state

    RingBuffer receiveBuffer_;  //!< Software RingBuffer for the received bytes
    RingBuffer transmitBuffer_; //!< Software RingBuffer for the bytes to transmit

    /**
     * @brief Start transmitting the next byte from the write buffer, if available.
     * @note Called from ISR context.
     */
    void startTx();

    /**
     * @brief Advance the Tx state machine by one bit period.
     * Called from timerInterruptHandler().
     */
    void handleTxBit();

    /**
     * @brief Advance the Rx state machine by one bit period.
     * Called from timerInterruptHandler().
     */
    void handleRxBit();

private:
    const uint32_t rxPin_; //!< GPIO pin for Rx
    const uint32_t txPin_; //!< GPIO pin for Tx
    bool enabled_; //!< true when begin() has been called and end() has not

    volatile uint8_t txBitIndex_; //!< Current Tx bit position (0..7)
    volatile uint8_t txShiftReg_; //!< Tx shift register (current byte being sent)

    volatile uint8_t rxBitIndex_; //!< Current Rx bit position (0..7)
    volatile uint8_t rxShiftReg_; //!< Rx shift register (byte being assembled)

    /**
     * @brief Enable the GPIO falling-edge interrupt on the Rx pin for
     * start-bit detection.
     */
    void enableRxInterrupt() const;

    /**
     * @brief Disable the GPIO interrupt on the Rx pin.
     */
    void disableRxInterrupt() const;
};

#endif /* SBLIB_SOFT_UART_H_ */
