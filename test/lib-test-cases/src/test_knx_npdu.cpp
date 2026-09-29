/**************************************************************************//**
 * @addtogroup SBLIB_MAIN_GROUP Selfbus KNX-Library
 * @defgroup SBLIB_SUB_GROUP_TEST KNX NPDU Unit Test
 * @ingroup SBLIB_MAIN_GROUP
 * @brief
 * @details
 *
 *
 * @{
 *
 * @file   test_knx_npdu.cpp
 * @bug No known bugs.
 ******************************************************************************/

/*
 This program is free software; you can redistribute it and/or modify
 it under the terms of the GNU General Public License version 3 as
 published by the Free Software Foundation.
 ---------------------------------------------------------------------------*/

#include <cstring>

#include <catch.hpp> // If possible, include catch.hpp as last header

#include <sblib/eib/knx_lpdu.h>
#include <sblib/eib/knx_npdu.h>

TEST_CASE("NPDU telegram size","[SBLIB][KNX][NPDU]")
{
    uint8_t testTelegram[24];

    SECTION("standard frame")
    {
        memset(testTelegram, 0, sizeof(testTelegram) / sizeof(testTelegram[0]));
        initLpdu(testTelegram, PRIORITY_LOW, false, FRAME_STANDARD);
        for (uint8_t length = 0; length <= 15; length++)
        {
            // routing counter and address type in the high nibble must not change the size
            testTelegram[5] = 0xE0 | length;
            REQUIRE(telegramSize(testTelegram) == 7 + length);
        }
    }

    SECTION("extended frame")
    {
        memset(testTelegram, 0, sizeof(testTelegram) / sizeof(testTelegram[0]));
        initLpdu(testTelegram, PRIORITY_LOW, false, FRAME_EXTENDED);
        // byte 5 holds the low byte of the destination address in an extended frame
        testTelegram[5] = 0xFF;
        for (uint16_t length = 0; length <= 255; length++)
        {
            testTelegram[6] = static_cast<uint8_t>(length);
            REQUIRE(telegramSize(testTelegram) == 8 + length);
        }
    }
}

/** @}*/
