#ifndef SBLIB_KNX_USEREEPROM_BCU1_H_
#define SBLIB_KNX_USEREEPROM_BCU1_H_

#include <sblib/eib/userEeprom.h>


/**
 * The BCU 1 user EEPROM
 * @details Can be accessed by name, like userEeprom.manuDataH() and as an array, like
 *          userEeprom[addr]. Please note that the @ref startAddress is subtracted.
 *          E.g. userEeprom[0x107] is the correct address for userEeprom.version() not userEeprom[0x07].
 *
 * @note KNX Spec. 3.0 9/4/1 Basic and System Components 3.1.10.3.1 p.13ff
 *       - EEPROM: 0x100 - 0x1ff (256 bytes)
 * @note KNX Spec. 3.0 06 Profiles 4.2.10 User EEPROM p. 41
 *       - User EEPROM: 0x119 - 0x1fe (230 bytes)
 */
class UserEepromBCU1 : public UserEeprom
{
public:
    // The total BCU1 EEPROM range is 0x100-0x1ff (System EEPROM and User EEPROM combined).
    // UserEepromBCU1 is implemented with this total BCU1 EEPROM range, which is fine,
    // but exposes BCU1 system EEPROM to the user, which is not great.
    explicit UserEepromBCU1() : UserEeprom(0x100, 256, 256) {}

    static constexpr uint32_t optionRegOffset = 0x00;         //!< 0x0100: EEPROM option register
    static constexpr uint32_t manuDataHOffset = 0x01;         //!< 0x0101: Manufacturing data high byte
    static constexpr uint32_t manuDataLOffset = 0x02;         //!< 0x0102: Manufacturing data low byte
    static constexpr uint32_t manufacturerHOffset = 0x03;     //!< 0x0103: Software manufacturer high byte
    static constexpr uint32_t manufacturerLOffset = 0x04;     //!< 0x0104: Software manufacturer low byte
    static constexpr uint32_t deviceTypeHOffset = 0x05;       //!< 0x0105: Device type high byte
    static constexpr uint32_t deviceTypeLOffset = 0x06;       //!< 0x0106: Device type low byte
    static constexpr uint32_t versionOffset = 0x07;           //!< 0x0107: Software version
    static constexpr uint32_t checkLimitOffset = 0x08;        //!< 0x0108: EEPROM check limit
    static constexpr uint32_t appPeiTypeOffset = 0x09;        //!< 0x0109: PEI type that the application program requires
    static constexpr uint32_t syncRateOffset = 0x0a;          //!< 0x010a: Baud rate for serial synchronous PEI
    static constexpr uint32_t portCDDROffset = 0x0b;          //!< 0x010b: Port C DDR settings (PEI type 17)
    static constexpr uint32_t portADDROffset = 0x0c;          //!< 0x010c: Port A DDR settings
    static constexpr uint32_t runErrorOffset = 0x0d;          //!< 0x010d: Runtime error flags
    static constexpr uint32_t routeCntOffset = 0x0e;          //!< 0x010e: Routing count constant
    static constexpr uint32_t maxRetransmitOffset = 0x0f;     //!< 0x010f: NAK and BUSY retransmit limit
    static constexpr uint32_t confDescOffset = 0x10;          //!< 0x0110: Configuration descriptor
    static constexpr uint32_t assocTabPtrOffset = 0x11;       //!< 0x0111: Pointer to association table
    static constexpr uint32_t commsTabPtrOffset = 0x12;       //!< 0x0112: Pointer to communication objects table
    static constexpr uint32_t usrInitPtrOffset = 0x13;        //!< 0x0113: Pointer to user initialization function
    static constexpr uint32_t usrProgPtrOffset = 0x14;        //!< 0x0114: Pointer to user program function
    static constexpr uint32_t usrSavePtrOffset = 0x15;        //!< 0x0115: Pointer to user save function (BCU1 only)
    static constexpr uint32_t addrTabSizeOffset = 0x16;       //!< 0x0116: Size of the address table
    static constexpr uint32_t addrTabOffset = 0x17;           //!< 0x0117+: Address table, 2 bytes per entry. Real array size is addrTabSize*2
    static constexpr uint32_t user230bytesStartOffset = 0x19; //!< 0x0119: User EEPROM: 230 bytes (BCU1)
    static constexpr uint32_t checksumOffset = 0xff;          //!< 0x01ff: EEPROM checksum (BCU1 only)

    [[nodiscard]] uint8_t& optionReg() const override { return userEepromData[optionRegOffset]; }
    [[nodiscard]] uint8_t& manuDataH() const override { return userEepromData[manuDataHOffset]; }
    [[nodiscard]] uint8_t& manuDataL() const override { return userEepromData[manuDataLOffset]; }
    [[nodiscard]] uint8_t& manufacturerH() const override { return userEepromData[manufacturerHOffset]; }
    [[nodiscard]] uint8_t& manufacturerL() const override { return userEepromData[manufacturerLOffset]; }
    [[nodiscard]] uint8_t& deviceTypeH() const override { return userEepromData[deviceTypeHOffset]; }
    [[nodiscard]] uint8_t& deviceTypeL() const override { return userEepromData[deviceTypeLOffset]; }
    [[nodiscard]] uint8_t& version() const override { return userEepromData[versionOffset]; }
    [[nodiscard]] uint8_t& checkLimit() const override { return userEepromData[checkLimitOffset]; }
    [[nodiscard]] uint8_t& appPeiType() const override { return userEepromData[appPeiTypeOffset]; }
    [[nodiscard]] uint8_t& syncRate() const override { return userEepromData[syncRateOffset]; }
    [[nodiscard]] uint8_t& portCDDR() const override { return userEepromData[portCDDROffset]; }
    [[nodiscard]] uint8_t& portADDR() const override { return userEepromData[portADDROffset]; }
    [[nodiscard]] uint8_t& runError() const override { return userEepromData[runErrorOffset]; }
    [[nodiscard]] uint8_t& routeCnt() const override { return userEepromData[routeCntOffset]; }
    [[nodiscard]] uint8_t& maxRetransmit() const override { return userEepromData[maxRetransmitOffset]; }
    [[nodiscard]] uint8_t& confDesc() const override { return userEepromData[confDescOffset]; }
    [[nodiscard]] uint8_t& assocTabPtr() const override { return userEepromData[assocTabPtrOffset]; }
    [[nodiscard]] uint8_t& commsTabPtr() const override { return userEepromData[commsTabPtrOffset]; };
    [[nodiscard]] uint8_t& usrInitPtr() const override { return userEepromData[usrInitPtrOffset]; }
    [[nodiscard]] uint8_t& usrProgPtr() const override { return userEepromData[usrProgPtrOffset]; }
    [[nodiscard]] virtual uint8_t& usrSavePtr() const { return userEepromData[usrSavePtrOffset]; }
    [[nodiscard]] uint8_t& addrTabSize() const override { return userEepromData[addrTabSizeOffset]; }
    [[nodiscard]] uint8_t* addrTab() const override { return &userEepromData[addrTabOffset]; }
    [[nodiscard]] virtual uint8_t* user230bytesStart() const { return &userEepromData[user230bytesStartOffset]; }
    [[nodiscard]] virtual uint8_t& checksum() const { return userEepromData[checksumOffset]; }

protected:
    // BCU1 has no variable EEPROM start or size, so make this constructor protected
    UserEepromBCU1(const uint32_t start, const uint32_t size, const uint32_t flashSize)
       :
        UserEeprom(start, size, flashSize) {}
};

#endif /* SBLIB_KNX_USEREEPROM_BCU1_H_ */
