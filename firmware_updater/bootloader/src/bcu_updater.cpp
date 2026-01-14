/**************************************************************************//**
 * @addtogroup SBLIB_BOOTLOADER Selfbus Bootloader
 * @addtogroup SBLIB_BOOTLOADER_BCU Bus coupling unit (BCU)
 * @ingroup SBLIB_BOOTLOADER
 *
 * @{
 *
 * @brief Implementation of the Bootloader's bus coupling unit (BCU 1)
 *
 * @file   bcu_updater.cpp
 * @author Martin Glueck <martin@mangari.org> Copyright (c) 2015
 * @author Stefan Haller Copyright (c) 2021
 * @author Darthyson <darth@maptrack.de> Copyright (c) 2022
 * @bug No known bugs.
 ******************************************************************************/

/*
 This program is free software; you can redistribute it and/or modify
 it under the terms of the GNU General Public License version 3 as
 published by the Free Software Foundation.
 -----------------------------------------------------------------------------*/

#include "bcu_updater.h"
#include "dump.h"
#include <sblib/eib/knx_lpdu.h>
#include <sblib/eib/knx_tpdu.h>
#include <sblib/eib/apci.h>
#include <sblib/digital_pin.h>
#include <sblib/interrupt.h>
#include <sblib/internal/bootloader_commands.h>
#include <sblib/bits.h>


/**
 * The Maskversion of the Bootloader (BCU1 1.2)
 */
constexpr uint16_t BootloaderMaskVersion = 0x0012;

#ifdef DEBUG
#   define DEFAULT_COUNT_TO_FAIL (30)
    int defaultCountToFail = DEFAULT_COUNT_TO_FAIL;
    int countToFail = defaultCountToFail;

    bool checkCountToFail()
    {
        return false; ///\done uncomment on release
        // ok lets drop connection for debugging
        countToFail--;
        if (countToFail)
        {
            return false;
        }
        defaultCountToFail++;
        countToFail = defaultCountToFail;
        return true;
    }
#endif

BcuUpdate::BcuUpdate() :
    BcuBase(nullptr, nullptr)
{
}

bool BcuUpdate::processApci(ApciCommand apciCmd, unsigned char * telegram, uint8_t telLength, uint8_t * sendBuffer)
{
    uint32_t offset = 8;
    uint32_t dataLength = telLength - offset - 1; // -1 exclude KNX checksum

    switch(apciCmd)
    {
        case APCI_MEMORY_WRITE_PDU:
            return handleDeprecatedApciMemoryWrite(sendBuffer);

        case APCI_USERMSG_MANUFACTURER_0:
            return handleApciUsermsgManufacturer(sendBuffer, &telegram[offset], dataLength);

        case APCI_BASIC_RESTART_PDU:
            dump(
                serial.println("APCI_BASIC_RESTART_PDU");
                serial.println();serial.println();serial.println();
                serial.println("disconnectCount ", disconnectCount);
                serial.println("repeated T_ACK  ", repeatedT_ACKcount);
                serial.println();serial.println();serial.println();
                serial.flush(); // give time to send serial data
            )
            return BcuBase::processApci(apciCmd, telegram, telLength, sendBuffer);

        case APCI_DEVICEDESCRIPTOR_READ_PDU:
        {
            // We need to process the A_DeviceDescriptor_Read to support
            // the management procedure NM_IndividualAddress_Write.
            // Check KNX Spec. 3.0 3/5/2 2.3 NM_IndividualAddress_Write for more details.
            // It´s here, and not in BcuBase to reduce the size of the BL a little bit.
            const uint8_t id = telegram[7] & 0x3f;
            if (id != 0)
            {
                return false; // unknown device descriptor
            }

            sendBuffer[5] = 0x60 + 3; // routing count in high nibble + response length in low nibble
            setApciCommand(sendBuffer, APCI_DEVICEDESCRIPTOR_RESPONSE_PDU, 0);
            sendBuffer[8] = HIGH_BYTE(BootloaderMaskVersion);
            sendBuffer[9] = lowByte(BootloaderMaskVersion);
            return true;
        }
        default:
            return false;
    }
}

void BcuUpdate::begin()
{
    BcuBase::_begin();
}

bool BcuUpdate::processBroadCastTelegram(ApciCommand apciCmd, unsigned char *telegram, uint8_t telLength)
{
    if (directConnection() && (apciCmd == APCI_INDIVIDUAL_ADDRESS_WRITE_PDU))
    {
        // Don´t handle address write while we have an open TL4 connection
        dump(serial.println("ADDRESS_WRITE ignored (TL4 active)");)
        return false;
    }

    dump(
        switch(apciCmd)
        {
            case APCI_INDIVIDUAL_ADDRESS_READ_PDU:
                serial.print("ADDRESS_READ ");
                break;
            case APCI_INDIVIDUAL_ADDRESS_WRITE_PDU:
                serial.print("ADDRESS_WRITE");
                break;
            default:
                break;
        }
    )

    const bool handled = handleIndividualAddressBroadcast(apciCmd, telegram, telLength);
    if (handled)
    {
        dump(
            serial.print(" ", knxAddressToArea(ownAddress()));
            serial.print(".", knxAddressToLine(ownAddress()));
            serial.println(".", knxAddressToDevice(ownAddress()));
            serial.flush();
        )

        if (apciCmd == APCI_INDIVIDUAL_ADDRESS_WRITE_PDU)
        {
            // Cache current physical address in RAM.
            // Next telegram will be an APCI_BASIC_RESTART_PDU
            // See KNX Spec. 3.0 3/5/2 2.3 NM_IndividualAddress_Write
            prepareRestartIntoBootloader(this->ownAddress());
        }
    }

    return handled;
}

bool BcuUpdate::processGroupAddressTelegram(ApciCommand apciCmd, uint16_t groupAddress, unsigned char *telegram, uint8_t telLength)
{
    return true;
}

uint8_t& BcuUpdate::layerStatus()
{
    return bcuStatus;
}

/** @}*/
