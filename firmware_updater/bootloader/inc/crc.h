/**************************************************************************//**
 * @addtogroup SBLIB_BOOTLOADER Selfbus Bootloader
 * @defgroup SBLIB_CRC Crc calculation
 * @ingroup SBLIB_BOOTLOADER
 * @brief   Crc calculation
 * @details
 *
 * @{
 *
 * @file   crc.h
 * @author Deti Fliegl <deti@fliegl.de> Copyright (c) 2015
 * @author Martin Glueck <martin@mangari.org> Copyright (c) 2015
 ******************************************************************************/

/*
 This program is free software; you can redistribute it and/or modify
 it under the terms of the GNU General Public License version 3 as
 published by the Free Software Foundation.
 -----------------------------------------------------------------------------*/

#ifndef SB_BOOTLOADER_CRC32_H_
#define SB_BOOTLOADER_CRC32_H_

#include <cstdint>


/**
 * Calculates the crc32 of provided buffer data.
 *
 * @param startCrc32 crc to start with (if unsure use 0xFFFFFFFF)
 * @param data       Buffer to calculate the crc32 of
 * @param count      Length of data
 * @return crc32 of data
 */
uint32_t crc32(uint32_t crc, const uint8_t * data, uint32_t count);

#endif /* SB_BOOTLOADER_CRC32_H_ */

/** @}*/
