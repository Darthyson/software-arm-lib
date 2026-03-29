/*
 * Unit tests for class RingBuffer
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 3 as
 * published by the Free Software Foundation.
 */

#include <sblib/ring_buffer.h>

#include <catch.hpp>


TEST_CASE("RingBuffer::", "[ring_buffer]")
{
    RingBuffer testRingBuffer(RingBuffer::Size::bytes_128);

    SECTION("empty() returns true when buffer is empty, false after push")
    {
        REQUIRE(testRingBuffer.empty() == true);
        REQUIRE(testRingBuffer.push('A') == true);
        REQUIRE(testRingBuffer.empty() == false);
    }

    SECTION("pop() returns -1 when buffer is empty")
    {
        testRingBuffer.clear();
        REQUIRE(testRingBuffer.pop() == -1);
    }

    SECTION("peek() does not consume the byte")
    {
        REQUIRE(testRingBuffer.peek() == -1);
        REQUIRE(testRingBuffer.push('G') == true);
        REQUIRE(testRingBuffer.available() == 1);
        REQUIRE(testRingBuffer.peek() == 'G');
        REQUIRE(testRingBuffer.available() == 1);
    }

    SECTION("available() returns 0 when buffer is empty")
    {
        testRingBuffer.clear();
        REQUIRE(testRingBuffer.available() == 0);
    }

    SECTION("pop() single byte")
    {
        REQUIRE(testRingBuffer.push('A') == true);
        REQUIRE(testRingBuffer.available() == 1);
        REQUIRE(testRingBuffer.pop() == 'A');
        REQUIRE(testRingBuffer.available() == 0);
        REQUIRE(testRingBuffer.pop() == -1);
    }

    SECTION("pop() multiple bytes in FIFO order")
    {
        REQUIRE(testRingBuffer.push('X') == true);
        REQUIRE(testRingBuffer.push('Y') == true);
        REQUIRE(testRingBuffer.push('Z') == true);
        REQUIRE(testRingBuffer.available() == 3);
        REQUIRE(testRingBuffer.pop() == 'X');
        REQUIRE(testRingBuffer.pop() == 'Y');
        REQUIRE(testRingBuffer.pop() == 'Z');
        REQUIRE(testRingBuffer.available() == 0);
    }

    SECTION("available() tracks count correctly")
    {
        constexpr uint8_t numBytes = 10;
        constexpr uint8_t startValue = 0;
        for (uint8_t i = startValue; i < numBytes; i++)
        {
            REQUIRE(testRingBuffer.push(i) == true);
        }
        REQUIRE(testRingBuffer.available() == numBytes);

        REQUIRE(testRingBuffer.pop() == startValue);
        REQUIRE(testRingBuffer.pop() == startValue + 1);
        REQUIRE(testRingBuffer.available() == numBytes - 2);
    }

    SECTION("buffer fills up to getBufferSize()")
    {
        const uint32_t maxElements = testRingBuffer.getBufferSize();
        for (uint32_t i = 0; i < maxElements; i++)
        {
            REQUIRE(testRingBuffer.push(static_cast<uint8_t>(i)) == true);
        }
        REQUIRE(testRingBuffer.available() == maxElements);
        REQUIRE(testRingBuffer.push('E') == false);
        REQUIRE(testRingBuffer.available() == maxElements);
    }

    SECTION("buffer wraps around correctly")
    {
        // Fill partially and drain to advance head
        const uint32_t initialFill = testRingBuffer.getBufferSize();
        for (uint32_t i = 0; i < initialFill; i++)
        {
            REQUIRE(testRingBuffer.push(static_cast<uint8_t>(i)) == true);
        }
        for (uint32_t i = 0; i < initialFill; i++)
        {
            REQUIRE(testRingBuffer.pop() == static_cast<int16_t>(i));
        }
        REQUIRE(testRingBuffer.available() == 0);

        // Now push and read again, this time the head/tail indices wrap around
        const uint32_t numBytes = testRingBuffer.getBufferSize() / 2;
        for (uint32_t i = 0; i < numBytes; i++)
        {
            REQUIRE(testRingBuffer.push(static_cast<uint8_t>(i)) == true);
        }
        REQUIRE(testRingBuffer.available() == numBytes);
        for (uint32_t i = 0; i < numBytes; i++)
        {
            REQUIRE(testRingBuffer.pop() == static_cast<int16_t>(i));
        }
        REQUIRE(testRingBuffer.available() == 0);
    }

    SECTION("clear() resets all cursor")
    {
        REQUIRE(testRingBuffer.push('A') == true);
        REQUIRE(testRingBuffer.push('B') == true);
        REQUIRE(testRingBuffer.push('C') == true);
        REQUIRE(testRingBuffer.push('D') == true);

        testRingBuffer.clear();

        REQUIRE(testRingBuffer.available() == 0);
        REQUIRE(testRingBuffer.pop() == -1);
        REQUIRE(testRingBuffer.peek() == -1);
    }

    SECTION("push() single byte to buffer")
    {
        REQUIRE(testRingBuffer.push('W') == true);
    }

    SECTION("interleaved push and pop operations")
    {
        REQUIRE(testRingBuffer.push('A') == true);
        REQUIRE(testRingBuffer.pop() == 'A');

        REQUIRE(testRingBuffer.push('B') == true);
        REQUIRE(testRingBuffer.push('C') == true);
        REQUIRE(testRingBuffer.pop() == 'B');

        REQUIRE(testRingBuffer.push('D') == true);
        REQUIRE(testRingBuffer.available() == 2);
        REQUIRE(testRingBuffer.pop() == 'C');
        REQUIRE(testRingBuffer.pop() == 'D');
        REQUIRE(testRingBuffer.available() == 0);
    }

    SECTION("continuous fill and drain cycle across multiple wraps")
    {
        // Simulate a producer-consumer pattern across multiple buffer wraps
        uint32_t produced = 0;
        uint32_t consumed = 0;
        const uint32_t totalItems = testRingBuffer.getBufferSize() * 3;

        while (consumed < totalItems)
        {
            // Produce a batch
            for (uint32_t i = 0; i < 10 && produced < totalItems; i++)
            {
                REQUIRE(testRingBuffer.push(static_cast<uint8_t>(produced & 0xff)) == true);
                produced++;
            }

            // Consume a batch
            for (uint32_t i = 0; i < 7; i++)
            {
                int16_t val = testRingBuffer.pop();
                if (val == -1)
                {
                    break;
                }
                REQUIRE(val == static_cast<int16_t>(consumed & 0xff));
                consumed++;
            }
        }
        REQUIRE(consumed == totalItems);
    }

    SECTION("full() returns true when buffer is at capacity")
    {
        REQUIRE(testRingBuffer.full() == false);
        const uint16_t maxElements = testRingBuffer.getBufferSize();
        for (uint16_t i = 0; i < maxElements; i++)
        {
            REQUIRE(testRingBuffer.push(static_cast<uint8_t>(i)) == true);
        }
        REQUIRE(testRingBuffer.full() == true);
        REQUIRE(testRingBuffer.push(0xff) == false);

        REQUIRE(testRingBuffer.pop() >= 0);
        REQUIRE(testRingBuffer.full() == false);
    }

    SECTION("getBufferSize() returns the size passed to constructor Minus 1 for empty() (head==tail) detection.")
    {
        REQUIRE(testRingBuffer.getBufferSize() == 127);
    }

    SECTION("available() correct after wrap-around")
    {
        // Advance head past the midpoint to force tail < head after next fill
        const uint16_t bufSize = testRingBuffer.getBufferSize();
        const uint16_t fillCount = bufSize;
        for (uint16_t i = 0; i < fillCount; i++)
        {
            REQUIRE(testRingBuffer.push(static_cast<uint8_t>(i)) == true);
        }

        for (uint16_t i = 0; i < fillCount; i++)
        {
            REQUIRE(testRingBuffer.pop() == static_cast<uint8_t>(i));
        }

        // head and tail are now at fillCount, near end of buffer
        // Push enough to wrap tail past 0
        const uint16_t wrapFill = bufSize / 2;
        for (uint16_t i = 0; i < wrapFill; i++)
        {
            REQUIRE(testRingBuffer.push(static_cast<uint8_t>(i)) == true);
        }
        REQUIRE(testRingBuffer.available() == wrapFill);
    }

    SECTION("clear() resets correctly after wrap-around")
    {
        const uint16_t fillCount = testRingBuffer.getBufferSize();
        for (uint16_t i = 0; i < fillCount; i++)
        {
            REQUIRE(testRingBuffer.push(static_cast<uint8_t>(i)) == true);
        }

        for (uint16_t i = 0; i < fillCount / 2; i++)
        {
            REQUIRE(testRingBuffer.pop() == static_cast<uint8_t>(i));
        }

        testRingBuffer.clear();

        REQUIRE(testRingBuffer.empty() == true);
        REQUIRE(testRingBuffer.full() == false);
        REQUIRE(testRingBuffer.available() == 0);
        REQUIRE(testRingBuffer.pop() == -1);
    }


    RingBuffer tinyBuffer(RingBuffer::Size::bytes_2);
    SECTION("Smallest size (bytes_2) has capacity for exactly 1 element")
    {
        REQUIRE(tinyBuffer.getBufferSize() == 1);
        REQUIRE(tinyBuffer.empty() == true);
        REQUIRE(tinyBuffer.full() == false);

        REQUIRE(tinyBuffer.push(0xAB) == true);
        REQUIRE(tinyBuffer.full() == true);
        REQUIRE(tinyBuffer.empty() == false);
        REQUIRE(tinyBuffer.available() == 1);

        REQUIRE(tinyBuffer.push(0xCD) == false);

        REQUIRE(tinyBuffer.pop() == 0xAB);
        REQUIRE(tinyBuffer.empty() == true);
    }

    SECTION("Smallest size (bytes_2) wraps around correctly with 1-element capacity")
    {
        for (uint8_t round = 0; round < 5; round++)
        {
            REQUIRE(tinyBuffer.push(round) == true);
            REQUIRE(tinyBuffer.full() == true);
            REQUIRE(tinyBuffer.pop() == static_cast<int16_t>(round));
            REQUIRE(tinyBuffer.empty() == true);
        }
    }
}