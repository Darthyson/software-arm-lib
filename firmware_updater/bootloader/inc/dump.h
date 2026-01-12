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
 * @author Darthyson <darth@maptrack.de> Copyright (c) 2021
 * @bug No known bugs.
 ******************************************************************************/

/*
 This program is free software; you can redistribute it and/or modify
 it under the terms of the GNU General Public License version 3 as
 published by the Free Software Foundation.
 ---------------------------------------------------------------------------*/

#ifndef SB_BOOTLOADER_DUMP_H_
#define SB_BOOTLOADER_DUMP_H_

#ifdef DUMP_TELEGRAMS_LVL1
#   include <sblib/serial.h>
#endif


#ifdef DUMP_TELEGRAMS_LVL1
#   define dump(x) {x;}
#else
#   define dump(x)
#endif

#endif /* SB_BOOTLOADER_DUMP_H_ */
/** @}*/
