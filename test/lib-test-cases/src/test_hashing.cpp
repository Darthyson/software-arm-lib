/*
 *  Unit tests for hashing function
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 3 as
 * published by the Free Software Foundation.
 */

#include "test_hashing_testcases.h"
#include <sblib/eib/serial_number.h>
#include <sblib/murmur_hash_3.h>
#include <array>
#include <cstring>
#include <iomanip>
#include <map>
#include <sstream>

#include <catch.hpp> ///\todo replace with catch2/catch_test_macros.hpp

///
/// uid test cases ('HashTestCase testCases') are generated with Python script;
///     software-arm-lib/test/lib-test-cases/tools/uid testcase generator/uid_testcase_generator.py
///
/// Online MurmurHash3 calculator:
///     https://www.codertools.net/tools/murmurhash.php
///

auto toHex = [](const uint8_t* data, const size_t len, const char separator)
{
    std::ostringstream oss;
    oss << std::hex << std::uppercase << std::setfill('0');
    for (size_t i = 0; i < len; ++i)
    {
        oss << std::setw(2) << static_cast<int>(data[i]);
        if ((i != len - 1) && (separator != '\0'))
        {
            oss << separator;
        }
    }
    return oss.str();
};

auto testCaseToString = [](const HashTestCase &testCase)
{
    std::ostringstream oss;
    oss << "id=" << testCase.id
        << " uid=" << toHex(testCase.uid, 16, ':')
        << " serial=" << toHex(testCase.serial, 6, '\0')
        << " murMurHash3_x86_32=" << toHex(testCase.murMurHash3_x86_32, 4, '\0');
    return oss.str();
};

TEST_CASE("Hashing", "KNX")
{
    SECTION("Existing uid collision check")
    {
        // search for collisions in old serials created with hashUID(...)
        std::map<std::array<uint8_t, 6>, const HashTestCase*> serialMap;

        for (auto& tc : testCases)
        {
            std::array<uint8_t, 6> s = {};
            std::copy_n(tc.serial, 6, s.begin());
            auto [it, inserted] = serialMap.emplace(s, &tc);
            if (!inserted)
            {
                const auto& partner = *it->second;
                WARN("serial collision detected:"
                    << "\n  existing: " << testCaseToString(partner)
                    << "\n  new:      " << testCaseToString(tc));
            }
        }

        // search for collisions in new murmurhash3
        std::map<std::array<uint8_t, 4>, const HashTestCase*> hashMap;
        for (auto& tc : testCases)
        {
            std::array<uint8_t, 4> h = {};
            std::copy_n(tc.murMurHash3_x86_32, 4, h.begin());
            auto [it, inserted] = hashMap.emplace(h, &tc);
            if (!inserted)
            {
                const auto& partner = *it->second;
                WARN("murMurHash3_x86_32 collision detected:"
                    << "\n  existing: " << testCaseToString(partner)
                    << "\n  new:      " << testCaseToString(tc));
            }
        }
    }

    SECTION("hashUID(...)")
    {
        for (auto& [id, uid, serial, murMurHash3_x86_32] : testCases)
        {
            constexpr uint8_t hashLength = 6;
            uint8_t hash[hashLength];
            hashUID(uid, 16, hash, hashLength);
            REQUIRE(std::vector<uint8_t>(hash, hash + hashLength) == std::vector<uint8_t>(serial, serial + hashLength));
        }
    }

    SECTION("MurmurHash3_x86_32(...)")
    {
        for (auto& [id, uid, serial, murMurHash3_x86_32] : testCases)
        {
            uint8_t newHash[4];
            murmurHash3_x86_32(uid, 16, 0, newHash);
            std::swap(newHash[0], newHash[3]);
            std::swap(newHash[1], newHash[2]);
            REQUIRE(std::vector<uint8_t>(newHash, newHash + 4) == std::vector<uint8_t>(murMurHash3_x86_32, murMurHash3_x86_32 + 4));
        }
    }
}

TEST_CASE("KNX serial number generation", "KNX")
{
    REQUIRE(KNX_SERIAL_NUMBER_LENGTH == 6);
    REQUIRE(KNX_SERIAL_NUMBER_MANUFACTURER_ID_HIGH_BYTE == 0x01);
    REQUIRE(KNX_SERIAL_NUMBER_MANUFACTURER_ID_LOW_BYTE == 0x3A);

    SECTION("createKNXSerial(...)")
    {
        constexpr uint8_t DATA_SIZE = 16;
        uint8_t data[DATA_SIZE];
        uint8_t newSerial[KNX_SERIAL_NUMBER_LENGTH];
        REQUIRE(createKNXSerial(data, 0, newSerial, KNX_SERIAL_NUMBER_LENGTH) == false);
        REQUIRE(createKNXSerial(data, DATA_SIZE, newSerial, 0) == false);
        REQUIRE(createKNXSerial(nullptr, DATA_SIZE, newSerial, KNX_SERIAL_NUMBER_LENGTH) == false);
        REQUIRE(createKNXSerial(data, DATA_SIZE, nullptr, KNX_SERIAL_NUMBER_LENGTH) == false);

        uint32_t checkedSerialCounter = 0;
        uint32_t collisionCount = 0;
        std::map<std::array<uint8_t, 6>, const HashTestCase*> knxSerialMap;
        for (auto& tc : testCases)
        {
            /// Checking ~90 guids the last 3 bytes are always 0x1c, 0x00, 0xf5,
            /// 0xff1x00f5 indicates that the last 4 bytes of the uid are undefined,
            /// so we iterate over all possible values of byte 12 to check for collisions.
            bool uidIsMissingLast4Bytes = (tc.uid[12] == 0xff) &&
                                          (tc.uid[13] == 0x1c) &&
                                          (tc.uid[14] == 0x00) &&
                                          (tc.uid[15] == 0xf5);
            for (int32_t byte12Value = 0; byte12Value <= 0xff; byte12Value++)
            {
                if (uidIsMissingLast4Bytes)
                {
                    // Last 4 bytes of uid are undefined, so we iterate over all values of byte 12 to find any collisions
                    tc.uid[12] = static_cast<uint8_t>(byte12Value);
                }

                REQUIRE(createKNXSerial(tc.uid, 16, newSerial, KNX_SERIAL_NUMBER_LENGTH) == true);
                REQUIRE(newSerial[0] == KNX_SERIAL_NUMBER_MANUFACTURER_ID_HIGH_BYTE);
                REQUIRE(newSerial[1] == KNX_SERIAL_NUMBER_MANUFACTURER_ID_LOW_BYTE);

                if (!uidIsMissingLast4Bytes)
                {
                    // Compare only against "saved" MurmurHast3 if it's a defined uid
                    REQUIRE(std::vector<uint8_t>(newSerial + 2, newSerial + 6) ==
                            std::vector<uint8_t>(tc.murMurHash3_x86_32, tc.murMurHash3_x86_32 + 4));
                }

                std::array<uint8_t, 6> s = {};
                std::copy_n(newSerial, 6, s.begin());
                auto [it, inserted] = knxSerialMap.emplace(s, &tc);
                if (!inserted)
                {
                    WARN("createKNXSerial collision detected:"
                        << "\n  existing:    " << testCaseToString(*it->second)
                        << "\n  new:         " << testCaseToString(tc)
                        << "\n  byte12Value: " << byte12Value);
                    collisionCount++;
                }
                CHECK(inserted == true);
                if (!uidIsMissingLast4Bytes)
                {
                    // If the last 4 bytes of the uid are defined, we don't need to iterate over all byte 12 values
                    break;
                }
                checkedSerialCounter++;
            }
        }
        WARN("createKNXSerial() found " << collisionCount << " collisions after checking " << checkedSerialCounter
             << " KNX serial numbers.");
        REQUIRE(collisionCount == 0);
    }

    ///\todo Delete this section after switching to new MurmurHash3 serial generation
    /// This section is 95% copy & paste of above SECTION("createKNXSerial(...)")
    SECTION("hashUID(...)")
    {
        uint8_t newSerial[KNX_SERIAL_NUMBER_LENGTH];

        uint32_t checkedSerialCounter = 0;
        uint32_t collisionCount = 0;
        std::map<std::array<uint8_t, 6>, const HashTestCase*> knxSerialMap;
        for (auto& tc : testCases)
        {
            /// Checking ~90 guids the last 3 bytes are always 0x1c, 0x00, 0xf5,
            /// 0xff1x00f5 indicates that the last 4 bytes of the uid are undefined,
            /// so we iterate over all possible values of byte 12 to check for collisions.
            bool uidIsMissingLast4Bytes = (tc.uid[12] == 0xff) &&
                                          (tc.uid[13] == 0x1c) &&
                                          (tc.uid[14] == 0x00) &&
                                          (tc.uid[15] == 0xf5);
            for (int32_t byte12Value = 0; byte12Value <= 0xff; byte12Value++)
            {
                if (uidIsMissingLast4Bytes)
                {
                    // Last 4 bytes of uid are undefined, so we iterate over all values of byte 12 to find any collisions
                    tc.uid[12] = static_cast<uint8_t>(byte12Value);
                }

                REQUIRE(hashUID(tc.uid, 16, newSerial, KNX_SERIAL_NUMBER_LENGTH) == true);
                if (!uidIsMissingLast4Bytes)
                {
                    // Compare only against "saved" MurmurHast3 if it's a defined uid
                    REQUIRE(std::vector<uint8_t>(newSerial, newSerial + 6) ==
                            std::vector<uint8_t>(tc.serial, tc.serial + 6));
                }

                std::array<uint8_t, 6> s = {};
                std::copy_n(newSerial, 6, s.begin());
                auto [it, inserted] = knxSerialMap.emplace(s, &tc);
                if (!inserted)
                {
                    INFO("hashUID collision detected:"
                        << "\n  existing:    " << testCaseToString(*it->second)
                        << "\n  new:         " << testCaseToString(tc)
                        << "\n  byte12Value: " << byte12Value);
                    collisionCount++;
                }

                if (!uidIsMissingLast4Bytes)
                {
                    // If the last 4 bytes of the uid are defined, we don't need to iterate over all byte 12 values
                    break;
                }
                checkedSerialCounter++;
            }
        }
        WARN("hashUID() found " << collisionCount << " collisions after checking " << checkedSerialCounter
             << " KNX serial numbers.");
    }
}