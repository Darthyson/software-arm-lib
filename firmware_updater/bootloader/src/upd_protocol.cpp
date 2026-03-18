/**************************************************************************//**
 * @addtogroup SBLIB_BOOTLOADER Selfbus Bootloader
 * @addtogroup SBLIB_UPD_UDP_PROTOCOL_1 UPD/UDP protocol
 * @ingroup SBLIB_BOOTLOADER
 *
 * @brief   Definition of the UPD/UDP bootloader protocol
 * @details 
 *
 *
 * @{
 *
 * @file   upd_protocol.cpp
 * @author Darthyson <darth@maptrack.de> Copyright (c) 2026
 ******************************************************************************/

/*
 This program is free software; you can redistribute it and/or modify
 it under the terms of the GNU General Public License version 3 as
 published by the Free Software Foundation.
 ---------------------------------------------------------------------------*/

#include "upd_protocol.h"
#include "dump.h"


UPD_Command code2UPDCommand(const uint8_t code)
{
    for (const auto updCommand : updCommands)
    {
        if (updCommand.code == code)
        {
            return updCommand;
        }
    }
    return updCommands[idxInvalidUPDCommand]; // UPD_INVALID
}

UDP_State iapResult2UDPState(const IAP_Status iapState)
{
    UDP_State result;
    switch (iapState)
    {
        case IAP_SUCCESS :
            result = UDP_IAP_SUCCESS;
            break;
        case IAP_INVALID_COMMAND :
            result = UDP_IAP_INVALID_COMMAND;
                        break;
        case IAP_SRC_ADDR_ERROR :
            result = UDP_IAP_SRC_ADDR_ERROR;
                        break;
        case IAP_DST_ADDR_ERROR :
            result = UDP_IAP_DST_ADDR_ERROR;
                        break;
        case IAP_SRC_ADDR_NOT_MAPPED :
            result = UDP_IAP_SRC_ADDR_NOT_MAPPED;
                        break;
        case IAP_DST_ADDR_NOT_MAPPED :
            result = UDP_IAP_DST_ADDR_NOT_MAPPED;
                        break;
        case IAP_COUNT_ERROR :
            result = UDP_IAP_COUNT_ERROR;
                        break;
        case IAP_INVALID_SECTOR :
            result = UDP_IAP_INVALID_SECTOR;
                        break;
        case IAP_SECTOR_NOT_BLANK :
            result = UDP_IAP_SECTOR_NOT_BLANK;
                        break;
        case IAP_SECTOR_NOT_PREPARED_FOR_WRITE_OPERATION :
            result = UDP_IAP_SECTOR_NOT_PREPARED_FOR_WRITE_OPERATION;
                        break;
        case IAP_COMPARE_ERROR :
            result = UDP_IAP_COMPARE_ERROR;
                        break;
        case IAP_BUSY :
            result = UDP_IAP_BUSY;
                        break;
        default :
            result = UDP_IAP_UNKNOWN;
    }
    return result;
}


void updCommand2Serial([[maybe_unused]] const UPD_Command cmd)
{
    dump(
        serial.print("UPD_");
        switch (cmd.code)
        {
            case UPD_SEND_DATA: serial.print("SEND_DATA"); break;
            case UPD_PROGRAM: serial.print("PROGRAM"); break;
            case UPD_UPDATE_BOOT_DESC: serial.print("UPDATE_BOOT_DESC"); break;
            case UPD_SEND_DATA_TO_DECOMPRESS: serial.print("SEND_DATA_TO_DECOMPRESS"); break;
            case UPD_PROGRAM_DECOMPRESSED_DATA: serial.print("PROGRAM_DECOMPRESSED_DATA"); break;
            case UPD_ERASE_COMPLETE_FLASH: serial.print("ERASE_COMPLETE_FLASH"); break;
            case UPD_ERASE_ADDRESS_RANGE: serial.print("UPD_ERASE_ADDRESS_RANGE"); break;
            case UPD_REQ_DATA: serial.print("REQ_DATA"); break;
            case UPD_DUMP_FLASH: serial.println("DUMP_FLASH"); break;
            case UPD_REQUEST_STATISTIC: serial.print("REQUEST_STATISTIC"); break;
            case UPD_RESPONSE_STATISTIC: serial.print("RESPONSE_STATISTIC"); break;
            case UPD_SEND_LAST_ERROR: serial.print("SEND_LAST_ERROR"); break;
            case UPD_UNLOCK_DEVICE: serial.print("UNLOCK_DEVICE"); break;
            case UPD_REQUEST_UID: serial.print("REQUEST_UID"); break;
            case UPD_RESPONSE_UID: serial.print("RESPONSE_UID"); break;
            case UPD_APP_VERSION_REQUEST: serial.print("APP_VERSION_REQUEST"); break;
            case UPD_APP_VERSION_RESPONSE: serial.print("APP_VERSION_RESPONSE"); break;
            case UPD_REQUEST_BOOT_DESC: serial.print("REQUEST_BOOT_DESC"); break;
            case UPD_RESPONSE_BOOT_DESC: serial.print("RESPONSE_BOOT_DESC"); break;
            case UPD_REQUEST_BL_IDENTITY: serial.print("REQUEST_BL_IDENTITY"); break;
            case UPD_RESPONSE_BL_IDENTITY: serial.print("RESPONSE_BL_IDENTITY"); break;
            case UPD_RESPONSE_BL_VERSION_MISMATCH: serial.print("RESPONSE_BL_VERSION_MISMATCH"); break;
            case UPD_SET_EMULATION: serial.print("SET_EMULATION"); break;
            default: serial.print("Command unknown", cmd.code); break;
        }
        serial.print(" ");
    )
}

void updResult2Serial([[maybe_unused]] const UDP_State result)
{
    dump(
        serial.print("UPD_");
        switch (result)
        {
            case UDP_IAP_SUCCESS: serial.print("IAP OK"); break;
            case UDP_IAP_INVALID_COMMAND: serial.print("IAP_INVALID_COMMAND."); break;
            case UDP_IAP_SRC_ADDR_ERROR: serial.print("IAP_SRC_ADDR_ERROR"); break;
            case UDP_IAP_DST_ADDR_ERROR: serial.print("IAP_DST_ADDR_ERROR"); break;
            case UDP_IAP_SRC_ADDR_NOT_MAPPED: serial.print("IAP_SRC_ADDR_NOT_MAPPED"); break;
            case UDP_IAP_DST_ADDR_NOT_MAPPED: serial.print("IAP_DST_ADDR_NOT_MAPPED"); break;
            case UDP_IAP_COUNT_ERROR: serial.print("IAP_COUNT_ERROR"); break;
            case UDP_IAP_INVALID_SECTOR: serial.print("IAP_INVALID_SECTOR"); break;
            case UDP_IAP_SECTOR_NOT_BLANK: serial.print("IAP_SECTOR_NOT_BLANK"); break;
            case UDP_IAP_SECTOR_NOT_PREPARED_FOR_WRITE_OPERATION: serial.print("IAP_SECTOR_NOT_PREPARED_FOR_WRITE_OPERATION"); break;
            case UDP_IAP_COMPARE_ERROR: serial.print("IAP_COMPARE_ERRO"); break;
            case UDP_IAP_BUSY: serial.print("IAP_BUSY"); break;
            case UDP_IAP_UNKNOWN: serial.print("UDP_IAP_UNKNOWN"); break;

            case UDP_UNKNOWN_COMMAND: serial.print("UNKNOWN_COMMAND"); break;
            case UDP_CRC_ERROR: serial.print("CRC_ERROR"); break;
            case UDP_ADDRESS_NOT_ALLOWED_TO_FLASH: serial.print("ADDRESS_NOT_ALLOWED_TO_FLASH"); break;
            case UDP_SECTOR_NOT_ALLOWED_TO_ERASE: serial.print("SECTOR_NOT_ALLOWED_TO_ERASE"); break;
            case UDP_RAM_BUFFER_OVERFLOW: serial.print("RAM_BUFFER_OVERFLOW"); break;
            case UDP_WRONG_DESCRIPTOR_BLOCK: serial.print("WRONG_DESCRIPTOR_BLOCK"); break;
            case UDP_APPLICATION_NOT_STARTABLE: serial.print("APPLICATION_NOT_STARTABLE"); break;
            case UDP_DEVICE_LOCKED: serial.print("DEVICE_LOCKED"); break;
            case UDP_UID_MISMATCH: serial.print("UID_MISMATCH"); break;
            case UDP_ERASE_FAILED: serial.print("ERASE_FAILED"); break;
            case UDP_INVALID_DATA: serial.print("UDP_INVALID_DATA"); break;
            case UDP_NO_DATA: serial.print("UDP_NO_DATA"); break;

            case UDP_FLASH_ERROR: serial.print("FLASH_ERROR"); break;
            case UDP_PAGE_NOT_ALLOWED_TO_ERASE: serial.print("PAGE_NOT_ALLOWED_TO_ERASE"); break;
            case UDP_ADDRESS_RANGE_NOT_ALLOWED_TO_ERASE: serial.print("ADDRESS_RANGE_NOT_ALLOWED_TO_ERASE"); break;
            case UDP_NOT_IMPLEMENTED: serial.print("NOT_IMPLEMENTED"); break;
            case UDP_INVALID: serial.print("UDP_INVALID"); break;
            default: serial.print("State unknown ", result); break;
        }
    )
}



/** @}*/
