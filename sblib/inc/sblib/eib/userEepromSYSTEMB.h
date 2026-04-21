#ifndef SBLIB_KNX_USEREEPROM_SYSTEMB_H_
#define SBLIB_KNX_USEREEPROM_SYSTEMB_H_

#include <sblib/eib/userEepromMASK0701.h>

/**
 * The BCU SYSTEM B user EEPROM
 */
class UserEepromSYSTEMB : public UserEepromMASK0701
{
public:
    ///\todo check start of 0x3300, maybe the same reason like for 0x0701 -> -0x100 is chosen to avoid address-offset calculations to a BCU1
    explicit UserEepromSYSTEMB() : UserEepromSYSTEMB(0x3300, 3072, 4096) {}

    static constexpr int addrTabAddrOffset = 0x21;  //!< 0x3321-0x3322
    static constexpr int assocTabAddrOffset = 0x23; //!< 0x3323-0x3324

    static constexpr int addrTabMcbOffset = 0x4D;   //!< 0x334D-0x3354
    static constexpr int assocTabMcbOffset = 0x55;  //!< 0x3355-0x335C
    static constexpr int commsTabMcbOffset = 0x5D;  //!< 0x335D-0x3364
    static constexpr int eibObjMcbOffset = 0x65;    //!< 0x3365-0x336C
    static constexpr int commsSeg0McbOffset = 0x6D; //!< 0x336D-0x3374
    static constexpr int eibObjVerOffset = 0x75;    //!< 0x3375-0x3379 Application program 1 version
    static constexpr int commsSeg0VerOffset = 0x7A; //!< 0x337A-0x337E Application program 2 version

    [[nodiscard]] uint16_t& addrTabAddr() const override { return *(uint16_t*)&userEepromData[addrTabAddrOffset]; }
    [[nodiscard]] uint16_t& assocTabAddr() const override { return *(uint16_t*)&userEepromData[assocTabAddrOffset]; }
    [[nodiscard]] virtual uint8_t* addrTabMcb() const { return &userEepromData[addrTabMcbOffset]; }
    [[nodiscard]] virtual uint8_t* assocTabMcb() const { return &userEepromData[assocTabMcbOffset]; }
    [[nodiscard]] virtual uint8_t* commsTabMcb() const { return &userEepromData[commsTabMcbOffset]; }
    [[nodiscard]] virtual uint8_t* commsSeg0Mcb() const { return &userEepromData[commsSeg0McbOffset]; }
    [[nodiscard]] virtual uint8_t* eibObjMcb() const { return &userEepromData[eibObjMcbOffset]; }

protected:
    UserEepromSYSTEMB(const uint32_t start, const uint32_t size, const uint32_t flashSize)
       :
        UserEepromMASK0701(start, size, flashSize) {}
};

#endif /* SBLIB_KNX_USEREEPROM_SYSTEMB_H_ */
