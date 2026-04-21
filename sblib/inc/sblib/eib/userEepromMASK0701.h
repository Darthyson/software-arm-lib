#ifndef SBLIB_KNX_USEREEPROM_MASK0701_H_
#define SBLIB_KNX_USEREEPROM_MASK0701_H_

#include <sblib/eib/userEepromBCU2.h>

/**
 * The BCU BIM M112 MaskVersion 0x0701 User EEPROM
 * @details Can be accessed by name, like userEeprom.manuDataH() and as an array, like
 *          userEeprom[addr]. Please note that the @ref startAddress is subtracted.
 *          E.g. userEeprom[0x107] is the correct address for userEeprom.version() not userEeprom[0x07].
 *
 * @note KNX Spec. 3.0 06 Profiles 4.2.10 User EEPROM p.41
 *       - User EEPROM BIM M112: 0x4000 - 0xcfff (36864 bytes / 36kB)
 */
class UserEepromMASK0701 : public UserEepromBCU2
{
public:
    ///\todo make start at 0x4000, right now 0x4000-0x100= 0x3f00 is chosen to avoid address-offset calculations to a BCU1
    UserEepromMASK0701() : UserEepromBCU2(0x3f00, 3072, 4096) {}

protected: ///\todo Access specifier does not change accessibility level
    UserEepromMASK0701(const unsigned int start, const unsigned int size, const unsigned int flashSize)
       :
        UserEepromBCU2(start, size, flashSize) {}
};

#endif /* SBLIB_KNX_USEREEPROM_MASK0701_H_ */
