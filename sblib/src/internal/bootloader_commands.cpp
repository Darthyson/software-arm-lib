/*
 This program is free software; you can redistribute it and/or modify
 it under the terms of the GNU General Public License version 3 as
 published by the Free Software Foundation.
 ---------------------------------------------------------------------------*/

#include "sblib/internal/bootloader_commands.h"

void prepareRestartIntoBootloader(const uint16_t physicalAddressToUse)
{
#ifndef IAP_EMULATION
    uint32_t* magicWord = BOOTLOADER_MAGIC_ADDRESS;
    *magicWord = BOOTLOADER_MAGIC_WORD;
    magicWord++;
    *magicWord = physicalAddressToUse;
#endif
}

bool checkApciForMagicWord(const byte eraseCode, const byte channelNumber)
{
    // The special version of APCI_MASTER_RESET_PDU used by Selfbus bootloader
    // restart with parameters, special meaning of erase=7 and channel=255 for bootloader mode
    return eraseCode == BOOTLOADER_MAGIC_ERASE && channelNumber == BOOTLOADER_MAGIC_CHANNEL;
}
