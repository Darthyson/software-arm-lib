/**************************************************************************//**
 * @addtogroup SBLIB_BOOTLOADER Selfbus Bootloader
 * @addtogroup SBLIB_CRC Crc calculation
 * @ingroup SBLIB_BOOTLOADER
 *
 * @{
 *
 * @file   crc.cpp
 * @author Deti Fliegl <deti@fliegl.de> Copyright (c) 2015
 * @author Martin Glueck <martin@mangari.org> Copyright (c) 2015
 ******************************************************************************/

/*
 This program is free software; you can redistribute it and/or modify
 it under the terms of the GNU General Public License version 3 as
 published by the Free Software Foundation.
 -----------------------------------------------------------------------------*/

#include "crc.h"


uint32_t crc32(uint32_t crc, const uint8_t * data, uint32_t count)
{
    // https://stackoverflow.com/questions/27939882/fast-crc-algorithm/27950866#27950866
    constexpr uint32_t POLYNOM = 0xEDB88320;
    while (count--) {
        crc ^= *data++; // Get next byte and XOR
        for (uint8_t i = 0; i < 8; i++)
        {
            if (crc & 1)
            {
                crc = (crc >> 1) ^ POLYNOM;
            }
            else
            {
                crc = crc >> 1;
            }
        }
    }
    return ~crc;
}

/** @}*/

