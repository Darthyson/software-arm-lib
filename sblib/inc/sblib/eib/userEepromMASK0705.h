#ifndef SBLIB_KNX_USEREEPROM_MASK0705_H_
#define SBLIB_KNX_USEREEPROM_MASK0705_H_

#include <sblib/eib/userEepromMASK0701.h>

/**
 * The BCU BIM112 MaskVersion 0x0705 user EEPROM
 */
class UserEepromMASK0705 : public UserEepromMASK0701
{
public:
    ///\todo make start at 0x4000, right now 0x4000-0x100= 0x3f00 is chosen to avoid address-offset calculations to a BCU1
    UserEepromMASK0705() : UserEepromMASK0701(0x3f00, 3072, 4096) {}

protected:
    UserEepromMASK0705(const unsigned int start, const unsigned int size, const unsigned int flashSize) : UserEepromMASK0701(start, size, flashSize) {};
};

#endif /* SBLIB_KNX_USEREEPROM_MASK0705_H_ */
