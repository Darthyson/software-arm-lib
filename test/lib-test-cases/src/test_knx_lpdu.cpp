/**************************************************************************//**
 * @addtogroup SBLIB_MAIN_GROUP Selfbus KNX-Library
 * @defgroup SBLIB_SUB_GROUP_TEST KNX LPDU Unit Test
 * @ingroup SBLIB_MAIN_GROUP
 * @brief   
 * @details 
 *
 *
 * @{
 *
 * @file   test_knx_lpdu.cpp
 * @author Darthyson <darth@maptrack.de> Copyright (c) 2022
 * @bug No known bugs.
 ******************************************************************************/

/*
 This program is free software; you can redistribute it and/or modify
 it under the terms of the GNU General Public License version 3 as
 published by the Free Software Foundation.
 ---------------------------------------------------------------------------*/

#include <string.h>
 
#include <catch.hpp> // If possible, include catch.hpp as last header

#include <sblib/eib/knx_lpdu.h>
 

const int testTelLength = 24;
unsigned char testTelegram[testTelLength];

TEST_CASE("LPDU processing","[SBLIB][KNX][LPDU]")
{
    byte testByte;
    char msg[200];

    for (int i = 0; i <= 0xFF; i++)
    {
        // fill telegram with some data
        for (int j = 0; j < testTelLength; j++)
        {
            testTelegram[j] = j;
        }
        // set controlByte data
        testTelegram[0] = i;
        snprintf(msg, sizeof(msg)/sizeof(msg[0]) - 1, "LPDU controlByte=0x%02X", testTelegram[0]);
        INFO(msg);

        // now let's test some stuff
        // check for correct returned controlByte data
        REQUIRE(controlByte(testTelegram) == i);

        // set repeated flag
        testTelegram[0] = testTelegram[0] & 0b11011111; // 5.bit 0 = repeated
        REQUIRE(isRepeated(testTelegram) == true);

        // unset repeated flag
        testTelegram[0] |= 0b00100000; // 5.bit 1 = not repeated
        REQUIRE(isRepeated(testTelegram) == false);

        // check setting of repeated = false
        testTelegram[0] = i;
        testByte = testTelegram[0] | 0b00100000; // 5.bit 1 = not repeated
        setRepeated(testTelegram, false);
        REQUIRE(testByte == testTelegram[0]);

        // check setting of repeated = true
        testTelegram[0] = i;
        testByte = testTelegram[0] & 0b11011111; // 5.bit 0 = repeated
        setRepeated(testTelegram, true);
        REQUIRE(testByte == testTelegram[0]);

        // check setting and getting of priority low
        testTelegram[0] = i;
        testByte = testTelegram[0] | 0b00001100; // 2. & 3.bit set = PRIORITY_LOW
        setPriority(testTelegram, PRIORITY_LOW);
        REQUIRE(testByte == testTelegram[0]);
        REQUIRE(PRIORITY_LOW == priority(testTelegram));

        // check setting and getting of priority high
        testTelegram[0] = i;
        testByte = (testTelegram[0] | 0b00000100) & (0b11110111); // 2.bit set & 3.bit unset = PRIORITY_HIGH
        setPriority(testTelegram, PRIORITY_HIGH);
        REQUIRE(testByte == testTelegram[0]);
        REQUIRE(PRIORITY_HIGH == priority(testTelegram));

        // check setting and getting of priority alarm
        testTelegram[0] = i;
        testByte = (testTelegram[0] | 0b00001000) & (0b11111011);; // 2.bit unset & 3.bit set = PRIORITY_ALARM
        setPriority(testTelegram, PRIORITY_ALARM);
        REQUIRE(testByte == testTelegram[0]);
        REQUIRE(PRIORITY_ALARM == priority(testTelegram));

        // check setting and getting of priority system
        testTelegram[0] = i;
        testByte = testTelegram[0] & 0b11110011; // 2. & 3.bit unset = PRIORITY_SYSTEM
        setPriority(testTelegram, PRIORITY_SYSTEM);
        REQUIRE(testByte == testTelegram[0]);
        REQUIRE(PRIORITY_SYSTEM == priority(testTelegram));

        // set extended frame type
        testTelegram[0] = testTelegram[0] & 0b01111111; // 7.bit 0 = extended frame length
        REQUIRE(frameType(testTelegram) == FRAME_EXTENDED);

        // set standard frame type
        testTelegram[0] |= 0b10000000; // 7.bit 1 = standard frame length
        REQUIRE(frameType(testTelegram) == FRAME_STANDARD);

        // check setting of standard frame type
        testTelegram[0] = i;
        testByte = testTelegram[0] | 0b10000000; // 7.bit 1 = standard frame length
        setFrameType(testTelegram, FRAME_STANDARD);
        REQUIRE(testByte == testTelegram[0]);

        // check setting of extended frame type
        testTelegram[0] = i;
        testByte = testTelegram[0] & 0b01111111; // 7.bit 0 = extended frame length
        setFrameType(testTelegram, FRAME_EXTENDED);
        REQUIRE(testByte == testTelegram[0]);

    }
}

TEST_CASE("LPDU sender/destination address processing","[SBLIB][KNX][LPDU]")
{
    unsigned short addr;
    byte highByte;
    byte lowByte;

    for (int i = 0; i <= 255; i++)
    {
        highByte = i;
        for (int j = 0; j <= 255; j++)
        {
            lowByte = j;
            addr = (highByte << 8) | lowByte;
            setDestinationAddress(testTelegram, addr);
            // check setting of destination address
            REQUIRE(addr == ((testTelegram[3] << 8) | testTelegram[4]));
            // check getting of destination address
            REQUIRE(addr == destinationAddress(testTelegram));
            // check getting of sender address
            testTelegram[1] = highByte;
            testTelegram[2] = lowByte;
            REQUIRE(addr == senderAddress(testTelegram));
        }
    }
}

TEST_CASE("LPDU initialization","[SBLIB][KNX][LPDU]")
{
    unsigned char testTelegram[24];
    memset(testTelegram, 0, sizeof(testTelegram));
    initLpdu(testTelegram, PRIORITY_LOW, false, FRAME_STANDARD);
    REQUIRE(testTelegram[0] ==  0xbc);
}

TEST_CASE("KNX physical address parsing","[SBLIB][KNX][LPDU]")
{
    // Test default KNX address
    REQUIRE(physAddressToArea(PHY_ADDR_DEFAULT) == 15);
    REQUIRE(physAddressToLine(PHY_ADDR_DEFAULT) == 15);
    REQUIRE(physAddressToDevice(PHY_ADDR_DEFAULT) == 255);
    
    // Test broadcast address
    REQUIRE(physAddressToArea(PHY_ADDR_BROADCAST) == 0);
    REQUIRE(physAddressToLine(PHY_ADDR_BROADCAST) == 0);
    REQUIRE(physAddressToDevice(PHY_ADDR_BROADCAST) == 0);

    // Test all possible values for Area, Line, and Device
    // Area: 4 bits (0-15), Line: 4 bits (0-15), Device: 8 bits (0-255)
    for (uint8_t area = 0; area <= 15; area++)
    {
        for (uint8_t line = 0; line <= 15; line++)
        {
            for (uint16_t device = 0; device <= 255; device++)
            {
                // Construct KNX physical address: Area.Line.Device
                // Area: bits 12-15, Line: bits 8-11, Device: bits 0-7
                uint16_t address = (area << 12) | (line << 8) | device;
                
                // Verify each function extracts the correct component
                REQUIRE(physAddressToArea(address) == area);
                REQUIRE(physAddressToLine(address) == line);
                REQUIRE(physAddressToDevice(address) == device);
            }
        }
    }
}

TEST_CASE("KNX group address parsing","[SBLIB][KNX][LPDU]")
{
    // Test all possible values for main, middle, and low
    // Main: 5 bits (0-31), Middle: 3 bits (0-7), Low: 8 bits (0-255)
    for (uint8_t mainGroup = 0; mainGroup <= 31; mainGroup++)
    {
        for (uint8_t middleGroup = 0; middleGroup <= 7; middleGroup++)
        {
            for (uint16_t lowGroup = 0; lowGroup <= 255; lowGroup++)
            {
                // Construct KNX group address: main/middle/low
                // Main: bits 11-15, Line: bits 8-10, Device: bits 0-7
                uint16_t address = (mainGroup << 11) | (middleGroup << 8) | lowGroup;

                // Verify each function extracts the correct component
                REQUIRE(mainGroupAddress(address) == mainGroup);
                REQUIRE(middleGroupAddress(address) == middleGroup);
                REQUIRE(lowGroupAddress(address) == lowGroup);
            }
        }
    }
}

/** @}*/
