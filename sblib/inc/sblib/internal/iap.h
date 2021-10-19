/*
 *  Copyright (c) 2014 Martin Glueck <martin@mangari.org>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 3 as
 *  published by the Free Software Foundation.
 */

#ifndef sblib_iap_h
#define sblib_iap_h

#include <sblib/platform.h>
#include <sblib/types.h>

#define IAP_UID_LENGTH (16) //!< number of bytes iapReadUID wants as buffer

/**
 * Status code of IAP commands
 */
enum IAP_Status
{
    IAP_SUCCESS,                                //!< CMD_SUCCESS
    IAP_INVALID_COMMAND,                        //!< INVALID_COMMAND
    IAP_SRC_ADDR_ERROR,                         //!< SRC_ADDR_ERROR
    IAP_DST_ADDR_ERROR,                         //!< DST_ADDR_ERROR
    IAP_SRC_ADDR_NOT_MAPPED,                    //!< SRC_ADDR_NOT_MAPPED
    IAP_DST_ADDR_NOT_MAPPED,                    //!< DST_ADDR_NOT_MAPPED
    IAP_COUNT_ERROR,                            //!< COUNT_ERROR
    IAP_INVALID_SECTOR,                         //!< INVALID_SECTOR
    IAP_SECTOR_NOT_BLANK,                       //!< SECTOR_NOT_BLANK
    IAP_SECTOR_NOT_PREPARED_FOR_WRITE_OPERATION,//!< SECTOR_NOT_PREPARED_FOR_WRITE_OPERATION
    IAP_COMPARE_ERROR,                          //!< COMPARE_ERROR
    IAP_BUSY                                    //!< BUSY
};


/**
 * Get the index of the FLASH sector for the passed address.
 *
 * @param address - the address inside the FLASH
 * @return The sector index of the address.
 */
int iapSectorOfAddress(const byte* address);

/**
 * Get the index of the FLASH page for the passed address.
 *
 * @param address - the address inside the FLASH
 * @return The sector index of the address.
 */
int iapPageOfAddress(const byte* address);

/**
 * Erase the specified sector.
 *  @param sector       index of the sector which should be erased
 *  @return             status code (0 == OK)
 */
IAP_Status iapEraseSector(int sector);

/**
 * Erase the specified page.
 *  @param pageNumber   index of the page which should be erased
 *  @return             status code (0 == OK)
 */
IAP_Status iapErasePage(int sector);

/**
 * Programs the specified number of bytes from the RAM to the specified location
 * inside the FLASH.
 * @param rom           start address of inside the FLASH
 * @param ram           start address if the buffer
 * @param size          number of bytes ot program
 * @return              status code, see enum IAP_Status above
 */
IAP_Status iapProgram(byte* rom, const byte* ram, unsigned int size);

/**
 * Read the unique ID of the CPU. The ID is 16 bytes long.
 *
 * @param uid - will contain the 16 byte UID after the call.
 *
 * @return Status code, see enum IAP_Status above
 */
IAP_Status iapReadUID(byte* uid);

/**
 * Read the 32 bit part identification number of the CPU.
 *
 * @param partId - will contain the 32 bit part identification number after the call.
 *
 * @return Status code, see enum IAP_Status above
 */
IAP_Status iapReadPartID(unsigned int* partId);

/**
 * Get the size of the flash memory. This is done by probing the flash sectors
 * until an error is encountered.
 *
 * @return the size of the flash memory.
 */
int iapFlashSize();


#endif /* sblib_iap_h */
