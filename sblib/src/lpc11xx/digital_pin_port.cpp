/*
 *  digital_pin_port.cpp - Port configuration functions for digital I/O
 *
 *  Copyright (c) 2014 Stefan Taferner <stefan.taferner@gmx.at>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 3 as
 *  published by the Free Software Foundation.
 */
#include <sblib/digital_pin.h>

#include <sblib/arrays.h>
#include <sblib/platform.h>
#include <sblib/utils.h>


void portMode(const uint8_t portNum, uint32_t pinMask, const uint32_t mode)
{
    LPC_GPIO_TypeDef* port = gpioPorts[portNum];
    const uint16_t type = mode & 0xf000;
    const uint32_t iocon = mode & 0xfff;

    if (type == OUTPUT || type == OUTPUT_MATCH)
    {
        port->DIR |= pinMask;
    }
    else // INPUT
    {
        port->DIR &= ~pinMask;
    }

    for (int pinNum = 0; pinMask != 0; ++pinNum, pinMask >>= 1)
    {
        if (pinMask & 1)
            *(ioconPointer(portNum, pinNum)) = iocon;
    }
}

void portDirection(const uint8_t portNum, const uint32_t pinMask, const uint32_t dir)
{
    LPC_GPIO_TypeDef* port = gpioPorts[portNum];
    if (dir == OUTPUT)
        port->DIR |= pinMask;
    else
        port->DIR &= ~pinMask;
}
