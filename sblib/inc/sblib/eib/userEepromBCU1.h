#ifndef sblib_usereeprom_bcu1_h
#define sblib_usereeprom_bcu1_h

#include <sblib/eib/userEeprom.h>

/**
 * The BCU 1 user EEPROM
 * @details Can be accessed by name, like userEeprom.manuDataH() and as an array, like
 *          userEeprom[addr]. Please note that the @ref startAddress is subtracted.
 *          E.g. userEeprom[0x107] is the correct address for userEeprom.version() not userEeprom[0x07].
 *
 * @note see KNX Spec. 2.1
 *       - BCU 1 (256 bytes) : 9/4/1 3.1.10.3.1 p.13ff
 */
class UserEepromBCU1 : public UserEeprom
{
public:
    UserEepromBCU1() : UserEeprom(0x100, 256, 256) {}

    static const int optionRegOffset = 0x00;         //!< 0x0100: EEPROM option register
    static const int manuDataHOffset = 0x01;         //!< 0x0101: Manufacturing data high byte
    static const int manuDataLOffset = 0x02;         //!< 0x0102: Manufacturing data low byte
    static const int manufacturerHOffset = 0x03;     //!< 0x0103: Software manufacturer high byte
    static const int manufacturerLOffset = 0x04;     //!< 0x0104: Software manufacturer low byte
    static const int deviceTypeHOffset = 0x05;       //!< 0x0105: Device type high byte
    static const int deviceTypeLOffset = 0x06;       //!< 0x0106: Device type low byte
    static const int versionOffset = 0x07;           //!< 0x0107: Software version
    static const int checkLimitOffset = 0x08;        //!< 0x0108: EEPROM check limit
    static const int appPeiTypeOffset = 0x09;        //!< 0x0109: PEI type that the application program requires
    static const int syncRateOffset = 0x0a;          //!< 0x010a: Baud rate for serial synchronous PEI
    static const int portCDDROffset = 0x0b;          //!< 0x010b: Port C DDR settings (PEI type 17)
    static const int portADDROffset = 0x0c;          //!< 0x010c: Port A DDR settings
    static const int runErrorOffset = 0x0d;          //!< 0x010d: Runtime error flags
    static const int routeCntOffset = 0x0e;          //!< 0x010e: Routing count constant
    static const int maxRetransmitOffset = 0x0f;     //!< 0x010f: INAK and BUSY retransmit limit
    static const int confDescOffset = 0x10;          //!< 0x0110: Configuration descriptor
    static const int assocTabPtrOffset = 0x11;       //!< 0x0111: Pointer to association table
    static const int commsTabPtrOffset = 0x12;       //!< 0x0112: Pointer to communication objects table
    static const int usrInitPtrOffset = 0x13;        //!< 0x0113: Pointer to user initialization function
    static const int usrProgPtrOffset = 0x14;        //!< 0x0114: Pointer to user program function
    static const int usrSavePtrOffset = 0x15;        //!< 0x0115: Pointer to user save function (BCU1 only)
    static const int addrTabSizeOffset = 0x16;       //!< 0x0116: Size of the address table
    static const int addrTabOffset = 0x17;           //!< 0x0117+: Address table, 2 bytes per entry. Real array size is addrTabSize*2
    static const int user230bytesStartOffset = 0x19; //!< 0x0119: User EEPROM: 230 bytes (BCU1)
    static const int checksumOffset = 0xff;          //!< 0x01ff: EEPROM checksum (BCU1 only)

    byte& optionReg() const override { return userEepromData[optionRegOffset]; }
    byte& manuDataH() const override { return userEepromData[manuDataHOffset]; }
    byte& manuDataL() const override { return userEepromData[manuDataLOffset]; }
    byte& manufacturerH() const override { return userEepromData[manufacturerHOffset]; }
    byte& manufacturerL() const override { return userEepromData[manufacturerLOffset]; }
    byte& deviceTypeH() const override { return userEepromData[deviceTypeHOffset]; }
    byte& deviceTypeL() const override { return userEepromData[deviceTypeLOffset]; }
    byte& version() const override { return userEepromData[versionOffset]; }
    byte& checkLimit() const override { return userEepromData[checkLimitOffset]; }
    byte& appPeiType() const override { return userEepromData[appPeiTypeOffset]; }
    byte& syncRate() const override { return userEepromData[syncRateOffset]; }
    byte& portCDDR() const override { return userEepromData[portCDDROffset]; }
    byte& portADDR() const override { return userEepromData[portADDROffset]; }
    byte& runError() const override { return userEepromData[runErrorOffset]; }
    byte& routeCnt() const override { return userEepromData[routeCntOffset]; }
    byte& maxRetransmit() const override { return userEepromData[maxRetransmitOffset]; }
    byte& confDesc() const override { return userEepromData[confDescOffset]; }
    byte& assocTabPtr() const override { return userEepromData[assocTabPtrOffset]; }
    byte& commsTabPtr() const override { return userEepromData[commsTabPtrOffset]; };
    byte& usrInitPtr() const override { return userEepromData[usrInitPtrOffset]; }
    byte& usrProgPtr() const override { return userEepromData[usrProgPtrOffset]; }
    virtual byte& usrSavePtr() const { return userEepromData[usrSavePtrOffset]; }
    byte& addrTabSize() const override { return userEepromData[addrTabSizeOffset]; }
    byte* addrTab() const override { return &userEepromData[addrTabOffset]; }
    virtual byte* user230bytesStart() const { return &userEepromData[user230bytesStartOffset]; }
    virtual byte& checksum() const { return userEepromData[checksumOffset]; }

protected:
    UserEepromBCU1(unsigned int start, unsigned int size, unsigned int flashSize) : UserEeprom(start, size, flashSize) {};
};

#endif /*sblib_usereeprom_bcu1_h*/
