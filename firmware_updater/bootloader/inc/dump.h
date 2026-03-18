/**************************************************************************//**
 * @addtogroup SBLIB_BOOTLOADER Selfbus Bootloader
 * @addtogroup SBLIB_BOOTLOADER_DUMP Debugging over serial port stuff
 * @ingroup SBLIB_BOOTLOADER
 * @brief   Some macros for debugging over serial port stuff
 * @details 
 *
 *
 * @{
 *
 * @file   dump.h
 * @author Darthyson <darth@maptrack.de> Copyright (c) 2026
 ******************************************************************************/

/*
 This program is free software; you can redistribute it and/or modify
 it under the terms of the GNU General Public License version 3 as
 published by the Free Software Foundation.
 ---------------------------------------------------------------------------*/

#ifndef SB_BOOTLOADER_DUMP_H_
#define SB_BOOTLOADER_DUMP_H_

#ifdef SERIAL_LOGGING
#   include <sblib/serial.h>
#endif


#ifdef SERIAL_LOGGING
#   define dump(x) {x;}
#else
#   define dump(x)
#endif

#endif /* SB_BOOTLOADER_DUMP_H_ */
/** @}*/
