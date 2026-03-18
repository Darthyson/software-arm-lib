/**************************************************************************//**
 * @addtogroup SBLIB_BOOTLOADER Selfbus Bootloader
 * @addtogroup SBLIB_BOOT_BLOCK_DESCRIPTOR Application Boot Block Descriptor
 * @ingroup SBLIB_BOOTLOADER
 *
 * @{
 *
 * @file   boot_descriptor_block.cpp
 * @author Martin Glueck <martin@mangari.org> Copyright (c) 2015
 * @author Stefan Haller Copyright (c) 2020
 * @author Darthyson <darth@maptrack.de> Copyright (c) 2026
 ******************************************************************************/

/*
 This program is free software; you can redistribute it and/or modify
 it under the terms of the GNU General Public License version 3 as
 published by the Free Software Foundation.
 -----------------------------------------------------------------------------*/

#include "boot_descriptor_block.h"
#include "crc.h"
#include <sblib/internal/iap.h>
#include <memory>


#ifndef IAP_EMULATION
    extern uint8_t __base_Flash[];      //!< marks the beginning of the flash memory (inserted by the linker)
                                        //!< used to protect the updater from killing itself with a new application downloaded over the bus
    extern uint8_t __top_Flash[];       //!< marks the end of the flash memory (inserted by the linker)
                                        //!< used to protect the updater from killing itself with a new application downloaded over the bus
    extern uint8_t _image_start[];      //!< marks the beginning of the bootloader firmware (inserted by the linker)
                                        //!< used to protect the updater from killing itself with a new application downloaded over the bus
    extern uint8_t _image_end[];        //!< marks the end of the bootloader firmware (inserted by the linker)
                                        //!< used to protect the updater from killing itself with a new application downloaded over the bus
    extern unsigned int _image_size;    //!< marks the size of the bootloader firmware (inserted by the linker)
                                        //!< used to protect the updater from killing itself with a new application downloaded over the bus
#else
    // for catch unit tests ///\todo move this to cpu-emulation
    uint8_t * __base_Flash = &FLASH[0x0000];
    uint8_t * __top_Flash = &FLASH[0x10000];
    uint8_t * _image_start = &FLASH[0x0000];
    uint8_t * _image_end = &FLASH[0x2F00];
    uint32_t _image_size = _image_end - _image_start;
#endif

char bl_id_string[BL_ID_STRING_LENGTH] = BL_ID_STRING;


/**
 * This is the sanity check as described in 26.3.3 of the UM10398 user guide
 *
 * However, this shouldn't matter since we boot into the BL anyway
 * and just it's vector table needs to be correct.
 * This test gets important if we like to bypass the KNX bootloader.
 * @note The LPC device has a built-in BL ROM for UART ISP, which is always started first.
 *       This ROM BL checks if the vector table is correct to determine if the KNX BL can be started.
 *
 * @param start Start address of the vector table
 * @return The checksum of the vector table.
 */
uint32_t checkVectorTable(const uint8_t * start)
{
    // Vector table starts always at base address, each entry is 4 bytes
    const uint32_t * address = reinterpret_cast<const uint32_t*>(start);
    uint32_t checkSum = 0;
    for (uint8_t i = 0; i < 7; i++) // Checksum is 2's complement of entries 0 through 6
    {
        checkSum += address[i];
    }

    return ~checkSum + 1;
}

bool checkApplication(const AppDescriptionBlock* block)
{
    if (block->startAddress < applicationFirstAddress() || block->startAddress > flashLastAddress()) // we have just 64k of Flash
    {
        return false;
    }
    if (block->endAddress > flashLastAddress()) // we have just 64k of Flash
    {
        return false;
    }
    if (block->startAddress >= block->endAddress)
    {
        return false;
    }

    const uint32_t blockSize = static_cast<uint32_t>(block->endAddress - block->startAddress + 1);
    uint32_t crc = crc32(0xFFFFFFFF, block->startAddress, blockSize);

    if (crc == block->crc)
    {
        return true;
        // see note from checkVectorTable
        // return checkVectorTable(block->startAddress);
    }
    return false;
}

char* getAppVersion(const AppDescriptionBlock * block)
{
    auto appVersionAddress = static_cast<void *>(block->appVersionAddress);
    if (appVersionAddress >= applicationFirstAddress() &&
        appVersionAddress < flashLastAddress() - BL_ID_STRING_LENGTH)
    {
        return block->appVersionAddress;
    }

    return bl_id_string; // Bootloader ID is invalid (address is out of range)
}

/**
 * @brief Return start address of application
 *
 * @param block Application description block to get the start address from
 * @return      Start address of application in case of valid descriptor block,
 *              otherwise base address of firmware area, directly behind bootloader
 */
uint8_t * getFirmwareStartAddress(const AppDescriptionBlock * block)
{
    if (checkApplication(block))
    {
        return block->startAddress;
    }

    return applicationFirstAddress();
}

uint8_t * bootLoaderFirstAddress()
{
    return _image_start;
}

uint8_t * bootLoaderLastAddress()
{
    // The linker sets this not correctly, so we need the -1
    return _image_end - 1;
}

uint32_t bootLoaderSize()
{
    // includes .text and .data
    return reinterpret_cast<uintptr_t>(&_image_size);
}

uint8_t * flashFirstAddress()
{
    return __base_Flash;
}

uint8_t * flashLastAddress()
{
    // The linker sets this not correctly, so we need the -1
    return __top_Flash - 1;
}

uint32_t flashSize()
{
    // add the -1 from flashLastAddress(void) back to size
    return static_cast<uint32_t>(flashLastAddress() - flashFirstAddress() + 1);
}

uint8_t * applicationFirstAddress()
{
    uint8_t * appFirstAddress = bootLoaderFirstAddress() + bootLoaderSize();
    // boot descriptor block is placed in front of the application,
    // so we need space after bootloader and before the application
    appFirstAddress += BOOT_BLOCK_DESC_SIZE;
    // align to the next flash page
    void * ptr = appFirstAddress;
    std::size_t space = FLASH_PAGE_ALIGNMENT;
    std::align(FLASH_PAGE_SIZE, 1, ptr, space);
    return static_cast<uint8_t *>(ptr);
}

uint8_t * bootDescriptorBlockAddress()
{
    // boot descriptor block is placed in front of the application
    return applicationFirstAddress() - BOOT_BLOCK_DESC_SIZE;
}

uint32_t bootDescriptorBlockPage()
{
    // every boot descriptor block is placed in front of the application, so subtract all of them
    return iapPageOfAddress(bootDescriptorBlockAddress());
}

/** @}*/
