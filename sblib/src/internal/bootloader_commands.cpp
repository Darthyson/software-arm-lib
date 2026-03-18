/*
 This program is free software; you can redistribute it and/or modify
 it under the terms of the GNU General Public License version 3 as
 published by the Free Software Foundation.
 ---------------------------------------------------------------------------*/

#include "sblib/internal/bootloader_commands.h"
#include "sblib/eib/apci.h"
#include "sblib/platform.h"

static_assert(sizeof(BootloaderDescriptor) % FLASH_RAM_BUFFER_ALIGNMENT == 0,
        "Check 4 byte alignment of BootloaderDescriptor, don't use packed!");


/**
 * Bootloader magic erase = FactoryResetWithoutIndividualAddress in calimero-core
 */
constexpr RestartPDUMasterReset BOOTLOADER_MAGIC_ERASE = T_MASTERRESET_FACTORY_WO_IA;

/**
 * Bootloader magic channel
 */
constexpr uint8_t BOOTLOADER_MAGIC_CHANNEL = 255;

/**
 * "Unique" BootloaderDescriptor identifier in RAM.
 */
constexpr uint32_t UID_BOOTLOADER_DESCRIPTOR = 0x5E1FB055;

/**
 * Pointer to BootloaderDescriptor in RAM.
 * @warning The bootLoaderDescriptor is stored in RAM at 0x100000C0.
 *          The application RAM must start behind the bootLoaderDescriptor to avoid overwriting it.
 * @def bootLoaderDescriptor
 */
#ifdef IAP_EMULATION ///\todo must be #ifdef on release! fix before release!
    uint32_t data;
    uint32_t * magicWord = &data; // I'm here for the unit-tests
    BootloaderDescriptor testBootLoaderDescriptor;
    BootloaderDescriptor* bootLoaderDescriptor = &testBootLoaderDescriptor;
#else
    auto magicWord = reinterpret_cast<uint32_t*>(0x10000000UL);
    auto bootLoaderDescriptor = reinterpret_cast<BootloaderDescriptor*>(0x100000C0UL);
#endif

void initBootloaderDescriptor(const BootState newBootState, const uint16_t physicalAddressToUse,
    const uint32_t programmingButton, const uint32_t applicationId, const uint32_t applicationVersion)
{
    bootLoaderDescriptor->bootState = newBootState;
    bootLoaderDescriptor->physicalAddress = physicalAddressToUse;
    bootLoaderDescriptor->programmingButton = programmingButton;
    bootLoaderDescriptor->applicationId = applicationId;
    bootLoaderDescriptor->applicationVersion = applicationVersion;

    switch (bootLoaderDescriptor->bootState)
    {
        case BootState::BootLoader:
        case BootState::BootLoaderUpdater:
        case BootState::Reset:
            *magicWord = UID_BOOTLOADER_DESCRIPTOR;
            break;

        case BootState::Application:
        default:
            *magicWord = 0;
            break;
    }
}

const BootloaderDescriptor* getBootloaderDescriptor()
{
    if (*magicWord != UID_BOOTLOADER_DESCRIPTOR)
    {
        return nullptr;
    }
    return bootLoaderDescriptor;
}

void clearBootloaderDescriptor()
{
    initBootloaderDescriptor(BootState::Application, 0, 0, 0, 0);
}

const BootloaderDescriptor* debugOnlyBootloaderDescriptor()
{
    return bootLoaderDescriptor;
}

bool checkApciForMagicWord(const uint8_t eraseCode, const uint8_t channelNumber)
{
    // The special version of APCI_MASTER_RESET_PDU used by Selfbus bootloader
    // restart with parameters, special meaning of erase=7 and channel=255 for bootloader mode
    return eraseCode == BOOTLOADER_MAGIC_ERASE && channelNumber == BOOTLOADER_MAGIC_CHANNEL;
}
