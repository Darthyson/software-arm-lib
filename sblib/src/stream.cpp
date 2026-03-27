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


int Stream::parseInt(const char skipChar)
{
    bool negative = false;
    int value = 0;

    int ch = peekNextDigit(); // skip leading non numeric characters
    if (ch == '-')
    {
        negative = true;
        ch = '0';
    }

    while (ch >= 0)
    {
        if (ch >= '0' && ch <= '9')
            value = value * 10 + ch - '0';
        else if (ch != skipChar)
            break;

        read(); // consume the character we got with peek
        ch = timedPeek();
    }

    if (negative)
        return -value;
    return value;
}

int Stream::_readBytesUntil(const int terminator, char* buffer, const int length)
{
    int count;
    for (count = 0; count < length; ++count)
    {
        const int ch = timedRead();
        if (ch < 0 || ch == terminator)
            break;

        buffer[count] = ch;
    }

    return count;
}

bool Stream::findUntil(const char* target, const int targetLen, const char* terminator, const int termLen)
{
    int targetIdx = 0;
    int termIdx = 0;
    int ch;

    while ((ch = timedRead()) >= 0)
    {
        if (ch == target[targetIdx])
        {
            if (++targetIdx >= targetLen)
                return true;
        }
        else
            targetIdx = 0;

        if (termLen > 0)
        {
            if (ch == terminator[termIdx])
            {
                if (++termIdx >= termLen)
                    return false;
            }
            else
                termIdx = 0;
        }
    }

    return false;
}

int Stream::timedRead()
{
    const int start = millis();
    int ch = read();

    while (ch < 0 && elapsed(start) < timeout)
    {
        ch = read();
    }

    return ch;
}

int Stream::timedPeek()
{
    const int start = millis();
    int ch = peek();

    while (ch < 0 && elapsed(start) < timeout)
    {
        ch = peek();
    }

    return ch;
}

int Stream::peekNextDigit()
{
    int ch;
    while (true)
    {
        ch = timedPeek();
        if (ch < 0)
            break; // timeout

        if (ch == '-')
            break;
        if (ch >= '0' && ch <= '9')
            break;

        read(); // discard non-numeric characters
    }

    return ch;
}
