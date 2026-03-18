/**************************************************************************//**
 * @addtogroup SBLIB_BOOTLOADER Selfbus Bootloader
 * @defgroup SBLIB_KNX_TELEGRAM_HANDLER_1 KNX-Telegramhandler
 * @ingroup SBLIB_BOOTLOADER
 * @brief   Handles @ref APCI_USERMSG_MANUFACTURER_0 and @ref APCI_USERMSG_MANUFACTURER_6 for the update process
 * @details
 *
 * @{
 *
 * @file   update.cpp
 * @author Martin Glueck <martin@mangari.org> Copyright (c) 2015
 * @author Stefan Haller Copyright (c) 2021
 * @author Darthyson <darth@maptrack.de> Copyright (c) 2026
 ******************************************************************************/

/*
 This program is free software; you can redistribute it and/or modify
 it under the terms of the GNU General Public License version 3 as
 published by the Free Software Foundation.
 -----------------------------------------------------------------------------*/

#include "update.h"
#include "upd_protocol.h"
#include "flash.h"
#include "bcu_updater.h"
#include "crc.h"
#include "dump.h"
#include "version.h"
#include <sblib/digital_pin.h>
#include <sblib/bits.h>
#include <sblib/eib/apci.h>
#include <sblib/internal/iap.h>
#include <sblib/version.h>
#include <cstring>

#if defined(DEBUG)
#   include "intelhex.h"
#   include <sblib/serial.h>
#   include <sblib/io_pin_names.h>
#endif

#ifdef DECOMPRESSOR
#   include "decompressor.h"
#endif


#ifdef DECOMPRESSOR
    static Decompressor decompressor(reinterpret_cast<AppDescriptionBlock*>(bootDescriptorBlockAddress())); //!< get application base address from boot descriptor
#endif

constexpr uint32_t DEVICE_LOCKED = 0x5AA55AA5;       //!< Magic number for the device is locked and can't be flashed.
constexpr uint32_t DEVICE_UNLOCKED = ~DEVICE_LOCKED; //!< Magic number for the device is unlocked and flashing is allowed.

/**
 * Size in bytes of the RAM buffer.
 * @details 1265 bytes chosen to fit @ref UPD_SEND_DATA of 5 extended frames<br>
 *          Maximum extended frame length is 254 bytes - 1 byte for the @ref UPD_Command totaling in 1265 bytes
 */
constexpr uint16_t bufferSize = 1265;
alignas(FLASH_RAM_BUFFER_ALIGNMENT) static uint8_t ramBuffer[bufferSize]; //!< RAM buffer used for flash operations
static uint8_t * retTelegram = nullptr;                  //!< pointer to return buffer, as a field for easier access and smaller code size

// Try to avoid direct access to these global variables.
// It's better to use their get, set and reset functions
static uint32_t deviceLocked = DEVICE_LOCKED;   //!< current device locking state @note Better use GetDeviceUnlocked() & setDeviceLockState()
static uint16_t ramBufferPositon = 0;           //!< current number of bytes cached in @ref ramBuffer
static uint16_t totalBytesReceived = 0;         //!< number of bytes received by @ref UPD_SEND_DATA since reset()
static uint16_t totalBytesFlashed = 0;          //!< number of bytes flashed by @ref UPD_PROGRAM since reset()

extern BcuUpdate bcu;

/**
 * Converts an uint32_t into 4 bytes long provided buffer
 * @details A direct cast does not work due to possible misaligned addresses.
 *          Therefore, a good old conversion has to be performed
 *
 * @param buffer in the 4 first bytes of the buffer the result will be stored
 * @param value  the uint32_t to be converted
 * @warning function doesn't perform any sanity-checks on the provided buffer
 */
void uInt32ToStream(uint8_t * buffer, uint32_t value);

/**
 * Send the flash content from startAddress to endAddress
 * in Intel(R) hex file format over serial port.
 *
 * @param startAddress The start address of the flash content to dump.
 * @param endAddress   The end address of the flash content to dump.
 */
#if defined(DEBUG)
void dumpFlashContent(uint8_t * startAddress, uint8_t * endAddress)
{
    if (startAddress > endAddress)
    {
        uint8_t * temp = startAddress;
        startAddress = endAddress;
        endAddress = temp;
    }

    if (startAddress < flashFirstAddress())
    {
        startAddress = flashFirstAddress();
    }

    if (endAddress > flashLastAddress())
    {
        endAddress = flashLastAddress();
    }

    dumpToSerialInIntelHex(&serial, startAddress, static_cast<uint16_t>(endAddress - startAddress + 1));
}
#endif

/**
 * Converts 4 bytes long provided buffer into an uint32_t
 * @details A direct cast does not work due to possible misaligned addresses.
 *          Therefore, a good old conversion has to be performed
 *
 * @param buffer data to convert
 * @return The converted uint32_t value of the 4 first bytes of the buffer
 * @warning function doesn't perform any sanity-checks on the provided buffer
 */
uint32_t streamToUIn32(const uint8_t * buffer)
{
    return static_cast<uint32_t>(buffer[3] << 24 | buffer[2] << 16 | buffer[1] << 8 | buffer[0]);
}

/**
 * @brief Converts 4 bytes long provided buffer into a pointer
 *
 * @param buffer data to convert
 * @return first 4 bytes of buffer converted to a pointer
 * @warning function doesn't perform any sanity-checks on the provided buffer
 */
uint8_t * streamToPtr(const uint8_t * buffer)
{
    return FLASH_BASE_ADDRESS + streamToUIn32(buffer);
}

///\todo implement universal numberToStream by providing the size as parameter
///      and delete uInt32ToStream + uShort16ToStream
void uInt32ToStream(uint8_t * buffer, const uint32_t value)
{
    buffer[3] = static_cast<uint8_t>(value >> 24);
    buffer[2] = static_cast<uint8_t>(value >> 16);
    buffer[1] = static_cast<uint8_t>(value >> 8);
    buffer[0] = static_cast<uint8_t>(value & 0xff);
}

/**
 * @brief Converts a pointer to a 4 byte uint32_t and writes it to the buffer
 *
 * @param buffer memory area to receive the converted value
 * @param value pointer to convert and write
 * @warning function doesn't perform any sanity-checks on the provided buffer
 */
void ptrToStream(uint8_t * buffer, const uint8_t * value)
{
    uInt32ToStream(buffer, static_cast<uint32_t>(value - FLASH_BASE_ADDRESS));
}

void uShort16ToStream(uint8_t * buffer, const uint16_t val)
{
    buffer[1] = static_cast<uint8_t>(val >> 8);
    buffer[0] = static_cast<uint8_t>(val & 0xff);
}

uint16_t streamToUShort16(const uint8_t * buffer)
{
    return static_cast<uint16_t>(buffer[1] << 8 | buffer[0]);
}

/**
 * Prepares a UPD/UDP telegram with command/response and the number of bytes
 *        the return telegram will have.
 *
 * @param count Number of bytes the return telegram shall have
 * @param cmd   UPD/UDP command/response to set the return telegram
 */
void prepareReturnTelegram(uint8_t count, const UPD_Code cmd)
{
    count += 2; // +1 byte because counting starts including retTelegram[7] (KNX Spec 2.1 3/3/3 2.1 NPDU p.6)
                // +1 byte for cmd (retTelegram[8])
    if (count > 0x0f) ///\todo maybe won't work with extended frames
    {
#ifdef DEBUG
        fatalError();
#else
        count &= 0x0f; // make sure not to mess up with routing count in release-build
#endif
    }
    retTelegram[5] = 0x60 + count; // routing count in high nibble + response length in low nibble
    setApciCommand(retTelegram, APCI_USERMSG_MANUFACTURER_6, 0);
    retTelegram[8] = cmd;
}

/**
 * Returns the unlocked status of the device
 *
 * @return true if the device is unlocked, otherwise false
 */
bool getDeviceUnlocked()
{
    dump(
        if (deviceLocked != DEVICE_UNLOCKED)
        {
            serial.print(": DEVICE_LOCKED");
        }
    )
    return deviceLocked == DEVICE_UNLOCKED;
}

/**
 * Sets the device lock state.
 *
 * @param newDeviceLockState The new device lock state
 */
void setDeviceLockState(const uint32_t newDeviceLockState)
{
    deviceLocked = newDeviceLockState;
    dump(
        if (deviceLocked == DEVICE_UNLOCKED)
        {
            serial.println("-->DEVICE_UNLOCKED");
        }
        else
        {
            serial.println("-->DEVICE_LOCKED");
        }
    )
}

uint16_t getRAMBufferPosition()
{
    return ramBufferPositon;
}

void setRAMBufferPosition(const uint16_t newRAMBufferPosition)
{
    ramBufferPositon = newRAMBufferPosition;
}

uint16_t getRAMBufferSize()
{
    return sizeof(ramBuffer)/sizeof(ramBuffer[0]);
}

void resetRAMBuffer()
{
    setRAMBufferPosition(0);
}

/**
 * Sets lastError and prepares the @ref UPD_SEND_LAST_ERROR response telegram
 *
 * @param errorToSet The error to set
 */
void setLastError(const UDP_State errorToSet)
{
    prepareReturnTelegram(1, UPD_SEND_LAST_ERROR);
    retTelegram[9] = errorToSet;
}

void resetUPDProtocol()
{
    resetRAMBuffer();
    totalBytesReceived = 0;
    totalBytesFlashed = 0;
    dump(serial.println("resetUPDProtocol");)
}

/**
 * Handles the @ref UPD_UNLOCK_DEVICE command.
 * The device is unlocked if the provided UID (guid) is valid.
 *
 * @param data    buffer for the UID
 * @param size    size of the buffer
 * @post          calls setLastError with UDP_IAP_SUCCESS if the device is unlocked, otherwise @ref UDP_UID_MISMATCH or a @ref UDP_State.
 */
void updUnlockDevice(uint8_t * data, const uint32_t size)
{
    // we need to ensure that only authorized operators can
    // update the application
    // as a simple method we use the unique ID of the CPU itself
    // only if this UID (GUID) is known, the device will be unlocked
    if (UID_LENGTH_USED != size)
    {
        setLastError(UDP_UID_MISMATCH);
        dump(serial.println(" size mismatch");)
        return;
    }

    uint8_t uid[IAP_UID_LENGTH];
    UDP_State error = iapResult2UDPState(iapReadUID(uid));
    if (error != UDP_IAP_SUCCESS)
    {
        // could not read UID of mcu
        setLastError(error);
        dump(serial.println(" iapReadUID failed");)
        return;
    }

    if (memcmp(uid, data, size) != 0)
    {
        ///\todo Delete this endian conversion of uid.
        //       It's only for comfort and eating up, costly flash space
        // uid is not correct, as a last chance we convert the uid from big-endian to little-endian as displayed by Flashmagic
        for (uint8_t i = 0; i < UID_LENGTH_USED; i = i + sizeof(uint32_t))
        {
            ///\todo this should be reworked, maybe somehow with __REV
            const uint32_t reversed = streamToUIn32(&data[i]);
            data[i] = static_cast<uint8_t>(reversed >> 24);
            data[i + 1] = static_cast<uint8_t>(reversed >> 16);
            data[i + 2] = static_cast<uint8_t>(reversed >> 8);
            data[i + 3] = static_cast<uint8_t>(reversed & 0xff);
        }

        if (memcmp(uid, data, size) != 0)
        {
            // uid is still not correct, finally decline access
            setLastError(UDP_UID_MISMATCH);
            dump(serial.println(" uid mismatch");)
            return;
        }
    }

    // we can now unlock the device
    setDeviceLockState(DEVICE_UNLOCKED);
    setLastError(UDP_IAP_SUCCESS);
    resetUPDProtocol();
}

/**
 * Handles the @ref UPD_APP_VERSION_REQUEST command and sends a @ref UPD_APP_VERSION_RESPONSE.
 * The response contains the address of the AppVersion string.
 */
void updAppVersionRequest()
{
    const char * appversion = getAppVersion(reinterpret_cast<AppDescriptionBlock *>(applicationFirstAddress() - BOOT_BLOCK_DESC_SIZE));
    prepareReturnTelegram(BL_ID_STRING_LENGTH - 1, UPD_APP_VERSION_RESPONSE);
    memcpy(retTelegram + 9, appversion, BL_ID_STRING_LENGTH - 1);
    dump(
        if (appversion != bl_id_string)
        {
            serial.print("AppVersionRequest OK: ");
        }
        else
        {
            serial.print("AppVersionRequest outside range (default): ");
        }
        for (int i = 0; i < BL_ID_STRING_LENGTH - 1; i++)
        {
            serial.print(appversion[i]);
        }
        serial.println();
    )
}

/**
 * Handles the @ref UPD_DUMP_FLASH command and dumps the given flash address range to the serial port in intel(R) hex
 *
 * @param data    data[0-3] contains startAddress, data[4-7] contains endAddress
 * @post          calls setLastError with @ref UDP_IAP_SUCCESS if successful, otherwise @ref UDP_SECTOR_NOT_ALLOWED_TO_ERASE or a @ref UDP_State.
 * @note          device must be unlocked
 */
void updDumpFlashRange([[maybe_unused]] const uint8_t * data)
{
#ifdef DEBUG
    uint8_t * startAddress = streamToPtr(&data[0]);
    uint8_t * endAddress = streamToPtr(&data[4]);
    setLastError(UDP_IAP_SUCCESS);
    bcu.bus->pause();
    dumpFlashContent(startAddress, endAddress);
    bcu.bus->resume();
#else
    setLastError(UDP_NOT_IMPLEMENTED);
#endif
}


/**
 * Handles the @ref UPD_ERASE_ADDRESS_RANGE command and erases the requested flash address range
 *
 * @param data    data[0-3] contains startAddress, data[4-7] contains endAddress
 * @post          calls setLastError with UDP_IAP_SUCCESS if successful, otherwise @ref UDP_ADDRESS_RANGE_NOT_ALLOWED_TO_ERASE or a @ref UDP_State.
 * @note          device must be unlocked
 */
void updEraseAddressRange(const uint8_t * data)
{
    const uint8_t * startAddress = streamToPtr(&data[0]);
    const uint8_t * endAddress = streamToPtr(&data[4]);
    bcu.bus->pause();
    setLastError(eraseAddressRange(startAddress, endAddress));
    bcu.bus->resume();
    resetUPDProtocol();
}

/**
 * Handles the @ref UPD_ERASE_COMPLETE_FLASH command and erases the entire flash except from the bootloader itself
 *
 * @post          calls setLastError with UDP_IAP_SUCCESS if successful, otherwise @ref UDP_SECTOR_NOT_ALLOWED_TO_ERASE or a @ref UDP_State.
 * @note          device must be unlocked
 */
void updEraseFullFlash()
{
    bcu.bus->pause();
    setLastError(eraseFullFlash());
    bcu.bus->resume();
    resetUPDProtocol();
}

/**
 * Handles the @ref UPD_SEND_DATA command and copies the received bytes from data to @ref ramBuffer
 *
 * @param data    The bytes to copy into @ref ramBuffer
 * @param nCount  Number of bytes to copy
 * @post          calls setLastError with UDP_IAP_SUCCESS if successful, otherwise @ref UDP_RAM_BUFFER_OVERFLOW or a @ref UDP_State
 * @note          device must be unlocked
 */
void updSendData(const uint8_t * data, const uint16_t nCount)
{
    if (getRAMBufferPosition() + nCount > getRAMBufferSize()) // enough space left?
    {
        setLastError(UDP_RAM_BUFFER_OVERFLOW);
        dump(serial.println("ramBuffer Full");)
        return;
    }

    memcpy(&ramBuffer[getRAMBufferPosition()], data, nCount);
    setRAMBufferPosition(getRAMBufferPosition() + nCount);
    totalBytesReceived += nCount;
    setLastError(UDP_IAP_SUCCESS);
    dump(
        for(uint32_t i = 0; i < nCount; i++)
        {
            serial.print(data[i+1], HEX, 2);
            serial.print(" ");
        }
        serial.print("at: ", getRAMBufferPosition(), DEC, 4);
        serial.println(" #", nCount, DEC, 3);
    )
}

/**
 * Handles the @ref UPD_PROGRAM command and copies the bytes from ramBuffer to flash
 *
 * @param data    the number of bytes to flash is in data[0-1], the flash address to program in data[2-5], and the crc32 in data[6-9]
 * @post          calls setLastError with UDP_IAP_SUCCESS if successful, otherwise a @ref UDP_State or a @ref UDP_State
 * @note          device must be unlocked
 * @warning       The function calls @ref executeProgramFlash, which calls @ref iapProgram, which by itself calls @ref noInterrupts().
 */
void updProgram(const uint8_t * data)
{
    uint16_t flash_count = streamToUShort16(data);
    uint8_t * address = streamToPtr(data + 2);
    const uint32_t crc32toCompare = streamToUIn32(data + 2 + 4);

    if (!addressAllowedToProgram(address, flash_count))
    {
        setLastError(UDP_ADDRESS_NOT_ALLOWED_TO_FLASH);
        return;
    }

    if (flash_count > getRAMBufferSize()) // Selfbus Updater has a too big Mcu.UPD_PROGRAM_SIZE set. (see Mcu.java)
    {
        setLastError(UDP_RAM_BUFFER_OVERFLOW);
        return;
    }

    if (getRAMBufferPosition() > flash_count)
    {
        setLastError(UDP_BYTECOUNT_RECEIVED_TOO_HIGH);
        resetRAMBuffer();
        return;
    }

    if (getRAMBufferPosition() < flash_count)
    {
        setLastError(UDP_BYTECOUNT_RECEIVED_TOO_LOW);
        resetRAMBuffer();
        return;
    }

    const uint32_t crcRamBuffer = crc32(0xFFFFFFFF, ramBuffer, flash_count);
    if (crcRamBuffer != crc32toCompare)
    {
        // invalid crcRamBuffer
        setLastError(UDP_CRC_ERROR);
        return;
    }

    dump(
        serial.print("to write ", flash_count);
        serial.print(" bytes @ 0x", address);
        serial.println(" crc 0x", crcRamBuffer, HEX)
    )

    totalBytesFlashed += flash_count;
    UDP_State error = UDP_IAP_SUCCESS;
    uint32_t bufferPosition = 0;
    bcu.bus->pause();
    // loop until error or all bytes flashed
    while (error == UDP_IAP_SUCCESS && flash_count > 0)
    {
        for (const uint16_t i : IapBlockSize) // test all block sizes
        {
            if (i > getRAMBufferSize())
            {
                continue;
            }

            while (flash_count >= i) // flash as long as we have enough bytes for the current blockSize
            {
                // Getting a UDP_IAP_COMPARE_ERROR here is an indicator of flash sectors/pages
                // not being erased before programming
                error = executeProgramFlash(address, &ramBuffer[bufferPosition], i);
                if (error != UDP_IAP_SUCCESS)
                {
                    break; // exit inner while on error
                }
                dump(
                    serial.print("wrote ", IapBlockSize[i]);
                    serial.println(" bytes @ 0x", address);
                )
                flash_count -= i;
                address += i;
                bufferPosition += i;
            }

            if (error != UDP_IAP_SUCCESS || flash_count <= 0)
            {
                break; // exit for on error or all bytes flashed
            }
        }

        if (error == UDP_IAP_SUCCESS && flash_count > 0 && flash_count <= FLASH_PAGE_SIZE)
        {
            // This is the final flash page to program
            error = executeProgramFlash(address, &ramBuffer[bufferPosition], FLASH_PAGE_SIZE);
            break; // exit topmost while
        }
    }
    resetRAMBuffer();
    bcu.bus->resume();
    setLastError(error);
}

/**
 * Handles the @ref UPD_REQUEST_BL_IDENTITY command.
 * Copies bootloader version (@ref BOOTLOADER_MAJOR_VERSION, @ref BOOTLOADER_MINOR_VERSION),
 *        bootloader features (@ref BL_FEATURES) and applications first possible start address (@ref applicationFirstAddress())
 *        to the return telegram.
 *
 * @param data  the major version of Selfbus Updater in data[0-3], the minor version of Selfbus Updater in data[4-7]
 */
void updRequestBootloaderIdentity(const uint8_t * data)
{
    const uint8_t majorVersionUpdater = data[0];
    const uint8_t minorVersionUpdater = data[1];
    const bool versionMatch = majorVersionUpdater > UPDATER_MIN_MAJOR_VERSION ||
              (majorVersionUpdater == UPDATER_MIN_MAJOR_VERSION && minorVersionUpdater >= UPDATER_MIN_MINOR_VERSION);

    uint8_t offset = 9;
    if (!versionMatch)
    {
        // version mismatch send the minimum requested major and minor version
        prepareReturnTelegram(sizeof(UPDATER_MIN_MAJOR_VERSION) + sizeof(UPDATER_MIN_MINOR_VERSION), UPD_RESPONSE_BL_VERSION_MISMATCH);
        retTelegram[offset] = UPDATER_MIN_MAJOR_VERSION;
        retTelegram[offset + sizeof(UPDATER_MIN_MAJOR_VERSION)] = UPDATER_MIN_MINOR_VERSION;
        dump(
            serial.print("Updater version mismatch! Required ", UPDATER_MIN_MAJOR_VERSION);
            serial.print(".", UPDATER_MIN_MINOR_VERSION);
            serial.print(" received: ", majorVersionUpdater);
            serial.println(".", minorVersionUpdater);
        )
        return;
    }

    constexpr uint16_t bootloaderFeatures = BL_FEATURES;
    constexpr uint8_t majorSBLibVersion = highByte(static_cast<uint16_t>(SBLIB_VERSION));
    constexpr uint8_t minorSBLibVersion = lowByte(SBLIB_VERSION);
    const uint8_t * appFirstAddress = applicationFirstAddress();
    constexpr uint8_t dataSize = sizeof(BOOTLOADER_MAJOR_VERSION) +
                                 sizeof(BOOTLOADER_MINOR_VERSION) +
                                 sizeof(majorSBLibVersion) +
                                 sizeof(minorSBLibVersion) +
                                 sizeof(bootloaderFeatures) +
                                 sizeof(appFirstAddress);

    prepareReturnTelegram(dataSize, UPD_RESPONSE_BL_IDENTITY);
    retTelegram[offset] = BOOTLOADER_MAJOR_VERSION;
    offset += sizeof(BOOTLOADER_MAJOR_VERSION);
    retTelegram[offset] = BOOTLOADER_MINOR_VERSION;
    offset += sizeof(BOOTLOADER_MINOR_VERSION);
    uShort16ToStream(retTelegram + offset, bootloaderFeatures);
    offset += sizeof(bootloaderFeatures);
    retTelegram[offset] = majorSBLibVersion;
    offset += sizeof(majorSBLibVersion);
    retTelegram[offset] = minorSBLibVersion;
    offset += sizeof(minorSBLibVersion);
    ptrToStream(retTelegram + offset, appFirstAddress);
    dump(
        serial.print("BL v", BOOTLOADER_MAJOR_VERSION, DEC);
        serial.print(".", BOOTLOADER_MINOR_VERSION, DEC);
        serial.print(", Feature 0x", bootloaderFeatures, HEX);
        serial.println(", FW start 0x", reinterpret_cast<uintptr_t>(appFirstAddress), HEX);
    )
}

/**
 * Handles the @ref UPD_REQUEST_STATISTIC command.
 */
void updRequestStatistic()
{
    constexpr uint8_t sizeTotal = sizeof(disconnectCount) + sizeof(repeatedT_ACKcount);

    prepareReturnTelegram(sizeTotal, UPD_RESPONSE_STATISTIC);
    uShort16ToStream(retTelegram + 9, disconnectCount);
    uShort16ToStream(retTelegram + 9 + sizeof(disconnectCount), repeatedT_ACKcount);
    dump(
        serial.print("#DC ", disconnectCount);
        serial.println(" #repT_ACK ", repeatedT_ACKcount);
    )
}

/**
 * Handles the @ref UPD_REQUEST_BOOT_DESC command.
 * Copies the application description block (@ref AppDescriptionBlock) to the return telegram.
 */
void udpRequestBootDescriptionBlock()
{
    const auto bootDescr = reinterpret_cast<AppDescriptionBlock *>(bootDescriptorBlockAddress()); // Address of boot block descriptor

    // check that the start address is not beyond the end address
    bool valid = bootDescr->startAddress <= bootDescr->endAddress;
    // addresses not outside the flash
    valid &= bootDescr->startAddress <= flashLastAddress() && bootDescr->endAddress <= flashLastAddress();
    // addresses are not smaller than allowed applications first address
    valid &= bootDescr->startAddress >= applicationFirstAddress() && bootDescr->endAddress >= applicationFirstAddress();

    if (!valid)
    {
        bootDescr->startAddress = reinterpret_cast<uint8_t *>(- 1);
        bootDescr->endAddress = reinterpret_cast<uint8_t *>(- 1);
        bootDescr->appVersionAddress = reinterpret_cast<char *>(- 1);
        bootDescr->crc = 0xffffffff;
    }

    prepareReturnTelegram(12, UPD_RESPONSE_BOOT_DESC);
    memcpy(retTelegram + 9, bootDescr, 12); // startAddress, endAddress, crc

    dump(
        serial.print("FW start@ 0x", bootDescr->startAddress); // Firmware start address
        serial.print(" end@ 0x", bootDescr->endAddress);       // Firmware end address
        serial.print(" Desc.@ 0x", bootDescr->appVersionAddress); // Firmware App descriptor address (for getAppVersion())
        serial.println(" CRC : 0x", bootDescr->crc, HEX);      // Firmware CRC
    )
}

/**
 * Function isn't implemented.
 *
 * @post    calls setLastError with @ref UDP_NOT_IMPLEMENTED
 * @warning function is not implemented, missing parameters address and count
 */
void updRequestData()
{
    ///\todo implement updRequestData
    //memcpy(retTelegram + 9, addressToCopyFrom, count);
    //prepareReturnTelegram(count, UPD_SEND_DATA);
    setLastError(UDP_NOT_IMPLEMENTED);
}

/**
 * Handles the @ref UPD_REQUEST_UID command.
 * Copies @ref UID_LENGTH_USED bytes to the return telegram.
 *
 * @post    Calls setLastError with @ref UDP_IAP_SUCCESS if successful, otherwise an @ref IAP_Status
 * @note    Device must be unlocked.
 */
void updRequestUID()
{
    uint8_t uid[4 * 4];
    const UDP_State result = iapResult2UDPState(iapReadUID(uid));
    if (result != UDP_IAP_SUCCESS)
    {
        dump(serial.println("iapReadUID error");)
        setLastError(result);
        return;
    }
    prepareReturnTelegram(UID_LENGTH_USED, UPD_RESPONSE_UID);
    memcpy(retTelegram + 9, uid, UID_LENGTH_USED);
    dump(serial.println(" OK");)
}

/**
 * Handles all unknown UPD/UDP commands.
 *
 * Calls @ref setLastError with @ref UDP_UNKNOWN_COMMAND
 */
void updUnknownCommand()
{
    setLastError(UDP_UNKNOWN_COMMAND);
}

/**
 * Handles the @ref UPD_UPDATE_BOOT_DESC command.
 *        - checks the received application boot descriptor block for a possible buffer overflow
 *        - checks the crc32 of the received application boot descriptor block
 *        - if the received application boot descriptor block differs from the one already in Flash,
 *          it checks that the address is allowed to program, erases the flash page, and flashes the new one.
 *
 * @param data    - data[0..3] contains the length of the application boot descriptor block received
 *                - data[4..7] contains the crc32 of the received bytes
 * @post          calls setLastError with UDP_IAP_SUCCESS if successful, otherwise a @ref UDP_State or @ref IAP_Status
 * @note          device must be unlocked
 * @warning       The function calls @ref executeProgramFlash, which calls @ref iapProgram, which by itself calls @ref noInterrupts().
 */
void updUpdateBootDescriptorBlock(const uint8_t * data)
{
    const uint32_t count = streamToUIn32(data); // data[0..3] length of the descriptor
    const uint32_t crcReceived = streamToUIn32(data + 4); // data[4..7]

    // check for a possible ramBuffer overflow
    if (count > sizeof(ramBuffer)/sizeof(ramBuffer[0]))
    {
        setLastError(UDP_RAM_BUFFER_OVERFLOW);
        dump(serial.println("ramBuffer Full");)
        return;
    }
    dump(
        totalBytesReceived -= static_cast<uint16_t>(count); // subtract bytes received for boot descriptor
        serial.println();
        serial.println("Bytes Rx    ", totalBytesReceived);
        serial.println("Bytes Flash ", totalBytesFlashed);
        serial.println("Diff        ", totalBytesFlashed - totalBytesReceived); // the difference here is normal because flashing is always in multiple of FLASH_PAGE_SIZE
        serial.println();
        serial.println("FW start@ 0x", streamToUIn32(ramBuffer), HEX, 4);    // Firmware start address
        serial.println("FW end  @ 0x", streamToUIn32(ramBuffer+4), HEX, 4);  // Firmware end address
        serial.println("FW Desc.@ 0x", streamToUIn32(ramBuffer+12), HEX, 4); // Firmware App descriptor address (for getAppVersion())
        serial.println("FWs CRC : 0x", streamToUIn32(ramBuffer+8), HEX, 8);  // Firmware CRC
        // serial.println("RamBuffer[16-20] 0x", streamToUIn32(ramBuffer+16), HEX, 8);// This is beyond the descriptor block
    )

    uint8_t * address = bootDescriptorBlockAddress(); // start address of boot block descriptor
    const uint32_t crc = crc32(0xFFFFFFFF, ramBuffer, count);  // checksum on used length only

    dump(
        serial.println("Desc.      @ 0x", address);
        serial.println("Desc.    CRC 0x", crc, HEX);
        serial.println("Received CRC 0x", crcReceived, HEX);
    )
    // compare calculated crc with the one we received for this packet
    if (crc != crcReceived)
    {
        dump(
            serial.print("-->UDP_CRC_ERROR ");
            serial.print(" data[3-0]:", streamToUIn32(data), HEX, 8);
            serial.print(" data[7-4]:", streamToUIn32(data+4), HEX, 8);
        )
        setLastError(UDP_CRC_ERROR);
        return;
    }

    UDP_State result;
    dump(serial.println("CRC MATCH, comparing MCUs BootDescriptor: count: ", count);)
    //If the received descriptor is not equal to the current one, flash it
    if(memcmp(address, ramBuffer, count) == 0)
    {
        dump(serial.println("is equal, skipping");)
        result = UDP_IAP_SUCCESS;
        // don't return here, let's also check the AppDescriptionBlock
    }
    else
    {
        dump(serial.print("it's different, Erase Page: ");)
        bcu.bus->pause();
        result = erasePageRange(bootDescriptorBlockPage(), bootDescriptorBlockPage());
        bcu.bus->resume();
        if (result != UDP_IAP_SUCCESS)
        {
            setLastError(result);
            return;
        }

        dump(serial.print("Flash Page:");)

        bcu.bus->pause();
        result = executeProgramFlash(address, ramBuffer, FLASH_PAGE_SIZE, true); // no less than 256 byte can be flashed
        bcu.bus->resume();
        dump(
           updResult2Serial(result);
           serial.println();
        )

        if (result != UDP_IAP_SUCCESS)
        {
            setLastError(result);
            if (result == UDP_ADDRESS_NOT_ALLOWED_TO_FLASH)
            {
                return;
            }
        }
    }

    if (!checkApplication(reinterpret_cast<AppDescriptionBlock*>(ramBuffer)))
    {
        dump(serial.println("-->UDP_APPLICATION_NOT_STARTABLE");)
        setLastError(UDP_APPLICATION_NOT_STARTABLE);
        return;
    }

    setLastError(result);
}

/**
 * Handles the @ref UPD_SEND_DATA_TO_DECOMPRESS command.
 * "Copies" the bytes from data to the @ref Decompressor.
 *
 * @param data    data[0...nCount-1] buffer containing the bytes to "copy" to the @ref Decompressor
 * @param nCount  Number of bytes to read from data
 * @post          calls setLastError with UDP_IAP_SUCCESS if successful, otherwise @ref UDP_RAM_BUFFER_OVERFLOW or @ref UDP_NOT_IMPLEMENTED
 * @note          device must be unlocked
 * @warning       The function calls @ref Decompressor.pageCompletedDoFlash,
 *                which calls @ref iapProgram, which by itself calls @ref noInterrupts().
 */
void updSendDataToDecompress(const uint8_t * data, const uint32_t nCount)
{
#ifndef DECOMPRESSOR
    dump(serial.println("-->not implemented");)
    setLastError(UDP_NOT_IMPLEMENTED);
#else
    dump(serial.println("-->decompressor");)
    for (uint32_t i = 0; i < nCount; i++)
    {
        decompressor.putByte(data[i]);
    }
    setLastError(UDP_IAP_SUCCESS);
#endif
}

/**
 * Handles the @ref UPD_PROGRAM_DECOMPRESSED_DATA command.
 *        - checks that the flash address is allowed to be programmed
 *        - checks the crc32 received in data with the one from the @ref Decompressor
 *        - calls @ref Decompressor.pageCompletedDoFlash to flash
 *
 * @param data    data[0-3] contains the crc32 for the received bytes
 * @post          calls setLastError with @ref UDP_IAP_SUCCESS if successful, otherwise a @ref UDP_State
 * @note          device must be unlocked
 * @warning       The function calls @ref Decompressor.pageCompletedDoFlash,
 *                which calls @ref iapProgram, which by itself calls @ref noInterrupts().
 */
void updProgramDecompressedDataToFlash(const uint8_t * data)
{
#ifndef DECOMPRESSOR
    setLastError(UDP_NOT_IMPLEMENTED);
#else
    const uint32_t crcReceived = streamToUIn32(data);
    const uint32_t count = decompressor.getBytesCountToBeFlashed();
    const uint8_t * address = decompressor.getStartAddrOfPageToBeFlashed();

    dump(
        serial.println();
        serial.print("Flash Diff address 0x", address);
        serial.print(" length: ", count, DEC, 3);
    )
    if (!addressAllowedToProgram(address, count))
    {
        dump(serial.println(" Address protected!");)
        setLastError(UDP_ADDRESS_NOT_ALLOWED_TO_FLASH);
        return;
    }

    dump(serial.print(" Address valid, ");)
    const uint32_t crc = decompressor.getCrc32();
    if (crc != crcReceived)
    {
        dump(serial.println("CRC Error!");)
        setLastError(UDP_CRC_ERROR);
        return;
    }

    dump(serial.println("CRC OK");)
    bcu.bus->pause();
    setLastError(decompressor.pageCompletedDoFlash());
    bcu.bus->resume();
    resetUPDProtocol(); // we need this, otherwise updSendDataToDecompress will run into a buffer overflow
#endif
}

void handleDeprecatedApciMemoryWrite(uint8_t * sendBuffer)
{
    sendBuffer[5] = 0x63 + 4; // routing count in high nibble + response length in low nibble
    sendBuffer[6] = 0x42;     // APCI_MEMORY_RESPONSE_PDU
    sendBuffer[7] = 0x40 | 4; // APCI_MEMORY_RESPONSE_PDU
    sendBuffer[8] = 0;        // [8-9] old value of UPD_SEND_LAST_ERROR = 0x0015
    sendBuffer[9] = 0x15;
    sendBuffer[10] = 0xff;    // [10-13] old value of UDP_NOT_IMPLEMENTED = 0x0000FFFF
    sendBuffer[11] = 0xff;
    sendBuffer[12] = 0x00;
    sendBuffer[13] = 0x00;
}

void handleApciUsermsgManufacturerInternal(uint8_t * data, uint16_t size)
{
    if (size < sizeof(updCommands[idxInvalidUPDCommand].code))
    {
        dump(serial.println("UDP_NO_DATA"))
        setLastError(UDP_NO_DATA);
        return;
    }

    const UPD_Command updCommand = code2UPDCommand(data[0]);
    if (updCommand.code == UPD_INVALID)
    {
        dump(serial.println("updCommand.code invalid"))
        setLastError(UDP_INVALID);
        return;
    }

    data++;
    size--;
    updCommand2Serial(updCommand); // simple command debugging message
    if (size < updCommand.minBytes || size > updCommand.maxBytes)
    {
        dump(
            serial.print(" UDP_INVALID_DATA minCount=", updCommand.minBytes);
            serial.print(" maxCount=", updCommand.maxBytes);
            serial.println(" length=", size);
        )
        setLastError(UDP_INVALID_DATA);
        return;
    }

#if defined(DEBUG) && (!(defined(TS_ARM)))
    digitalWrite(PIN_INFO, !digitalRead(PIN_INFO));
#endif
    // check for commands allowed on a locked device
    if (!getDeviceUnlocked())
    {
        switch (updCommand.code)
        {
            // list of commands allowed on a locked device
            case UPD_UNLOCK_DEVICE:
            case UPD_REQUEST_UID:
            case UPD_APP_VERSION_REQUEST:
                break;

            default:
                // device is locked -> command not allowed, send lastError and return
                setLastError(UDP_DEVICE_LOCKED);
                return;
        }
    }

    // now comes the real work on the unlocked device
    switch (updCommand.code)
    {
        case UPD_UNLOCK_DEVICE:
            updUnlockDevice(data, size);
            break;

        case UPD_REQUEST_UID:
            updRequestUID();
            break;

        case UPD_APP_VERSION_REQUEST:
            updAppVersionRequest();
            break;

        case UPD_SEND_DATA:
            updSendData(data, size);
            break;

        case UPD_PROGRAM:
            updProgram(data);
            break;

        case UPD_SEND_DATA_TO_DECOMPRESS:
            updSendDataToDecompress(data, size);
            break;

        case UPD_PROGRAM_DECOMPRESSED_DATA:
            updProgramDecompressedDataToFlash(data);
            break;

        case UPD_ERASE_COMPLETE_FLASH:
            updEraseFullFlash();
            break;

        case UPD_ERASE_ADDRESS_RANGE:
            updEraseAddressRange(data);
            break;

        case UPD_DUMP_FLASH:
            updDumpFlashRange(data);
            break;

        case UPD_REQUEST_STATISTIC:
            updRequestStatistic();
            break;

        case UPD_UPDATE_BOOT_DESC:
            updUpdateBootDescriptorBlock(data);
            break;

        case UPD_REQUEST_BOOT_DESC:
            udpRequestBootDescriptionBlock();
            break;

        case UPD_REQUEST_BL_IDENTITY:
            updRequestBootloaderIdentity(data);
            break;

        case UPD_REQ_DATA:
            updRequestData();
            break;

        default:
            updUnknownCommand();
            break;
    }
}

void handleApciUsermsgManufacturer(uint8_t * sendBuffer, uint8_t * data, const uint16_t size)
{
    retTelegram = sendBuffer;
    handleApciUsermsgManufacturerInternal(data, size);
    retTelegram = nullptr;
}

/** @}*/
