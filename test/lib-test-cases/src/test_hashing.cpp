/*
 *  Unit tests for hashing function
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 3 as
 * published by the Free Software Foundation.
 */

#include "test_hashing_testcases.h"
#include <sblib/utils.h>
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

auto testCaseToString = [](const HashTestCase testCase)
{
    std::ostringstream oss;
    oss << "id=" << testCase.id
        << " uid=" << toHex(testCase.uid, 16, ':')
        << " serial=" << toHex(testCase.serial, 6, '\0')
        << " murMurHash3_x86_32=" << toHex(testCase.murMurHash3_x86_32, 4, '\0');
    return oss.str();
};

TEST_CASE("Hashing", "hash")
{
    SECTION("Existing uid collision check")
    {
        std::map<std::array<uint8_t, 6>, const HashTestCase*> serialMap;
        std::map<std::array<uint8_t, 4>, const HashTestCase*> hashMap;

        // search for collisions in old serial
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