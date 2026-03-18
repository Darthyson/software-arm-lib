/**************************************************************************//**
 * @addtogroup SBLIB_BOOTLOADER Selfbus Bootloader
 * @defgroup SBLIB_UPD_UDP_FLASH_1 Flash access utilities
 * @ingroup SBLIB_BOOTLOADER
 * @brief   Provides several functions for accessing and flashing the MCU's flash
 * @details 
 *
 *
 * @{
 *
 * @file   flash.cpp
 * @author Darthyson <darth@maptrack.de> Copyright (c) 2026
 ******************************************************************************/

/*
 This program is free software; you can redistribute it and/or modify
 it under the terms of the GNU General Public License version 3 as
 published by the Free Software Foundation.
 ---------------------------------------------------------------------------*/

#include "flash.h"
#include "boot_descriptor_block.h"
#include "dump.h"
#include <sblib/internal/iap.h>


/**
 * @brief Checks if the pointer is aligned.
 *
 * @param  ptr       Pointer to check
 * @param  alignment Expected alignment needs to be a power of 2
 * @return           true if the pointer is aligned to alignment, otherwise false
 */
bool is_aligned(const uint8_t * ptr, const uint32_t alignment)
{
    // See https://stackoverflow.com/a/1898487 and https://stackoverflow.com/a/28760180 for reasoning.
    return (reinterpret_cast<uintptr_t>(static_cast<const void *>(ptr)) & (alignment - 1)) == 0;
}

bool addressAllowedToProgram(const uint8_t * start, const uint32_t length, const bool isBootDescriptor)
{
    if (!is_aligned(start, FLASH_PAGE_SIZE) || !length) // not aligned to page or 0 length
    {
        return false;
    }

    const uint8_t * end = start + length - 1;
    if (isBootDescriptor)
    {
        return start >= bootDescriptorBlockAddress() && end < applicationFirstAddress();
    }

    return start >= applicationFirstAddress() && end <= flashLastAddress();
}

/**
 * @brief Checks if the requested page is allowed to be erased.
 *
 * @param  pageNumber Page number to check if erase is allowed
 * @return            true if the page is allowed to erase, otherwise false
 */
static bool pageAllowedToErase(const uint32_t pageNumber)
{
    return pageNumber > iapPageOfAddress(bootLoaderLastAddress()) &&
           pageNumber <= iapPageOfAddress(flashLastAddress());
}

/**
 * @brief Checks if the requested sector is allowed to be erased.
 *
 * @param  sectorNumber Sector number to check erase is allowed
 * @return              true if the sector is allowed to erase, otherwise false
 */
static bool sectorAllowedToErase(const uint32_t sectorNumber)
{
    return sectorNumber > iapSectorOfAddress(bootLoaderLastAddress()) &&
           sectorNumber <= iapSectorOfAddress(flashLastAddress());
}

UDP_State erasePageRange(const uint32_t startPage, const uint32_t endPage)
{
    dump(serial.print("page   0x", startPage, HEX, 2));
    dump(serial.print(" - 0x", endPage, HEX, 2));
    dump(serial.print(" "));
    if (!pageAllowedToErase(startPage) || !pageAllowedToErase(endPage))
    {
        dump(serial.println("not allowed!");)
        return UDP_PAGE_NOT_ALLOWED_TO_ERASE;
    }

    const UDP_State result = iapResult2UDPState(iapErasePageRange(startPage, endPage));
    dump(
        if (result != UDP_IAP_SUCCESS)
        {
            updResult2Serial(result);
            serial.println(" iapErasePageRange failed!");
        }
        else
        {
            serial.println("OK");
        }
    );
    return result;
}

/**
 * @brief Erases if allowed the requested sector.
 *
 * @param startSector Start sector number to be erased
 * @param endSector   End sector number to be erased
 * @return  @ref UDP_IAP_SUCCESS if successful, otherwise @ref UDP_SECTOR_NOT_ALLOWED_TO_ERASE or an @ref IAP_Status
 */
static UDP_State eraseSectorRange(const uint32_t startSector, const uint32_t endSector)
{
    dump(serial.print("sector 0x", startSector, HEX, 2));
    dump(serial.print(" - 0x", endSector, HEX, 2));
    if (!sectorAllowedToErase(startSector) || !sectorAllowedToErase(endSector))
    {
        dump(serial.println(" not allowed!");)
        return UDP_SECTOR_NOT_ALLOWED_TO_ERASE;
    }

    const UDP_State result = iapResult2UDPState(iapEraseSectorRange(startSector, endSector));
    dump(
        if (result != UDP_IAP_SUCCESS)
        {
            updResult2Serial(result);
            serial.println(" iapEraseSectorRange failed!");
        }
        else
        {
            serial.println(" OK");
        }
    );
    return result;
}

UDP_State eraseAddressRange(const uint8_t * startAddress, const uint8_t * endAddress, const bool rangeCheck)
{
    UDP_State result = UDP_ADDRESS_RANGE_NOT_ALLOWED_TO_ERASE;
    dump(
        serial.print(" eraseAddressRange 0x", startAddress);
        serial.println("-0x", endAddress);
    );

    if (rangeCheck && !addressAllowedToProgram(startAddress, static_cast<uint32_t>(endAddress - startAddress + 1), false))
    {
        dump(serial.println(" not allowed!");)
        return UDP_ADDRESS_RANGE_NOT_ALLOWED_TO_ERASE;
    }

    uint32_t start;
    uint32_t end;
    const uint32_t endPage = iapPageOfAddress(endAddress);

    uint32_t startSector = iapSectorOfAddress(startAddress);
    uint32_t endSector = iapSectorOfAddress(endAddress);
    const uint32_t startPage = iapPageOfAddress(startAddress);

    const bool lessThenOneSector = endPage - startPage + 1 < FLASH_SECTOR_SIZE / FLASH_PAGE_SIZE;
    if (lessThenOneSector)
    {
        result = erasePageRange(startPage, endPage); // this is slow and can take up to 15*100ms = ~1,5s
        return result;
    }

    if (!is_aligned(startAddress, FLASH_SECTOR_SIZE))
    {
        // start address is not sector aligned, let's erase on a page level
        start = startPage;
        startSector++;
        // from start to last page of the sector
        const uint8_t * nextSectorStartAddress = FLASH_BASE_ADDRESS + startSector * FLASH_SECTOR_SIZE;
        end = iapPageOfAddress(nextSectorStartAddress) - 1;
        result = erasePageRange(start, end); // this is slow and can take up to 15*100ms = ~1,5s
        if (result != UDP_IAP_SUCCESS)
        {
            return result;
        }
        startAddress = nextSectorStartAddress; // set new startAddress
    }

    startSector = iapSectorOfAddress(startAddress);

    start = endSector * FLASH_SECTOR_SIZE / FLASH_PAGE_SIZE;
    const bool lastPageInSector = endPage - start + 1 == FLASH_SECTOR_SIZE / FLASH_PAGE_SIZE;
    if (!lastPageInSector)
    {
        // end address is not sector aligned, let's erase on a page level
        end = iapPageOfAddress(endAddress);

        result = erasePageRange(start, end); //  this is slow and can take up to 15*100ms = ~1,5s
        if (result != UDP_IAP_SUCCESS)
        {
            return result;
        }
        endSector--;
    }

    if (startSector <= endSector)
    {
        result = eraseSectorRange(startSector, endSector); // this is fast and should always take ~100ms
    }

    return result;
}

UDP_State eraseFullFlash()
{
    uint32_t page = iapPageOfAddress(bootLoaderLastAddress());
    page++;
    return eraseAddressRange(iapAddressOfPage(page), flashLastAddress(), false);
}

UDP_State executeProgramFlash(uint8_t * address, const uint8_t * ram, const uint32_t size, const bool isBootDescriptor)
{
    if (!addressAllowedToProgram(address, size, isBootDescriptor))
    {
        return UDP_ADDRESS_NOT_ALLOWED_TO_FLASH;
    }

    const UDP_State result = iapResult2UDPState(iapProgram(address, ram, size));
    return result;
}


/** @}*/
