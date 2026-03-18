/**************************************************************************//**
 * @addtogroup SBLIB_BOOTLOADER Selfbus Bootloader
 * @defgroup SBLIB_BOOTLOADER_DECOMPRESSOR Decompressor
 * @ingroup SBLIB_BOOTLOADER
 * @brief   Differential decompressor
 * @details This is an extended version of the knxduino differential updater from
 *          https://github.com/pavkriz/knxduino/tree/master/bootloader/src
 *
 * @{
 *
 * @file   decompressor.h
 * @author Pavel Kriz <https://github.com/pavkriz> Copyright (c) 2019
 * @author Stefan Haller Copyright (c) 2021
 ******************************************************************************/

/*
 This program is free software; you can redistribute it and/or modify
 it under the terms of the GNU General Public License version 3 as
 published by the Free Software Foundation.
 -----------------------------------------------------------------------------*/

#ifndef SB_BOOTLOADER_DECOMPRESSOR_H_
#define SB_BOOTLOADER_DECOMPRESSOR_H_

#include "upd_protocol.h"
#include "boot_descriptor_block.h"
#include <cstdint>


constexpr uint8_t REMEMBER_OLD_PAGES_COUNT = 2; //!< There seems to be a RAM leak/overflow somewhere, 2 seems to work for the moment!

/**
 * Apply differential stream and produce one page to be flashed
 * (based on differential stream, original ROM content, and RAM buffer to store some latest ROM pages already flashed)
 */
class Decompressor
{
public:
    explicit Decompressor(const AppDescriptionBlock * BaseAddress);
    // delete default constructors
    Decompressor() = delete;
    Decompressor(const Decompressor&) = delete;
    Decompressor& operator=(const Decompressor&) = delete;

    void putByte(uint8_t data);

    UDP_State pageCompletedDoFlash();

    [[nodiscard]] uint32_t getCrc32() const;

    [[nodiscard]] uint8_t * getStartAddrOfPageToBeFlashed() const;

    [[nodiscard]] uint32_t getBytesCountToBeFlashed() const;

    uint8_t getFlashPageNumberToBeFlashed();

private:
    enum class State
    {
        EXPECT_COMMAND_BYTE,
        EXPECT_COMMAND_PARAMS,
        EXPECT_RAW_DATA
    };

    uint8_t cmdBuffer[5] = {};
    uint16_t expectedCmdLength = 0;
    uint16_t cmdBufferLength = 0;
    alignas(FLASH_RAM_BUFFER_ALIGNMENT) uint8_t scratchpad[FLASH_PAGE_SIZE] = {};
    uint8_t oldPages[FLASH_PAGE_SIZE * REMEMBER_OLD_PAGES_COUNT] = {};
    uint16_t bytesToFlash = 0;
    uint16_t rawLength = 0;
    State state = State::EXPECT_COMMAND_BYTE;
    alignas(FLASH_RAM_BUFFER_ALIGNMENT) uint8_t * startAddrOfPageToBeFlashed = nullptr;
    uint8_t * startAddrOfFlash = nullptr;

    [[nodiscard]] uint16_t getLength() const;

    [[nodiscard]] bool isCopyFromRam() const;

    [[nodiscard]] uint32_t getCopyAddress() const;

    void resetStateMachine();
};

#endif /* SB_BOOTLOADER_DECOMPRESSOR_H_ */

/** @}*/
