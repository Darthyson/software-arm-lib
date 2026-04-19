/**
 * Simple software UART implementation for LPC11xx
 *
 * @author Darthyson <darth@maptrack.de> Copyright (c) 2026
 *
 * @par
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 3 as
 * published by the Free Software Foundation.
 */

#include "sblib/soft_uart.h"
#include "sblib/digital_pin.h"
#include "sblib/interrupt.h"
#include "sblib/io_pin_names.h"
#include "sblib/platform.h"


SoftUART::SoftUART(const uint32_t rxPin, const uint32_t txPin, Timer& timer,
                   const BaudRate baudRate, const uint32_t systemClock)
    : bitTimer_(timer),
      timerMask_(0),
      bitPeriodTicks_(computeBitPeriod(baudRate)),
      halfBitTicks_(computeHalfBitPeriod(baudRate)),
      prescaler_(computePrescaler(systemClock)),
      txState_(UartState::Idle),
      rxState_(UartState::Idle),
      receiveBuffer_(BUFFER_SIZE),
      transmitBuffer_(BUFFER_SIZE),
      rxPin_(rxPin),
      txPin_(txPin),
      enabled_(false),
      txBitIndex_(0),
      txShiftReg_(0),
      rxBitIndex_(0),
      rxShiftReg_(0)
{
}

void SoftUART::begin()
{
    if (enabled_)
    {
        end();
    }

    // Resource conflict checks, Bus uses timer16_1, PIO1_8 and PIO1_9
    if (&bitTimer_ == &timer16_1)
    {
        fatalError(); // timer16_1 is reserved by the Bus class
    }
    if (rxPin_ == PIN_EIB_RX || rxPin_ == PIN_EIB_TX ||
        txPin_ == PIN_EIB_RX || txPin_ == PIN_EIB_TX)
    {
        fatalError(); // PIN_EIB_RX/PIN_EIB_TX are reserved by the Bus class
    }

    // Configure GPIO pins
    pinMode(txPin_, OUTPUT);
    digitalWrite(txPin_, true);
    constexpr uint32_t rxPinModeConfig = INPUT | PULL_UP | HYSTERESIS;
    pinMode(rxPin_, rxPinModeConfig);

    // Determine timer counter mask for wrapping arithmetic
    timerMask_ = bitTimer_.is32bitTimer() ? 0xFFFFFFFF : 0xFFFF;

    // Configure the hardware timer (free-running, no auto-reset)
    bitTimer_.begin();

    // Prescaler: one tick = 1 µs
    bitTimer_.prescaler(prescaler_);

    // MAT2 = Tx timing, MAT3 = Rx timing
    // No RESET flag: the timer free-runs and each channel independently
    // schedules its next match value.
    bitTimer_.matchMode(TIMER_MATCH_MAT2, DISABLE);
    bitTimer_.matchMode(TIMER_MATCH_MAT3, DISABLE);

    // Start with the timer stopped, it will be started when Tx begins or
    // when a Rx start bit is detected.
    bitTimer_.stop();
    bitTimer_.reset();

    // Clear any stale interrupt flags
    bitTimer_.resetFlags();

    // Set low interrupt priority
    bitTimer_.setIRQPriority(InterruptPriority::low);
    bitTimer_.interrupts();

    setInterruptPriority(digitalPinToIRQn(rxPin_), InterruptPriority::low);

    // Clear buffers
    receiveBuffer_.clear();
    transmitBuffer_.clear();

    // Init state machines
    txState_ = UartState::Idle;
    rxState_ = UartState::Idle;

    // Explicitly configure the Rx pin for falling-edge GPIO interrupt
    pinInterruptMode(rxPin_, INTERRUPT_EDGE_FALLING, rxPinModeConfig);

    // Enable Rx start-bit detection (GPIO falling-edge interrupt)
    enableRxInterrupt();
    enabled_ = true;
}

void SoftUART::end()
{
    if (!enabled_)
    {
        return;
    }

    flush();
    disableRxInterrupt();

    bitTimer_.matchMode(TIMER_MATCH_MAT2, DISABLE);
    bitTimer_.matchMode(TIMER_MATCH_MAT3, DISABLE);
    bitTimer_.stop();
    bitTimer_.noInterrupts();
    bitTimer_.end();

    enabled_ = false;
}

int16_t SoftUART::read()
{
    if (!enabled_)
    {
        return -1;
    }
    return receiveBuffer_.pop();
}

int16_t SoftUART::peek()
{
    if (!enabled_)
    {
        return -1;
    }
    return receiveBuffer_.peek();
}

uint32_t SoftUART::available()
{
    if (!enabled_)
    {
        return 0;
    }
    return receiveBuffer_.available();
}

uint32_t SoftUART::write(const uint8_t ch)
{
    if (!enabled_)
    {
        return 0;
    }

    // Block until the write buffer has space
    // interrupts must stay enabled, so the Tx ISR can drain the buffer
    while (transmitBuffer_.full())
    {
        waitForInterrupt();
    }

    // Critical section: push the byte and kick off Tx atomically so that
    // the ISR cannot race with the idle-check / startTx() call.
    noInterrupts();
    transmitBuffer_.push(ch);

    // If the Tx state machine is idle, kick off transmission
    if (txState_ == UartState::Idle)
    {
        startTx();
    }
    interrupts();

    return 1;
}

void SoftUART::flush()
{
    if (!enabled_)
    {
        return;
    }
    // Wait until TX is finished
    while (txState_ != UartState::Idle || !transmitBuffer_.empty())
    {
        waitForInterrupt();
    }
}

// ============================================================================
// RX start-bit detection (GPIO interrupt)
// ============================================================================

void SoftUART::enableRxInterrupt() const
{
    const uint32_t mask = digitalPinToBitMask(rxPin_);

    // Clear any stale GPIO interrupt flag for the RX pin.
    // During byte reception the Rx line toggles, and each falling edge
    // latches the raw interrupt status even when the pin interrupt is
    // disabled. Without clearing, re-enabling would immediately trigger
    // a spurious start-bit detection.
    gpioPorts[digitalPinToPort(rxPin_)]->IC = mask;
    clearPendingInterrupt(digitalPinToIRQn(rxPin_));

    // Re-enable the Rx pin interrupt (edge config was already set in begin())
    pinEnableInterrupt(rxPin_);
    enableInterrupt(digitalPinToIRQn(rxPin_));
}

void SoftUART::disableRxInterrupt() const
{
    pinDisableInterrupt(rxPin_);
}

void SoftUART::rxStartBitDetected()
{
    if (rxState_ != UartState::Idle)
    {
        fatalError();
        return; // already receiving --> ignore glitch
    }

    // Disable further GPIO interrupts on RX until this byte is complete
    disableRxInterrupt();

    rxState_ = UartState::StartBit;
    rxBitIndex_ = 0;
    rxShiftReg_ = 0;

    // Schedule the first RX sample at the center of the start bit (half-bit
    // from now) on MAT3.  The timer may already be running for TX.
    const uint32_t now = bitTimer_.value();
    bitTimer_.match(TIMER_MATCH_MAT3, (now + halfBitTicks_) & timerMask_);

    // Clear any stale MAT3 interrupt flag before enabling the interrupt.
    // While MAT3 was disabled (MCR bit = 0), the hardware still sets the IR
    // flag whenever TC matches MR3.  A leftover flag would fire the ISR
    // immediately instead of at the intended half-bit time.
    bitTimer_.resetFlag(TIMER_MATCH_MAT3);
    bitTimer_.matchMode(TIMER_MATCH_MAT3, INTERRUPT);
    bitTimer_.start(); // idempotent if already running for TX
}

void SoftUART::timerInterruptHandler()
{
    // Check and handle each match channel independently
    if (bitTimer_.flag(TIMER_MATCH_MAT2))
    {
        bitTimer_.resetFlag(TIMER_MATCH_MAT2);
        handleTxBit();
    }

    if (bitTimer_.flag(TIMER_MATCH_MAT3))
    {
        bitTimer_.resetFlag(TIMER_MATCH_MAT3);
        handleRxBit();
    }

    // If both idle, stop the timer to save power
    if (txState_ == UartState::Idle && rxState_ == UartState::Idle)
    {
        bitTimer_.stop();
    }
}

void SoftUART::startTx()
{
    if (transmitBuffer_.empty())
    {
        return; // nothing to send
    }

    txShiftReg_ = transmitBuffer_.pop();

    txState_ = UartState::StartBit;
    txBitIndex_ = 0;

    // Ensure the timer is running BEFORE driving the start bit so that
    // bitTimer_.value() returns a live counter value.  When both Tx and Rx
    // were idle the timer is stopped; reading 'now' from a frozen counter
    // and only calling start() afterwards extends the start bit by the
    // setup overhead – enough to corrupt the byte at higher baud rates
    // (e.g. 38400). start() is idempotent if the timer is already running.
    bitTimer_.start();

    // Send the start bit (line goes low)
    digitalWrite(txPin_, false);

    // Schedule the first TX interrupt one bit period from now on MAT2.
    const uint32_t now = bitTimer_.value();
    bitTimer_.match(TIMER_MATCH_MAT2, (now + bitPeriodTicks_) & timerMask_);

    // Clear any stale MAT2 interrupt flag before enabling the interrupt.
    // While MAT2 was disabled (MCR bit = 0), the hardware still sets the IR
    // flag whenever TC matches MR2 (e.g. after the 16-bit counter wraps).
    // A leftover flag would fire the ISR immediately, cutting the start bit
    // to ~1-2 µs instead of the full bit period – corrupting the entire byte.
    bitTimer_.resetFlag(TIMER_MATCH_MAT2);
    bitTimer_.matchMode(TIMER_MATCH_MAT2, INTERRUPT);
}

void SoftUART::handleTxBit()
{
    switch (txState_)
    {
        case UartState::Idle:
            break;

        case UartState::StartBit:
            // Start bit was already output in startTx().
            // Now transition to data bits --> output bit 0.
            txState_ = UartState::DataBits;
            txBitIndex_ = 0;
            digitalWrite(txPin_, (txShiftReg_ & 1) != 0);
            txShiftReg_ >>= 1;
            bitTimer_.match(TIMER_MATCH_MAT2,
                (bitTimer_.match(TIMER_MATCH_MAT2) + bitPeriodTicks_) & timerMask_);
            break;

        case UartState::DataBits:
            txBitIndex_++;
            if (txBitIndex_ < 8)
            {
                digitalWrite(txPin_, (txShiftReg_ & 1) != 0);
                txShiftReg_ >>= 1;
            }
            else
            {
                // All 8 data bits sent --> now send stop bit (high)
                txState_ = UartState::StopBit;
                digitalWrite(txPin_, true);
            }
            bitTimer_.match(TIMER_MATCH_MAT2,
                (bitTimer_.match(TIMER_MATCH_MAT2) + bitPeriodTicks_) & timerMask_);
            break;

        case UartState::StopBit:
            // The stop bit period is over.
            // Check if there is another byte to send.
            if (!transmitBuffer_.empty())
            {
                startTx();
            }
            else
            {
                txState_ = UartState::Idle;
                bitTimer_.matchMode(TIMER_MATCH_MAT2, DISABLE);
            }
            break;

        default:
            fatalError(); // Invalid state which should never happen
            break;
    }
}

void SoftUART::handleRxBit()
{
    switch (rxState_)
    {
        case UartState::Idle:
            break;

        case UartState::StartBit:
            // We are at the center of the start bit --> verify it is still low
            if (!digitalRead(rxPin_))
            {
                // Valid start bit --> schedule data bit sampling at full-bit intervals
                rxState_ = UartState::DataBits;
                rxBitIndex_ = 0;
                rxShiftReg_ = 0;
                bitTimer_.match(TIMER_MATCH_MAT3,
                               (bitTimer_.match(TIMER_MATCH_MAT3) + bitPeriodTicks_) & timerMask_);
            }
            else
            {
                // False start --> disable MAT3 and re-enable GPIO interrupt
                rxState_ = UartState::Idle;
                bitTimer_.matchMode(TIMER_MATCH_MAT3, DISABLE);
                enableRxInterrupt();
            }
            break;

        case UartState::DataBits:
            if (digitalRead(rxPin_))
            {
                rxShiftReg_ |= static_cast<uint8_t>(1 << rxBitIndex_);
            }

            rxBitIndex_++;
            if (rxBitIndex_ >= 8)
            {
                rxState_ = UartState::StopBit;
            }
            bitTimer_.match(TIMER_MATCH_MAT3,
                (bitTimer_.match(TIMER_MATCH_MAT3) + bitPeriodTicks_) & timerMask_);
            break;

        case UartState::StopBit:
            // Verify stop bit is high (we do not flag an error, just accept the byte)
            // Store the received byte if there is room in the buffer
            if (!receiveBuffer_.full())
            {
                receiveBuffer_.push(rxShiftReg_);
            }
            // else: byte is dropped (overrun)

            rxState_ = UartState::Idle;
            bitTimer_.matchMode(TIMER_MATCH_MAT3, DISABLE);

            // Re-enable GPIO falling-edge interrupt for the next start bit
            enableRxInterrupt();
            break;

        default:
            fatalError(); // Invalid state which should never happen
            break;
    }
}

void SoftUART::handleGpioInterrupt()
{
    const uint32_t rxPinMask = digitalPinToBitMask(rxPin_);
    //Check if the interrupt was caused by our Rx pin
    if (!(gpioPorts[digitalPinToPort(rxPin_)]->MIS & rxPinMask))
    {
        return;
    }

    // Clear the interrupt flag for this pin (IC is write-only, write-1-to-clear)
    gpioPorts[digitalPinToPort(rxPin_)]->IC = rxPinMask;
    rxStartBitDetected();
}

constexpr uint32_t SoftUART::computeBitPeriod(const BaudRate baudRate)
{
    return 1000000 / static_cast<uint32_t>(baudRate);
}

constexpr uint32_t SoftUART::computeHalfBitPeriod(const BaudRate baudRate)
{
    return computeBitPeriod(baudRate) / 2;
}

constexpr uint32_t SoftUART::computePrescaler(const uint32_t systemClock)
{
    return (systemClock / 1000000) - 1;
}
