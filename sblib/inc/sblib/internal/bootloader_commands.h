/*
 This program is free software; you can redistribute it and/or modify
 it under the terms of the GNU General Public License version 3 as
 published by the Free Software Foundation.
 ---------------------------------------------------------------------------*/

#ifndef SBLIB_INTERNAL_BOOTLOADER_COMMANDS_H_
#define SBLIB_INTERNAL_BOOTLOADER_COMMANDS_H_

#include "sblib/eib/apci.h"
#include <cstdint>

/**
 * Magic word, which will be checked on startup of the bootloader
 * weather or not to go into bootloader mode
 */
constexpr uint32_t BOOTLOADER_MAGIC_WORD = 0x5E1FB055;

/**
 * Magic address for the magic word to be checked on startup of the bootloader
 * weather or not to go into bootloader mode
 */
#define BOOTLOADER_MAGIC_ADDRESS ((uint32_t *) 0x10000000)
//uint32_t* BOOTLOADER_MAGIC_ADDRESS = reinterpret_cast<uint32_t*>(0x10000000UL);

/**
 * Bootloader magic erase = FactoryResetWithoutIndividualAddress in calimero-core
 */
constexpr RestartPDUMasterReset BOOTLOADER_MAGIC_ERASE = T_MASTERRESET_FACTORY_WO_IA;

/**
 * Bootloader magic channel
 */
constexpr uint8_t BOOTLOADER_MAGIC_CHANNEL = 255;

enum class RestartType : uint8_t
{
    None,
    Basic,
    Master,
    MasterIntoBootloader,
    IndividualAddressChanged
};

/**
 * Set magicWord to start in bootloader mode after reset.
 **/
void prepareRestartIntoBootloader(uint16_t physicalAddressToUse);

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
bool checkApciForMagicWord(byte eraseCode, byte channelNumber);

#endif /* SBLIB_INTERNAL_BOOTLOADER_COMMANDS_H_ */
