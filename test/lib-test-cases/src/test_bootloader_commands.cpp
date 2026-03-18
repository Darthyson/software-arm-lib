/*
 * Tests for bootloader_commands.h
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 3 as
 * published by the Free Software Foundation.
 */

#include <sblib/internal/bootloader_commands.h>
#include <sblib/eib/apci.h>
#include <catch.hpp> // If possible, include catch.hpp as the last header

// Note: The magic word is stored separately in memory, not in the descriptor struct itself
constexpr uint32_t UID_BOOTLOADER_DESCRIPTOR = 0x5E1FB055;
constexpr uint8_t eraseCode = T_MASTERRESET_FACTORY_WO_IA;
constexpr uint8_t channelNumber = 255;

TEST_CASE("initBootloaderDescriptor", "[bootloader_commands]")
{
    SECTION("Initialize with valid parameters")
    {
        constexpr uint16_t physAddr = 0x1234;
        constexpr uint32_t gpioButton = 0x5678;
        constexpr uint32_t appId = 0xABCD;
        constexpr uint32_t appVer = 0x0102;

        initBootloaderDescriptor(BootState::BootLoader, physAddr, gpioButton, appId, appVer);

        const BootloaderDescriptor* descriptor = getBootloaderDescriptor();
        REQUIRE(descriptor != nullptr);
        REQUIRE(descriptor->bootState == BootState::BootLoader);
        REQUIRE(descriptor->physicalAddress == physAddr);
        REQUIRE(descriptor->programmingButton == gpioButton);
        REQUIRE(descriptor->applicationId == appId);
        REQUIRE(descriptor->applicationVersion == appVer);
    }

    SECTION("Initialize with zero values")
    {
        initBootloaderDescriptor(BootState::Reset, 0, 0, 0, 0);

        const BootloaderDescriptor* descriptor = getBootloaderDescriptor();
        REQUIRE(descriptor != nullptr);
        REQUIRE(descriptor->bootState == BootState::Reset);
        REQUIRE(descriptor->physicalAddress == 0);
        REQUIRE(descriptor->programmingButton == 0);
        REQUIRE(descriptor->applicationId == 0);
        REQUIRE(descriptor->applicationVersion == 0);
    }

    SECTION("Initialize with maximum values")
    {
        initBootloaderDescriptor(BootState::BootLoaderUpdater, 0xFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF);

        const BootloaderDescriptor* descriptor = getBootloaderDescriptor();
        REQUIRE(descriptor != nullptr);
        REQUIRE(descriptor->bootState == BootState::BootLoaderUpdater);
        REQUIRE(descriptor->physicalAddress == 0xFFFF);
        REQUIRE(descriptor->programmingButton == 0xFFFFFFFF);
        REQUIRE(descriptor->applicationId == 0xFFFFFFFF);
        REQUIRE(descriptor->applicationVersion == 0xFFFFFFFF);
    }

    SECTION("Overwrite existing descriptor")
    {
        // First initialization
        initBootloaderDescriptor(BootState::BootLoader, 0x1111, 0x2222, 0x3333, 0x4444);

        const BootloaderDescriptor* descriptor_1 = getBootloaderDescriptor();
        REQUIRE(descriptor_1 != nullptr);
        REQUIRE(descriptor_1->bootState == BootState::BootLoader);
        REQUIRE(descriptor_1->physicalAddress == 0x1111);

        // Second initialization - should overwrite
        initBootloaderDescriptor(BootState::Reset, 0x5555, 0x6666, 0x7777, 0x8888);

        const BootloaderDescriptor* descriptor_2 = getBootloaderDescriptor();
        REQUIRE(descriptor_2 != nullptr);
        REQUIRE(descriptor_2->bootState == BootState::Reset);
        REQUIRE(descriptor_2->physicalAddress == 0x5555);
        REQUIRE(descriptor_2->programmingButton == 0x6666);
        REQUIRE(descriptor_2->applicationId == 0x7777);
        REQUIRE(descriptor_2->applicationVersion == 0x8888);
    }

    SECTION("Initialize with Application state - descriptor becomes invalid")
    {
        initBootloaderDescriptor(BootState::Application, 0x1234, 0x5678, 0xABCD, 0xEF01);

        // With Application state, the magic word is cleared, making descriptor invalid
        const BootloaderDescriptor* descriptor = getBootloaderDescriptor();
        REQUIRE(descriptor == nullptr);

        // But we can still read the data using debugOnlyBootloaderDescriptor
        const BootloaderDescriptor* debugDesc = debugOnlyBootloaderDescriptor();
        REQUIRE(debugDesc != nullptr);
        REQUIRE(debugDesc->bootState == BootState::Application);
        REQUIRE(debugDesc->physicalAddress == 0x1234);
        REQUIRE(debugDesc->programmingButton == 0x5678);
    }
}

TEST_CASE("getBootloaderDescriptor", "[bootloader_commands]")
{
    SECTION("Get valid descriptor after initialization")
    {
        initBootloaderDescriptor(BootState::BootLoader, 0xABCD, 0x1234, 0x5678, 0x9ABC);

        const BootloaderDescriptor* descriptor = getBootloaderDescriptor();
        REQUIRE(descriptor != nullptr);
        REQUIRE(descriptor->bootState == BootState::BootLoader);
    }

    SECTION("Get descriptor returns nullptr after clear")
    {
        initBootloaderDescriptor(BootState::BootLoader, 0x1234, 0x5678, 0xABCD, 0xEF01);
        REQUIRE(getBootloaderDescriptor() != nullptr);
        REQUIRE(getBootloaderDescriptor()->bootState == BootState::BootLoader);

        clearBootloaderDescriptor();
        REQUIRE(getBootloaderDescriptor() == nullptr);
        // After clear, bootState is set to Application
        REQUIRE(debugOnlyBootloaderDescriptor()->bootState == BootState::Application);
    }

    SECTION("Multiple calls return consistent data")
    {
        initBootloaderDescriptor(BootState::Reset, 0x9999, 0x8888, 0x7777, 0x6666);

        const BootloaderDescriptor* desc1 = getBootloaderDescriptor();
        const BootloaderDescriptor* desc2 = getBootloaderDescriptor();

        REQUIRE(desc1 == desc2);
        REQUIRE(desc1->bootState == desc2->bootState);
        REQUIRE(desc1->physicalAddress == desc2->physicalAddress);
        REQUIRE(desc1->programmingButton == desc2->programmingButton);
        REQUIRE(desc1->applicationId == desc2->applicationId);
        REQUIRE(desc1->applicationVersion == desc2->applicationVersion);
    }

    SECTION("Get descriptor returns nullptr with Application state")
    {
        initBootloaderDescriptor(BootState::Application, 0x1234, 0x5678, 0xABCD, 0xEF01);

        // Application state clears the magic word, so descriptor is invalid
        const BootloaderDescriptor* descriptor = getBootloaderDescriptor();
        REQUIRE(descriptor == nullptr);
    }
}

TEST_CASE("clearBootloaderDescriptor", "[bootloader_commands]")
{
    SECTION("Clear is idempotent")
    {
        initBootloaderDescriptor(BootState::BootLoader, 0x1111, 0x2222, 0x3333, 0x4444);
        clearBootloaderDescriptor();
        REQUIRE(getBootloaderDescriptor() == nullptr);

        // Clearing again should not cause issues
        clearBootloaderDescriptor();
        REQUIRE(getBootloaderDescriptor() == nullptr);
    }

    SECTION("Re-initialize after clear")
    {
        initBootloaderDescriptor(BootState::BootLoader, 0x1234, 0x5678, 0xABCD, 0xEF01);
        clearBootloaderDescriptor();
        REQUIRE(getBootloaderDescriptor() == nullptr);

        // Re-initialize with different values
        initBootloaderDescriptor(BootState::BootLoaderUpdater, 0x9999, 0x8888, 0x7777, 0x6666);

        const BootloaderDescriptor* descriptor = getBootloaderDescriptor();
        REQUIRE(descriptor != nullptr);
        REQUIRE(descriptor->bootState == BootState::BootLoaderUpdater);
        REQUIRE(descriptor->physicalAddress == 0x9999);
        REQUIRE(descriptor->programmingButton == 0x8888);
        REQUIRE(descriptor->applicationId == 0x7777);
        REQUIRE(descriptor->applicationVersion == 0x6666);
    }

    SECTION("Clear sets bootState to Application")
    {
        initBootloaderDescriptor(BootState::Reset, 0x1111, 0x2222, 0x3333, 0x4444);
        REQUIRE(getBootloaderDescriptor() != nullptr);

        clearBootloaderDescriptor();

        // Descriptor is now invalid via getBootloaderDescriptor
        REQUIRE(getBootloaderDescriptor() == nullptr);

        // But we can verify the bootState was set to Application
        const BootloaderDescriptor* debugDesc = debugOnlyBootloaderDescriptor();
        REQUIRE(debugDesc->bootState == BootState::Application);
    }
}

TEST_CASE("debugOnlyBootloaderDescriptor", "[bootloader_commands]")
{
    SECTION("Always returns non-null pointer")
    {
        const BootloaderDescriptor* descriptor = debugOnlyBootloaderDescriptor();
        REQUIRE(descriptor != nullptr);
    }

    SECTION("Returns pointer even when descriptor is invalid")
    {
        clearBootloaderDescriptor();
        REQUIRE(getBootloaderDescriptor() == nullptr);

        // debugOnlyBootloaderDescriptor should still return a pointer
        const BootloaderDescriptor* descriptor = debugOnlyBootloaderDescriptor();
        REQUIRE(descriptor != nullptr);
    }

    SECTION("Points to same location as getBootloaderDescriptor when valid")
    {
        initBootloaderDescriptor(BootState::Reset, 0x1234, 0x5678, 0xABCD, 0xEF01);

        const BootloaderDescriptor* validDesc = getBootloaderDescriptor();
        const BootloaderDescriptor* debugDesc = debugOnlyBootloaderDescriptor();

        REQUIRE(validDesc == debugDesc);
    }

    SECTION("Can read data through debug pointer")
    {
        constexpr uint16_t physAddr = 0xABCD;
        constexpr uint32_t gpioButton = 0x12345678;
        constexpr uint32_t appId = 0x87654321;
        constexpr uint32_t appVer = 0x01020304;

        initBootloaderDescriptor(BootState::BootLoaderUpdater, physAddr, gpioButton, appId, appVer);

        const BootloaderDescriptor* debugDesc = debugOnlyBootloaderDescriptor();
        REQUIRE(debugDesc->bootState == BootState::BootLoaderUpdater);
        REQUIRE(debugDesc->physicalAddress == physAddr);
        REQUIRE(debugDesc->programmingButton == gpioButton);
        REQUIRE(debugDesc->applicationId == appId);
        REQUIRE(debugDesc->applicationVersion == appVer);
    }
}

TEST_CASE("BootloaderDescriptor lifecycle", "[bootloader_commands]")
{
    SECTION("Multiple init-clear cycles")
    {
        const BootState states[] = {BootState::Reset, BootState::BootLoader, BootState::BootLoaderUpdater, BootState::Reset, BootState::BootLoader};
        
        for (uint8_t i = 0; i < 5; i++)
        {
            initBootloaderDescriptor(states[i], i * 0x1000, i * 0x100, i * 0x10, i);

            const BootloaderDescriptor* desc = getBootloaderDescriptor();
            REQUIRE(desc != nullptr);
            REQUIRE(desc->bootState == states[i]);
            REQUIRE(desc->physicalAddress == i * 0x1000);
            REQUIRE(desc->applicationVersion == static_cast<uint32_t>(i));

            clearBootloaderDescriptor();
            REQUIRE(getBootloaderDescriptor() == nullptr);
        }
    }
}

TEST_CASE("BootloaderDescriptor structure size", "[bootloader_commands]")
{
    SECTION("Verify structure size with proper alignment")
    {
        constexpr size_t size = sizeof(BootloaderDescriptor);
        // bootState (1) + reserved (1) + physicalAddress (2) + programmingButton (4) + 
        // applicationId (4) + applicationVersion (4) = 16 bytes
        // Verify 4 byte alignment
        REQUIRE(size % 4 == 0);
    }
}

TEST_CASE("checkApciForMagicWord", "[bootloader_commands]")
{
    SECTION("Valid magic word - T_MASTERRESET_FACTORY_WO_IA and channel 255")
    {
        for (uint16_t code = 0; code <= 0xff; code++)
        {
            for (uint16_t channel = 0; channel <= 0xff; channel++)
            {
                const bool isMagicCode = code == T_MASTERRESET_FACTORY_WO_IA && channel == 255;
                REQUIRE(checkApciForMagicWord(code, channel) == isMagicCode);
            }
        }
    }
}

TEST_CASE("BootState behavior", "[bootloader_commands]")
{
    SECTION("BootLoader state sets magic word")
    {
        initBootloaderDescriptor(BootState::BootLoader, 0x1234, 0x5678, 0xABCD, 0xEF01);
        REQUIRE(getBootloaderDescriptor() != nullptr);
    }

    SECTION("BootLoaderUpdater state sets magic word")
    {
        initBootloaderDescriptor(BootState::BootLoaderUpdater, 0x1234, 0x5678, 0xABCD, 0xEF01);
        REQUIRE(getBootloaderDescriptor() != nullptr);
    }

    SECTION("Reset state sets magic word")
    {
        initBootloaderDescriptor(BootState::Reset, 0x1234, 0x5678, 0xABCD, 0xEF01);
        REQUIRE(getBootloaderDescriptor() != nullptr);
    }

    SECTION("Application state clears magic word")
    {
        // First set a valid descriptor
        initBootloaderDescriptor(BootState::BootLoader, 0x1234, 0x5678, 0xABCD, 0xEF01);
        REQUIRE(getBootloaderDescriptor() != nullptr);

        // Switch to Application state
        initBootloaderDescriptor(BootState::Application, 0x1234, 0x5678, 0xABCD, 0xEF01);
        REQUIRE(getBootloaderDescriptor() == nullptr);
    }

    SECTION("Transition between bootloader states")
    {
        // Start with BootLoader
        initBootloaderDescriptor(BootState::BootLoader, 0x1111, 0x2222, 0x3333, 0x4444);
        const BootloaderDescriptor* desc1 = getBootloaderDescriptor();
        REQUIRE(desc1 != nullptr);
        REQUIRE(desc1->bootState == BootState::BootLoader);

        // Switch to BootLoaderUpdater
        initBootloaderDescriptor(BootState::BootLoaderUpdater, 0x5555, 0x6666, 0x7777, 0x8888);
        const BootloaderDescriptor* desc2 = getBootloaderDescriptor();
        REQUIRE(desc2 != nullptr);
        REQUIRE(desc2->bootState == BootState::BootLoaderUpdater);
        REQUIRE(desc2->physicalAddress == 0x5555);

        // Switch to Reset
        initBootloaderDescriptor(BootState::Reset, 0x9999, 0xAAAA, 0xBBBB, 0xCCCC);
        const BootloaderDescriptor* desc3 = getBootloaderDescriptor();
        REQUIRE(desc3 != nullptr);
        REQUIRE(desc3->bootState == BootState::Reset);
        REQUIRE(desc3->physicalAddress == 0x9999);
    }

    SECTION("Each BootState preserves descriptor data")
    {
        const BootState states[] = {BootState::Reset, BootState::BootLoader, BootState::BootLoaderUpdater};
        
        for (const auto state : states)
        {
            initBootloaderDescriptor(state, 0xABCD, 0x12345678, 0x87654321, 0x01020304);
            
            const BootloaderDescriptor* desc = getBootloaderDescriptor();
            REQUIRE(desc != nullptr);
            REQUIRE(desc->bootState == state);
            REQUIRE(desc->physicalAddress == 0xABCD);
            REQUIRE(desc->programmingButton == 0x12345678);
            REQUIRE(desc->applicationId == 0x87654321);
            REQUIRE(desc->applicationVersion == 0x01020304);
        }
    }
}
