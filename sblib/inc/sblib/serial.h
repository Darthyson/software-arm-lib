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

#include "sblib/stream.h"
#include "sblib/ring_buffer.h"

#ifdef IAP_EMULATION
#    include <vector>
#endif


/**
 * @brief Receiver (Rx) FIFO interrupt trigger level in the UART FIFO Control Register (FCR).
 */
enum class RxTriggerLevel : uint8_t
{
    CHAR_1, //!< 1 character in the Rx FIFO
    CHAR_4, //!< 4 characters in the Rx FIFO
    CHAR_8, //!< 8 characters in the Rx FIFO
    CHAR_14 //!< 14 characters in the Rx FIFO
};

/**
 * @brief Serial line error conditions that can be reported
 */
enum SerialError: uint8_t
{
    // Same bit values as UART_LSR defined in serial_registers.h
    SERIAL_OVERRUN_ERROR    = 1 << 1, //!< Overrun error (OE)
    SERIAL_PARITY_ERROR     = 1 << 2, //!< Parity error (PE)
    SERIAL_FRAME_ERROR      = 1 << 3, //!< Framing error (FE)
    SERIAL_BREAK_INDICATION = 1 << 4, //!< Break indication (BI)
};

/**
 * Callback for serial line error conditions.
 *
 * @param errorFlags Error flags of the faulty byte.
 * @param faultyByte The byte that caused the error.
 * @param context    User-provided context pointer.
 *
 * errors bits:
 * - @c SerialError::SERIAL_OVERRUN_ERROR Overrun error (OE)
 * - @c SerialError::SERIAL_PARITY_ERROR Parity error (PE)
 * - @c SerialError::SERIAL_FRAME_ERROR Framing error (FE)
 * - @c SerialError::SERIAL_BREAK_INDICATION Break interrupt (BI)
 */
typedef void (*SerialErrorCallback)(uint8_t errorFlags, uint8_t faultyByte, void* context);

/** @defgroup SERIAL_11XX CHIP: LPC11xx Serial port driver
 * @ingroup CHIP_11XX_Drivers
 * @{
 */

class Serial;

/**
 * @brief The serial port, also known as UART.
 * This serial port uses by default PIO1_6 for RXD and PIO1_7 for TXD.
 */
extern Serial serial;

/** @brief Common baud rates for the serial port */
enum SerialBaudRate : uint32_t
{
    SERIAL_BAUD_RATE_1200 = 1200,
    SERIAL_BAUD_RATE_2400 = 2400,
    SERIAL_BAUD_RATE_4800 = 4800,
    SERIAL_BAUD_RATE_9600 = 9600,
    SERIAL_BAUD_RATE_19200 = 19200,
    SERIAL_BAUD_RATE_28800 = 28800,
    SERIAL_BAUD_RATE_38400 = 38400,
    SERIAL_BAUD_RATE_57600 = 57600,
    SERIAL_BAUD_RATE_76800 = 76800,
    SERIAL_BAUD_RATE_115200 = 115200,
    SERIAL_BAUD_RATE_230400 = 230400,
    SERIAL_BAUD_RATE_460800 = 460800,
    SERIAL_BAUD_RATE_576000 = 576000,
    SERIAL_BAUD_RATE_661765 = 661765,
    SERIAL_BAUD_RATE_750000 = 750000,
    SERIAL_BAUD_RATE_921600 = 921600,
    SERIAL_BAUD_RATE_1000000 = 1000000,
    SERIAL_BAUD_RATE_1500000 = 1500000
};

/**
 * @brief The configuration for opening the serial port.
 */
enum SerialConfig : uint8_t
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
class Serial : public Stream
{
public:
    /**
     * @brief Create a serial port access object.
     *
     * @param rxPin The pin to use for RXD: PIO1_6, PIO2_7, PIO3_1, or PIO3_4
     * @param txPin The pin to use for TXD: PIO1_7, PIO2_8, PIO3_0, or PIO3_5
     */
    Serial(uint32_t rxPin, uint32_t txPin);

    ~Serial() override;

    /**
     * @brief Set Rx pin for serial communication.
     *
     * @param rxPin The pin to use for RXD: PIO1_6, PIO2_7, PIO3_1, or PIO3_4
     */
    void setRxPin(uint32_t rxPin);

    /**
     * @brief Set Tx pin for serial communication.
     *
     * @param txPin The pin to use for TXD: PIO1_7, PIO2_8, PIO3_0, or PIO3_5
     */
    void setTxPin(uint32_t txPin);

    /**
     * @brief Begin using the serial port with the specified baud rate.
     *        - 8 data bits, no parity bit, 1 stop bit
     * @param baudRate The baud rate: 9600, 19200, ...
     */
    void begin(uint32_t baudRate);

    /**
     * @brief Begin using the serial port with the specified baud rate.
     *        - 8 data bits, no parity bit, 1 stop bit
     * @param baudRate The baud rate @ref SerialBaudRate
     */
    void begin(SerialBaudRate baudRate);

    /**
     * @brief Begin using the serial port.
     *
     * @param baudRate The baud rate @ref SerialBaudRate
     * @param config   The configuration for data bits, parity, stop bits, e.g. SERIAL_8N1
     * @param rxTriggerLevel     The trigger level for the Rx FIFO. Default is RxTriggerLevel::CHAR_1 (1 character in the Rx FIFO).
     * @param receiveBufferSize  The size of the Rx software buffer. Default is 128 bytes.
     * @param transmitBufferSize The size of the Tx software buffer. Default is 128 bytes.
     */
    void begin(SerialBaudRate baudRate, SerialConfig config, RxTriggerLevel rxTriggerLevel = RxTriggerLevel::CHAR_1,
               RingBuffer::Size receiveBufferSize = RingBuffer::Size::bytes_128,
               RingBuffer::Size transmitBufferSize = RingBuffer::Size::bytes_128);

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

    /**
     * @brief Query the next byte to be read, without reading it.
     *
     * @return The next byte (0..255) or -1 if no byte is available
     *         for reading.
     */
    [[nodiscard]] int16_t peek() override;

    /**
     * @brief Query the number of bytes available for reading.
     *
     * @return The number of bytes that are available for reading.
     */
    [[nodiscard]] uint32_t available() override;

    /**
     * @brief Read a single byte with timeout.
     * 
     * @note This method is inherited from Stream and can be used with Serial::setTimeout() to specify the timeout duration.
     */
    using Stream::timedRead;

    /**
     * @brief Write a zero-terminated string or number of bytes.
     * 
     * @note This method is inherited from Print and can be used to write strings or byte arrays to the serial port.
     */
    using Print::write;

    /**
     * @brief Write a single byte to the serial port.
     *
     * @param ch The byte to write.
     * @return 1 If the byte was written, 0 if not.
     */
    uint32_t write(uint8_t ch) override;

    /**
     * @brief Write a number of bytes to the serial port.
     *
     * @param data  The bytes to write.
     * @param count The number of bytes to write.
     * @return The number of bytes that were written.
     */
    uint32_t write(const uint8_t* data, uint32_t count) override;

    /**
     * @brief Wait until all bytes are written.
     */
    void flush() override;

    /**
     * @brief Clear the UART Rx RingBuffer.
     */
    void clearRxBuffer();

    /**
     * @brief Clear the UART Tx RingBuffer.
     */
    void clearTxBuffer();

    /**
     * @brief Clear the internal receive and transfer software buffers.
     */
    void clearBuffers();

    /**
     * @brief Reset the UART Rx FIFO.
     */
    void resetUartRxFifo() const;

    /**
     * @brief Reset the UART Tx FIFO.
     */
    void resetUartTxFifo() const;

    /**
     * @brief Send a BREAK condition on the Tx line.
     *
     * Forces the Tx output LOW for the specified duration.
     * Any pending transmission is flushed before the BREAK is sent.
     *
     * @param durationMicroseconds Duration of the BREAK in microseconds.
     *        Must be between @ref MIN_DELAY_MICROSECONDS and @ref MAX_DELAY_MICROSECONDS.
     * @note This method is blocking and will wait until the BREAK condition has been sent for the specified duration.
     */
    void sendBreak(uint32_t durationMicroseconds);

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
     *                 Receives the errorFlags value and the user context.
     * @param context  Optional user context pointer passed to the callback (default: nullptr).
     *
     * @code
     *      // Example using a lambda to call a member function:
     *      serial.setErrorCallback([](uint8_t errorFlags, void* ctx) {
     *          auto* self = static_cast<MyClass*>(ctx);
     *          if (errorFlags & SerialError::SERIAL_OVERRUN_ERROR) { self->onOverrunError(); }
     *          if (errorFlags & SerialError::SERIAL_BREAK_INDICATION) { self->onBreak(); }
     *          if (errorFlags & SerialError::SERIAL_FRAME_ERROR) { self->onFrameError(); }
     *          if (errorFlags & SerialError::SERIAL_PARITY_ERROR) { self->onParityError(); }
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
    void uartInterruptHandler();

    uint32_t readCounter = 0; ///\todo delete on release
    uint32_t writeDirect = 0; ///\todo delete on release
    uint32_t writeQueued = 0; ///\todo delete on release
    uint32_t writeTotal = 0;  ///\todo delete on release
    uint32_t transmitCounter = 0; ///\todo delete on release
    uint32_t receiveCounter = 0; ///\todo delete on release
    uint32_t receiveDropped = 0; ///\todo delete on release
    uint32_t isrRLScounter = 0; ///\todo delete on release
    uint32_t isrRDAcounter = 0; ///\todo delete on release
    uint32_t isrCTIcounter = 0; ///\todo delete on release
    uint32_t isrTHREcounter = 0; ///\todo delete on release
    uint32_t isrMODEMCounter = 0; ///\todo delete on release
    uint32_t isrEntries = 0; ///\todo delete on release
    uint32_t isrRealPendings = 0; ///\todo delete on release
    uint32_t isrFakePendings = 0; ///\todo delete on release
    uint32_t isrIIR = 0; ///\todo delete on release

private:
    bool enabled_;               //!> True if serial port is enabled, otherwise false
    uint32_t savedFIFOcontrolRegister; //!> Saved FCR value for resetting the UART FIFOs
    RingBuffer * receiveBuffer;  //!> Software RingBuffer for the received bytes
    RingBuffer * transmitBuffer; //!> Software RingBuffer for the bytes to transmit
    SerialErrorCallback errorCallback; //!> Optional callback for serial line errors
    void* errorCallbackContext;  //!> User context for error callback

    /**
     * @brief Fills the UART Tx FIFO with bytes from the transmit ring buffer.
     * 
     * @return The number of bytes written to the UART Tx FIFO, zero if no bytes were written.
     */
    uint32_t fillUartTxFifoWithRingBuffer();

    /**
     * @brief Allocate the internal send and receive SPSC ring buffers with the specified sizes.
     * 
     * @param receiveBufferSize The size of the software Rx RingBuffer in bytes.
     * @param transmitBufferSize The size of the software Tx RingBuffer in bytes.
     */
    void allocateBuffers(RingBuffer::Size receiveBufferSize, RingBuffer::Size transmitBufferSize);

    /**
     * @brief Deallocate the internal send and receive SPSC ring buffers.
     */
    void deallocateBuffers();

    /**
     * @brief Handle UART line errors detected in the line status register (LSR).
     *
     * @param errorFlags The UART line status register value containing the error bits.
     * @param faultyByte The byte that caused the error.
     */
    void handleLineError(uint8_t errorFlags, uint8_t faultyByte) const;

    /********************************************************************************************/
    /* All methods below are only for unit testing and should never be used in production code. */
    /* They are only included when IAP_EMULATION is defined.                                    */
    /********************************************************************************************/
#ifdef IAP_EMULATION
public:
    /****************************************************/
    /* Public methods below are only for unit testing.  */
    /****************************************************/
    /**
     * @brief Return all bytes captured by testSimulateByteSent() since the last call to testClearSentBytes().
     *
     * @return A vector of bytes that were "sent" via write().
     * @note Must be compiled with IAP_EMULATION defined to have any effect.
     * @warning This method is only for unit testing and should never be used in production code.
     */
    static const std::vector<uint8_t>& testGetSentBytes();

    /**
     * @brief Clear the captured sent-bytes buffer.
     *
     * @note Must be compiled with IAP_EMULATION defined to have any effect.
     * @warning This method is only for unit testing and should never be used in production code.
     */
    static void testClearSentBytes();

    /*****************************************************/
    /* Private methods below are only for unit testing.  */
    /*****************************************************/
private:
    /**
     * @brief Simulate that a byte was sent.
     *
     * @param sentByte The byte that was sent.
     * @warning This method is only for unit testing and should never be used in production code.
     */
    static void testSimulateByteSent(uint8_t sentByte);

    /**
     * @brief Simulate initializing the Interrupt Identification Register (IIR) for unit testing.
     *
     * @warning This method is only for unit testing and should never be used in production code.
     */
    static void testInitIIR();
    /**
     * @brief Simulate setting a UART register value for unit testing.
     *
     * @param registerToSet The UART register to set.
     * @param value         The value to write to the UART register.
     * @note Must be compiled with IAP_EMULATION defined to have any effect.
     * @warning This method is only for unit testing and should never be used in production code.
     */
    static void testSimulateRegisterValue(volatile uint32_t * registerToSet, uint32_t value);

    /**
     * @brief Simulate changing specific bits in a UART register for unit testing.
     *
     * @param registerToChange The UART register to modify.
     * @param bitMask          The bitMask to write to the UART register.
     * @param setBits          If true, the bits in bitMask will be set; if false, they will be cleared.
     * @note Must be compiled with IAP_EMULATION defined to have any effect.
     * @warning This method is only for unit testing and should never be used in production code.
     */
    static void testSimulateRegisterBitsChange(volatile uint32_t * registerToChange, uint32_t bitMask, bool setBits);

    /**
     * @brief Simulate that a UART interrupt was set/cleared
     * 
     * @param interruptID      The ID of the interrupt to set/clear.
     * @param setPending       If true, the interrupt is set; if false, the interrupt is cleared.
     * @param registerToSet    The UART register to set. nullptr if no register should be set.
     * @param valueToSet       The value to write to the UART register.
     * @param registerToChange The UART register to apply the bit mask to when writing to the UART register.
     *                         nullptr if no bit mask should be applied.
     * @param bitMaskToSet     The bit mask to set in the UART register.
     * @param setBits          If true, the bits in bitMaskToSet will be set; if false, they will be cleared.
     * @note Must be compiled with IAP_EMULATION defined to have any effect.
     * @warning This method is only for unit testing and should never be used in production code.
     */
    static void testChangeInterruptPending(uint32_t interruptID, bool setPending, volatile uint32_t * registerToSet,
         uint32_t valueToSet, volatile uint32_t * registerToChange, uint8_t bitMaskToSet, bool setBits);

    /**    
     * @brief Simulate that a UART interrupt was cleared by the ISR, e.g. the THRE interrupt after the THR is empty set.
     * 
     * @param interruptID      The ID of the interrupt to clear.
     * @param registerToSet    The UART register to set. nullptr if no register should be set.
     * @param valueToSet       The value to write to the UART register.
     * @param registerToChange The UART register to apply the bit mask to when writing to the UART register.
     *                         nullptr if no bit mask should be applied.
     * @param bitMaskToChange  The bit mask to set in the UART register.
     * @param setBits          If true, the bits in bitMaskToChange will be set; if false, they will be cleared.
     * @note Must be compiled with IAP_EMULATION defined to have any effect.
     * @warning This method is only for unit testing and should never be used in production code.
     */
    static void testClearInterrupt(uint32_t interruptID, volatile uint32_t * registerToSet,
         uint32_t valueToSet, volatile uint32_t * registerToChange, uint8_t bitMaskToChange, bool setBits);

    /**    
     * @brief Simulate that a UART interrupt is set, e.g. to trigger the ISR for unit testing.
     * 
     * @param interruptID      The ID of the interrupt to set.
     * @param registerToSet    The UART register to set. nullptr if no register should be set.
     * @param valueToSet       The value to write to the UART register.
     * @param registerToChange The UART register to apply the bit mask to when writing to the UART register.
     *                         nullptr if no bit mask should be applied.
     * @param bitMaskToChange  The bit mask to set in the UART register.
     * @param setBits          If true, the bits in bitMaskToChange will be set; if false, they will be cleared.
     * @note Must be compiled with IAP_EMULATION defined to have any effect.
     * @warning This method is only for unit testing and should never be used in production code.
     */
    static void testSetInterruptPending(uint32_t interruptID, volatile uint32_t * registerToSet,
         uint32_t valueToSet, volatile uint32_t * registerToChange, uint8_t bitMaskToChange, bool setBits);

    /**
     * @brief Buffer that captures every byte "sent" through testSimulateByteSent().
     * @note Must be compiled with IAP_EMULATION defined to have any effect.
     * @warning This buffer is only for unit testing and should never be used in production code.
     */
    static std::vector<uint8_t> sentBytesBuffer;
#endif
};

/** @}*/
#endif /* SBLIB_SERIAL_H_ */
