/*
 *  KNX serial number generation.
 *
 *  Copyright (c) 2026 Darthyson <darth@maptrack.de>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 3 as
 *  published by the Free Software Foundation.
 */

#include "sblib/eib/serial_number.h"
#include "sblib/murmur_hash_3.h"
#include <utility>
#include <cstring>


bool createKNXSerial(const uint8_t* data, const uint8_t dataSize, uint8_t* knxSerial,
    const uint8_t knxSerialSize)
{
    if ((dataSize == 0) || (knxSerialSize != KNX_SERIAL_NUMBER_LENGTH))
    {
        return false;
    }

    if ((data == nullptr) || (knxSerial == nullptr))
    {
        return false;
    }

    // Set Manufacturer ID to "Not Assigned"
    knxSerial[0] = KNX_SERIAL_NUMBER_MANUFACTURER_ID_HIGH_BYTE;
    knxSerial[1] = KNX_SERIAL_NUMBER_MANUFACTURER_ID_LOW_BYTE;

    // Calculate and set 32bit MurmurHash3 based on provided data
    murmurHash3_x86_32(data, dataSize, 0, &knxSerial[2]);

    // swap to match ETS view of serial with MurmurHash3 calculators
    std::swap(knxSerial[2], knxSerial[5]);
    std::swap(knxSerial[3], knxSerial[4]);
    return true;
}

bool hashUID(const uint8_t* uid, const int8_t len_uid, uint8_t* hash, const int8_t len_hash)
{
    constexpr uint8_t MAX_HASH_WIDE = 16;
    constexpr uint64_t BigPrime48 = 281474976710597u; // FF FF FF FF FF C5
    uint64_t a, b;

    if ((len_uid <= 0) || (len_uid > MAX_HASH_WIDE)) // maximum of 16 bytes can be hashed by this function
        return false;
    if ((len_hash <= 0) || (len_hash > len_uid))
        return false;

    const unsigned int mid = len_uid / 2;
    memcpy(&a, &uid[0], mid);             // copy first half of uid-bytes to a
    memcpy(&b, &uid[mid], len_uid - mid); // copy second half of uid-bytes to b

    // do some modulo a big primenumber
    a = a % BigPrime48;
    b = b % BigPrime48;
    a = a ^ b;
    // copy the generated hash to provided buffer
    for (int i = 0; i < len_hash; i++)
        hash[i] = a >> 8 * i & 0xFF;
    return true;
}
