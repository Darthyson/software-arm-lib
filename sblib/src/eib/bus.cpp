/**
 * bus.cpp - Low level EIB bus access.
 *
 * Copyright (c) 2014 Stefan Taferner <stefan.taferner@gmx.at>
 * Copyright (c) 2021 Horst Rauch
 * ********************************************************************
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 3 as
 *  published by the Free Software Foundation.
 *
 **/

#include <sblib/eib/bus.h>
#include <sblib/eib/knx_lpdu.h>
#include <sblib/eib/knx_npdu.h>
#include <sblib/core.h>
#include <sblib/interrupt.h>
#include <sblib/platform.h>
#include <sblib/eib/addr_tables.h>
#include <sblib/eib/bcu_base.h>
#include <sblib/eib/bus_const.h>
#include <sblib/eib/bus_debug.h>

// constructor for Bus object. Initialize basic interface parameter to bus and set SM to IDLE
Bus::Bus(BcuBase* bcuInstance, Timer& aTimer, int aRxPin, int aTxPin, TimerCapture aCaptureChannel, TimerMatch aPwmChannel)
:bcu(bcuInstance)
,timer(aTimer)
,rxPin(aRxPin)
,txPin(aTxPin)
,captureChannel(aCaptureChannel)
,pwmChannel(aPwmChannel)
{
    timeChannel = (TimerMatch) ((pwmChannel + 2) & 3);  // +2 to be compatible to old code during refactoring
    state = Bus::INIT;
    sendRetriesMax = NACK_RETRY_DEFAULT;
    sendBusyRetriesMax = BUSY_RETRY_DEFAULT;
    setKNX_TX_Pin(txPin);
    telegram = new byte[bcu->maxTelegramSize()]();
}

/**
 * Start BUS operation
 *
 * reset operating parameters, start the timer
 * set the pwm parameter so that we have no pulse on bus and no interrupt from timer
 * activate capture interrupt, set bus pins to respective mode
 *
 * //todo get defined values from usereprom for busy-retry and nack-retry
 */
void Bus::begin()
{
    //todo load send-retries from eprom -- this should actually be done by the BCU
    //sendRetriesMax = userEeprom.maxRetransmit & 0x03;
    //sendBusyRetriesMax = (userEeprom.maxRetransmit >> 5) & 0x03;

    telegramLen = 0;
    rx_error = RX_OK;

    tx_error = TX_OK;
    sendCurTelegram = nullptr;
    prepareForSending();
    //initialize bus-timer( e.g. defined as 16bit timer1)
    timer.setIRQPriority(0); // ensure highest IRQ-priority for the Bus timer
    timer.begin();
    timer.pwmEnable(pwmChannel);
    //timer.counterMode(DISABLE,  captureChannel | FALLING_EDGE); // todo  enabled the timer reset by the falling edge of cap event
    timer.start();
    timer.prescaler(TIMER_PRESCALER);
    initState();

    // wait until output is driven low before enabling output pin.
    // Using digitalWrite(txPin, 0) does not work with MAT channels.
    timer.value(0xffff); // trigger the next event immediately
    while (timer.getMatchChannelLevel(pwmChannel) == true);
    pinMode(txPin, OUTPUT_MATCH);   // Configure bus output
    pinMode(rxPin, INPUT_CAPTURE | HYSTERESIS);  // Configure bus input

    timer.resetFlags();
    timer.interrupts();

    DB_TELEGRAM(serial.println("DUMP_TELEGRAMS Bus telegram dump enabled."));
#ifdef DEBUG_BUS
    IF_DEBUG(serial.println("DEBUG_BUS dump enabled."));
#endif

#ifdef DEBUG_BUS_BITLEVEL
    IF_DEBUG(serial.println("DEBUG_BUS_BITLEVEL dump enabled."));
#endif

    DB_BUS(
        //we use 32bit timer 0 as debugging timer running with 1Mhz or system clock (48MHz)
        ttimer.begin();
        ttimer.start();
        ttimer.noInterrupts();
        ttimer.restart();
        // ttimer.prescaler(0);
        ttimer.prescaler(TIMER_PRESCALER);
        serial.print("Bus begin - Timer prescaler: ", (unsigned int)TIMER_PRESCALER, DEC, 6);
        serial.print(" ttimer prescaler: ", ttimer.prescaler(), DEC, 6);
        serial.println(" ttimer value: ", ttimer.value(), DEC, 6);
        serial.print("nak retries: ", sendRetriesMax, DEC, 6);
        serial.print(" busy retries: ", sendBusyRetriesMax, DEC, 6);
        serial.print(" phy addr: ", PHY_ADDR_AREA(bcu->ownAddress()), DEC);
        serial.print(".", PHY_ADDR_LINE(bcu->ownAddress()), DEC);
        serial.print(".", PHY_ADDR_DEVICE(bcu->ownAddress()), DEC);
        serial.print(" (0x",  bcu->ownAddress(), HEX, 4);
        serial.println(")");
    ); // DB_BUS

#ifdef PIO_FOR_TEL_END_IND
    pinMode(PIO_FOR_TEL_END_IND, OUTPUT);
    digitalWrite(PIO_FOR_TEL_END_IND, 0);
#endif
}

void Bus::pause(bool waitForTelegramSent)
{
    auto paused = false;

    while (!paused)
    {
        // Need to be atomic here, otherwise we could transition to a non-pausable state after we've
        // determined it's possible to pause.
        noInterrupts();

        if (canPause(waitForTelegramSent))
        {
            // Continue capturing falling edges on the bus to enable optimized resume.
            timer.captureMode(captureChannel, FALLING_EDGE);
            timer.matchMode(timeChannel, RESET);
            timer.match(timeChannel, 0xfffe);
            // In both pausable states, pwmChannel is set to 0xffff already.
            state = INIT;
            paused = true;
        }

        interrupts();
        waitForInterrupt();
    }
}

void Bus::resume()
{
    // It is possible to optimize this and resume in INIT with smaller wait time or directly in IDLE
    // or WAIT_50BT_FOR_NEXT_RX_OR_PENDING_TX_OR_IDLE. That costs quite some code size, though, so
    // take the easy (and small) route.

    noInterrupts();
    initState();
    interrupts();
}

bool Bus::canPause(bool waitForTelegramSent)
{
    // Trivial case: IDLE is always safe.
    if (state == IDLE)
        return true;

    // WAIT_50BT_FOR_NEXT_RX_OR_PENDING_TX_OR_IDLE might be safe (see below), but all others are not.
    if (state != WAIT_50BT_FOR_NEXT_RX_OR_PENDING_TX_OR_IDLE)
        return false;

    // In case we want to send a telegram, if we
    // * have sent the telegram at least once and received a negative (or no) confirmation, we
    //   cannot pause as we must retry within a certain time frame.
    // * have not sent the telegram yet, it depends on @ref waitForTelegramSent whether the user
    //   wants us to pause or not.
    if (sendCurTelegram != nullptr)
    {
        if (repeatTelegram)
            return false;

        return !waitForTelegramSent;
    }

    return true;
}

void Bus::prepareTelegram(unsigned char* telegram, unsigned short length) const
{
    setSenderAddress(telegram, (uint16_t)bcu->ownAddress());

    // Calculate the checksum
    unsigned char checksum = 0xff;
    for (unsigned short i = 0; i < length; ++i)
    {
        checksum ^= telegram[i];
    }
    telegram[length] = checksum;
}

/**
 *       Interface to upper layer for sending a telegram
 *
 * Is called from within the BCU-loop method. Is blocking if there is no space
 * in the Telegram buffer (as we have only one buffer at BCU level, the check for buffer free is
 * in the BCU-loop on bus.sendingFrame())
 *
 * Send a telegram. The checksum byte will be added at the end of telegram[].
 * Ensure that there is at least one byte space at the end of telegram[].
 *
 * @param telegram - the telegram to be sent.
 * @param length - the length of the telegram in sbSendTelegram[], without the checksum
 */
void Bus::sendTelegram(unsigned char* telegram, unsigned short length)
{
    prepareTelegram(telegram, length);

    // Wait until there is space in the sending queue
    while (sendCurTelegram != nullptr);

    sendCurTelegram = telegram;

    DB_TELEGRAM(
        unsigned int t;
        t = ttimer.value();
        serial.print("QUE: (", t, DEC, 8); // queued to send
        serial.print(") ");
        for (int i = 0; i <= length; ++i)
        {
            if (i) serial.print(" ");
            serial.print(telegram[i], HEX, 2);
        }
        serial.println();
    );

    // Start sending if the bus is idle or sending will be triggered in WAIT_50BT_FOR_NEXT_RX_OR_PENDING_TX_OR_IDLE after finishing current TX/RX
    noInterrupts();
    if (state == IDLE)
    {
        startSendingImmediately();
    }
    interrupts();
}

void Bus::initState()
{
    // Any capture interrupt during INIT resets the timer (see timerInterruptHandler).
    // Interesting is the amount of time to wait, though.
    //
    // At the time of the last falling edge, a device sent a 0 bit.
    //
    // In the worst case, this was a 0xFE checksum byte of a telegram that was not acknowledged.
    // Then, the minimum time to wait would be the time of the 0 bit, seven 1 bits, parity bit,
    // stop bit, plus 50 bits idle time, i.e. a whopping 60 bit times.
    //
    // In the common case, though, the last frame was an acknowledge frame, and those all have
    // parityBit=0, i.e. the last falling edge was the parity bit of the acknowledge frame.
    // Then, it's sufficient to wait for parity bit, stop bit, and 50 bits idle time,
    // i.e. 52 bit times.
    //
    // If we go with waiting only for 52 bit times to optimize for the common case, but we are
    // actually in the worst case, that is still fine with the KNX spec: We'd have start_of_frame
    // already after 42 bit times, and the KNX spec v2.1 chapter 3/2/2 section 2.3.1 figure 40
    // (p.35) requires a minimum start_of_frame of 40 bit times, so we're within in spec.
    //
    // So, wait for 42 bit times in INIT, then transition to
    // WAIT_50BT_FOR_NEXT_RX_OR_PENDING_TX_OR_IDLE.

    const uint16_t waitTime = BIT_TIMES_DELAY(2) + WAIT_40BIT;

    timer.captureMode(captureChannel, FALLING_EDGE | INTERRUPT);
    timer.matchMode(timeChannel, INTERRUPT); // at timeout we can start to receive, but need to wait 10BT more before starting to send
    timer.restart();
    timer.match(timeChannel, waitTime);
    timer.match(pwmChannel, 0xffff);
    state = INIT;
    sendAck = 0;
}

void Bus::idleState()
{
    tb_t( 99, ttimer.value(), tb_in);
    tb_h( 99, sendAck, tb_in);

    timer.captureMode(captureChannel, FALLING_EDGE | INTERRUPT ); // for any receiving start bit on the Bus
    timer.matchMode(timeChannel, RESET); // no timeout interrupt, reset at match todo we could stop timer for power saving
    timer.match(timeChannel, 0xfffe); // stop pwm pulse generation, set output to low
    timer.match(pwmChannel, 0xffff);
    //timer.counterMode(DISABLE,  captureChannel | FALLING_EDGE); //todo enabled the  timer reset by the falling edge of cap event
    state = Bus::IDLE;
}

void Bus::startSendingImmediately()
{
    state = Bus::WAIT_50BT_FOR_NEXT_RX_OR_PENDING_TX_OR_IDLE;
    timer.restart();
    timer.match(timeChannel, 1);
    timer.matchMode(timeChannel, INTERRUPT | RESET);
}

void Bus::prepareForSending()
{
    tx_error = TX_OK;

    collisions = 0;
    sendRetries = 0;
    sendBusyRetries = 0;
    sendTelegramLen = 0;
    wait_for_ack_from_remote = false;
    repeatTelegram = false;
    busy_wait_from_remote = false;
}

/*
 * *********** Layer 2 rx/tx handling, part of the interrupt processing -> keep as short as possible**********
 *
 * We arrive here after we received  data from bus indicated by a time out of >2BT after RX of bits
 *
 * Check for valid data reception in rx_error. If we received some bytes, process data (valid telegram) and if tel is for us
 * check for need of sending ACK to sender side and start respective process:  time for ACK (15BT) or inter telegram wait (50BT)
 * and set WAIT_50BT_FOR_NEXT_RX_OR_PENDING_TX_OR_IDLE state
 * If we had a collision,  no action
 * End of telegram is indicated by a timeout. As the timer is still running we have the accurate time since the last stop bit.
 *
 * copy local rx-data to 2nd rx buffer for parallel processing on higher layer and SM

 * Provide some Data-layer functions:
 *    check telegram type
 *    check if it is for us (phy adr, or Grp Adr belong to us)
 *    check if we want it anyhow (TL, LL info)
 *    send ACK/NACK if requested
 *    send BUSYACK if we are still busy on higher layers - no free buffer available
 *    handling of repeated telegrams
 *
 * Input by global data:
 *    timer flags: timeout indicates end of last telegram, timer value hold the time since rx of last stop bit
 *    rx_error: hold error status of rx process
 *    collision: indicated a collision during last tx process which is then continued in RX process
 *    parameter valid: true if parity and checksum of received tel are ok
 *
 * Output data:
 *    processTel: indicate telegram reception to the looping function by setting <processTel> to true
 *    telegram[]: received telegram in telegram buffer <telegram[]>, length in telegramLen
 *    telegramLen: rx telegram length
 *    sendAck:  !0:  RX process need to send ack to sending side back, set wait timer accordingly
 *
 *
 * @param bool of all received char parity and frame checksum error
 *
 */
void Bus::handleTelegram(bool valid)
{
#ifdef DEBUG_BUS
    b1 = (((unsigned int)rx_telegram[0]<<24) |((unsigned int)rx_telegram[1]<<16) |((unsigned int)rx_telegram[2]<<8)| (rx_telegram[3]));
    b2= ( ((unsigned int)rx_telegram[4]<<8) | (rx_telegram[5]));
    b3= (( unsigned int)rx_telegram[6]<<8)|((unsigned int)rx_telegram[7]);
    b4= nextByteIndex;
    b5= ( collisions + (valid ? 8 : 0));
    tb2( 9000, b5,  b1, b2, b3, b4, tb_in);
#endif

    DB_TELEGRAM(
        if (nextByteIndex){
            for (int i = 0; i < nextByteIndex; ++i)
            {
                telBuffer[i] = rx_telegram[i];
            }
            telLength = nextByteIndex;
            telcollisions = collisions;
        }
    );

    sendAck = 0; // clear any pending ACK TX
    int time = SEND_WAIT_TIME -  PRE_SEND_TIME; // default wait time after bus action
    state = Bus::WAIT_50BT_FOR_NEXT_RX_OR_PENDING_TX_OR_IDLE;//  default next state is wait for 50 bit times for pending tx or new rx
    tb_h( 908, currentByte, tb_in);
    tb_h( 909, parity, tb_in);

#ifndef BUSMONITOR // no processing if we are in monitor mode

    // Received a valid telegram with correct checksum and valid control byte (normal data frame with preamble bits)?
    //todo extended tel, check tel len, give upper layer error info
    if (nextByteIndex >= 8 && valid && (( rx_telegram[0] & VALID_DATA_FRAME_TYPE_MASK) == VALID_DATA_FRAME_TYPE_VALUE)
        && nextByteIndex <= bcu->maxTelegramSize()  )
    {
        int destAddr = (rx_telegram[3] << 8) | rx_telegram[4];
        bool processTel = false;

        // Only process the telegram if it is for us
        if (rx_telegram[5] & 0x80) // group address or physical address
        {
            processTel = (destAddr == 0); // broadcast
            processTel |= (bcu->addrTables != nullptr) && (bcu->addrTables->indexOfAddr(destAddr) >= 0); // known group address
        }
        else if (destAddr == bcu->ownAddress())
        {
            processTel = true;
        }

        // with disabled TL we also process the telegram, so the application (e.g. ft12, knx-if) can handle it completely by itself
        processTel |= !(bcu->userRam->status() & BCU_STATUS_TRANSPORT_LAYER);

        DB_TELEGRAM(telRXNotProcessed = !processTel);

        if (processTel)
        {// check for repeated telegram, did we already received it
            // check the repeat bit in header and compare with previous received telegram still stored in the telegram[] buffer
            bool already_received = false;
            if (!(rx_telegram[0] & SB_TEL_REPEAT_FLAG)) // a repeated tel
            {// compare telegrams
                if ((rx_telegram[0] & ~SB_TEL_REPEAT_FLAG) == (telegram[0] & ~SB_TEL_REPEAT_FLAG))
                {// same header -> compare remaining bytes, excluding the checksum byte
                    int i;
                    for (i = 1; (i < nextByteIndex - 1) && (rx_telegram[i] == telegram[i]); i++);
                    if (i == nextByteIndex - 1) {
                        already_received = true;
                    }
                }
            }

            // check for space in rx buffer for next telegram, if no space available, send nothing
            if (telegramLen)
            {
                // KNX Spec. 2.1. 3/2/2 2.4.1 p.38
                // Device should only send a LL_BUSY if it knows that the telegram can be processed within the next 100ms.
                // Since we know nothing about the running application we better send nothing
                sendAck = 0;
                rx_error |= RX_BUFFER_BUSY;
            }
            else
            {
                sendAck = SB_BUS_ACK;
                // store data in telegram buffer for higher layers, set telegramLen to indicate data available
                if (!already_received)
                {
                    for (int i = 0; i < nextByteIndex; i++) telegram[i] = rx_telegram[i];
                    telegramLen = nextByteIndex;
                    rx_error = RX_OK;
                }
            }

            // LL_ACK only allowed, if link layer is in normal mode, not busmonitor mode
            auto suppressAck = !(bcu->userRam->status() & BCU_STATUS_LINK_LAYER);
            // LL_ACK only allowed for L_Data frames
            suppressAck |= rx_telegram[0] & SB_TEL_DATA_FRAME_FLAG;
            if (suppressAck)
            {
                sendAck = 0;
            }

            if (sendAck)
            {
                // ACK has priority, no rx/tx in between
                state = Bus::RECV_WAIT_FOR_ACK_TX_START;
                time = SEND_ACK_WAIT_TIME - PRE_SEND_TIME;
            }
        }
    }
    else if (nextByteIndex == 1 && wait_for_ack_from_remote) // Received a spike or a bus acknowledgment, only parity, no checksum
    {
        tb_h( 907, currentByte, tb_in);

        wait_for_ack_from_remote = false;

        // received an ACK frame so clear checksum bit previously set in ISR Bus::timerInterruptHandler
        rx_error &= ~RX_CHECKSUM_ERROR;

        // received telegram is ACK or repetition max -> send next telegram
        if ((parity && currentByte == SB_BUS_ACK) || sendRetries >= sendRetriesMax || sendBusyRetries >= sendBusyRetriesMax)
        {
            // last sending to remote was ok or max retry, prepare for next tx telegram
            if (!(parity && currentByte == SB_BUS_ACK))
                tx_error |= TX_RETRY_ERROR;
            tb_h( 906, tx_error, tb_in);
            finishSendingTelegram();
        }
        else if (parity && (currentByte == SB_BUS_BUSY || currentByte == SB_BUS_NACK_BUSY))
        {
            time = BUSY_WAIT_150BIT - PRE_SEND_TIME;
            tx_error |= TX_REMOTE_BUSY_ERROR;
            busy_wait_from_remote = true;
            repeatTelegram = true;
        }
        else
        {
            // we received nack or something else, need to repeat last telegram
            tx_error |= TX_NACK_ERROR;
            busy_wait_from_remote = false;
            repeatTelegram = true;
        }
    }
    else // We received an acknowledge frame, wrong checksum/parity or more than one byte but too short for a telegram
    {
        auto isAcknowledgeFrame = (nextByteIndex == 1) &&
                                  (currentByte == SB_BUS_ACK ||
                                   currentByte == SB_BUS_NACK ||
                                   currentByte == SB_BUS_BUSY ||
                                   currentByte == SB_BUS_NACK_BUSY);
        if (isAcknowledgeFrame)
        {
            rx_error &= ~RX_INVALID_TELEGRAM_ERROR;
            rx_error &= ~RX_CHECKSUM_ERROR; // received an ACK frame so clear checksum bit previously set in ISR Bus::timerInterruptHandler
        }
        else
        {
            rx_error |= RX_INVALID_TELEGRAM_ERROR;
        }
    }

    // After sending a telegram we're waiting to receive an LL_ACK. If that does not arrive and we receive
    // anything else (e.g. another device sneaks in a full telegram), we need to repeat the telegram we sent
    // and stop waiting for the LL_ACK. If we don't, then we'd erroneously interpret an LL_ACK after the
    // foreign telegram as belonging to the telegram we sent.
    if (wait_for_ack_from_remote)
    {
        repeatTelegram = true;
        wait_for_ack_from_remote = false;
    }

    DB_TELEGRAM(telrxerror = rx_error);

    tb_d( 901, state, tb_in);    tb_d( 902, sendRetries, tb_in);   tb_d( 903, sendBusyRetries, tb_in); tb_h( 904, sendAck, tb_in);
    tb_h( 905, rx_error, tb_in); tb_d( 910, telegramLen, tb_in); tb_d( 911, nextByteIndex, tb_in);

#endif
#ifdef BUSMONITOR
    rx_error = RX_OK;
#endif

    //we received a telegram, next action wait to send ack back or wait 50 bit times for next rx/tx (todo check for improved noise margin with cap event disabled)
    //timer.captureMode(captureChannel, FALLING_EDGE); // no capture during wait- improves bus noise margin l
    timer.captureMode(captureChannel, FALLING_EDGE | INTERRUPT ); // todo enable timer reset by cap event
    timer.match(timeChannel, time - 1); // todo adjust time value by processing timer since we had the end of telegram detection
}

/*
 * Finish the telegram sending process.
 *
 * Notify upper layer of completion and prepare for next telegram transmission.
 */
void Bus::finishSendingTelegram()
{
    if (sendCurTelegram != nullptr)
    {
        sendCurTelegram = nullptr;
        bcu->finishedSendingTelegram(!(tx_error & TX_RETRY_ERROR));
    }

    prepareForSending();
}

/*
 * Track collision in sending process correctly.
 */
void Bus::encounteredCollision()
{
    // We do not care about collisions in acknowledge frames as these frames will not be repeated.
    // Thus, track the number of collisions only in normal frames.
    if (!sendAck)
    {
        collisions++;
        tx_error |= TX_COLLISION_ERROR;
    }
}

/*
 * State Machine - driven by interrupts of timer and capture input
 *
 * Interrupt prolog (from event at cap pin or timer match) takes about 3-5us processing time (incl SM state select)
 * Selecting the right state of SM Bus
 */
__attribute__((optimize("Os"))) void Bus::timerInterruptHandler()
{
    bool timeout;
    int time;
    unsigned int dt, tv, cv;
    auto isCaptureEvent = timer.flag(captureChannel);

    // debug processing takes about 7-8us
    tbint(state+8000, ttimer.value(), isCaptureEvent, timer.capture(captureChannel), timer.value(), timer.match(timeChannel), tb_in);

    // If we captured a falling edge (bit), read the pin repeatedly over a duration of at least 3us to ensure it's
    // not just a spike. Except it's us who are pulling down the bus, then this would be a waste of time.
    if (isCaptureEvent)
    {
        auto captureValue = timer.capture(captureChannel);
        auto matchValue = timer.match(timeChannel);

        // If captureValue is >= timer.match(pwmChannel), then it's us pulling down the bus.
        if (captureValue < timer.match(pwmChannel))
        {
            while (true)
            {
                // If the pin went HIGH in the meantime, it was just a spike. In this case, just reset the pending bit of
                // the capture channel (this keeps a potentially set pending bit of the time channel alive) and that's it.
                if (digitalRead(rxPin))
                {
                    timer.resetFlag(captureChannel);
                    return;
                }

                // Break out of the loop after at least 3 microseconds elapsed. The falling edge can occur at a
                // high value of the prescale counter, e.g. when a fractional representation of the timer value
                // would be 1.9us. Then captureValue is 1 (it's an integer) and we must ensure to wait until
                // timerValue is 5 such that the real wait time is >=3us -- if we would only wait till 4, the
                // real wait time would only be >=2us.
                auto timerValue = timer.value();
                auto elapsedMicroseconds = (timerValue >= captureValue) ? (timerValue - captureValue) : (matchValue + 1 - captureValue + timerValue);
                if (elapsedMicroseconds > ZERO_BIT_MIN_TIME)
                {
                    break;
                }
            }
        }
    }

    STATE_SWITCH:
    switch (state)
    {
    // BCU is in start-up phase, we wait for 50 bits inactivity of the bus
    case Bus::INIT:
        tb_t( state, ttimer.value(), tb_in);
        DB_TELEGRAM(telRXWaitInitTime = ttimer.value()); // if it is less than 50 we have a failure on the bus

        if (!timer.flag(timeChannel))
        {
            // cap event: bus not in idle state before we have a timeout, restart waiting time. Time must be mostly
            // aligned to falling edge, and we waited ZERO_BIT_MIN_TIME already, plus had some processing time.
            timer.value(ZERO_BIT_MIN_TIME + 2);
            break;
        }

        // Timeout. Enhance the timer to 9 bit times more in state WAIT_50BT_FOR_NEXT_RX_OR_PENDING_TX_OR_IDLE, so
        // we can start receiving right away (even if it's a cap event at the same time), but wait some more time
        // before starting to send.
        timer.match(timeChannel, BIT_TIMES_DELAY(2) + WAIT_50BIT_FOR_IDLE - PRE_SEND_TIME);
        timer.matchMode(timeChannel, INTERRUPT | RESET);
        state = WAIT_50BT_FOR_NEXT_RX_OR_PENDING_TX_OR_IDLE;
        if (timer.flag(captureChannel))
            goto STATE_SWITCH;
        break;

    // The bus is idle for at least 50BT. Usually we come here when we finished a TX/RX on the Bus and waited 50BT for next event without receiving a start bit on the Bus
    // or at least one pending Telegram in the queue.
    // A timeout  (after 0xfffe us) should not be received (indicating no bus activity) match interrupt is disabled
    // A reception of a new telegram is triggered by the falling edge of the received start bit and we collect the bits in the receiving process
    // Sending is triggered in idle state by state switch from IDLE to WAIT_50BT_FOR_NEXT_RX_OR_PENDING_TX_OR_IDLE to send pending the telegram
    case Bus::IDLE:
        tb_d( state+100, ttimer.value(), tb_in);
        DB_TELEGRAM(telRXWaitIdleTime = ttimer.value());

        if (!isCaptureEvent) // Not a bus-in signal or Tel in the queue: do nothing
            break;

    // RX process functions
    //initialize the RX process for a new telegram reception.
    //triggered by a capture event while waiting for a new telegram or ACK or an early
    //capture while trying to send a start bit in the TX process
    case Bus::INIT_RX_FOR_RECEIVING_NEW_TEL:
        tb_d( state+100, ttimer.value(), tb_in);

        DB_TELEGRAM(
            // correct the timer start value by the process time (about 13us) we had since the capture event
            tv=timer.value(); cv= timer.capture(captureChannel);
            if ( tv > cv ) dt= tv - cv; // check for timer overflow since cap event
            else dt = (timer.match(timeChannel) + 1 - cv) + tv;

            telRXStartTime= ttimer.value()- dt;
        );

        nextByteIndex = 0;
        rx_error  = RX_OK;
        checksum = 0xff;
        sendAck = 0;
        valid = 1;

        //todo if timer was  disabled for power saving and enable in this state
        // no break here as we have received a capture event - falling edge of the start bit
        // we continue with receiving of start bit.

    // A start bit (by cap event) is expected to arrive here. If we have a timeout instead, the
    // transmission of a frame is over.  (after 11 bit plus 2 fill bits :13*104us  + margin (1452us) after start of last char)
    // we expect that the timer was restarted by the end of the last frame (end of stop bit) and not restarted by a cap event
    // timer will be pre-set with for the capture timing (few us), mode reset, interrupt, match of frame time (11bits), capture interrupt
    case Bus:: RECV_WAIT_FOR_STARTBIT_OR_TELEND:
        // Regardless of the concrete situation (captured start bit, end of data frame, end of acknowledge frame),
        // we will enter the next state after some time and will need that time as reference, so it's safe
        // to enable RESET right away -- provided we update the match value to a big one beforehand such that
        // the timer does not wrap around inadvertently.
        // The match value will be overwritten with the correct value later in processing.
        // Doing this so early in this state simplifies debugging: If we would not set these values here,
        // we'd see the timer count up to 0xffff and trigger a PWM pulse of 1us length before it wraps around.
        time = timer.match(timeChannel);
        timer.match(timeChannel, 0xfffe);
        timer.matchMode(timeChannel, INTERRUPT | RESET);

        // No start bit: then it is a timeout of end of frame
        if (!isCaptureEvent)
        {
            if (checksum)
            {
                rx_error |= RX_CHECKSUM_ERROR;
            }
            DB_TELEGRAM(telRXEndTime = telRXTelByteEndTime);
#           ifdef PIO_FOR_TEL_END_IND
                digitalWrite(PIO_FOR_TEL_END_IND, 1); // set handleTelegram() PIO
#           endif
            handleTelegram(valid && !checksum);
            break;
        }

        //tb_h( state +100, currentByte, tb_in);
        // we captured a startbit falling edge trigger

        // we received a start bit interrupt - reset timer for next byte reception,
        // set byte time incl stop bit to 1144us and use that as ref for all succeeding timings in RX process
        // any rx-bit start should be within n*104 -7us, n*104 + 33us -> max 1177us
        // correct the timer start value by the process time (about 13us) we had since the capture event
        //todo  restart timer by capture in order to have a ref point for the frame timeout
        // and check the capture event to be in the allowed time window for the start bit

        tv=timer.value(); cv= timer.capture(captureChannel);
        if ( tv >= cv ) dt= tv - cv; // check for timer overflow since cap event
        else dt = (time + 1 - cv) + tv;
        timer.value(dt+2); // restart timer and pre-load with processing time of 2us
        timer.match(timeChannel, BYTE_TIME_INCL_STOP - 1);
        timer.captureMode(captureChannel, FALLING_EDGE | INTERRUPT); // next state interrupt at first low bit  - falling edge, no reset
        //timer.counterMode(DISABLE,  DISABLE); // disabled the timer reset by the falling edge of cap event
        state = Bus::RECV_BITS_OF_BYTE;
        currentByte = 0;
        bitTime = 0;
        bitMask = 1;
        parity = 1;

        DB_TELEGRAM(telRXTelByteStartTime = ttimer.value() - dt); // correct the timer start value by the process time (about 13us) we had since the capture event
        break;

    // we received next capture event for a low bit at position n*104us or timeout if we have end of byte received
    // bitMask hold the position of the expected low bit, bitTime the start-time of the expected expected low bit
    // if we received an edge interrupt later than the expected bitTime we had some high bits in between and need to calculate how many.
    // bitMask: bit8 : parity; bit9 : stop bit
    case Bus::RECV_BITS_OF_BYTE:
        //tb_t( RECV_BITS_OF_BYTE, ttimer.value(), tb_in);

        timeout = timer.flag(timeChannel); // timeout--> end of rx byte
        if (timeout) time = timer.match(timeChannel) + 1; // end of stop bit
        else
        {
            time = timer.capture(captureChannel); // we received an capt. event: new low bit
        }

        // find the bit position after last low bit and add high bits accordingly, window for the reception of falling edge of a bit is:
        //min: n*104us-7us, typ: n*104us, max: n*104us+33us. bitTime holds the start time of the last bit, so the new received
        //cap event should be between bitTime + BIT_TIME -7us and bitTime + BIT_TIME+33us
        //bittime hold the time of the n-th bit (0..9) time=n*104, timer is counting from startbit edge- should be 104us in advance

        if (time >= bitTime + BIT_TIME - 35 ) // check window should be at least  n*104-7us  *** we use -35us to be more tolerant
        {
            // bit is not to early- check for to late - we might have some high bits received since last low bit
            bitTime += BIT_TIME; //set bittime to next expected bit edge
            while (time >= bitTime + BIT_WAIT_TIME && bitMask <= 0x100) // high bit found or bit 9 (stop bit) found - move check to next bit position
            {
                currentByte |= bitMask; // add high bit until we found current position
                parity = !parity;
                bitTime += BIT_TIME; // next bit time
                bitMask <<= 1; // next bit is in bitmask
            }

            if (time > bitTime + BIT_OFFSET_MAX && bitMask <= 0x100)
            {
                rx_error |= RX_TIMING_ERROR_SPIKE; // bit edge receive but pulse to short late- window error
                DB_TELEGRAM(telRXTelBitTimingErrorLate = time); //report timing error for debugging
            }
            bitMask <<= 1; //next bit or stop bit
            //tb_d( RECV_BITS_OF_BYTE +400, time, tb_in);
            //tb_d( RECV_BITS_OF_BYTE +500, bitTime, tb_in);
        }
        else
        {
            // we might have received a additional edge due to bus reflection, tx-delay, edge should be within bit pulse +30us else ignore edge
            rx_error |= RX_TIMING_ERROR_SPIKE; // bit edge receive but pulse to short late- window error
            DB_TELEGRAM(telRXTelBitTimingErrorEarly = time); // report timing error for debugging
        }

        if (timeout)  // Timer timeout: end of byte
        {
            DB_TELEGRAM(telRXTelByteEndTime = ttimer.value() - timer.value()); // timer was restarted

            currentByte &= 0xff;

            // check bit0 and bit 1 of first byte for low level preamble bits
            if ( (!nextByteIndex) && (currentByte & PREAMBLE_MASK) )
                rx_error |= RX_PREAMBLE_ERROR;// preamble error, continue to read bytes - possibility to discard the telegram at higher layer

            if (nextByteIndex < bcu->maxTelegramSize())
            {
                rx_telegram[nextByteIndex++] = currentByte;
                checksum ^= currentByte;
            }
            else
            {
                rx_error |= RX_LENGTH_ERROR;
            }

            if (!parity) rx_error |= RX_PARITY_ERROR;
            valid &= parity;
            tb_h( RECV_BITS_OF_BYTE +300, currentByte, tb_in);

            //wait for the next byte's start bit or end of telegram and set timer to inter byte time + margin
            //timeout was at 11 bit times (1144us), timeout for end of telegram - no more bytes after 2bit times after
            //last stop bit, with up to 30us extra time
            //we disable reset of timer by match to have fixed ref point at end of last RX-byte,
            //timer was restarted by timeout event at end of stop bit, we just set new match value
            //next state interrupt at start bit falling edge
            state = Bus:: RECV_WAIT_FOR_STARTBIT_OR_TELEND;
            timer.match(timeChannel, MAX_INTER_CHAR_TIME - 1);
            timer.matchMode(timeChannel, INTERRUPT);
            timer.captureMode(captureChannel, FALLING_EDGE | INTERRUPT );

        }// cap event during stop bit: error, we should received byte-timeout later
        else if (time > BYTE_TIME_EXCL_STOP )
            rx_error |= RX_STOPBIT_ERROR;
        //tb_h( RECV_BITS_OF_BYTE +200, rx_error, tb_in);
        break;

    //timeout: we waited 15BT - PRE_SEND_TIME after rx process, start sending an ack
    //timer was reseted by match for ref for tx process
    //if cap event, we received an early ack - continue with rx process
    //todo disable cap event in previous state - not needed during waiting for ack start
    case Bus::RECV_WAIT_FOR_ACK_TX_START:
        tb_t( state, ttimer.value(), tb_in);

        //cap event- should not happen here;  start receiving,  maybe ack or early tx from other device,
        //fixme: should not happen here, probably timing error

        if (isCaptureEvent)
        {
            sendAck = 0;  // we stop sending an Ack to remote side and start receiving the char
            state = Bus::INIT_RX_FOR_RECEIVING_NEW_TEL;  // init RX of new telegram
            goto STATE_SWITCH;
        }
        sendTelegramLen = 0;

        DB_TELEGRAM(
            telTXAck = sendAck;
            telTXStartTime = ttimer.value() + PRE_SEND_TIME; // set start time of sending telegram
        );

        //set timer for TX process: init PWM pulse generation, interrupt at pulse end and cap event (pulse start)
        timer.match(pwmChannel, PRE_SEND_TIME); // waiting time till start of first bit- falling edge 104us + n*104us ( n=0 or3)
        timer.match(timeChannel, PRE_SEND_TIME  + BIT_PULSE_TIME - 1); // end of bit pulse 35us later
        timer.matchMode(timeChannel, RESET | INTERRUPT); //reset timer after bit pulse end
        timer.captureMode(captureChannel, FALLING_EDGE | INTERRUPT );
        nextByteIndex = 0;
        tx_error = TX_OK;
        state = Bus::SEND_START_BIT;

        break;

    /*
     * ************Sending states**************
     * To allow for the use of the timer PWM generator, the timing of the sending process is in phase shift of -69us with respect to
     * normal bit start: The PWM pulse starts at -69us, high pulse phase starts at 0us, pulse high end at 35us ->period is 104us. In order to be
     * in sync with the bus after a complete telegram is send we need to correct the timing again by the phase shift.
     *
     * All bus wait time before a TX could start are therefore reduced by PRE_SEND_TIME (104us). This allows to set the PWM and timer match
     * to begin the start bit of first byte  in PRE_SEND_TIME and check for any bus activity before the edge of the our start bit is received
     * if any other device did send a start bit before us (bus busy window before start bit).
     */

    /* WAIT_50BT_FOR_NEXT_RX_OR_PENDING_TX_OR_IDLE
     * is entered by match interrupt some usec (PRE_SEND_TIME or 1us coming from idle state) before sending the start bit of the first byte
     * of a pending telegram. It is always entered after receiving or sending is done and we waited the respective time for next action. If no tel
     * is pending, we enter idle state. Any new sending or receiving process will be started there. If there is a tel pending, we check for prio
     * and start sending after pre-send-time of 104us or if we have a normal Telegram after pre-send-time + 3*BitTime,
     * resulting in 50/53BT between telegrams
     *
     * State should be entered with timer reset by match to have valid ref point for tx cap event with timer still running
     */
    case Bus::WAIT_50BT_FOR_NEXT_RX_OR_PENDING_TX_OR_IDLE:
        tb_t( state, ttimer.value(), tb_in);

        if (isCaptureEvent) // cap event- start receiving,  maybe ack or early tx from other device - fixme: should not happen here!
        {
            state = Bus::INIT_RX_FOR_RECEIVING_NEW_TEL;
            goto STATE_SWITCH;
        }

        // timeout -  check if there is anything to send
        // check if we have max resend for last telegram.
        if ((repeatTelegram && (sendRetries >= sendRetriesMax || sendBusyRetries >= sendBusyRetriesMax)) ||
            collisions > COLLISION_RETRY_MAX)
        {
            tb_h( state+ 100, sendRetries + 10 * sendBusyRetries + 100 * collisions, tb_in);
            tx_error |= TX_RETRY_ERROR;
            finishSendingTelegram(); // then send next, this also informs upper layer on sending error of last telegram
        }

        if (sendCurTelegram != nullptr)  // Send a telegram pending?
        {    //tb_t( state+200, ttimer.value(), tb_in);
            tb_h( state+ 200, repeatTelegram, tb_in);
            //tb_h( state+ 300,sendCurTelegram[0], tb_in);

            sendTelegramLen = telegramSize(sendCurTelegram) + 1;
            //tb_h( state+ 1000,sendTelegramLen, tb_in);

            if (repeatTelegram && (sendCurTelegram[0] & SB_TEL_REPEAT_FLAG) )
            {// If it is the first repeat, then mark the telegram as being repeated and correct the checksum
                tb_d( state+ 700, sendRetries, tb_in);
                tb_d( state+ 800, sendBusyRetries, tb_in);
                sendCurTelegram[0] &= ~SB_TEL_REPEAT_FLAG;
                sendCurTelegram[sendTelegramLen - 1] ^= SB_TEL_REPEAT_FLAG;
            }
            // if we have repetition of telegram or system or alarm prio, we wait only 50bit time
            if (((sendCurTelegram[0] & SB_TEL_REPEAT_FLAG)) && ((sendCurTelegram[0] & PRIO_FLAG_HIGH)) ) {
                time = PRE_SEND_TIME + BIT_TIMES_DELAY(3);
            }
            else
                time = PRE_SEND_TIME;
            // KNX spec 2.1 chapter 3/2/2 section 2.3.4 p. 37: Guarantee of access fairness
            // Add some random delay of up to 3 bit times if it is not the last try.
            auto canRepeat = sendRetriesMax > 0 && sendBusyRetriesMax > 0;
            auto isLastRepeatChance = repeatTelegram && (sendRetries + 1 >= sendRetriesMax || sendBusyRetries + 1 >= sendBusyRetriesMax);
            auto isLastCollisionChance = collisions == COLLISION_RETRY_MAX;
            if (canRepeat && !isLastRepeatChance && !isLastCollisionChance)
            {
                time += (millis() * RANDOMIZE_FACTOR) % RANDOMIZE_MODULUS;
            }
            //tb_d( state+ 300, time, tb_in);
            //tb_h( state+ 400,sendCurTelegram[0], tb_in);
        }
        else  // Send nothing: transition to idle state
        {
            DB_BUS(
               if (sendCurTelegram != nullptr)
               {
                   tb_h( state+ 900,sendCurTelegram[0], tb_in);
               }
               //tb_t( state*100+4, ttimer.value(), tb_in);
            );

            idleState();
            break;
        }

        tb_t( state+500, ttimer.value(), tb_in);
        tb_d( state+600, time, tb_in);
        // set timer for TX process: init PWM pulse generation, interrupt at pulse end and cap event (pulse start)
        timer.match(pwmChannel, time); // waiting time till start of first bit- falling edge 104us + n*104us ( n=0 or3)
        timer.match(timeChannel, time + (BIT_PULSE_TIME - 1)); // end of bit pulse 35us later
        timer.matchMode(timeChannel, RESET | INTERRUPT); //reset timer after bit pulse end
        nextByteIndex = 0;
        tx_error = TX_OK;
        state = Bus::SEND_START_BIT;

        DB_TELEGRAM(telTXStartTime = ttimer.value() + time); // set start time of sending telegram
        break;

    /* SEND_START_BIT
     * start bit edge is in sync with bus timing!
     * The start bit of the first byte is being sent. We should come here when the edge of the start bit is captured by bus-in of the pwmChannel.
     * We started the timer PRE_SEND_TIME before the start bit is send by the PWM, we might come here when somebody else started
     * sending before us, or if a timeout occurred. In case of a timeout, we have a hardware problem as receiving our sent signal does not work.
     * For start of normal frames we check for bus free in a window  < -7us of before the start bit is send.  If bus is busy (capture received
     * in window), we stop sending and start receiving instead. For sending of ACK, bus free detection  is optional (windows < - 16us before
     * start bit) as other devices probably responding with ack as well (defined in Vol8.2.2).
     */
    case Bus::SEND_START_BIT:
        //tb_d( SEND_START_BIT+100, timer.match (pwmChannel), tb_in);
        //tb_h(SEND_START_BIT+200, timer.captureMode(captureChannel),  tb_in);

        // We will receive our own start bit here too.
        if (!timer.flag(timeChannel))
        {
            auto captureTime = timer.capture(captureChannel);
            auto pwmTime = timer.match(pwmChannel);

            // If it's too early, we can either ignore it or switch to RX. Obviously, ignoring
            // actually means to lose a telegram (which would need to be repeated by the sender
            // if addressed to us), so always switching to RX makes more sense in practice.
            if (captureTime < (pwmTime - STARTBIT_OFFSET_MIN))
            {
                // KNX spec 2.1 chapter 3/2/2 section 1.4.1 p. 24: Handling of p_class
                //
                //     * ack_char (sendAck != 0): No bus free detection, but collision avoidance,
                //       i.e. do not defer sending, but step back. ack_char will not be repeated.
                //
                //     * inner_Frame_char (nextByteIndex != 0): Same, but it this case it counts
                //       as a collision, and the frame will be repeated.
                //
                //     * start_of_Frame (else): Do bus free detection. This means another device
                //       started sending before us. Defer sending, no special handling necessary.

                if (nextByteIndex)
                {
                    // Note: We could interpret the incoming bytes as continuation of the
                    // telegram we started to send, or as an independent telegram where the other
                    // device started to send early. There is no right or wrong here, so just use
                    // one approach.
                    encounteredCollision();
                }

                // Stop transmission and let the other device continue (switch to RX).
                tb_d( state+300, ttimer.value(), tb_in);
                timer.match(pwmChannel, 0xffff);
                state = Bus::INIT_RX_FOR_RECEIVING_NEW_TEL;
                goto STATE_SWITCH;
            }

            // If it's at most 30us earlier than the falling edge we were about to send, sync to it.
            if (captureTime < pwmTime)
            {
                tb_d( state+300, ttimer.value(), tb_in);
                timer.match(pwmChannel, timer.value() + 1);
                timer.match(timeChannel, captureTime + (BIT_PULSE_TIME - 1));
            }

            tb_t( state+400, ttimer.value(), tb_in);
            state = Bus::SEND_BIT_0; // start bit edge in time, prepare to send bit 0 when timer times out (rising edge)
#       ifdef PIO_FOR_TEL_END_IND
            if (sendAck)
                digitalWrite(PIO_FOR_TEL_END_IND, 0);
#       endif
            break;
        }
        else
        {
            // Timeout: we have a hardware problem as receiving our sent signal does not work. set error and just continue sending bit0
            tb_t( state+400, ttimer.value(), tb_in);
            state = Bus::SEND_BIT_0; //   prepare to send bit 0 immediately
            tx_error |= TX_PWM_STARTBIT_ERROR;
        }// no break, continue with bit0 as we have a timeout here


    /* SEND_BIT_0
     *  state is in phase shift with respect to bus timing, entered by match/period interrupt from pwm
     *  start bit low pulse end now after 35us by time match interrupt, we are in the middle of the start bit at rising edge of start
     *  bit pulse. Prepare for sending bits of frame.
     *
     */
    case Bus::SEND_BIT_0:
        //tb_d( state+100, timer.match (pwmChannel), tb_in);
        // get byte to send
        if (sendAck)
        {
            currentByte = sendAck;
        }
        else
        {
            currentByte = sendCurTelegram[nextByteIndex++];
        }

        // Calculate the parity bit
        for (bitMask = 1; bitMask < 0x100; bitMask <<= 1)
        {
            if (currentByte & bitMask) // current bit high
                currentByte ^= 0x100;  // toggle/xor parity bit
        }
        bitMask = 1;
        state = Bus::SEND_BITS_OF_BYTE; //set next state, no break here, continue sending first bit/ LSB
        tb_h( SEND_BIT_0 +200, currentByte, tb_in);

    /* SEND_BITS_OF_BYTE
     * state is in phase shift, entered by cap event or match/period interrupt from pwm
     * n-bit low pulse end now after 35us by time match interrupt, send next bit of byte till end of byte (stop bit)
     */
    case Bus::SEND_BITS_OF_BYTE:
    {
        tb_t( state, ttimer.value(), tb_in);
        //tb_h( state+100, bitMask, tb_in);
        //tb_d( state+200, time, tb_in);
        //tb_h( state+100, sendAck, tb_in);
        //tb_d( state+100, timer.match(pwmChannel), tb_in);
        //tb_h( state+200, timer.captureMode(captureChannel), tb_in);

        if (!timer.flag(timeChannel))
        {
            /* Capture event from bus-in. This should be from us sending a zero bit, but it might as well be from somebody else in case of a
             * collision. Our low bit starts at pwmChannel time and ends at match of timeChannel.
             * Check for collision during sending of high bits. As our timing is related to the rising edge of a bit we need to measure accordingly:
             * next bit start window is in 69us, and the n-bit low pulse starts at n*104 - 35us and ends at n*104 -> check for edge window: the high phase
             * of the last bit : 69us - margin till 69us before next falling edge at pwmChannel time + margin
             */
            auto captureTime = timer.capture(captureChannel);

            if (captureTime < REFLECTION_IGNORE_DELAY)
            {
                // We finished pulling the bus low, i.e. sent a rising edge, which caused a reflection. Ignore it
                // and continue sending.
                break;
            }

            if ((captureTime % BIT_TIME) < (BIT_WAIT_TIME - BIT_OFFSET_MIN))
            {
                // Falling edge captured between a rising edge (reference time 0) and when a falling edge would be ok
                // (up to 7us early and 33us late per KNX spec 2.1 chapter 3/2/2 section 1.2.2.8 figure 22 p.19).
                // Collision avoidance says we must stop sending. Timing of the falling edge is so far off that
                // it does not make sense to try reception, therefore re-sync to bus via INIT.
                encounteredCollision();
                initState();
                break;
            }

            // It's an edge at a time when edges are allowed to happen, i.e. it is from another device that sends
            // concurrently. If it is before (timer.match(pwmChannel) - BIT_OFFSET_MIN), that means we
            // receive a 0-bit while we're sending a 1-bit, i.e. it is a collision and we need to switch to RX.
            // Otherwise, it's either our own 0-bit or a foreign 0-bit that just starts a bit earlier than ours,
            // and we need to continue sending and let collision detection sort it out in a later bit.
            if (( captureTime < timer.match(pwmChannel) - BIT_OFFSET_MIN ))
            {
                tb_d( state+400, captureTime, tb_in);
                tb_t( state+300, ttimer.value(), tb_in);

                // A collision. Stop sending and switch to receiving the current transmission.
                encounteredCollision();
                rx_error = RX_OK;
                checksum = 0xff;
                valid = 1;
                parity = 1;

                if (sendAck)
                {
                    // LL acknowledgment frames are not repeated (ACK, NACK, BUSY).
                    sendAck = 0;
                }
                else
                {
                    // KNX spec 2.1 chapter 3/2/2 section 2.3.1 p. 35: "After detection that the bus
                    // is not free and after a collision, the device shall wait until the end of the
                    // message cycle in progress and shall make another attempt to transmit the data
                    // link request PDU after 50 bit times or more line idle time."
                    //
                    // KNX spec 2.1 chapter 3/2/2 section 2.4.1 p. 39: "repetition: this shall specify
                    // whether the local Data Link Layer shall repeat the Frame on the medium in case
                    // of transmission errors (NAK, BUSY or acknowledge time-out)"
                    //
                    // Both citations yield the same conclusion: On collision, the frame is transmitted
                    // again, but the "repeat flag" in the Control field and therefore also the timing
                    // does not change. This means we must not set repeatTelegram here.

                    // For TX, nextByteIndex is incremented before we start transmitting. For RX, nextByteIndex is incremented after
                    // we received a byte completely. To account for this difference, we need to decrement when we switch from TX to RX.
                    nextByteIndex--;

                    // Copy all bytes we transmitted without collision over to the receive buffer and update checksum accordingly.
                    for (auto i = 0; i < nextByteIndex; i++)
                    {
                        auto b = sendCurTelegram[i];
                        rx_telegram[i] = b;
                        checksum ^= b;
                    }
                }

                // Scale back bitMask to match the collided bit. pwmChannel is when we would have sent
                // the next falling edge, captureTime when we received it. Unfortunately, pwmChannel can
                // be 0xffff in case there are no 0 bits left to send. Thus, use (timeChannel - BIT_PULSE_TIME)
                // instead. BIT_OFFSET_MAX is to account for slight timing differences and integer arithmetic.
                auto collisionBitCount = (timer.match(timeChannel) - captureTime + (BIT_OFFSET_MAX - BIT_PULSE_TIME)) / BIT_TIME;
                bitMask >>= collisionBitCount + 1;

                // Pretend that we also received a 0 bit last time, such that there is no need to set any
                // bits to 1 in RECV_BITS_OF_BYTE.
                bitTime = captureTime - BIT_TIME;

                // Only keep those bits of currentByte that we sent without collision, and clear the rest.
                currentByte &= (bitMask - 1);

                // Adjust timer and parity accordingly. In a non-collided byte, timer starts at 0 and runs
                // to BYTE_TIME_INCL_STOP with bitTime and captureChannel relative to the start bit's
                // falling edge. As we cannot set captureChannel, we configure reception of collided bytes
                // relative to the rising edge of the last non-collided bit.
                auto missingBits = 10;
                for (auto i = bitMask >> 1; i; i >>= 1)
                {
                    missingBits--;
                    if (currentByte & i)
                    {
                        parity = !parity;
                    }
                }

                timer.match(timeChannel, captureTime + missingBits * BIT_TIME - 1);
                timer.matchMode(timeChannel, INTERRUPT | RESET);

                timer.match(pwmChannel, 0xffff); // set PWM bit to low next interrupt is on timeChannel match (value :time)

                state = Bus::RECV_BITS_OF_BYTE;
                goto STATE_SWITCH;
            }

            //tb_t( state+200, ttimer.value(), tb_in);
            //tb_d( state+500,timer.match(pwmChannel), tb_in);
            // we captured our sending low bit edge, continue sending, wait for bit end with match intr
            break;
        }

        // Timeout event. Either we reached the end of the byte or the end of a 0 bit.
        if (bitMask <= 0x200)
        {
            // Stop bit not reached yet, continue sending.
            // Search for the next zero bit and count the one bits for the wait time only till we reach the parity bit
            time = BIT_TIME ;
            while ((currentByte & bitMask) && bitMask <= 0x100)
            {
                bitMask <<= 1;
                time += BIT_TIME;
            }
            bitMask <<= 1; // next low bit or stop bit if mask > 0x200

            auto stopBitReached = (bitMask > 0x200);

            // set next match/interrupt
            // if we are sending high bits, we wait for next low bit edge by cap interrupt which might be a collision
            // or timeout interrupt indicating end of bit pulse sending (high or low)
            //tb_h( SEND_BITS_OF_BYTE+300, bitMask, tb_in);
            //tb_d( SEND_BITS_OF_BYTE+400, time, tb_in);

            if (stopBitReached)
                timer.match(pwmChannel, 0xffff); //stop pwm pulses - low output
            // as we are at the raising edge of the last pulse, the next falling edge will be n*104 - 35us (min69us) away
            else
                timer.match(pwmChannel, time - BIT_PULSE_TIME); // start of pulse for next low bit - falling edge on bus will not trigger cap interrupt
            //tb_d( SEND_BITS_OF_BYTE+500, time, tb_in);
            //tb_h( SEND_BITS_OF_BYTE+600, timer.captureMode(captureChannel),  tb_in);

            timer.match(timeChannel, time - 1); // interrupt at end of low/high bit pulse - next raising edge or after stop bit + 2 wait bits
            break;
        }

        // Stop bit reached.
        state = Bus::SEND_END_OF_BYTE;
        // Intentionally fall through to SEND_END_OF_BYTE.
    }

    // Completed transmission of parity bit and are in the middle of the stop bit transmission.
    // What do we need to do next?
    case Bus::SEND_END_OF_BYTE:
        tb_t( state, ttimer.value(), tb_in);
        if (nextByteIndex < sendTelegramLen && !sendAck)
        {
            // There are more bytes to send. Finish stop bit, send two fill bits, and start bit pulse of next byte.
            time = BIT_TIMES_DELAY(3);
            state = Bus::SEND_START_BIT;  // state for bit-0 of next byte to send
            timer.match(pwmChannel, time - BIT_PULSE_TIME); // start of pulse for next low bit - falling edge on bus will trigger cap interrupt
        }
        else
        {
            // We're done. Finish to send stop bit and sync with bus timing.
            state = Bus::SEND_END_OF_TX;
            time = BIT_TIME - BIT_PULSE_TIME;
            timer.captureMode(captureChannel, FALLING_EDGE);
        }
        timer.match(timeChannel, time - 1);
        break;

    //state is in sync with resp. to bus timing,  entered by match interrupt after last bytes stop bit was send
    //for normal frames we should wait for ack from remote layer2 after ack-waiting time or if we sent an ACK we wait 50bittimnes for idle
    //timer was reset by match
    case Bus::SEND_END_OF_TX:
        tb_t( state, ttimer.value(), tb_in);
        tb_h( SEND_END_OF_TX+700, repeatTelegram, tb_in);
        DB_TELEGRAM(telTXEndTime = ttimer.value());

        if (sendAck){ // we send an ack for last received frame, wait for idle for next action
            tb_h( SEND_END_OF_TX+200, tx_error, tb_in);
            DB_TELEGRAM(
                txtelBuffer[0] = sendAck;
                txtelLength = 1;
                tx_rep_count = sendRetries;
                tx_busy_rep_count = sendBusyRetries;
                tx_telrxerror = tx_error;
            );

            sendAck = 0;
            state = Bus::WAIT_50BT_FOR_NEXT_RX_OR_PENDING_TX_OR_IDLE;
            time =  SEND_WAIT_TIME - PRE_SEND_TIME;// we wait 50 BT- pre-send-time for next rx/tx window
            timer.captureMode(captureChannel, FALLING_EDGE | INTERRUPT);
            // todo inform receiving process of pos ack tx
        }
        else
        {
            tb_h( SEND_END_OF_TX+600, repeatTelegram, tb_in);

            // normal data frame,  L2 need to wait for ACK from remote for our telegram
            wait_for_ack_from_remote = true; // default for data layer: acknowledge each telegram
            time = ACK_WAIT_TIME_MIN; //we wait 15BT-margin for ack rx window, cap intr disabled
            state = Bus::SEND_WAIT_FOR_RX_ACK_WINDOW;
            timer.matchMode(timeChannel, INTERRUPT); // no timer reset after timeout
            if (repeatTelegram) // if last telegram was repeated, increase respective counter
            {
                if (busy_wait_from_remote)
                    sendBusyRetries++;
                else
                    sendRetries++;
            }
            // dump previous tx-telegram and repeat counter and busy retry
            DB_TELEGRAM(
                for (int i =0; i< sendTelegramLen; i++)
                {
                    txtelBuffer[i] = sendCurTelegram[i];
                }
                txtelLength = sendTelegramLen;
                tx_rep_count = sendRetries;
                tx_busy_rep_count = sendBusyRetries;
                tx_telrxerror = tx_error;
            );
        }
        tb_d( SEND_END_OF_TX+300, wait_for_ack_from_remote , tb_in);
        tb_d( SEND_END_OF_TX+400, sendRetries, tb_in);
        tb_d(SEND_END_OF_TX+500, sendBusyRetries, tb_in);

        timer.match(timeChannel, time - 1); // we wait respective time - pre-send-time for next rx/tx window, cap intr disabled
        break;

    //ACK receive windows starts now after the timeout event
    //enable cap event and wait till end of ACK receive window for the ACK
    //timer is counting since end of last stop bit
    case Bus::SEND_WAIT_FOR_RX_ACK_WINDOW:
        tb_t( state, ttimer.value(), tb_in);

        state = Bus::SEND_WAIT_FOR_RX_ACK;
        //timer.matchMode(timeChannel, INTERRUPT | RESET); // timer reset after timeout to have ref point in next RX/TX state
        //timer.counterMode(DISABLE,  captureChannel  | FALLING_EDGE); // enabled the  timer reset by the falling edge of cap event
        timer.captureMode(captureChannel, FALLING_EDGE | INTERRUPT );
        timer.match(timeChannel, ACK_WAIT_TIME_MAX - 1); // we wait 15BT+ marging for ack rx window, cap intr enabled
        break;

    // we wait here for the cap event of the ACK. If we receive a timeout- no ack was received and we need
    // to start a repetition of the last telegram
    case Bus::SEND_WAIT_FOR_RX_ACK:
        tb_t( state, ttimer.value(), tb_in);

        if (isCaptureEvent){
            state = Bus::INIT_RX_FOR_RECEIVING_NEW_TEL;  // start bit of ack received - continue rx process for rest of byte
            // todo rx-ack process ongoing inform to RX process for optimization??
            goto STATE_SWITCH;
        }
        repeatTelegram = true;
        wait_for_ack_from_remote = false;
        tx_error|= TX_ACK_TIMEOUT_ERROR; // todo  ack timeout - inform upper layer on error state and repeat tx if needed
        state = Bus::WAIT_50BT_FOR_NEXT_RX_OR_PENDING_TX_OR_IDLE;

        //timer is counting since last stop bit so we need to wait 50BT till idle for repeated frames (see KNX spec v2.1 3/2/2 2.3.1 Figure 38)
        timer.match(timeChannel, SEND_WAIT_TIME - PRE_SEND_TIME - 1);
        timer.matchMode(timeChannel, INTERRUPT | RESET); // timer reset after timeout to have ref point in next RX/TX state
        break;

    // we should never land here, except someone has forgotten to implement a state
    default:
        tb_d( 9999, ttimer.value(), tb_in);
        fatalError();
        break;
    }

    timer.resetFlags();
}

void Bus::loop()
{
    ///\todo implement enabled property
    /*
    if (!enabled)
    {
        return;
    }
    */
    DB_TELEGRAM(dumpTelegrams());
#if defined (DEBUG_BUS) || defined (DEBUG_BUS_BITLEVEL)
    debugBus();
#endif
}
