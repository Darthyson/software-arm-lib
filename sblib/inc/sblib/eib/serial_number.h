/*
 *  KNX serial number generation.
 *
 *  Copyright (c) 2026 Darthyson <darth@maptrack.de>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 3 as
 *  published by the Free Software Foundation.
 */

#ifndef SBLIB_KNX_SERIAL_NUMBER_H
#define SBLIB_KNX_SERIAL_NUMBER_H

#include <cstdint>


/** The size of the KNX serial number in bytes. **/
constexpr uint8_t KNX_SERIAL_NUMBER_LENGTH = 6;

/** The high byte of the KNX manufacturer ID of the serial number. **/
constexpr uint8_t KNX_SERIAL_NUMBER_MANUFACTURER_ID_HIGH_BYTE = 0x01;

/** The low byte of the KNX manufacturer ID of the serial number. **/
constexpr uint8_t KNX_SERIAL_NUMBER_MANUFACTURER_ID_LOW_BYTE = 0x3A;

/**
 * Creates a KNX serial number based on the provided data buffer.
 *
 * @param data Data buffer to use for serial number creation
 * @param dataSize Size of the data buffer
 * @param knxSerial Buffer for generated serial number
 * @param knxSerialSize Size of serial number buffer (always 6 bytes)
 * @return True if KNX serial number was created successfully, otherwise false.
 */
bool createKNXSerial(const uint8_t* data, uint8_t dataSize, uint8_t* knxSerial, uint8_t knxSerialSize);

/**
 * Creates a len_hash wide hash of the uid.
 * Hash will be generated in provided hash buffer
 *
 * @param uid LPC-serial (128bit GUID) returned by iapReadUID() which will be hashed
 * @param len_uid size of uid  (normally 16 byte)
 * @param hash buffer for generated hash
 * @param len_hash size of provided hash buffer (normally 6byte/48bit for EIB)
 * @return True if hash successfully created, false if not.
 */
//todo delete or mark as [[deprecated("Use createKNXSerial() instead")]]
bool hashUID(const uint8_t* uid, int8_t len_uid, uint8_t* hash, int8_t len_hash);

#endif //SBLIB_KNX_SERIAL_NUMBER_H
