/*
 * Unit tests for class Stream
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 3 as
 * published by the Free Software Foundation.
 */

#include <sblib/stream.h>
#include <system_time_simulator.h>
#include <string>
#include <vector>
#include <cstring>

#include <catch.hpp>


/**
 * Mock implementation of Stream class for testing.
 * Uses an internal buffer to simulate incoming data.
 */
class MockStream final : public Stream
{
public:
    std::vector<byte> buffer;
    int readPos = 0;

    /*********************+***
     * Virtual Print methods *
     *********************+***/
    uint32_t write(const byte ch) override
    {
        buffer.push_back(ch);
        return 1;
    }

    using Print::write;

    /***********************+**
     * Virtual Stream methods *
     **********************+***/
    int read() override
    {
        if (readPos >= static_cast<int>(buffer.size()))
        {
            return -1;
        }
        // return the next byte and advance the read position
        return buffer.at(readPos++);
    }

    int peek() override
    {
        if (readPos >= static_cast<int>(buffer.size()))
        {
            return -1;
        }

        return buffer.at(readPos);
    }

    int available() override
    {
        return static_cast<int>(buffer.size()) - readPos;
    }

    void flush() override
    {
        buffer.clear();
        readPos = 0;
    }

    /********************+**
     * Test helper methods *
     *******************+***/
    void setInput(const char* data)
    {
        buffer.assign(data, data + strlen(data));
        readPos = 0;
    }

    void setInput(const std::vector<byte>& data)
    {
        buffer = data;
        readPos = 0;
    }

    using Stream::timeout;
    using Stream::timedPeek;
    using Stream::peekNextDigit;
};


TEST_CASE("Stream::", "[stream]")
{
    // system time simulator for timedPeek and other functions,
    // which are using functions of timer.h
    SystemTimeSimulator sim(1);
    sim.start();
    MockStream mock;

    SECTION("setTimeout")
    {
        mock.setTimeout(500);
        REQUIRE(mock.timeout == 500);
        mock.setTimeout(0);
        REQUIRE(mock.timeout == 0);
    }

    struct PeekTestCase {
        const char* input;
        const uint32_t timeoutToSet;
        const int expectedPeek;
        const int expectedPeekDigit;
        const int32_t expectedNewReadPos;
        const char* info;
    };

    SECTION("peek(), timedPeek() and peekNextDigit()")
    {
        // timedPeek() returns the first byte as-is (without advancing readPos), or -1 on timeout.
        // Non-digit/non-minus characters are also returned by timedPeek() without discarding.
        constexpr PeekTestCase peekTestCases[] = {
            {" ",      0, ' ',    -1, 1, "Simple first char test without timeout"},
            {"TestA",  0, 'T',    -1, 5, "Simple Test without timeout"},
            {"Test",  50, 'T',    -1, 4, "Simple Test with timeout"},
            {"est",   10, 'e',    -1, 3, "peek should never advance readPos"},
            {"",      50, -1,     -1, 0, "No data available"},
            {"0",     50, '0',   '0', 0, "Zero digit"},
            {"5",     50, '5',   '5', 0, "Single non-zero digit"},
            {"-42",   50, '-',   '-', 0, "Minus sign as first character"},
            {"+42",   50, '+',   '4', 1, "Plus sign as first character"},
            {"123",   50, '1',   '1', 0, "Multi-digit number"},
            {" test", 50, ' ',    -1, 5, "Space character"},
            {"\x01",  50,   1,    -1, 1, "Minimum positive byte value"},
            {"\xff",  50, 255,    -1, 1, "Maximum positive byte value"},
            {"0",     10, '0',   '0', 0, "0 - decimal digit"},
            {"1",     10, '1',   '1', 0, "1 - decimal digit"},
            {"2",     10, '2',   '2', 0, "2 - Valid decimal digit"},
            {"3",     10, '3',   '3', 0, "3 - Valid decimal digit"},
            {"4",     10, '4',   '4', 0, "4 - Valid decimal digit"},
            {"5",     10, '5',   '5', 0, "5 - Valid decimal digit"},
            {"6",     10, '6',   '6', 0, "6 - Valid decimal digit"},
            {"7",     10, '7',   '7', 0, "7 - Valid decimal digit"},
            {"8",     10, '8',   '8', 0, "8 - Valid decimal digit"},
            {"9",     10, '9',   '9', 0, "9 - Valid decimal digit"},
            {"a0",    10, 'a',   '0', 1, "'a' followed by '0'"},
            {"bc9",   10, 'b',   '9', 2, "'b' followed by '9'"},
            {"123",  500, '1',   '1', 0, "Multi-digit number"},
            {"ff",    10, 'f',    -1, 2, "Hex number"},
        };

        for (const auto&[input, timeoutToSet, expectedPeek, expectedPeekDigit, expectedNewReadPos, info] : peekTestCases)
        {
            mock.setTimeout(timeoutToSet);
            mock.setInput(input);
            int32_t oldReadPos = mock.readPos;

            INFO("mock.peek() " << info);
            REQUIRE(mock.peek() == expectedPeek);
            REQUIRE(mock.readPos == oldReadPos);

            INFO("mock.timedPeek() " << info);
            REQUIRE(mock.timedPeek() == expectedPeek);
            REQUIRE(mock.readPos == oldReadPos);

            INFO("mock.peekNextDigit()  " << info);
            REQUIRE(mock.peekNextDigit() == expectedPeekDigit);
            REQUIRE(mock.readPos == expectedNewReadPos);
        }
    }

    SECTION("parseInt()")
    {
        struct ParseIntTestCase {
            const char* input;
            int result;
            int newReadPos;
        };

        ParseIntTestCase parseTestCases[] = {
            {"123 ", 123, 3},
            {"-456 ", -456, 4},
            {"abc123 ", 123, 6},
            {"0 ", 0, 1},
            {"98765 ", 98765, 5},
            {"", 0, 0},
            {"abcd", 0, 4},
        };

        for (const auto&[input, result, newReadPos] : parseTestCases)
        {
            mock.setInput(input);
            int parsedResult = mock.parseInt();
            INFO("Expected '" << result << "' after parsing '" << input <<"', but got: '" << parsedResult << "'");
            REQUIRE(parsedResult == result);
            INFO("Expected readPos=" << newReadPos << " after parsing '" << input <<"', but got: " << mock.readPos);
            REQUIRE(mock.readPos == newReadPos);
        }
    }

    SECTION("parseInt(skipChar)")
    {
        struct ParseIntSkipTestCase {
            const char* input;
            int result;
            char skipCHar;
        };
        ParseIntSkipTestCase parseSkipTestCases[] = {
            {"1.234 ", 1234, '.'},
            {"-456,789 ", -456789, ','},
            {"abc 1 2 3 ", 123,' '},
            {"-0 ", 0, 'a'},
            {"+98765.12 ", 9876512, '.'},
        };

        for (const auto&[input, result, skipCHar] : parseSkipTestCases)
        {
            mock.setInput(input);
            int parsedResult = mock.parseInt(skipCHar);
            INFO("Expected '" << result << "' after parsing '" << input <<"', but got: '" << parsedResult << "'");
            REQUIRE(parsedResult == result);
        }
    }

    SECTION("readBytes(char*, int)")
    {
        SECTION("Read exact number of bytes")
        {
            mock.setInput("Hello");
            char buffer[6] = {};
            int count = mock.readBytes(buffer, 5);
            REQUIRE(count == 5);
            REQUIRE(std::string(buffer, 5) == "Hello");
        }

        SECTION("Read fewer bytes than available")
        {
            mock.setInput("Hello World");
            char buffer[4] = {};
            int count = mock.readBytes(buffer, 3);
            REQUIRE(count == 3);
            REQUIRE(std::string(buffer, 3) == "Hel");
        }

        SECTION("Read zero bytes")
        {
            mock.setInput("Hello");
            char buffer[1] = {};
            int count = mock.readBytes(buffer, 0);
            REQUIRE(count == 0);
        }
    }

    SECTION("readBytes(byte*, int)")
    {
        SECTION("Read bytes into byte buffer")
        {
            mock.setInput("ABC");
            byte buffer[4] = {};
            int count = mock.readBytes(buffer, 3);
            REQUIRE(count == 3);
            REQUIRE(buffer[0] == 'A');
            REQUIRE(buffer[1] == 'B');
            REQUIRE(buffer[2] == 'C');
        }
    }

    SECTION("readBytesUntil(char, char*, int)")
    {
        SECTION("Read until terminator found")
        {
            mock.setInput("Hello\nWorld");
            char buffer[12] = {};
            int count = mock.readBytesUntil('\n', buffer, 11);
            REQUIRE(count == 5);
            REQUIRE(std::string(buffer, 5) == "Hello");
        }

        SECTION("Read until length reached before terminator")
        {
            mock.setInput("Hello\nWorld");
            char buffer[4] = {};
            int count = mock.readBytesUntil('\n', buffer, 3);
            REQUIRE(count == 3);
            REQUIRE(std::string(buffer, 3) == "Hel");
        }

        SECTION("Terminator not present reads all available")
        {
            mock.setInput("Hello");
            char buffer[10] = {};
            int count = mock.readBytesUntil('\n', buffer, 10);
            REQUIRE(count == 5);
            REQUIRE(std::string(buffer, 5) == "Hello");
        }
    }

    SECTION("readBytesUntil(char, byte*, int)")
    {
        SECTION("Read into byte buffer until terminator")
        {
            mock.setInput("AB;CD");
            byte buffer[6] = {};
            int count = mock.readBytesUntil(';', buffer, 5);
            REQUIRE(count == 2);
            REQUIRE(buffer[0] == 'A');
            REQUIRE(buffer[1] == 'B');
        }
    }

    SECTION("find(const char*)")
    {
        SECTION("Find existing string")
        {
            mock.setInput("Hello World");
            REQUIRE(mock.find("World") == true);
        }

        SECTION("Find string at beginning")
        {
            mock.setInput("Hello World");
            REQUIRE(mock.find("Hello") == true);
        }

        SECTION("String not found")
        {
            mock.setInput("Hello World");
            REQUIRE(mock.find("xyz") == false);
        }

        SECTION("Find single character string")
        {
            mock.setInput("abcdef");
            REQUIRE(mock.find("d") == true);
        }
    }

    SECTION("find(const char*, int)")
    {
        SECTION("Find with explicit length")
        {
            mock.setInput("Hello World");
            REQUIRE(mock.find("World", 5) == true);
        }

        SECTION("Find partial match with length")
        {
            mock.setInput("Hello World");
            REQUIRE(mock.find("Wor", 3) == true);
        }
    }

    SECTION("find(const byte*)")
    {
        SECTION("Find byte string")
        {
            mock.setInput("Hello World");
            const byte target[] = "World";
            REQUIRE(mock.find(target) == true);
        }
    }

    SECTION("find(const byte*, int)")
    {
        SECTION("Find byte string with length")
        {
            mock.setInput("Hello World");
            const byte target[] = "World";
            REQUIRE(mock.find(target, 5) == true);
        }
    }

    SECTION("findUntil(const char*, const char*)")
    {
        SECTION("Find target before terminator")
        {
            mock.setInput("Hello World End");
            REQUIRE(mock.findUntil("World", "End") == true);
        }

        SECTION("Terminator found before target")
        {
            mock.setInput("Hello End World");
            REQUIRE(mock.findUntil("World", "End") == false);
        }
    }

    SECTION("findUntil(const byte*, const char*)")
    {
        SECTION("Find byte target before terminator")
        {
            mock.setInput("Hello World End");
            const byte target[] = "World";
            REQUIRE(mock.findUntil(target, "End") == true);
        }
    }

    SECTION("findUntil(const char*, int, const char*, int)")
    {
        SECTION("Find with lengths, target before terminator")
        {
            mock.setInput("Hello World End");
            REQUIRE(mock.findUntil("World", 5, "End", 3) == true);
        }

        SECTION("Find with lengths, terminator before target")
        {
            mock.setInput("Hello End World");
            REQUIRE(mock.findUntil("World", 5, "End", 3) == false);
        }

        SECTION("Find with no terminator (null, 0)")
        {
            mock.setInput("Hello World");
            REQUIRE(mock.findUntil("World", 5, (const char*)0, 0) == true);
        }

        SECTION("Target not found, no terminator")
        {
            mock.setInput("Hello");
            REQUIRE(mock.findUntil("xyz", 3, (const char*)0, 0) == false);
        }
    }

    SECTION("findUntil(const byte*, int, const char*, int)")
    {

        SECTION("Find byte target with lengths")
        {
            mock.setInput("Hello World End");
            constexpr byte target[] = "World";
            REQUIRE(mock.findUntil(target, 5, "End", 3) == true);
        }
    }

    SECTION("read()")
    {
        SECTION("Read returns bytes in order")
        {
            mock.setInput("ABC");
            REQUIRE(mock.read() == 'A');
            REQUIRE(mock.read() == 'B');
            REQUIRE(mock.read() == 'C');
        }

        SECTION("Read returns -1 when empty")
        {
            mock.setInput("");
            REQUIRE(mock.read() == -1);
        }

        SECTION("Read returns -1 after all bytes consumed")
        {
            mock.setInput("X");
            REQUIRE(mock.read() == 'X');
            REQUIRE(mock.read() == -1);
        }
    }

    SECTION("available()")
    {
        SECTION("Returns correct count")
        {
            mock.setInput("Hello");
            REQUIRE(mock.available() == 5);
        }

        SECTION("Decreases after read")
        {
            mock.setInput("Hi");
            REQUIRE(mock.available() == 2);
            mock.read();
            REQUIRE(mock.available() == 1);
            mock.read();
            REQUIRE(mock.available() == 0);
        }

        SECTION("Returns zero when empty")
        {
            mock.setInput("");
            REQUIRE(mock.available() == 0);
        }
    }

    SECTION("flush()")
    {
        mock.setInput("Hello");
        REQUIRE(mock.available() == 5);
        mock.flush();
        REQUIRE(mock.available() == 0);
        REQUIRE(mock.read() == -1);
    }

    SECTION("write()")
    {
        REQUIRE(mock.write(static_cast<byte>('A')) == 1);
        REQUIRE(mock.write(static_cast<byte>('B')) == 1);
        REQUIRE(mock.buffer.size() == 2);
        REQUIRE(mock.buffer[0] == 'A');
        REQUIRE(mock.buffer[1] == 'B');
    }

    SECTION("parseInt() with zeros in number")
    {
        // Verify that the default no-argument parseInt() correctly handles
        // zeros in numbers. The implementation uses skipChar='0' as sentinel,
        // but '0' is always handled as a digit before the skipChar check.
        struct ParseIntZeroTestCase {
            const char* input;
            int result;
            int newReadPos;
        };

        ParseIntZeroTestCase zeroTestCases[] = {
            {"1000 ", 1000, 4},
            {"10203 ", 10203, 5},
            {"007 ", 7, 3},
            {"-1000 ", -1000, 5},
            {"-0 ", 0, 2},
        };

        for (const auto&[input, result, newReadPos] : zeroTestCases)
        {
            mock.setInput(input);
            int parsedResult = mock.parseInt();
            INFO("Expected '" << result << "' after parsing '" << input <<"', but got: '" << parsedResult << "'");
            REQUIRE(parsedResult == result);
            INFO("Expected readPos=" << newReadPos << " after parsing '" << input <<"', but got: " << mock.readPos);
            REQUIRE(mock.readPos == newReadPos);
        }
    }

    SECTION("parseInt() edge cases")
    {
        mock.setTimeout(10); // short timeout for edge cases with no valid data

        SECTION("Lone minus sign")
        {
            mock.setInput("- ");
            int result = mock.parseInt();
            REQUIRE(result == 0);
            REQUIRE(mock.readPos == 1); // only '-' consumed
        }

        SECTION("Double minus sign")
        {
            mock.setInput("--5 ");
            int result = mock.parseInt();
            REQUIRE(result == 0);
            REQUIRE(mock.readPos == 1); // only first '-' consumed
        }

        SECTION("Only whitespace")
        {
            mock.setInput("   ");
            int result = mock.parseInt();
            REQUIRE(result == 0);
        }

        SECTION("Plus sign is not treated as sign")
        {
            mock.setInput("+42 ");
            int result = mock.parseInt();
            // '+' is discarded by peekNextDigit(), only digits parsed
            REQUIRE(result == 42);
        }
    }

    SECTION("readBytesUntil() edge cases")
    {
        SECTION("Terminator at first position")
        {
            mock.setInput("\nHello");
            char buffer[10] = {};
            int count = mock.readBytesUntil('\n', buffer, 10);
            REQUIRE(count == 0);
        }

        SECTION("Empty input")
        {
            mock.setTimeout(10);
            mock.setInput("");
            char buffer[10] = {};
            int count = mock.readBytesUntil('\n', buffer, 10);
            REQUIRE(count == 0);
        }
    }

    SECTION("readBytes() timeout behavior")
    {
        SECTION("Fewer bytes available than requested")
        {
            mock.setTimeout(10);
            mock.setInput("Hi");
            char buffer[10] = {};
            int count = mock.readBytes(buffer, 10);
            REQUIRE(count == 2);
            REQUIRE(std::string(buffer, 2) == "Hi");
        }
    }

    SECTION("findUntil() edge cases")
    {
        SECTION("Overlapping pattern - known naive matching limitation")
        {
            // The naive matching algorithm resets targetIdx to 0 on mismatch
            // without re-checking the current character against target[0].
            // This causes overlapping patterns to be missed.
            mock.setInput("aaab");
            REQUIRE(mock.findUntil("aab", 3, (const char*)0, 0) == false);
            // NOTE: "aab" exists at position 1 in "aaab", but is not found
        }

        SECTION("Target equals terminator - target wins")
        {
            mock.setInput("Hello");
            // Target is checked before terminator, so target match wins
            REQUIRE(mock.findUntil("Hello", "Hello") == true);
        }

        SECTION("Target at very end of stream")
        {
            mock.setInput("xxxWorld");
            REQUIRE(mock.findUntil("World", 5, (const char*)0, 0) == true);
        }

        SECTION("Single character target found before terminator")
        {
            mock.setInput("abcde");
            REQUIRE(mock.findUntil("c", 1, "e", 1) == true);
        }

        SECTION("Terminator found before single character target")
        {
            mock.setInput("abcde");
            REQUIRE(mock.findUntil("e", 1, "c", 1) == false);
        }
    }

    SECTION("find() edge cases")
    {
        SECTION("Find single character at end")
        {
            mock.setInput("abcde");
            REQUIRE(mock.find("e") == true);
        }

        SECTION("Find entire input")
        {
            mock.setInput("Hello");
            REQUIRE(mock.find("Hello") == true);
        }
    }

    sim.stop();
}
