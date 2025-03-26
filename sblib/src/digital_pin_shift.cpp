/*
 *  digital_pin_shift.cpp - Functions for bit shifting digital I/O
 *
 *  Copyright (c) 2014 Stefan Taferner <stefan.taferner@gmx.at>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 3 as
 *  published by the Free Software Foundation.
 */
#include <sblib/digital_pin.h>

constexpr static uint8_t BitsPerByte = 8;

uint8_t shiftIn(const uint32_t dataPin, const uint32_t clockPin, const BitOrder bitOrder)
{
    uint8_t value = 0;
    for (uint8_t i = 0; i < BitsPerByte; i++)
    {
        digitalWrite(clockPin, true);

        if (bitOrder == LSBFIRST)
            value |= static_cast<uint8_t>(digitalRead(dataPin)) << i;
        else
            value |= static_cast<uint8_t>(digitalRead(dataPin)) << (BitsPerByte - i - 1);

        digitalWrite(clockPin, false);
    }
    return value;
}

void shiftOut(const uint32_t dataPin, const uint32_t clockPin, const BitOrder bitOrder, const uint8_t val)
{
    for (uint8_t i = 0; i < BitsPerByte; i++)
    {
        if (bitOrder == LSBFIRST)
            digitalWrite(dataPin, val & (1 << i));
        else
            digitalWrite(dataPin, val & (1 << (BitsPerByte - i - 1)));
        __NOP();
        digitalWrite(clockPin, true);
        __NOP();
        digitalWrite(clockPin, false);
    }
}
