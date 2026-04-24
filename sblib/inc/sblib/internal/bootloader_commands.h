/*
 This program is free software; you can redistribute it and/or modify
 it under the terms of the GNU General Public License version 3 as
 published by the Free Software Foundation.
 ---------------------------------------------------------------------------*/

#ifndef SBLIB_INTERNAL_BOOTLOADER_COMMANDS_H_
#define SBLIB_INTERNAL_BOOTLOADER_COMMANDS_H_

#include <cstdint>


 /** Default bootloader KNX address (15.15.192) */
constexpr uint16_t DEFAULT_BL_KNX_ADDRESS = (((15 << 12) | (15 << 8) | 192));

enum class BootState : uint8_t
{
    Reset = 0,
    BootLoader = 1,
    BootLoaderUpdater = 2,
    Application = 3,
};

/**
 * Bootloader descriptor structure stored in RAM to pass parameters to the bootloader.
 * \note Total size is 20 bytes and must be reserved by the link script of the application
 */
struct BootloaderDescriptor
{
    BootState bootState;            //!< The current boot state
    uint8_t reserved;               //!< Reserved for alignment, feel free to use
    uint16_t physicalAddress;       //!< Physical address to use in bootloader
    uint32_t programmingButton;     //!< GPIO of the programming button to use in bootloader
    uint32_t applicationId;         //!< Application ID of the application
    uint32_t applicationVersion;    //!< Application version of the application
};

constexpr uint32_t BOOTLOADER_DESCRIPTOR_SIZE = 20;

/**
 * Initialize the BootloaderDescriptor in RAM.
 *  
 * @param newBootState          The @ref BootState to set
 * @param physicalAddressToUse  Physical address to use in bootloader
 * @param programmingButton     The GPIO of the programming button
 * @param applicationId         The application ID of the application
 * @param applicationVersion    The application version of the application
 * @warning The BootloaderDescriptor is stored in RAM at 0x10000000. 
 *          The application RAM must start at 0x10000100 or higher to avoid overwriting this structure.
 */
void initBootloaderDescriptor(BootState newBootState, uint16_t physicalAddressToUse, uint32_t programmingButton,
        uint32_t applicationId, uint32_t applicationVersion);

/**
 * Retrieves the BootloaderDescriptor from RAM.
 * 
 * @return Pointer to the BootloaderDescriptor if valid, nullptr otherwise
 * @warning The BootloaderDescriptor is stored in RAM at 0x10000000. 
 *          The application RAM must start at 0x10000100 or higher to avoid overwriting this structure.
 */
const BootloaderDescriptor* getBootloaderDescriptor();

/**
 * Clears the BootloaderDescriptor in RAM.
 * @warning The BootloaderDescriptor is stored in RAM at 0x10000000. 
 *          The application RAM must start at 0x10000100 or higher to avoid overwriting this structure.
 */
void clearBootloaderDescriptor();

/**
 * Debug-only function to retrieve the BootloaderDescriptor from RAM.
 * 
 * @return Pointer to the BootloaderDescriptor in RAM
 * @warning Use of this function is intended for debugging purposes only.
 */
const BootloaderDescriptor* debugOnlyBootloaderDescriptor();

/**
 * Checks a APCI for the bus-updater Magic word
 * @details This is a bus-updater/bootloader and BCU special function
 *          used for the flashing process to boot into bootloader mode
 *
 * @param eraseCode     eraseCode of the @ref APCI_MASTER_RESET_PDU telegram
 * @param channelNumber channelNumber of the @ref APCI_MASTER_RESET_PDU telegram
 *
 * @return True if apci is a APCI_RESTART_TYPE1_PDU with a magic word<br/>
 *         otherwise false
 */
bool checkApciForMagicWord(uint8_t eraseCode, uint8_t channelNumber);

#endif /* SBLIB_INTERNAL_BOOTLOADER_COMMANDS_H_ */
