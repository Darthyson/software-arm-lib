/*
 * Unit tests for the SoftUART class.
 *
 * Strategy
 * --------
 * SoftUART uses hardware registers (Timer, GPIO) of the LPC11xx, which are
 * emulated as plain-memory C structs by cpu-emu/system_lpc11xx.cpp.  All
 * hardware writes therefore succeed without any real interrupt mechanism.
 *
 * The internal state machines (Tx/Rx) are exercised by:
 *  - calling the public ISR entry-points timerInterruptHandler() and
 *    rxStartBitDetected() directly, and
 *  - using MockSoftUart, a thin test-only subclass that re-exposes the
 *    protected members and methods needed here as public, without touching
 *    any production code.
 *
 * GPIO levels are controlled by writing directly to the MASKED_ACCESS array
 * of the emulated LPC_GPIO_TypeDef.  The timer interrupt-flag register (IR)
 * is manipulated in the same way to simulate match-channel interrupts.
 *
 * Avoided: write() / flush() calls that would spin on transmitBuffer_.full()
 * or txState_ != Idle, because _test_wfi() is a no-op in the test host build
 * (see cpu-emu/system_lpc11xx.cpp) so those would loop forever.
 */

#include <sblib/soft_uart.h>
#include <sblib/digital_pin.h>
#include <sblib/platform.h>
#include <sblib/timer.h>
#include <sblib/types.h>
#include <cstring>
#include <vector>
#include <catch.hpp>

/**
 * @brief Timer helper class that exposes Timer's protected LPC_TMR_TypeDef* member as public
 */
class MockTimer : public Timer
{
public:
    using Timer::timer; // expose LPC_TMR_TypeDef* timer
};

/**
 * @brief Test mock that exposes the SoftUART members and methods needed for unit testing
 */
class MockSoftUart : public SoftUART
{
public:
    MockSoftUart(const uint32_t rxPin, const uint32_t txPin, Timer& timer,
                 const BaudRate baudRate, const uint32_t systemClock)
        : SoftUART(rxPin, txPin, timer, baudRate, systemClock)
    {
        // Reset all hardware registers relevant to the SoftUART tests.
        memset(gpioPorts[PIO0], 0, sizeof(LPC_GPIO_TypeDef));
        memset(mockTimerRegisters(), 0, sizeof(LPC_TMR_TypeDef));
    }

    // enums
    using SoftUART::UartState;

    // members
    using SoftUART::txState_;
    using SoftUART::rxState_;
    using SoftUART::timerMask_;
    using SoftUART::transmitBuffer_;
    using SoftUART::receiveBuffer_;
    using SoftUART::bitTimer_;
    using SoftUART::bitPeriodTicks_;
    using SoftUART::halfBitTicks_;
    using SoftUART::prescaler_;

    // methods
    using SoftUART::startTx;
    using SoftUART::handleTxBit;
    using SoftUART::handleRxBit;

    /** Access the raw timer hardware registers. */
    // NOLINT: safe downcast – MockTimer adds no data members, only re-exposes Timer::timer as public
    [[nodiscard]] LPC_TMR_TypeDef* mockTimerRegisters() const { return static_cast<MockTimer&>(bitTimer_).timer; } // NOLINT(cppcoreguidelines-pro-type-static-cast-downcast)

    /**
     * @brief Override flush() to drain the Tx state machine
     * @details Calls handleTxBit() directly, avoiding the deadlock caused by
     *          waitForInterrupt() being a no-op in unit testing
     */
    void flush() override
    {
        while (txState_ != UartState::Idle || !transmitBuffer_.empty())
        {
            handleTxBit();
        }
    }
};

/** Test Rx pin for SoftUART */
static constexpr uint32_t TEST_RX_PIN = PIO0_3;

/** Test Tx pin for SoftUART */
static constexpr uint32_t TEST_TX_PIN = PIO0_6;

/** Helper to read the Tx pin level from the emulated GPIO MASKED_ACCESS register. */
static bool mockGetTxLevel()
{
    constexpr uint32_t mask = digitalPinToBitMask(TEST_TX_PIN);
    return gpioPorts[digitalPinToPort(TEST_TX_PIN)]->MASKED_ACCESS[mask] != 0;
}

/** Helper to write the Rx pin level into the emulated GPIO MASKED_ACCESS register. */
static void mockSetRxLevel(const bool high)
{
    constexpr uint32_t mask = digitalPinToBitMask(TEST_RX_PIN);
    gpioPorts[digitalPinToPort(TEST_RX_PIN)]->MASKED_ACCESS[mask] = high ? mask : 0;
}

/** Helper to clear all timer interrupt flags */
static void mockClearTimerInterrupts(LPC_TMR_TypeDef* timer)
{
    timer->IR = 0;
}

/** Helper to clear a specific timer interrupt flag */
static void mockClearTimerInterruptBit(LPC_TMR_TypeDef* timer, const uint32_t bit)
{
    timer->IR &= ~bit;
}

/** Helper to set a specific timer interrupt flag */
static void mockSetTimerInterruptBit(LPC_TMR_TypeDef* timer, const uint32_t bit)
{
    timer->IR |= bit;
}

/** Simulate a MAT2 (Tx) match interrupt and then clear only the MAT2 bit. */
static void mockFireMat2Interrupt(MockSoftUart& uart)
{
    auto* timer = uart.mockTimerRegisters();
    mockSetTimerInterruptBit(timer, 1 << TIMER_MATCH_MAT2);
    uart.timerInterruptHandler();
    mockClearTimerInterruptBit(timer, 1 << TIMER_MATCH_MAT2);
}

/** Simulate a MAT3 (Rx) match interrupt and then clear only the MAT3 bit. */
static void mockFireMat3Interrupt(MockSoftUart& uart)
{
    auto* tmr = uart.mockTimerRegisters();
    mockSetTimerInterruptBit(tmr, 1 << TIMER_MATCH_MAT3);
    uart.timerInterruptHandler();
    mockClearTimerInterruptBit(tmr, 1 << TIMER_MATCH_MAT3);
}

/** Timer to use for unit tests */
constexpr Timer& testTimer = timer16_0;

// ---------------------------------
// Begin of the SoftUART unit tests
// ---------------------------------
TEST_CASE("SoftUART:: check states/methods before begin()", "[soft_uart]")
{
    MockSoftUart uart(TEST_RX_PIN, TEST_TX_PIN, testTimer, SoftUART::BaudRate::Baud9600, 48000000);
    SECTION("enabled() returns false")
    {
        REQUIRE(uart.enabled() == false);
    }

    SECTION("operator bool() returns false")
    {
        REQUIRE(!uart);
    }

    SECTION("read() returns -1")
    {
        REQUIRE(uart.read() == -1);
    }

    SECTION("peek() returns -1")
    {
        REQUIRE(uart.peek() == -1);
    }

    SECTION("available() returns 0")
    {
        REQUIRE(uart.available() == 0);
    }

    SECTION("write() returns 0")
    {
        REQUIRE(uart.write(0x55) == 0);
    }
}

TEST_CASE("SoftUART:: begin() and end()", "[soft_uart]")
{
    MockSoftUart uart(TEST_RX_PIN, TEST_TX_PIN, testTimer, SoftUART::BaudRate::Baud19200, 48000000);
    SECTION("enabled() is true after begin()")
    {
        uart.begin();
        REQUIRE(uart.enabled() == true);
        REQUIRE(static_cast<bool>(uart) == true);
        uart.end();
    }

    SECTION("enabled() is false after end()")
    {
        uart.begin();
        uart.end();
        REQUIRE(uart.enabled() == false);
        REQUIRE(static_cast<bool>(uart) == false);
    }

    SECTION("Tx pin is driven high (idle) after begin()")
    {
        uart.begin();
        REQUIRE(mockGetTxLevel() == true);
        uart.end();
    }

    SECTION("Tx pin is configured as output after begin()")
    {
        uart.begin();
        constexpr uint32_t txMask = digitalPinToBitMask(TEST_TX_PIN);
        REQUIRE((gpioPorts[PIO0]->DIR & txMask) == txMask);
        uart.end();
    }

    SECTION("Rx pin is configured as input after begin()")
    {
        uart.begin();
        constexpr uint32_t rxMask = digitalPinToBitMask(TEST_RX_PIN);
        REQUIRE((gpioPorts[PIO0]->DIR & rxMask) == 0);
        uart.end();
    }

    SECTION("Rx interrupt is enabled on the correct port after begin()")
    {
        uart.begin();
        // GPIO IE bit for the Rx pin must be set
        constexpr uint32_t rxMask = digitalPinToBitMask(TEST_RX_PIN);
        REQUIRE((gpioPorts[PIO0]->IE & rxMask) == rxMask);
        uart.end();
    }

    SECTION("bitTimer_ is started (TCR bit 0) after begin()")
    {
        uart.begin();
        // The timer should be stopped (no Tx/Rx in progress)
        REQUIRE((uart.mockTimerRegisters()->TCR & 1) == 0);
        uart.end();
    }

    SECTION("second begin() is idempotent (no crash, still enabled)")
    {
        uart.begin();
        uart.begin();
        REQUIRE(uart.enabled() == true);
        REQUIRE(static_cast<bool>(uart) == true);
        uart.end();
    }

    SECTION("end() before begin() is a safe no-op")
    {
        uart.end();
        REQUIRE(uart.enabled() == false);
        REQUIRE(static_cast<bool>(uart) == false);
    }

    SECTION("timerMask_ is 0xffff for 16-bit timer")
    {
        uart.begin();
        REQUIRE(uart.timerMask_ == 0xffff);
        uart.end();
    }
}

TEST_CASE("SoftUART:: Tx state machine", "[soft_uart]")
{
    MockSoftUart uart(TEST_RX_PIN, TEST_TX_PIN, testTimer, SoftUART::BaudRate::Baud38400, 12000000);
    uart.begin();

    SECTION("startTx() drives Tx pin low (start bit) and enters StartBit state")
    {
        uart.transmitBuffer_.push(0xaa);
        uart.startTx();

        REQUIRE(uart.txState_ == MockSoftUart::UartState::StartBit);
        REQUIRE(mockGetTxLevel() == false); // start bit = low
        uart.end();
    }

    SECTION("transmit bytes 0x00 to 0xff and verify bits on the Tx pin")
    {
        for (uint32_t counter = 0x00; counter <= 0xff; counter++)
        {
            const auto byteValue = static_cast<uint8_t>(counter);
            uart.transmitBuffer_.push(byteValue);
            uart.startTx();

            // Start bit --> Tx is low
            REQUIRE(mockGetTxLevel() == false);

            // 8 data bits (LSB first)
            for (uint8_t bit = 0; bit < 8; bit++)
            {
                uart.handleTxBit();
                const bool expectedBit = (byteValue >> bit) & 1;
                REQUIRE(mockGetTxLevel() == expectedBit);
            }

            // Stop bit --> Tx is high
            uart.handleTxBit();
            REQUIRE(uart.txState_ == MockSoftUart::UartState::StopBit);
            REQUIRE(mockGetTxLevel() == true);

            // StopBit --> State is Idle
            uart.handleTxBit();
            REQUIRE(uart.txState_ == MockSoftUart::UartState::Idle);
            REQUIRE(uart.transmitBuffer_.empty() == true);
            REQUIRE(mockGetTxLevel() == true);
        }
        uart.end();
    }

    SECTION("second byte is chained automatically")
    {
        // Push two bytes; first is processed by startTx(),
        // second should be sent as the next frame when StopBit ends.
        uart.transmitBuffer_.push(0xaa);
        uart.transmitBuffer_.push(0xbb);
        uart.startTx();

        // Drive through the first frame (10 handleTxBit() calls)
        for (uint8_t i = 0; i < 10; i++)
        {
            uart.handleTxBit();
        }
        // After the StopBit of the first byte, startTx() should have been
        // called again for the second byte.
        REQUIRE(uart.txState_ == MockSoftUart::UartState::StartBit);
        REQUIRE(mockGetTxLevel() == false); // the start bit of second frame

        // Drive through second frame
        for (uint8_t i = 0; i < 10; i++)
        {
            uart.handleTxBit();
        }
        REQUIRE(uart.txState_ == MockSoftUart::UartState::Idle);
        REQUIRE(uart.transmitBuffer_.empty() == true);
        uart.end();
    }
}

TEST_CASE("SoftUART:: Rx state machine", "[soft_uart]")
{
    MockSoftUart uart(TEST_RX_PIN, TEST_TX_PIN, testTimer, SoftUART::BaudRate::Baud2400, 150000000);
    uart.begin();

    SECTION("rxStartBitDetected() transitions from Idle to StartBit")
    {
        mockSetRxLevel(false); // simulate falling edge
        uart.rxStartBitDetected();
        REQUIRE(uart.rxState_ == MockSoftUart::UartState::StartBit);
        uart.end();
    }

    SECTION("false start bit is discarded and state returns to Idle")
    {
        mockSetRxLevel(false);
        uart.rxStartBitDetected();

        // Pin returns high before the center sample --> false start
        mockSetRxLevel(true);
        uart.handleRxBit();

        REQUIRE(uart.rxState_ == MockSoftUart::UartState::Idle);
        REQUIRE(uart.available() == 0);
        uart.end();
    }

    SECTION("receive bytes 0x00 to 0xff correctly")
    {
        for (uint32_t counter = 0x00; counter <= 0xff; counter++)
        {
            const auto byteValue = static_cast<uint8_t>(counter);

            // Start bit (falling edge)
            mockSetRxLevel(false);
            uart.rxStartBitDetected();

            // Validate start bit (pin still low)
            mockSetRxLevel(false);
            uart.handleRxBit();
            REQUIRE(uart.rxState_ == MockSoftUart::UartState::DataBits);

            // 8 data bits (LSB first)
            for (uint8_t bit = 0; bit < 8; bit++)
            {
                mockSetRxLevel((byteValue >> bit) & 1);
                uart.handleRxBit();
            }
            REQUIRE(uart.rxState_ == MockSoftUart::UartState::StopBit);

            // Stop bit
            mockSetRxLevel(true);
            uart.handleRxBit();
            REQUIRE(uart.rxState_ == MockSoftUart::UartState::Idle);
            REQUIRE(uart.available() == 1);
            REQUIRE(uart.read() == byteValue);
        }
        uart.end();
    }

    SECTION("receive buffer overrun: byte is silently dropped when buffer full")
    {
        // Fill the receiver buffer (SoftUART::BUFFER_SIZE - 1)
        constexpr auto bufCap = static_cast<uint32_t>(SoftUART::BUFFER_SIZE) - 1;
        for (uint32_t i = 0; i < bufCap; i++)
        {
            uart.receiveBuffer_.push(static_cast<uint8_t>(i & 0xff));
        }
        REQUIRE(uart.receiveBuffer_.full() == true);

        // Attempt to receive one more byte (0xaa) — it must be dropped
        mockSetRxLevel(false);
        uart.rxStartBitDetected();
        mockSetRxLevel(false);
        uart.handleRxBit();

        for (uint8_t i = 0; i < 8; i++)
        {
            mockSetRxLevel((i % 2) == 0); // alternating bits
            uart.handleRxBit();
        }
        mockSetRxLevel(true);
        uart.handleRxBit();

        // Buffer must still be exactly full (byte was dropped)
        REQUIRE(uart.receiveBuffer_.full() == true);
        REQUIRE(uart.available() == bufCap);
        uart.end();
    }

    SECTION("peek() does not consume the received byte")
    {
        mockSetRxLevel(false);
        uart.rxStartBitDetected();
        mockSetRxLevel(false);
        uart.handleRxBit();

        for (uint8_t i = 0; i < 8; i++)
        {
            mockSetRxLevel(false);
            uart.handleRxBit();
        }
        mockSetRxLevel(true);
        uart.handleRxBit();

        REQUIRE(uart.available() == 1);
        REQUIRE(uart.peek() == 0x00);
        REQUIRE(uart.available() == 1); // still 1 after peek
        REQUIRE(uart.read() == 0x00);
        REQUIRE(uart.available() == 0);
        uart.end();
    }
}

TEST_CASE("SoftUART:: timerInterruptHandler() dispatch", "[soft_uart]")
{
    MockSoftUart uart(TEST_RX_PIN, TEST_TX_PIN, testTimer, SoftUART::BaudRate::Baud57600, 20000000);
    uart.begin();

    SECTION("MAT2 flag routes to handleTxBit(): StartBit --> DataBits")
    {
        uart.transmitBuffer_.push(0xaa);
        uart.startTx();
        REQUIRE(uart.txState_ == MockSoftUart::UartState::StartBit);

        mockFireMat2Interrupt(uart);

        REQUIRE(uart.txState_ == MockSoftUart::UartState::DataBits);
        uart.end();
    }

    SECTION("MAT3 flag routes to handleRxBit(): StartBit --> DataBits")
    {
        mockSetRxLevel(false);
        uart.rxStartBitDetected();
        REQUIRE(uart.rxState_ == MockSoftUart::UartState::StartBit);

        mockSetRxLevel(false); // pin still low = valid start bit
        mockFireMat3Interrupt(uart);

        REQUIRE(uart.rxState_ == MockSoftUart::UartState::DataBits);
        uart.end();
    }

    SECTION("no flags set: state machines remains Idle")
    {
        mockClearTimerInterrupts(uart.mockTimerRegisters());
        uart.timerInterruptHandler();
        REQUIRE(uart.txState_ == MockSoftUart::UartState::Idle);
        REQUIRE(uart.rxState_ == MockSoftUart::UartState::Idle);
        uart.end();
    }
}

TEST_CASE("SoftUART:: write()", "[soft_uart]")
{
    MockSoftUart uart(TEST_RX_PIN, TEST_TX_PIN, testTimer, SoftUART::BaudRate::Baud57600, 24000000);
    SECTION("write() when not enabled returns 0")
    {
        REQUIRE(uart.write(0x42) == 0);
    }

    SECTION("write() returns 1 when enabled and buffer wasn't full")
    {
        uart.begin();
        REQUIRE(uart.write(0x42) == 1);
        uart.end();
    }

    SECTION("write() triggers Tx start: pin goes low after first byte")
    {
        uart.begin();
        uart.write(0x42);
        // startTx() is called inside write() because txState_ was Idle
        REQUIRE(mockGetTxLevel() == false); // start bit
        REQUIRE(uart.txState_ == MockSoftUart::UartState::StartBit);
        uart.end();
    }

    SECTION("write() does not call startTx() again when Tx is already active")
    {
        uart.begin();
        uart.write(0x01); // starts Tx
        REQUIRE(uart.txState_ == MockSoftUart::UartState::StartBit);

        uart.write(0x02); // queued, must NOT restart Tx
        REQUIRE(uart.txState_ == MockSoftUart::UartState::StartBit);
        REQUIRE(uart.transmitBuffer_.available() == 1); // 0x02 still in transmitBuffer_
    }
}

TEST_CASE("SoftUART:: bit timing for all baud rates at 48 MHz", "[soft_uart]")
{
    struct BitTimingTestCase
    {
        SoftUART::BaudRate baudRate;
        uint32_t expectedBitPeriod;
        uint32_t expectedHalfBit;
    };

    const std::vector<BitTimingTestCase> testCases = {
        { SoftUART::BaudRate::Baud2400,    416, 208 },
        { SoftUART::BaudRate::Baud4800,    208, 104 },
        { SoftUART::BaudRate::Baud9600,    104,  52 },
        { SoftUART::BaudRate::Baud19200,    52,  26 },
        { SoftUART::BaudRate::Baud38400,    26,  13 },
        { SoftUART::BaudRate::Baud57600,    17,   8 },
        { SoftUART::BaudRate::Baud115200,    8,   4 },
        { SoftUART::BaudRate::Baud230400,    4,   2 },
        { SoftUART::BaudRate::Baud460800,    2,   1 },
        { SoftUART::BaudRate::Baud576000,    1,   0 },
    };

    for (const auto& [baudRate, expectedBitPeriod, expectedHalfBit] : testCases)
    {
        MockSoftUart uart(TEST_RX_PIN, TEST_TX_PIN, testTimer, baudRate, 48000000);
        REQUIRE(uart.bitPeriodTicks_ == expectedBitPeriod);
        REQUIRE(uart.halfBitTicks_ == expectedHalfBit);
        REQUIRE(uart.prescaler_ == 47); // 48 MHz => prescaler always 47
    }
}

TEST_CASE("SoftUART:: prescaler with different system clocks", "[soft_uart]")
{
    struct PrescalerTestCase
    {
        uint32_t systemClock;
        uint32_t expectedPrescaler;
    };

    const std::vector<PrescalerTestCase> testCases = {
        {  1000000,  0 },
        { 12000000, 11 },
        { 24000000, 23 },
        { 48000000, 47 },
        { 72000000, 71 },
    };

    for (const auto& [systemClock, expectedPrescaler] : testCases)
    {
        MockSoftUart uart(TEST_RX_PIN, TEST_TX_PIN, testTimer,
                          SoftUART::BaudRate::Baud9600, systemClock);
        REQUIRE(uart.prescaler_ == expectedPrescaler);
    }
}

TEST_CASE("SoftUART:: prescaler is applied to timer after begin()", "[soft_uart]")
{
    struct PrescalerAppliedTestCase
    {
        uint32_t systemClock;
        uint32_t expectedPR;
    };

    const std::vector<PrescalerAppliedTestCase> testCases = {
        { 12000000, 11 },
        { 48000000, 47 },
    };

    for (const auto& [systemClock, expectedPR] : testCases)
    {
        MockSoftUart uart(TEST_RX_PIN, TEST_TX_PIN, testTimer,
                          SoftUART::BaudRate::Baud9600, systemClock);
        uart.begin();
        REQUIRE(uart.mockTimerRegisters()->PR == expectedPR);
        uart.end();
    }
}

TEST_CASE("SoftUART:: Tx works at various baud rates", "[soft_uart]")
{
    const std::vector baudRates = {
        SoftUART::BaudRate::Baud2400,
        SoftUART::BaudRate::Baud9600,
        SoftUART::BaudRate::Baud19200,
        SoftUART::BaudRate::Baud57600,
        SoftUART::BaudRate::Baud115200,
    };

    for (const auto baudRate : baudRates)
    {
        MockSoftUart uart(TEST_RX_PIN, TEST_TX_PIN, testTimer, baudRate, 48000000);
        uart.begin();

        constexpr uint8_t byteValue = 0x55;
        uart.transmitBuffer_.push(byteValue);
        uart.startTx();

        // Start bit --> Tx is low
        REQUIRE(mockGetTxLevel() == false);

        // 8 data bits (LSB first)
        for (uint8_t bit = 0; bit < 8; bit++)
        {
            uart.handleTxBit();
            const bool expectedBit = (byteValue >> bit) & 1;
            REQUIRE(mockGetTxLevel() == expectedBit);
        }

        // Stop bit --> Tx is high
        uart.handleTxBit();
        REQUIRE(uart.txState_ == MockSoftUart::UartState::StopBit);
        REQUIRE(mockGetTxLevel() == true);

        // StopBit --> State is Idle
        uart.handleTxBit();
        REQUIRE(uart.txState_ == MockSoftUart::UartState::Idle);

        uart.end();
    }
}

TEST_CASE("SoftUART:: Rx works at various baud rates", "[soft_uart]")
{
    const std::vector baudRates = {
        SoftUART::BaudRate::Baud2400,
        SoftUART::BaudRate::Baud9600,
        SoftUART::BaudRate::Baud19200,
        SoftUART::BaudRate::Baud57600,
        SoftUART::BaudRate::Baud115200,
    };

    for (const auto baudRate : baudRates)
    {
        MockSoftUart uart(TEST_RX_PIN, TEST_TX_PIN, testTimer, baudRate, 48000000);
        uart.begin();

        constexpr uint8_t byteValue = 0xA3;

        // Start bit (falling edge)
        mockSetRxLevel(false);
        uart.rxStartBitDetected();

        // Validate start bit (pin still low)
        mockSetRxLevel(false);
        uart.handleRxBit();
        REQUIRE(uart.rxState_ == MockSoftUart::UartState::DataBits);

        // 8 data bits (LSB first)
        for (uint8_t bit = 0; bit < 8; bit++)
        {
            mockSetRxLevel((byteValue >> bit) & 1);
            uart.handleRxBit();
        }
        REQUIRE(uart.rxState_ == MockSoftUart::UartState::StopBit);

        // Stop bit
        mockSetRxLevel(true);
        uart.handleRxBit();
        REQUIRE(uart.rxState_ == MockSoftUart::UartState::Idle);
        REQUIRE(uart.available() == 1);
        REQUIRE(uart.read() == byteValue);

        uart.end();
    }
}

