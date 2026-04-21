/*
 This program is free software; you can redistribute it and/or modify
 it under the terms of the GNU General Public License version 3 as
 published by the Free Software Foundation.
 ---------------------------------------------------------------------------*/

#include "sblib/eib/knx_lpdu.h"
#include "sblib/bits.h"
#include "sblib/utils.h"


constexpr uint8_t LPDU_CONTROL_BYTE = 0;
constexpr uint8_t LPDU_SENDER_HIGH_BYTE = 1;
constexpr uint8_t LPDU_SENDER_LOW_BYTE = 2;
constexpr uint8_t LPDU_DESTINATION_HIGH_BYTE = 3;
constexpr uint8_t LPDU_DESTINATION_LOW_BYTE = 4;

constexpr uint8_t MASK_REPEATED = 1 << 5;
constexpr uint8_t MASK_FRAMETYPE = 1 << 7;

void initLpdu(uint8_t* telegram, const KNXPriority newPriority, const bool newRepeated, const KNXFrameType newFrameType)
{
    telegram[LPDU_CONTROL_BYTE] = 0xB0;
    setRepeated(telegram, newRepeated);
    setPriority(telegram, newPriority);
    setFrameType(telegram, newFrameType);
}

uint8_t controlByte(const uint8_t* telegram)
{
    return telegram[LPDU_CONTROL_BYTE];
}

bool isRepeated(const uint8_t* telegram)
{
    // 5.bit not set => repeated
    return (controlByte(telegram) & MASK_REPEATED) == 0;
}

void setRepeated(uint8_t* telegram, const bool repeated)
{
    if (repeated)
    {
        // 5.bit not set => repeated
        telegram[LPDU_CONTROL_BYTE] &= static_cast<uint8_t>(~MASK_REPEATED);
    }
    else
    {
        telegram[LPDU_CONTROL_BYTE] |= MASK_REPEATED;
    }
}

KNXPriority priority(const uint8_t* telegram)
{
    // 3. and 4. bit control priority
    return static_cast<KNXPriority>(controlByte(telegram) >> 2 & 0x03);
}

void setPriority(uint8_t* telegram, const KNXPriority newPriority)
{
 // ReSharper disable once CppRedundantParentheses
    telegram[LPDU_CONTROL_BYTE] = static_cast<uint8_t>(newPriority << 2) | (telegram[LPDU_CONTROL_BYTE] & 0xF3);
}

uint16_t senderAddress(const uint8_t* telegram)
{
    return static_cast<uint16_t>(telegram[LPDU_SENDER_HIGH_BYTE] << 8 | telegram[LPDU_SENDER_LOW_BYTE]);
}

void setSenderAddress(uint8_t* telegram, const uint16_t newSenderAddress)
{
    telegram[LPDU_SENDER_HIGH_BYTE] = HIGH_BYTE(newSenderAddress);
    telegram[LPDU_SENDER_LOW_BYTE] = lowByte(newSenderAddress);
}

uint16_t destinationAddress(const uint8_t* telegram)
{
    return makeWord(telegram[LPDU_DESTINATION_HIGH_BYTE], telegram[LPDU_DESTINATION_LOW_BYTE]);
}

void setDestinationAddress(uint8_t* telegram, const uint16_t newDestinationAddress)
{
    telegram[LPDU_DESTINATION_HIGH_BYTE] = HIGH_BYTE(newDestinationAddress);
    telegram[LPDU_DESTINATION_LOW_BYTE] = lowByte(newDestinationAddress);
}

KNXFrameType frameType(const uint8_t* telegram)
{
    if (controlByte(telegram) & MASK_FRAMETYPE)
    {
        return FRAME_STANDARD;
    }

    return FRAME_EXTENDED;
}

void setFrameType(uint8_t* telegram, const KNXFrameType newFrameType)
{
    if (newFrameType == FRAME_EXTENDED)
    {
        // 7.bit not set => extended
        telegram[LPDU_CONTROL_BYTE] &= static_cast<uint8_t>(~MASK_FRAMETYPE);
    }
    else
    {
        telegram[LPDU_CONTROL_BYTE] |= MASK_FRAMETYPE;
    }
}

uint8_t physAddressToArea(const uint16_t address)
{
    return address >> 12 & 0x0f;
}

uint8_t physAddressToLine(const uint16_t address)
{
    return address >> 8 & 0x0f;
}

uint8_t physAddressToDevice(const uint16_t address)
{
    return address & 0xff;
}

uint8_t mainGroupAddress(const uint16_t address)
{
    return highByte(address) >> 3;
}

uint8_t middleGroupAddress(const uint16_t address)
{
    return highByte(address) & 0x07;
}

uint8_t lowGroupAddress(const uint16_t address)
{
    return lowByte(address);
}
