/******************************************************************************
 * @addtogroup SBLIB_MAIN_GROUP Selfbus KNX-Library
 * @defgroup SBLIB_SUB_GROUP_KNX KNX LPDU Handling
 * @ingroup SBLIB_MAIN_GROUP
 * @brief
 * @details
 *
 *
 * @{
 *
 * @file   knx_lpdu.h
 * @author Darthyson <darth@maptrack.de> Copyright (c) 2022
 * @bug No known bugs.
 ******************************************************************************/

/*
 This program is free software; you can redistribute it and/or modify
 it under the terms of the GNU General Public License version 3 as
 published by the Free Software Foundation.
 ---------------------------------------------------------------------------*/

#ifndef SBLIB_KNX_LPDU_H_
#define SBLIB_KNX_LPDU_H_

#include <cstdint>


/**
 * Default physical address 15.15.255
 */
constexpr uint16_t PHY_ADDR_DEFAULT = 0xffff;

/**
 * Broadcast address 0.0.0
 */
constexpr uint16_t PHY_ADDR_BROADCAST = 0;

/**
 *  KNX Priority
 */
enum KNXPriority : uint8_t
{
    PRIORITY_SYSTEM = 0,
    PRIORITY_HIGH = 1,
    PRIORITY_ALARM = 2,
    PRIORITY_LOW = 3
};

/**
 *  KNX Frame format
 */
enum KNXFrameType
{
    FRAME_STANDARD,
    FRAME_EXTENDED
};

/**
 * Initialize a KNX LPDU telegram
 * @param telegram     Pointer to the telegram buffer
 * @param newPriority  Priority level for the telegram
 * @param newRepeated  Flag indicating if this is a repeated telegram
 * @param newFrameType Frame type (standard or extended)
 */
void initLpdu(uint8_t* telegram, KNXPriority newPriority, bool newRepeated, KNXFrameType newFrameType);

/**
 * Get the control byte from a KNX telegram
 * @param telegram Pointer to the telegram buffer
 * @return The control byte value
 */
uint8_t controlByte(const uint8_t* telegram);

/**
 * Check if a telegram is repeated
 * @param telegram Pointer to the telegram buffer
 * @return true if the telegram is repeated, false otherwise
 */
bool isRepeated(const uint8_t* telegram);

/**
 * Set the repeated flag in a telegram
 * @param telegram Pointer to the telegram buffer
 * @param repeated true to mark as repeated, false otherwise
 */
void setRepeated(uint8_t* telegram, bool repeated);

/**
 * Get the priority of a telegram
 * @param telegram Pointer to the telegram buffer
 * @return The priority level of the telegram
 */
KNXPriority priority(const uint8_t* telegram);

/**
 * Set the priority of a telegram
 * @param telegram Pointer to the telegram buffer
 * @param newPriority The new priority level to set
 */
void setPriority(uint8_t* telegram, KNXPriority newPriority);

/**
 * Get the sender address from a telegram
 * @param telegram Pointer to the telegram buffer
 * @return The sender's physical address
 */
uint16_t senderAddress(const uint8_t* telegram);

/**
 * Set the sender address in a telegram
 * @param telegram Pointer to the telegram buffer
 * @param newSenderAddress The sender's physical address to set
 */
void setSenderAddress(uint8_t* telegram, uint16_t newSenderAddress);

/**
 * Get the destination address from a telegram
 * @param telegram Pointer to the telegram buffer
 * @return The destination address (physical or group)
 */
uint16_t destinationAddress(const uint8_t* telegram);

/**
 * Set the destination address in a telegram
 * @param telegram Pointer to the telegram buffer
 * @param newDestinationAddress The destination address to set
 */
void setDestinationAddress(uint8_t* telegram, uint16_t newDestinationAddress);

/**
 * Get the frame type from a telegram
 * @param telegram Pointer to the telegram buffer
 * @return The frame type (standard or extended)
 */
KNXFrameType frameType(const uint8_t* telegram);

/**
 * Set the frame type in a telegram
 * @param telegram Pointer to the telegram buffer
 * @param newFrameType The frame type to set (standard or extended)
 */
void setFrameType(uint8_t* telegram, KNXFrameType newFrameType);

/**
 * Return the area number of a given physical KNX address
 * @param address Individual KNX address
 * @return The area number of the individual address
 */
uint8_t physAddressToArea(uint16_t address);

/**
 * Return the line number of a given physical KNX address
 * @param address Individual KNX address
 * @return The line number of the individual address
 */
uint8_t physAddressToLine(uint16_t address);

/**
 * Return the device number of a given physical KNX address
 * @param address Individual KNX address
 * @return The device number of the individual address
 */
uint8_t physAddressToDevice(uint16_t address);

/**
 * Return the main group of a given KNX group address
 * @param address Group KNX address
 * @return The main group of the group address
 */ 
uint8_t mainGroupAddress(uint16_t address);

/**
 * Return the middle group of a given KNX group address
 * @param address Group KNX address
 * @return The middle group of the group address
 */
uint8_t middleGroupAddress(uint16_t address);

/**
 * Return the low group of a given KNX group address
 * @param address Group KNX address
 * @return The low group of the group address
 */
uint8_t lowGroupAddress(uint16_t address);

#endif /* SBLIB_KNX_LPDU_H_ */
/** @}*/
