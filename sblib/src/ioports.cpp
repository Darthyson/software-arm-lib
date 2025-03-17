/*
 *  ioports.h - Definition of the I/O ports and port pins.
 *
 *  Copyright (c) 2014 Stefan Taferner <stefan.taferner@gmx.at>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 3 as
 *  published by the Free Software Foundation.
 */
#include <sblib/ioports.h>


int8_t getPinFunctionNumber(uint32_t pin, const uint16_t func)
{
    pin >>= PF0_SHIFT;

    for (uint8_t funcNumber = 0; funcNumber < 4; ++funcNumber)
    {
        if ((pin & PFF_MASK) == func)
            return static_cast<int8_t>(funcNumber);

        pin >>= PFF_SHIFT_OFFSET;
    }

    return -1;
}
