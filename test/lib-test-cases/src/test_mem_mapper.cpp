/*
 * Unit tests for class MemMapper
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 3 as
 * published by the Free Software Foundation.
 */

#include <sblib/mem_mapper.h>
#include <iap_emu.h>
#include <cstring>
#include <iomanip>
#include <sys/param.h>
#include <catch.hpp>

/**
 * \brief Flash region used by tests: 0xe000..0xefff (4 KB = 16 pages)
 * - Page 0xe0 = allocation table
 * - Pages 0xe1..0xef = 15 data pages
 */
constexpr uint32_t TEST_FLASH_BASE = 0xe000;
constexpr uint32_t TEST_FLASH_SIZE = 0x1000;

void resetFlash()
{
    IAP_Init_Flash(0xff);
}

TEST_CASE("MemMapper::", "[MemMapper]")
{
    SECTION("constructor")
    {
        SECTION("clean flash: all virtual pages start unmapped")
        {
            resetFlash();
            MemMapper mm(TEST_FLASH_BASE, TEST_FLASH_SIZE, false);
            REQUIRE(mm.isMapped(0x0000) == false);
            REQUIRE(mm.isMapped(0x0100) == false);
            // This checks only start and end address mapping
            REQUIRE(mm.isMappedRange(TEST_FLASH_BASE, TEST_FLASH_BASE + TEST_FLASH_SIZE - 1) == false);
            // This checks all pages within the range
            for (uint32_t i = TEST_FLASH_BASE; i < TEST_FLASH_BASE + TEST_FLASH_SIZE; i += FLASH_PAGE_SIZE)
            {
                REQUIRE(mm.isMapped(i) == false);
            }
            REQUIRE(mm.isMapped(0xe000) == false);
            REQUIRE(mm.isMapped(0xefff) == false);
            REQUIRE(mm.isMapped(0xffff) == false);
        }

        SECTION("single zero byte in allocTable is not treated as corrupt")
        {
            resetFlash();
            constexpr uint32_t MAPPED_PAGE = 5;
            // allocTable[MAPPED_PAGE] = FLASH[TEST_FLASH_BASE + MAPPED_PAGE] = 0x00 means virtual page maps to physical page 0xff (255)
            FLASH[TEST_FLASH_BASE + MAPPED_PAGE] = 0x00;
            MemMapper mm(TEST_FLASH_BASE, TEST_FLASH_SIZE, false);
            REQUIRE(mm.isMapped(MAPPED_PAGE * FLASH_PAGE_SIZE) == true);
        }

        SECTION("two zero bytes in allocTable triggers corruption reset")
        {
            resetFlash();
            constexpr uint32_t FIRST_ZERO_MAPPED = 0;
            constexpr uint32_t SECOND_ZERO_MAPPED = 1;
            FLASH[TEST_FLASH_BASE + FIRST_ZERO_MAPPED] = 0x00;
            FLASH[TEST_FLASH_BASE + SECOND_ZERO_MAPPED] = 0x00;
            MemMapper mm(TEST_FLASH_BASE, TEST_FLASH_SIZE, false);
            REQUIRE(mm.isMapped(FIRST_ZERO_MAPPED * FLASH_PAGE_SIZE) == false);
            REQUIRE(mm.isMapped(SECOND_ZERO_MAPPED * FLASH_PAGE_SIZE) == false);
        }
    }

    SECTION("addRange", "[MemMapper]")
    {
        resetFlash();
        auto* mm = new MemMapper(TEST_FLASH_BASE, TEST_FLASH_SIZE, false);

        SECTION("length zero returns INVALID_LENGTH")
        {
            REQUIRE(mm->addRange(0x0000, 0) == MemMapper::Error::InvalidLength);
        }

        SECTION("length not a multiple of page size returns INVALID_LENGTH")
        {
            REQUIRE(mm->addRange(0x0000, 1)  == MemMapper::Error::InvalidLength);
            REQUIRE(mm->addRange(0x0000, FLASH_PAGE_SIZE - 1) == MemMapper::Error::InvalidLength);
            REQUIRE(mm->addRange(0x0000, FLASH_PAGE_SIZE + 1) == MemMapper::Error::InvalidLength);
        }

        SECTION("unaligned address returns INVALID_ADDRESS")
        {
            REQUIRE(mm->addRange(0x0042, FLASH_PAGE_SIZE) == MemMapper::Error::InvalidAddress);
        }

        SECTION("address beyond 16-bit space returns INVALID_ADDRESS")
        {
            REQUIRE(mm->addRange(0x10000, FLASH_PAGE_SIZE) == MemMapper::Error::InvalidAddress);
        }

        SECTION("valid page-aligned call succeeds")
        {
            for (uint32_t i = 0; i < TEST_FLASH_SIZE; i += FLASH_PAGE_SIZE)
            {
                delete mm;
                resetFlash();
                mm = new MemMapper(TEST_FLASH_BASE, TEST_FLASH_SIZE, false);

                // -FLASH_PAGE_SIZE because one page is reserved for the allocTable
                for (uint32_t j = FLASH_PAGE_SIZE; j < TEST_FLASH_SIZE - FLASH_PAGE_SIZE; j += FLASH_PAGE_SIZE)
                {
                    INFO("i = 0x" << std::hex << i << ", j = 0x" << j);
                    REQUIRE(mm->addRange(i, j) == MemMapper::Error::Success);
                }
            }
        }

        SECTION("calling addRange twice on the same range is idempotent")
        {
            REQUIRE(mm->addRange(0x0000, FLASH_PAGE_SIZE) == MemMapper::Error::Success);
            REQUIRE(mm->addRange(0x0000, FLASH_PAGE_SIZE) == MemMapper::Error::Success);
            REQUIRE(mm->isMapped(0x0000) == true);
        }

        SECTION("multiple consecutive pages all become mapped")
        {
            REQUIRE(mm->addRange(0x0000, 3 * FLASH_PAGE_SIZE) == MemMapper::Error::Success);
            REQUIRE(mm->isMapped(0x0000) == true);
            REQUIRE(mm->isMapped(0x0100) == true);
            REQUIRE(mm->isMapped(0x0200) == true);
            REQUIRE(mm->isMapped(0x0300) == false); // one past the allocated range
        }
    }

    SECTION("writeMem / readMem", "[MemMapper]")
    {
        SECTION("read from unmapped address returns NOT_MAPPED and clears data")
        {
            resetFlash();
            MemMapper mm(TEST_FLASH_BASE, TEST_FLASH_SIZE, false);
            uint8_t data = 0xAB;
            REQUIRE(mm.readMem(0x0042, data) == MemMapper::Error::NotMapped);
            REQUIRE(data == 0x00);
        }

        SECTION("write to unmapped address returns SUCCESS but read still returns NOT_MAPPED")
        {
            resetFlash();
            MemMapper mm(TEST_FLASH_BASE, TEST_FLASH_SIZE, false);
            REQUIRE(mm.writeMem(0x0042, 0xAB) == MemMapper::Error::Success);
            uint8_t data = 0;
            REQUIRE(mm.readMem(0x0042, data) == MemMapper::Error::NotMapped);
        }

        SECTION("write and read on a page mapped via addRange")
        {
            resetFlash();
            MemMapper mm(TEST_FLASH_BASE, TEST_FLASH_SIZE, false);
            REQUIRE(mm.addRange(0x0000, FLASH_PAGE_SIZE) == MemMapper::Error::Success);
            REQUIRE(mm.writeMem(0x0042, 0xAB) == MemMapper::Error::Success);
            uint8_t data = 0;
            REQUIRE(mm.readMem(0x0042, data) == MemMapper::Error::Success);
            REQUIRE(data == 0xAB);
        }

        SECTION("second write to same address returns the latest value")
        {
            resetFlash();
            MemMapper mm(TEST_FLASH_BASE, TEST_FLASH_SIZE, false);
            REQUIRE(mm.addRange(0x0000, FLASH_PAGE_SIZE) == MemMapper::Error::Success);
            REQUIRE(mm.writeMem(0x0010, 0xAA) == MemMapper::Error::Success);
            REQUIRE(mm.writeMem(0x0010, 0xBB) == MemMapper::Error::Success);
            uint8_t data = 0;
            REQUIRE(mm.readMem(0x0010, data) == MemMapper::Error::Success);
            REQUIRE(data == 0xBB);
        }

        SECTION("write and read boundary addresses within a page")
        {
            resetFlash();
            MemMapper mm(TEST_FLASH_BASE, TEST_FLASH_SIZE, false);
            REQUIRE(mm.addRange(0x0000, FLASH_PAGE_SIZE) == MemMapper::Error::Success);
            uint8_t data = 0;

            REQUIRE(mm.writeMem(0x0000, 0x00) == MemMapper::Error::Success);
            REQUIRE(mm.readMem(0x0000, data)  == MemMapper::Error::Success);
            REQUIRE(data == 0x00);

            REQUIRE(mm.writeMem(0x00FF, 0xFF) == MemMapper::Error::Success);
            REQUIRE(mm.readMem(0x00FF, data)  == MemMapper::Error::Success);
            REQUIRE(data == 0xFF);
        }

        SECTION("negative virtual address returns INVALID_ADDRESS")
        {
            resetFlash();
            MemMapper mm(TEST_FLASH_BASE, TEST_FLASH_SIZE, false);
            REQUIRE(mm.writeMem(-1, 0x42) == MemMapper::Error::InvalidAddress);
            uint8_t data = 0xAB;
            REQUIRE(mm.readMem(-1, data)  == MemMapper::Error::InvalidAddress);
            REQUIRE(data == 0x00);
        }

        SECTION("virtual address beyond 16-bit space returns INVALID_ADDRESS")
        {
            resetFlash();
            MemMapper mm(TEST_FLASH_BASE, TEST_FLASH_SIZE, false);
            REQUIRE(mm.writeMem(0x10000, 0x42) == MemMapper::Error::InvalidAddress);
            uint8_t data = 0xAB;
            REQUIRE(mm.readMem(0x10000, data)  == MemMapper::Error::InvalidAddress);
            REQUIRE(data == 0x00);
        }
    }

    // ---------------------------------------------------------------------------

    SECTION("autoAddPage", "[MemMapper]")
    {
        SECTION("write and read succeed without an explicit addRange call")
        {
            resetFlash();
            MemMapper mm(TEST_FLASH_BASE, TEST_FLASH_SIZE, true);
            REQUIRE(mm.writeMem(0x0042, 0xCD) == MemMapper::Error::Success);
            uint8_t data = 0;
            REQUIRE(mm.readMem(0x0042, data) == MemMapper::Error::Success);
            REQUIRE(data == 0xCD);
        }

        SECTION("isMapped returns true for any valid address")
        {
            resetFlash();
            MemMapper mm(TEST_FLASH_BASE, TEST_FLASH_SIZE, true);
            REQUIRE(mm.isMapped(0x0000) == true);
            REQUIRE(mm.isMapped(0x1234) == true);
            REQUIRE(mm.isMapped(0xFF00) == true);
        }

        SECTION("sequential writes on the same page all read back correctly")
        {
            resetFlash();
            MemMapper mm(TEST_FLASH_BASE, TEST_FLASH_SIZE, true);
            for (uint8_t i = 0; i < 16; i++)
            {
                REQUIRE(mm.writeMem(0x0000 + i, i) == MemMapper::Error::Success);
            }
            for (uint8_t i = 0; i < 16; i++)
            {
                uint8_t data = 0;
                REQUIRE(mm.readMem(0x0000 + i, data) == MemMapper::Error::Success);
                REQUIRE(data == i);
            }
        }
    }

    // ---------------------------------------------------------------------------

    SECTION("isMapped", "[MemMapper]")
    {
        resetFlash();
        MemMapper mm(TEST_FLASH_BASE, TEST_FLASH_SIZE, false);

        SECTION("unmapped virtual pages return false")
        {
            REQUIRE(mm.isMapped(0x0000) == false);
            REQUIRE(mm.isMapped(0x0100) == false);
        }

        SECTION("all offsets within a mapped page return true")
        {
            REQUIRE(mm.addRange(0x0000, FLASH_PAGE_SIZE) == MemMapper::Error::Success);
            REQUIRE(mm.isMapped(0x0000) == true);
            REQUIRE(mm.isMapped(0x0080) == true);
            REQUIRE(mm.isMapped(0x00FF) == true);
        }

        SECTION("adjacent page that was not mapped returns false")
        {
            REQUIRE(mm.addRange(0x0000, FLASH_PAGE_SIZE) == MemMapper::Error::Success);
            REQUIRE(mm.isMapped(0x0100) == false);
        }
    }

    // ---------------------------------------------------------------------------

    SECTION("isMappedRange", "[MemMapper]")
    {
        resetFlash();
        MemMapper mm(TEST_FLASH_BASE, TEST_FLASH_SIZE, false);

        SECTION("both endpoints unmapped returns false")
        {
            REQUIRE(mm.isMappedRange(0x0000, 0x0100) == false);
        }

        SECTION("start mapped but end unmapped returns false")
        {
            REQUIRE(mm.addRange(0x0000, FLASH_PAGE_SIZE) == MemMapper::Error::Success);
            REQUIRE(mm.isMappedRange(0x0000, 0x0100) == false);
        }

        SECTION("both endpoints on the same mapped page returns true")
        {
            REQUIRE(mm.addRange(0x0000, FLASH_PAGE_SIZE) == MemMapper::Error::Success);
            REQUIRE(mm.isMappedRange(0x0000, 0x00FF) == true);
        }

        SECTION("endpoints on two separately mapped pages returns true")
        {
            REQUIRE(mm.addRange(0x0000, 2 * FLASH_PAGE_SIZE) == MemMapper::Error::Success);
            REQUIRE(mm.isMappedRange(0x0000, 0x0100) == true);
        }
    }

    // ---------------------------------------------------------------------------

    SECTION("setUInt8 / getUInt8", "[MemMapper]")
    {
        resetFlash();
        MemMapper mm(TEST_FLASH_BASE, TEST_FLASH_SIZE, false);
        REQUIRE(mm.addRange(0x0000, FLASH_PAGE_SIZE) == MemMapper::Error::Success);

        SECTION("store and retrieve a byte")
        {
            REQUIRE(mm.setUInt8(0x0010, 0xAB) == MemMapper::Error::Success);
            REQUIRE(mm.getUInt8(0x0010) == 0xAB);
        }

        SECTION("zero is stored and retrieved correctly")
        {
            REQUIRE(mm.setUInt8(0x0000, 0x00) == MemMapper::Error::Success);
            REQUIRE(mm.getUInt8(0x0000) == 0x00);
        }

        SECTION("0xFF is stored and retrieved correctly")
        {
            REQUIRE(mm.setUInt8(0x00FF, 0xFF) == MemMapper::Error::Success);
            REQUIRE(mm.getUInt8(0x00FF) == 0xFF);
        }
    }

    // ---------------------------------------------------------------------------

    SECTION("setUInt16 / getUInt16 endianness", "[MemMapper]")
    {
        resetFlash();
        MemMapper mm(TEST_FLASH_BASE, TEST_FLASH_SIZE, false);
        REQUIRE(mm.addRange(0x0000, FLASH_PAGE_SIZE) == MemMapper::Error::Success);

        SECTION("LITTLE_ENDIAN (default): LSB at lower address")
        {
            mm.setEndianess(LITTLE_ENDIAN);
            REQUIRE(mm.setUInt16(0x0010, 0x1234) == MemMapper::Error::Success);
            REQUIRE(mm.getUInt16(0x0010) == 0x1234);
            REQUIRE(mm.getUInt8(0x0010) == 0x34); // LSB
            REQUIRE(mm.getUInt8(0x0011) == 0x12); // MSB
        }

        SECTION("BIG_ENDIAN: MSB at lower address")
        {
            mm.setEndianess(BIG_ENDIAN);
            REQUIRE(mm.setUInt16(0x0010, 0x1234) == MemMapper::Error::Success);
            REQUIRE(mm.getUInt16(0x0010) == 0x1234);
            REQUIRE(mm.getUInt8(0x0010) == 0x12); // MSB
            REQUIRE(mm.getUInt8(0x0011) == 0x34); // LSB
        }
    }

    // ---------------------------------------------------------------------------

    SECTION("setUInt32 / getUInt32 endianness", "[MemMapper]")
    {
        resetFlash();
        MemMapper mm(TEST_FLASH_BASE, TEST_FLASH_SIZE, false);
        REQUIRE(mm.addRange(0x0000, FLASH_PAGE_SIZE) == MemMapper::Error::Success);

        SECTION("LITTLE_ENDIAN: LSB at lowest address")
        {
            mm.setEndianess(LITTLE_ENDIAN);
            REQUIRE(mm.setUInt32(0x0010, 0x12345678) == MemMapper::Error::Success);
            REQUIRE(mm.getUInt32(0x0010) == 0x12345678u);
            REQUIRE(mm.getUInt8(0x0010) == 0x78);
            REQUIRE(mm.getUInt8(0x0011) == 0x56);
            REQUIRE(mm.getUInt8(0x0012) == 0x34);
            REQUIRE(mm.getUInt8(0x0013) == 0x12);
        }

        SECTION("BIG_ENDIAN: MSB at lowest address")
        {
            mm.setEndianess(BIG_ENDIAN);
            REQUIRE(mm.setUInt32(0x0010, 0x12345678) == MemMapper::Error::Success);
            REQUIRE(mm.getUInt32(0x0010) == 0x12345678u);
            REQUIRE(mm.getUInt8(0x0010) == 0x12);
            REQUIRE(mm.getUInt8(0x0011) == 0x34);
            REQUIRE(mm.getUInt8(0x0012) == 0x56);
            REQUIRE(mm.getUInt8(0x0013) == 0x78);
        }
    }

    // ---------------------------------------------------------------------------

    SECTION("writeMemPtr / readMemPtr", "[MemMapper]")
    {
        resetFlash();
        MemMapper mm(TEST_FLASH_BASE, TEST_FLASH_SIZE, false);
        REQUIRE(mm.addRange(0x0000, FLASH_PAGE_SIZE) == MemMapper::Error::Success);

        SECTION("write and read back a short array within one page")
        {
            uint8_t writeData[] = {0x11, 0x22, 0x33, 0x44, 0x55};
            uint8_t readData[5] = {};
            REQUIRE(mm.writeMemPtr(0x0010, writeData, 5) == MemMapper::Error::Success);
            REQUIRE(mm.readMemPtr(0x0010, readData, 5, false)   == MemMapper::Error::Success);
            REQUIRE(memcmp(writeData, readData, 5) == 0);
        }

        SECTION("write and read back an entire page (256 bytes)")
        {
            uint8_t writeData[FLASH_PAGE_SIZE];
            uint8_t readData[FLASH_PAGE_SIZE] = {};
            for (int i = 0; i < FLASH_PAGE_SIZE; i++)
                writeData[i] = static_cast<uint8_t>(i);
            REQUIRE(mm.writeMemPtr(0x0000, writeData, FLASH_PAGE_SIZE) == MemMapper::Error::Success);
            REQUIRE(mm.readMemPtr(0x0000, readData, FLASH_PAGE_SIZE, false)   == MemMapper::Error::Success);
            REQUIRE(memcmp(writeData, readData, FLASH_PAGE_SIZE) == 0);
        }

        SECTION("write straddling page boundary flushes first page and writes to second")
        {
            REQUIRE(mm.addRange(0x0100, FLASH_PAGE_SIZE) == MemMapper::Error::Success);

            uint8_t writeData[4] = {0xAA, 0xBB, 0xCC, 0xDD};
            // Last 2 bytes of page 0, first 2 bytes of page 1
            REQUIRE(mm.writeMemPtr(0x00FE, writeData, 4) == MemMapper::Error::Success);
            uint8_t readData[4] = {};
            REQUIRE(mm.readMemPtr(0x00FE, readData, 4, false)   == MemMapper::Error::Success);
            REQUIRE(memcmp(writeData, readData, 4) == 0);
        }
    }

    SECTION("memoryPtr", "[MemMapper]")
    {
        SECTION("returns nullptr for an unmapped address")
        {
            resetFlash();
            MemMapper mm(TEST_FLASH_BASE, TEST_FLASH_SIZE, false);
            REQUIRE(mm.memoryPtr(0x0000, false) == nullptr);
        }

        SECTION("returns nullptr for a negative (invalid) address")
        {
            resetFlash();
            MemMapper mm(TEST_FLASH_BASE, TEST_FLASH_SIZE, false);
            REQUIRE(mm.memoryPtr(-1, false) == nullptr);
        }

        SECTION("returns a valid pointer that reads back the written value")
        {
            resetFlash();
            MemMapper mm(TEST_FLASH_BASE, TEST_FLASH_SIZE, false);
            REQUIRE(mm.addRange(0x0000, FLASH_PAGE_SIZE) == MemMapper::Error::Success);
            REQUIRE(mm.writeMem(0x0010, 0xAB) == MemMapper::Error::Success);
            const uint8_t* ptr = mm.memoryPtr(0x0010, false);
            REQUIRE(ptr != nullptr);
            REQUIRE(*ptr == 0xAB);
        }
    }

    SECTION("operator[]", "[MemMapper]")
    {
        resetFlash();
        MemMapper mm(TEST_FLASH_BASE, TEST_FLASH_SIZE, false);
        REQUIRE(mm.addRange(0x0000, FLASH_PAGE_SIZE) == MemMapper::Error::Success);

        SECTION("reads correct value from a mapped address")
        {
            REQUIRE(mm.writeMem(0x0020, 0x42) == MemMapper::Error::Success);
            REQUIRE(mm[0x0020] == 0x42);
        }
    }

    SECTION("doFlash persists data across instances", "[MemMapper]")
    {
        resetFlash();
        {
            MemMapper mm(TEST_FLASH_BASE, TEST_FLASH_SIZE, false);
            REQUIRE(mm.addRange(0x0000, FLASH_PAGE_SIZE) == MemMapper::Error::Success);
            REQUIRE(mm.writeMem(0x0042, 0xAB) == MemMapper::Error::Success);
            REQUIRE(mm.doFlash() == MemMapper::FlashedWriteBuffer); // must have flushed the dirty page
        }

        // A new MemMapper on the same region must find the persisted value.
        MemMapper mm2(TEST_FLASH_BASE, TEST_FLASH_SIZE, false);
        REQUIRE(mm2.isMapped(0x0000) == true);
        uint8_t data = 0;
        REQUIRE(mm2.readMem(0x0042, data) == MemMapper::Error::Success);
        REQUIRE(data == 0xAB);
    }

    SECTION("out of memory", "[MemMapper]")
    {
        // TEST_FLASH_SIZE = 0x1000 → 16 pages: 1 alloc table + 15 data pages.

        SECTION("filling all 15 available data pages succeeds")
        {
            resetFlash();
            MemMapper mm(TEST_FLASH_BASE, TEST_FLASH_SIZE, false);
            REQUIRE(mm.addRange(0, 15 * FLASH_PAGE_SIZE) == MemMapper::Error::Success);
            for (int page = 0; page < 15; page++)
            {
                REQUIRE(mm.isMapped(page * FLASH_PAGE_SIZE) == true);
            }
        }

        SECTION("allocating a 16th data page returns OUT_OF_MEMORY")
        {
            resetFlash();
            MemMapper mm(TEST_FLASH_BASE, TEST_FLASH_SIZE, false);
            REQUIRE(mm.addRange(0, 15 * FLASH_PAGE_SIZE) == MemMapper::Error::Success);
            REQUIRE(mm.addRange(15 * FLASH_PAGE_SIZE, FLASH_PAGE_SIZE) == MemMapper::Error::OutOfMemory);
        }
    }
}

TEST_CASE("allocating a 16th data page returns OUT_OF_MEMORY")
{
    resetFlash();
    MemMapper mm(TEST_FLASH_BASE, TEST_FLASH_SIZE, false);
    REQUIRE(mm.addRange(0, 15 * FLASH_PAGE_SIZE) == MemMapper::Error::Success);
    REQUIRE(mm.addRange(15 * FLASH_PAGE_SIZE, FLASH_PAGE_SIZE) == MemMapper::Error::OutOfMemory);
}