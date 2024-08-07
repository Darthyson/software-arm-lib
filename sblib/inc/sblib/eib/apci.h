/*
 *  apci.h - Telegram application types.
 *
 *  Copyright (c) 2014 Stefan Taferner <stefan.taferner@gmx.at>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 3 as
 *  published by the Free Software Foundation.
 */
#ifndef sblib_apci_h
#define sblib_apci_h

#include <sblib/types.h>

#define APCI_HIGH_BYTE       (6)
#define APCI_LOW_BYTE        (7)

/**
 * Application Layer commands (APCI)
 * @details <br>see KNX spec 2.1 3/3/7 page 8 ff.</b>
 */
enum ApciCommand
{
    APCI_GROUP_MASK = 0x3c0,                            /**< APCI_GROUP_MASK */

    APCI_GROUP_VALUE_READ_PDU                   = 0x000, //!< A_GroupValue_Read-PDU, multicast
    APCI_GROUP_VALUE_RESPONSE_PDU               = 0x040, //!< A_GroupValue_Response-PDU, multicast
    APCI_GROUP_VALUE_WRITE_PDU                  = 0x080, //!< A_GroupValue_Write-PDU, multicast

    APCI_INDIVIDUAL_ADDRESS_WRITE_PDU           = 0x0c0, //!< A_IndividualAddress_Write-PDU, broadcast
    APCI_INDIVIDUAL_ADDRESS_READ_PDU            = 0x100, //!< A_IndividualAddress_Read-PDU, broadcast
    APCI_INDIVIDUAL_ADDRESS_RESPONSE_PDU        = 0x140, //!< A_IndividualAddress_Response-PDU, broadcast

    APCI_ADC_READ_PDU                           = 0x180, //!< A_ADC_Read-PDU, connection-oriented
    APCI_ADC_RESPONSE_PDU                       = 0x1C0, //!< A_ADC_Response-PDU, connection-oriented

    APCI_SYSTEM_NETWORK_PARAMETER_READ_PDU      = 0x1C8, //!< A_SystemNetworkParameter_Read-PDU, system broadcast
    APCI_SYSTEM_NETWORK_PARAMETER_RESPONSE_PDU  = 0x1C9, //!< A_SystemNetworkParameter_Response-PDU, system broadcast
    APCI_SYSTEM_NETWORK_PARAMETER_WRITE_PDU     = 0x1CA, //!< A_SystemNetworkParameter_Write-PDU, system broadcast
    APCI_SYSTEM_NETWORK_PARAMETER_RESERVED_PDU  = 0x1CB, //!< planned for future system broadcast service, system broadcast

    APCI_MEMORY_READ_PDU                        = 0x200, //!< A_Memory_Read-PDU, connection-oriented
    APCI_MEMORY_RESPONSE_PDU                    = 0x240, //!< A_Memory_Response-PDU, connection-oriented
    APCI_MEMORY_WRITE_PDU                       = 0x280, //!< A_Memory_Write-PDU, connection-oriented

    APCI_USER_MEMORY_READ_PDU                   = 0x2C0, //!< A_UserMemory_Read-PDU, connection-oriented
    APCI_USER_MEMORY_RESPONSE_PDU               = 0x2C1, //!< A_UserMemory_Response-PDU, connection-oriented
    APCI_USER_MEMORY_WRITE_PDU                  = 0x2C2, //!< A_UserMemory_Write-PDU, connection-oriented

    APCI_USER_MEMORY_BIT_WRITE_PDU              = 0x2C4, //!< deprecated, A_UserMemoryBit_Write-PDU, connection-oriented

    APCI_USER_MANUFACTURER_INFO_READ_PDU        = 0x2C5, //!< A_UserManufacturerInfo_Read-PDU, connection-oriented
    APCI_USER_MANUFACTURER_INFO_RESPONSE_PDU    = 0x2C6, //!< A_UserManufacturerInfo_Response-PDU, connection-oriented

    APCI_FUNCTIONPROPERTY_COMMAND_PDU           = 0x2C7, //!< A_FunctionPropertyCommand-PDU, connectionless and connection-oriented
    APCI_FUNCTIONPROPERTY_STATE_READ_PDU        = 0x2C8, //!< A_FunctionPropertyState_Read-PDU, connectionless and connection-oriented
    APCI_FUNCTIONPROPERTY_STATE_RESPONSE_PDU    = 0x2C9, //!< A_FunctionPropertyState_Response-PDU, connectionless and connection-oriented

    APCI_USERMSG_RESERVED_0                     = 0x2CA,/**< APCI_USERMSG_RESERVED_0 */
    // ... 0x2CA -> 0x2F7 reserved usermsg
    APCI_USERMSG_RESERVED_45                    = 0x2F7,/**< APCI_USERMSG_RESERVED_45 */

    // 0x2F8 -> 0x2FE manufacturer specific area for usermsg
    APCI_USERMSG_MANUFACTURER_0                 = 0x2F8,/**< APCI_USERMSG_MANUFACTURER_0 */
    APCI_USERMSG_MANUFACTURER_1                 = 0x2F9,/**< APCI_USERMSG_MANUFACTURER_1 */
    APCI_USERMSG_MANUFACTURER_2                 = 0x2FA,/**< APCI_USERMSG_MANUFACTURER_2 */
    APCI_USERMSG_MANUFACTURER_3                 = 0x2FB,/**< APCI_USERMSG_MANUFACTURER_3 */
    APCI_USERMSG_MANUFACTURER_4                 = 0x2FC,/**< APCI_USERMSG_MANUFACTURER_4 */
    APCI_USERMSG_MANUFACTURER_5                 = 0x2FD,/**< APCI_USERMSG_MANUFACTURER_5 */
    APCI_USERMSG_MANUFACTURER_6                 = 0x2FE,/**< APCI_USERMSG_MANUFACTURER_6 */

    APCI_DEVICEDESCRIPTOR_READ_PDU              = 0x300, //!< A_DeviceDescriptor_Read-PDU, connection-oriented
    APCI_DEVICEDESCRIPTOR_RESPONSE_PDU          = 0x340, //!< A_DeviceDescriptor_Response-PDU, connection-oriented

    APCI_BASIC_RESTART_PDU                      = 0x380, //!< A_Restart-PDU, Basic Restart, connection-oriented

    APCI_MASTER_RESET_PDU                       = 0x381, //!< A_Restart-PDU, Master Reset, connection-oriented
                                                         //!< used by Selfbus bootloader to reset into flash mode
                                                         //!< special meaning of eraseCode=T_RESTART_PDU_MASTERRESET_FACTORY_WO_IA and
                                                         //!< channelNumber=255 for bootloader mode
    APCI_MASTER_RESET_RESPONSE_PDU              = 0x3A1, //!< A_Restart_Response-PDU , connection-oriented


    // 0x3C0 -> 0x3D0 coupler specific?

    APCI_AUTHORIZE_REQUEST_PDU                  = 0x3d1, //!< A_Authorize_Request-PDU, connection-oriented
    APCI_AUTHORIZE_RESPONSE_PDU                 = 0x3d2, //!< A_Authorize_Response-PDU, connection-oriented

    APCI_KEY_WRITE_PDU                          = 0x3D3, //!< A_Key_Write-PDU, connection-oriented
    APCI_KEY_RESPONSE                           = 0x3D4, //!< A_Key_Response-PDU, connection-oriented

    APCI_PROPERTY_VALUE_READ_PDU                = 0x3d5, //!< A_PropertyValue_Read-PDU, connectionless and connection-oriented
    APCI_PROPERTY_VALUE_RESPONSE_PDU            = 0x3d6, //!< A_PropertyValue_Response-PDU, connectionless and connection-oriented
    APCI_PROPERTY_VALUE_WRITE_PDU               = 0x3d7, //!< A_PropertyValue_Write-PDU, connectionless and connection-oriented

    APCI_PROPERTY_DESCRIPTION_READ_PDU          = 0x3d8, //!< A_PropertyDescription_Read-PDU, connectionless and connection-oriented
    APCI_PROPERTY_DESCRIPTION_RESPONSE_PDU      = 0x3d9, //!< A_PropertyDescription_Response-PDU, connectionless and connection-oriented

    APCI_NETWORKPARAMETER_READ_PDU              = 0x3DA, //!< A_NetworkParameter_Read-PDU, broadcast
    APCI_NETWORKPARAMETER_RESPONSE_PDU          = 0x3DB, //!< A_NetworkParameter_Response-PDU, broadcast and connectionless

    APCI_INDIVIDUALADDRESS_SERIALNUMBER_READ_PDU        = 0x3DC, //!< A_IndividualAddressSerialNumber_Read-PDU, broadcast
    APCI_INDIVIDUALADDRESS_SERIALNUMBER_RESPONSE_PDU    = 0x3DD, //!< A_IndividualAddressSerialNumber_Response-PDU, broadcast
    APCI_INDIVIDUALADDRESS_SERIALNUMBER_WRITE_PDU       = 0x3DE, //!< A_IndividualAddressSerialNumber_Write-PDU, broadcast
};

/**
 * A_Restart-PDU Master Reset Erase Code
 * @details <br>see KNX spec 2.1 3/5/2 3.7.1.2.3.1 page 67</b>
 */
enum RestartPDUMasterReset : uint8_t
{
    T_MASTERRESET_CONFIRMED_RESTART = 0x01, //!< Confirmed restart
    T_MASTERRESET_FACTORY_RESET     = 0x02, //!< Factory reset, channel=0 all resources, channel!=0 only specified resources
    T_MASTERRESET_RESET_IA          = 0x03, //!< Reset physical address to its default (15.15.255)
    T_MASTERRESET_RESET_AP          = 0x04, //!< Reset to default application program
    T_MASTERRESET_RESET_PARAM       = 0x05, //!< Reset application parameter memory, channel=0 all channel, channel!=0 only specified channel
    T_MASTERRESET_RESET_LINKS       = 0x06, //!< Reset link information for group objects (address table, association table), channel=0 all channel, channel!=0 only specified channel
    T_MASTERRESET_FACTORY_WO_IA     = 0x07  //!< Factory Reset without physical address erase, channel=0 all channel, channel!=0 only specified channel
};

/**
 * A_Restart_Response-PDU Code
 * @details <br>see KNX spec 2.1 3/5/2 3.7.1.2.3.1 page 68</b>
 */
enum RestartPDUErrorcode
{
    T_RESTART_NO_ERROR               = 0x00,
    T_RESTART_ACCESS_DENIED          = 0x01,
    T_RESTART_UNSUPPORTED_ERASE_CODE = 0x02,
    T_RESTART_INVALID_CHANNEL_NUMBER = 0x03
};

#define BOOTLOADER_MAGIC_WORD (0x5E1FB055)      //!< magic word which will be checked on startup of the bootloader
                                                //!< weather or not to go into bootloader mode
#define BOOTLOADER_MAGIC_ADDRESS ((unsigned int *) 0x10000000) //!< magic address for the magic word to be checked on startup of the bootloader
                                                               //!< weather or not to go into bootloader mode
#define BOOTLOADER_MAGIC_ERASE T_MASTERRESET_FACTORY_WO_IA     //!< bootloader magic erase = FactoryResetWithoutIndividualAddress in calimero-core
#define BOOTLOADER_MAGIC_CHANNEL 255                           //!< bootloader magic channel

/**
 * @brief   Checks a APCI for the bus-updater Magic word
 * @details This is a bus-updater/bootloader and BCU special function
 *          used for the flashing process to boot into bootloader mode
 *
 * @param eraseCode     eraseCode of the @ref APCI_MASTER_RESET_PDU telegram
 * @param channelNumber channelNumber of the @ref APCI_MASTER_RESET_PDU telegram
 *
 * @return true if apci is a APCI_RESTART_TYPE1_PDU with magic word<br/>
 *         otherwise false
 */
bool checkApciForMagicWord(byte eraseCode, byte channelNumber);

ApciCommand apciCommand(unsigned char *telegram);
void setApciCommand(unsigned char *telegram, ApciCommand newApciCommand, byte additionalData);

uint8_t mainGroup(uint16_t address);
uint8_t middleGroup(uint16_t address);
uint8_t lowGroup(uint16_t address);

#endif /*sblib_apci_h*/
