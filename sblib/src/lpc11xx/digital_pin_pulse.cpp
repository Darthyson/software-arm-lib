/*
 *  digital_pin_shift.cpp - Functions for pulse measurement on digital I/O
 *
 *  Copyright (c) 2014 Stefan Taferner <stefan.taferner@gmx.at>
 *  Copyright (c) 2005-2006 David A. Mellis
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License as published by the Free Software Foundation; either
 *  version 3 of the License, or (at your option) any later version.
 */

#include <sblib/digital_pin.h>
#include <sblib/timer.h>


uint32_t pulseIn(const uint32_t pin, const bool state, const uint32_t timeout)
{
    const LPC_GPIO_TypeDef* port = gpioPorts[digitalPinToPort(pin)];
    const uint32_t bitMask = digitalPinToBitMask(pin);
    const uint32_t stateMask = state ? bitMask : 0;

    uint32_t width = 0;
    uint32_t numloops = 0;
    const uint32_t maxloops = microsecondsToClockCycles(timeout);

    // Wait for any previous pulse to end
    while (port->MASKED_ACCESS[bitMask] == stateMask)
        if (numloops++ == maxloops)
            return 0;

    // Wait for the pulse to start
    while (port->MASKED_ACCESS[bitMask] != stateMask)
        if (numloops++ == maxloops)
            return 0;

    // Wait for the pulse to stop
    while (port->MASKED_ACCESS[bitMask] == stateMask)
    {
        if (numloops++ == maxloops)
            return 0;
        width++;
    }

    // Convert the reading to microseconds. The loop has been determined to be
    // 20 clock cycles long and have about 16 clocks between the edge and the
    // start of the loop. There will be some error introduced by the interrupt
    // handlers.
    return clockCyclesToMicroseconds(width * 21 + 16);
}
