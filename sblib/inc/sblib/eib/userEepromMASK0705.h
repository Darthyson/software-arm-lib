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
    explicit UserEepromMASK0705() : UserEepromMASK0705(0x3f00, 3072, 4096) {}

protected: ///\todo Access specifier does not change accessibility level
    // BIM M112 (Mask 0x0705) has a fixed start and size of the EEPROM, so make constructor protected
    UserEepromMASK0705(const uint32_t start, const uint32_t size, const uint32_t flashSize)
       :
        UserEepromMASK0701(start, size, flashSize) {}
};

#endif /* SBLIB_KNX_USEREEPROM_MASK0705_H_ */
