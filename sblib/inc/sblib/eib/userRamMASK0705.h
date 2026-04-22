/*
 * userRam.h
 *
 *  Created on: 18.11.2021
 *      Author: dridders
 */

#ifndef SBLIB_KNX_USERRAM_MASK0705_H_
#define SBLIB_KNX_USERRAM_MASK0705_H_

#include <cstdint>
#include <sblib/eib/userRamMASK0701.h>

/**
 * The mask version 0x0705 user RAM
 */
class UserRamMASK0705 : public UserRamMASK0701
{
public:
    explicit UserRamMASK0705() : UserRamMASK0705(0x5FC, 0x304, 3) {}

protected:
    UserRamMASK0705(const uint32_t start, const uint32_t size, const uint32_t shadowSize)
        : UserRamMASK0701(start, size, shadowSize) {}
};

#endif /* SBLIB_KNX_USERRAM_MASK0705_H_ */
