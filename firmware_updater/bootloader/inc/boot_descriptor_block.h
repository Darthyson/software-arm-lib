/**************************************************************************//**
 * @addtogroup SBLIB_BOOTLOADER Selfbus Bootloader
 * @defgroup SBLIB_BOOT_BLOCK_DESCRIPTOR Application Boot Block Descriptor
 * @ingroup SBLIB_BOOTLOADER
 * @brief   Application Boot Block Descriptor
 * @details
 *
 * @{
 *
 * @file   boot_descriptor_block.h
 * @author Martin Glueck <martin@mangari.org> Copyright (c) 2015
 * @author Stefan Haller Copyright (c) 2021
 * @author Darthyson <darth@maptrack.de> Copyright (c) 2026
 ******************************************************************************/

/*
 This program is free software; you can redistribute it and/or modify
 it under the terms of the GNU General Public License version 3 as
 published by the Free Software Foundation.
 -----------------------------------------------------------------------------*/

#ifndef SB_BOOTLOADER_BOOT_DESCRIPTOR_BLOCK_H_
#define SB_BOOTLOADER_BOOT_DESCRIPTOR_BLOCK_H_

///\todo Refactor all functions which return a address to return the address as uintptr_t instead of uint8_t*.

#include <sblib/platform.h>



#ifdef DEBUG
    constexpr uint16_t BL_FEATURES = 0x8100; //!< Feature list of bootloader in the Debug version
#else
    constexpr uint16_t BL_FEATURES = 0x0100; //!< Feature list of bootloader in the Release version
#endif

#define BL_ID_STRING         "[SB KNX BL ]" //!< boot loader identity string for getAppVersion()
constexpr uint8_t BL_ID_STRING_LENGTH = 13; //!< length of the bootloader identity string

constexpr uint16_t BOOT_BLOCK_DESC_SIZE = FLASH_PAGE_SIZE; //!< 1 flash page, any changes must also be done in the BLU's app_main.cpp

extern char bl_id_string[BL_ID_STRING_LENGTH]; //!< default bootloader identity "string" used in @ref getAppVersion()

/**
 * Application Description Block
 */
typedef struct AppDescriptionBlock
{
    uint8_t * startAddress;         ///< start address of the application
    uint8_t * endAddress;           ///< end address of the application
    uint32_t crc;                   //!< crc from startAddress to end endAddress
    char * appVersionAddress;       //!< address of the APP_VERSION[20] @note string MUST start with "!AVP!@:" e.g. "!AVP!@:SBuid   1.00"
}__attribute__ ((aligned (BOOT_BLOCK_DESC_SIZE))) AppDescriptionBlock;

/**
 * Checks the application description block for valid
 *        start and end addresses
 *
 * @param block Application description block to check start and end address
 * @return true if the block is valid, otherwise false
 */
bool checkApplication(const AppDescriptionBlock * block);

/**
 * Returns the address of the @ref APP_VERSION_STRING of the application starting after the magic identifier !AVP!@:
 *
 * @param block Application description block to get the address
 * @return      if valid, pointer to buffer of application version string (length @ref BL_ID_STRING_LENGTH)
 *              otherwise bl_id_string
 */
char * getAppVersion(const AppDescriptionBlock * block);

/**
 * Return start address of application
 *
 * @param block Application description block to get the start address from
 * @return      Start address of application in case of valid descriptor block,
 *              otherwise base address of firmware area, directly behind bootloader
 */
uint8_t * getFirmwareStartAddress(const AppDescriptionBlock * block);

/**
 * Returns the first address of the bootloader image (_image_start symbol included by the linker)
 *
 * @return first address of the bootloader image
 */
uint8_t * bootLoaderFirstAddress();

/**
 * Returns the last address of the bootloader image (__image_end symbol included by the linker)
 *
 * @return last address of the bootloader image
 */
uint8_t * bootLoaderLastAddress();

/**
 * Returns the size of the bootloader image in bytes (__image_end - _image_start - 1)
 *
 * @return size of the bootloader image in bytes
 */
uint32_t bootLoaderSize();

/**
 * Returns the first address of the default flash memory (__base_Flash symbol included by the linker)
 *
 * @return first address of the default flash memory
 */
uint8_t * flashFirstAddress();

/**
 * Returns the last address of the default flash memory (__top_Flash symbol included by the linker)
 *
 * @return last address of the default flash memory
 */
uint8_t * flashLastAddress();

/**
 * Returns the size of the default flash memory in bytes (__top_Flash - __base_Flash - 1)
 *
 * @return size of the default flash memory in bytes
 */
uint32_t flashSize();

/**
 * Returns the first address of the application's firmware
 *
 * @return first address of the application's firmware
 */
uint8_t * applicationFirstAddress();

/**
 * Returns the first address of the boot descriptor block
 *
 * @return first address of the boot descriptor block
 */
uint8_t * bootDescriptorBlockAddress();

/**
 * Returns the page number of the boot descriptor block
 *
 * @return page number of the boot descriptor block
 */
uint32_t bootDescriptorBlockPage();

#endif /* SB_BOOTLOADER_BOOT_DESCRIPTOR_BLOCK_H_ */

/** @}*/
