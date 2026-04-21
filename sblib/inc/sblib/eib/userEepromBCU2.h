#ifndef SBLIB_KNX_USEREEPROM_BCU2_H_
#define SBLIB_KNX_USEREEPROM_BCU2_H_

#include <sblib/eib/userEepromBCU1.h>

/**
 * The BCU 2 user EEPROM
 * @details Can be accessed by name, like userEeprom.manuDataH() and as an array, like
 *          userEeprom[addr]. Please note that the @ref startAddress is subtracted.
 *          E.g. userEeprom[0x107] is the correct address for userEeprom.version() not userEeprom[0x07].
 *
 * @note KNX Spec. 3.0 9/4/1 Basic and System Components 5.1.2.12 Memory Map p.43ff
 *       - EEPROM: 0x100 - 0x4df (992 bytes)
 *       - single chip protected EEPROM: 0x4e0 - 0x4ff (32 bytes)
 *       - total: 0x100 - 0x4ff (1024 bytes)
 * @note KNX Spec. 3.0 06 Profiles 4.2.10 User EEPROM p. 41
 *       - User EEPROM: 0x119 - 0x46f (855 bytes)
 */
class UserEepromBCU2 : public UserEepromBCU1
{
public:
    /// The total BCU2 EEPROM range is 0x100-0x4ff (System EEPROM, single chip protected EEPROM, User EEPROM combined).
    /// UserEepromBCU2 is implemented with this total BCU2 EEPROM range, which is fine,
    /// but exposes BCU2 system EEPROM to the user, which is not great.

    explicit UserEepromBCU2() : UserEepromBCU2(0x100, 1024, 1024) {}

    static constexpr int appTypeOffset = 0x015; //!< 0x0115: \todo Application program type: 0=BCU2, else BCU1

    static constexpr int loadStateOffset = 0x370;      //!< 0x0470: Load state of the system interface objects
    static constexpr int addrTabAddrOffset = 0x378;    //!< 0x0478: Address of the address table
    static constexpr int assocTabAddrOffset = 0x37A;   //!< 0x047a: Address of the association table
    static constexpr int commsTabAddrOffset = 0x37C;   //!< 0x047c: Address of the communication object table
    static constexpr int commsSeg0AddrOffset = 0x37E;  //!< 0x047e: Address of communication object memory segment 0 // TODO needs implementation, see handleTaskCtrl2(...) in properties.cpp
    static constexpr int commsSeg1AddrOffset = 0x380;  //!< 0x0480: Address of communication object memory segment 1 // TODO needs implementation, see handleTaskCtrl2(...) in properties.cpp
    static constexpr int eibObjAddrOffset = 0x382;     //!< 0x0482: Address of the application program EIB objects, 0 if unused. // TODO needs implementation, see handleTaskCtrl1(...) in properties.cpp
    static constexpr int eibObjCountOffset = 0x384;    //!< 0x0484: Number of application program EIB objects. // TODO needs implementation, see handleTaskCtrl1(...) in properties.cpp
    static constexpr int padding1Offset = 0x385;       //!< 0x0485: Padding 1
    static constexpr int serviceControlOffset = 0x386; //!< 0x0486: Service control
    static constexpr int padding2Offset = 0x388;       //!< 0x0488: Padding 2
    static constexpr int serialOffset = 0x38A;         //!< 0x048A-0x48f: Hardware serial number (4 byte aligned)

    static constexpr int orderOffset = 0x390;     //!< 0x0490-0x0499: Hardware Type
    static constexpr int orderInfoOffset = 0x39A; //!< 0x049A-0x04A3: Ordering information

    ///\todo why here, isn't this system b specific?
    static constexpr int addrTabMcbOffset = 0x3A4;   //!< 0x04A4-0x04AB:
    static constexpr int assocTabMcbOffset = 0x3AC;  //!< 0x04AC-0x04B3:
    static constexpr int commsTabMcbOffset = 0x3B4;  //!< 0x04B4-0x04BB:
    static constexpr int eibObjMcbOffset = 0x3BC;    //!< 0x04BC-0x04C3:
    static constexpr int commsSeg0McbOffset = 0x3C3; //!< 0x04C4-0x04CB: ///\todo address missmatch?
    static constexpr int eibObjVerOffset = 0x3CC;    //!< 0x04CC-0x04D0: Application program 1 version
    static constexpr int commsSeg0VerOffset = 0x3D1; //!< 0x04D1-0x04D5: Application program 2 version
    // end todo

    [[nodiscard]] virtual int serialSize() const { return 6; };
    [[nodiscard]] virtual int orderSize() const { return 10; };
    [[nodiscard]] virtual int orderInfoSize() const { return 10; };

    [[nodiscard]] virtual uint8_t& appType() const { return userEepromData[appTypeOffset]; }
    [[nodiscard]] virtual uint8_t* loadState() const { return &userEepromData[loadStateOffset]; }
    [[nodiscard]] virtual uint16_t& commsTabAddr() const { return *(uint16_t*)&userEepromData[commsTabAddrOffset]; }
    [[nodiscard]] virtual uint16_t& commsSeg0Addr() const { return *(uint16_t*)&userEepromData[commsSeg0AddrOffset]; }
    [[nodiscard]] virtual uint16_t& commsSeg1Addr() const { return *(uint16_t*)&userEepromData[commsSeg1AddrOffset]; }
    [[nodiscard]] virtual uint16_t& eibObjAddr() const { return *(uint16_t*)&userEepromData[eibObjAddrOffset]; }
    [[nodiscard]] virtual uint8_t& eibObjCount() const { return userEepromData[eibObjCountOffset]; }
    [[nodiscard]] virtual uint16_t& addrTabAddr() const { return *(uint16_t*)&userEepromData[addrTabAddrOffset]; }
    [[nodiscard]] virtual uint16_t& assocTabAddr() const { return *(uint16_t*)&userEepromData[assocTabAddrOffset]; }
    [[nodiscard]] virtual uint8_t* serial() const { return &userEepromData[serialOffset]; }
    [[nodiscard]] virtual uint8_t* order() const { return &userEepromData[orderOffset]; }

    [[nodiscard]] virtual uint8_t& padding1() const { return userEepromData[padding1Offset]; }
    [[nodiscard]] virtual uint8_t& serviceControl() const { return userEepromData[serviceControlOffset]; }
    [[nodiscard]] virtual uint8_t& padding2() const { return userEepromData[padding2Offset]; }
    [[nodiscard]] virtual uint8_t* orderInfo() const { return &userEepromData[orderInfoOffset]; }

protected:
    // BCU2 has no variable EEPROM start or size, so make this constructor protected
    UserEepromBCU2(const uint32_t start, const uint32_t size, const uint32_t flashSize)
       :
        UserEepromBCU1(start, size, flashSize) {}
};

#endif /* SBLIB_KNX_USEREEPROM_BCU2_H_ */
