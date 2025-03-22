/*
 *  bcu_base.cpp
 *
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 3 as
 *  published by the Free Software Foundation.
 */

#include <sblib/io_pin_names.h>
#include <sblib/eib/knx_lpdu.h>
#include <sblib/eib/bcu_base.h>
#include <sblib/eib/bus.h>
#include <sblib/eib/bcu_const.h>

static Bus* timerBusObj;
// The interrupt handler for the EIB bus access object
BUS_TIMER_INTERRUPT_HANDLER(TIMER16_1_IRQHandler, (*timerBusObj))

#if defined(INCLUDE_SERIAL)
#   include <sblib/serial.h>
#endif

BcuBase::BcuBase(UserRam* userRam, AddrTables* addrTables) :
        TLayer4(TelegramBufferSize),
        bus(new Bus(this, timer16_1, PIN_EIB_RX, PIN_EIB_TX, CAP0, MAT0)),
        progPin(PIN_PROG),
        userRam(userRam),
        addrTables(addrTables),
        comObjects(nullptr),
        progButtonDebouncer(),
        restartType(RestartType::None),
        restartSendDisconnect(false),
        restartTimeout(Timeout())
{
    timerBusObj = bus;
    setFatalErrorPin(progPin);
}

void BcuBase::_begin()
{
    TLayer4::_begin();
    bus->begin(ownAddress());
    progButtonDebouncer.init(1);
}

void BcuBase::loop()
{
    bus->loop();
    TLayer4::loop();

    // We want to process a received telegram only if there is nothing to send because:
    //
    //     1) Processing the telegram can cause a response telegram, e.g. a T_ACK in
    //        connection-oriented Transport Layer messages, and we need to have an empty
    //        buffer to be able to store and send such responses.
    //
    //     2) When debugging, it's crucial to only stop in safe states, i.e. only when
    //        there is nothing to send, not even an acknowledge frame. Otherwise, the
    //        Bus timer is configured to pull the bus low (send a 0 bit) for some time
    //        and the MCU continues timer operation, even when a breakpoint is active.
    //
    if (bus->telegramReceived() && !bus->sendingFrame() && (userRam->status() & BCU_STATUS_TRANSPORT_LAYER))
    {
        processTelegram(bus->telegram, (uint8_t)bus->telegramLen); // if processed successfully, received telegram will be discarded by processTelegram()
    }

    if (progPin)
    {
        // Detect the falling edge of pressing the prog button
        pinMode(progPin, INPUT|PULL_UP);
        int oldValue = progButtonDebouncer.value();
        if (!progButtonDebouncer.debounce(digitalRead(progPin), 50) && oldValue)
        {
            userRam->status() ^= BCU_STATUS_PARITY | BCU_STATUS_PROGRAMMING_MODE;  // toggle programming mode and parity bit
        }
        pinMode(progPin, OUTPUT);
        digitalWrite(progPin, !programmingMode());
    }

    // Rest of this function is only relevant if currently able to send another telegram.
    if (bus->sendingFrame())
    {
        return;
    }

    if (restartType != RestartType::None)
    {
        // Tests require inspection of the sent telegram before calling softSystemReset().
        // So instead of calling the method after disconnect() in the same loop iteration,
        // let's defer that to the next iteration by moving it to an otherwise unneeded
        // else block.
        // KNX spec v2.1 chapter 3/5/2 sections 3.7.1.1 p.63 and 3.7.3 p. 72 say the
        // Management Server should send a T_DISCONNECT and the Management Client must send
        // one as well. So send one out immediately, and stay around for a bit to receive
        // one from the client and ACK it.
        // The T_DISCONNECT messages might also be sent in different order, i.e. we might
        // receive one from the Management Client before we even get the change to send
        // one. In such a case we need to send it anyway. Therefore, don't check the current
        // connection status, but the one when we processed the restart request.
        // Although the spec clearly says that clients should ignore T_DISCONNECT messages
        // as well as errors, calimero warns about "negative confirmation" frames if we
        // don't ACK it. So be nice and try to avoid these warnings.
        if (restartSendDisconnect)
        {
            disconnect();
            restartSendDisconnect = false;
        }
        else if (restartTimeout.expired())
        {
            softSystemReset();
        }
    }
}

bool BcuBase::setProgrammingMode(bool newMode)
{
    if (!progPin)
    {
        return false;
    }

    if (newMode != programmingMode())
    {
        userRam->status() ^= BCU_STATUS_PARITY | BCU_STATUS_PROGRAMMING_MODE;  // toggle programming mode and parity bit
    }
    pinMode(progPin, OUTPUT);
    digitalWrite(progPin, !programmingMode());
    return true;
}

bool BcuBase::processApci(ApciCommand apciCmd, unsigned char * telegram, uint8_t telLength, uint8_t * sendBuffer)
{
    switch (apciCmd)
    {
        case APCI_BASIC_RESTART_PDU:
            scheduleRestart(RestartType::Basic);
            return (false);
        default:
            return (TLayer4::processApci(apciCmd, telegram, telLength, sendBuffer));
    }
    return (false);
}

void BcuBase::sendApciIndividualAddressReadResponse()
{
    auto sendBuffer = acquireSendBuffer();
    initLpdu(sendBuffer, PRIORITY_SYSTEM, false, FRAME_STANDARD);
    // 1+2 contain the sender address, which is set by bus.sendTelegram()
    setDestinationAddress(sendBuffer, 0x0000); // Zero target address, it's a broadcast
    sendBuffer[5] = 0xe0 + 1; // address type & routing count in high nibble + response length in low nibble
    setApciCommand(sendBuffer, APCI_INDIVIDUAL_ADDRESS_RESPONSE_PDU, 0);
    sendPreparedTelegram();
}

void BcuBase::end()
{
    enabled = false;
    bus->end();
}

bool BcuBase::programmingMode() const
{
    return (userRam->status() & BCU_STATUS_PROGRAMMING_MODE) == BCU_STATUS_PROGRAMMING_MODE;
}

void BcuBase::discardReceivedTelegram()
{
    bus->discardReceivedTelegram();
}

void BcuBase::send(unsigned char* telegram, unsigned short length)
{
    bus->sendTelegram(telegram, length);
}

void BcuBase::scheduleRestart(RestartType type)
{
    restartType = type;
    restartSendDisconnect = directConnection();
    restartTimeout.start(250);
}

void BcuBase::softSystemReset()
{
    bus->end();

    // Set magicWord to start in bootloader mode after reset.
    // As this overwrites the start of the interrupt vector table, disable interrupts.
    if (restartType == RestartType::MasterIntoBootloader)
    {
        noInterrupts();
#ifndef IAP_EMULATION
        unsigned int * magicWord = BOOTLOADER_MAGIC_ADDRESS;
        *magicWord = BOOTLOADER_MAGIC_WORD;
#endif
    }

    NVIC_SystemReset();
}

void BcuBase::setProgPin(int prgPin) {
    progPin=prgPin;
    setFatalErrorPin(progPin);
}

void BcuBase::setOwnAddress(uint16_t addr)
{
    bus->setOwnAddress(addr);
    TLayer4::setOwnAddress(addr);
}
