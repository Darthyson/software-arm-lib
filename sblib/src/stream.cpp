/*
 *  stream.cpp - Base class for character-based streams.
 *
 *  Copyright (c) 2014 Stefan Taferner <stefan.taferner@gmx.at>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 3 as
 *  published by the Free Software Foundation.
 */

#include <sblib/stream.h>
#include <sblib/timer.h>


int32_t Stream::parseInt(const char skipChar)
{
    bool negative = false;
    int32_t value = 0;

    int16_t ch = peekNextDigit(); // skip leading non-numeric characters
    if (static_cast<uint8_t>(ch) == '-')
    {
        negative = true;
        ch = '0';
    }

    while (ch >= 0)
    {
        if (ch >= '0' && ch <= '9')
        {
            value = value * 10 + ch - '0';
        }
        else if (ch != skipChar)
        {
            break;
        }

        read(); // consume the character we got with peek
        ch = timedPeek();
    }

    if (negative)
    {
        return -value;
    }
    return value;
}

uint32_t Stream::_readBytesUntil(const int32_t terminator, char* buffer, const uint32_t length)
{
    uint32_t count;
    for (count = 0; count < length; ++count)
    {
        const int16_t ch = timedRead();
        if (ch < 0 || ch == terminator)
        {
            break;
        }

        buffer[count] = static_cast<char>(ch);
    }
    return count;
}

bool Stream::findUntil(const char* target, const uint32_t targetLen, const char* terminator, const uint32_t termLen)
{
    uint32_t targetIdx = 0;
    uint32_t termIdx = 0;
    int16_t ch;

    while ((ch = timedRead()) >= 0)
    {
        if (ch == target[targetIdx])
        {
            if (++targetIdx >= targetLen)
                return true;
        }
        else
        {
            targetIdx = 0;
        }

        if (termLen > 0)
        {
            if (ch == terminator[termIdx])
            {
                if (++termIdx >= termLen)
                    return false;
            }
            else
            {
                termIdx = 0;
            }
        }
    }
    return false;
}

int16_t Stream::timedRead()
{
    const uint32_t start = millis();
    int16_t ch = read();

    while (ch < 0 && elapsed(start) < timeout)
    {
        ch = read();
    }
    return ch;
}

int16_t Stream::timedPeek()
{
    const uint32_t start = millis();
    int16_t ch = peek();

    while (ch < 0 && elapsed(start) < timeout)
    {
        ch = peek();
    }
    return ch;
}

int16_t Stream::peekNextDigit()
{
    int16_t ch;
    while (true)
    {
        ch = timedPeek();
        if (ch < 0)
        {
            break; // timeout
        }

        if (static_cast<uint8_t>(ch) == '-')
        {
            break;
        }

        if (ch >= '0' && static_cast<uint8_t>(ch) <= '9')
        {
            break;
        }

        read(); // discard non-numeric characters
    }
    return ch;
}
