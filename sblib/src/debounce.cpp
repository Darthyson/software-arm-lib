/*
 *  debounce.cpp - A debouncer.
 *
 *  Copyright (c) 2014 Stefan Taferner <stefan.taferner@gmx.at>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 3 as
 *  published by the Free Software Foundation.
 */

#include <sblib/debounce.h>
#include <sblib/timer.h>

int32_t Debouncer::debounce(const int32_t current, const uint32_t timeout)
{
    const uint32_t now = millis();
    if (last != current)
    {
        time = now;
        last = current;
    }
    else if (time && (static_cast<int32_t>(now - (time + timeout)) >= 0))
    {
        time = 0;
        valid = current;
    }

    return valid;
}

int32_t Debouncer::value() const
{
    return valid;
}

void Debouncer::init(const int32_t newValue)
{
    valid = newValue;
    time = 0;
}

int32_t Debouncer::lastValue() const
{
    return last;
}

