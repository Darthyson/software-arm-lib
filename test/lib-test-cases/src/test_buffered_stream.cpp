/*
 * Unit tests for class BufferedStream
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 3 as
 * published by the Free Software Foundation.
 */

#include <sblib/buffered_stream.h>
#include <cstdint>

#include <catch.hpp>


class MockBufferedStream final : public BufferedStream
{
public:
    using Print::write;
    using BufferedStream::clearBuffers;
    using BufferedStream::readEmpty;
    using BufferedStream::writeEmpty;
    using BufferedStream::pushRead;
    using BufferedStream::popWrite;

    uint32_t write(const uint8_t ch) override
    {
        if (pushWrite(ch))
        {
            return 1;
        }

        return 0;
    }

    void flush() override
    {
        clearBuffers();
    }
};


TEST_CASE("BufferedStream::", "[buffered_stream]")
{
    MockBufferedStream mockSPSCRingBuffer;
    mockSPSCRingBuffer.clearBuffers();

    SECTION("writeBufferEmpty() returns true when buffer is empty")
    {
        REQUIRE(mockSPSCRingBuffer.writeEmpty() == true);
        REQUIRE(mockSPSCRingBuffer.write('A') == 1);
        REQUIRE(mockSPSCRingBuffer.writeEmpty() == false);
    }

    SECTION("readBufferEmpty() returns true when buffer is empty")
    {
        REQUIRE(mockSPSCRingBuffer.readEmpty() == true);
        REQUIRE(mockSPSCRingBuffer.pushRead('B') == 1);
        REQUIRE(mockSPSCRingBuffer.readEmpty() == false);
    }

    SECTION("read() returns -1 when buffer is empty")
    {
        REQUIRE(mockSPSCRingBuffer.read() == -1);
    }

    SECTION("peek()")
    {
        REQUIRE(mockSPSCRingBuffer.peek() == -1);
        REQUIRE(mockSPSCRingBuffer.pushRead('G') == 1);
        REQUIRE(mockSPSCRingBuffer.available() == 1);
        REQUIRE(mockSPSCRingBuffer.peek() == 'G');
        REQUIRE(mockSPSCRingBuffer.available() == 1);
    }

    SECTION("peekWriteBuffer()")
    {
        REQUIRE(mockSPSCRingBuffer.peekWrite() == -1);
        REQUIRE(mockSPSCRingBuffer.write('H') == 1);
        REQUIRE(mockSPSCRingBuffer.availableWrite() == 1);
        REQUIRE(mockSPSCRingBuffer.peekWrite() == 'H');
        REQUIRE(mockSPSCRingBuffer.availableWrite() == 1);
    }

    SECTION("available() returns 0 when buffer is empty")
    {
        REQUIRE(mockSPSCRingBuffer.available() == 0);
    }

    SECTION("read() single byte")
    {
        REQUIRE(mockSPSCRingBuffer.write('A') == 1);
        REQUIRE(mockSPSCRingBuffer.availableWrite() == 1);
        REQUIRE(mockSPSCRingBuffer.popWrite() == 'A');
        REQUIRE(mockSPSCRingBuffer.availableWrite() == 0);
        REQUIRE(mockSPSCRingBuffer.popWrite() == -1);
    }

    SECTION("peek() does not consume the byte")
    {
        REQUIRE(mockSPSCRingBuffer.write('B') == 1);
        REQUIRE(mockSPSCRingBuffer.peekWrite() == 'B');
        REQUIRE(mockSPSCRingBuffer.availableWrite() == 1);
        REQUIRE(mockSPSCRingBuffer.peekWrite() == 'B');
        REQUIRE(mockSPSCRingBuffer.popWrite() == 'B');
        REQUIRE(mockSPSCRingBuffer.availableWrite() == 0);
    }

    SECTION("read() multiple bytes in FIFO order")
    {
        REQUIRE(mockSPSCRingBuffer.write('X') == 1);
        REQUIRE(mockSPSCRingBuffer.write('Y') == 1);
        REQUIRE(mockSPSCRingBuffer.write('Z') == 1);
        REQUIRE(mockSPSCRingBuffer.availableWrite() == 3);
        REQUIRE(mockSPSCRingBuffer.popWrite() == 'X');
        REQUIRE(mockSPSCRingBuffer.popWrite() == 'Y');
        REQUIRE(mockSPSCRingBuffer.popWrite() == 'Z');
        REQUIRE(mockSPSCRingBuffer.availableWrite() == 0);
    }

    SECTION("available() tracks count correctly")
    {
        constexpr uint8_t numBytes = 10;
        for (uint8_t i = 0; i < numBytes; i++)
        {
            REQUIRE(mockSPSCRingBuffer.pushRead(i) == 1);
        }
        REQUIRE(mockSPSCRingBuffer.available() == numBytes);

        mockSPSCRingBuffer.read();
        mockSPSCRingBuffer.read();
        REQUIRE(mockSPSCRingBuffer.available() == numBytes - 2);
    }

    SECTION("read buffer fills up to BUFFER_SIZE - 1")
    {
        constexpr uint32_t maxElements = BufferedStream::getBufferSize() - 1;
        for (uint32_t i = 0; i < maxElements; i++)
        {
            REQUIRE(mockSPSCRingBuffer.pushRead(static_cast<uint8_t>(i)) == 1);
        }
        REQUIRE(mockSPSCRingBuffer.available() == maxElements);
        REQUIRE(mockSPSCRingBuffer.pushRead('E') == 0);
        REQUIRE(mockSPSCRingBuffer.available() == maxElements);
    }

    SECTION("write buffer fills up to BUFFER_SIZE - 1")
    {
        constexpr uint32_t maxElements = BufferedStream::getBufferSize() - 1;
        for (uint32_t i = 0; i < maxElements; i++)
        {
            REQUIRE(mockSPSCRingBuffer.write(static_cast<uint8_t>(i)) == 1);
        }
        REQUIRE(mockSPSCRingBuffer.availableWrite() == maxElements);
        REQUIRE(mockSPSCRingBuffer.write('E') == 0);
        REQUIRE(mockSPSCRingBuffer.availableWrite() == maxElements);
    }

    SECTION("read buffer wraps around correctly")
    {
        // Fill partially and drain to advance head
        constexpr uint32_t initialFill = BufferedStream::getBufferSize() - 1;
        for (uint32_t i = 0; i < initialFill; i++)
        {
            REQUIRE(mockSPSCRingBuffer.pushRead(static_cast<uint8_t>(i)) == 1);
        }
        for (uint32_t i = 0; i < initialFill; i++)
        {
            REQUIRE(mockSPSCRingBuffer.read() == static_cast<uint8_t>(i));
        }
        REQUIRE(mockSPSCRingBuffer.available() == 0);

        // Now push and read again, this time the head/tail indices wrap around
        constexpr uint32_t numBytes = BufferedStream::getBufferSize() / 2;
        for (uint32_t i = 0; i < numBytes; i++)
        {
            REQUIRE(mockSPSCRingBuffer.pushRead(static_cast<uint8_t>(i)) == 1);
        }
        REQUIRE(mockSPSCRingBuffer.available() == numBytes);
        for (uint32_t i = 0; i < numBytes; i++)
        {
            REQUIRE(mockSPSCRingBuffer.read() == static_cast<uint8_t>(i));
        }
        REQUIRE(mockSPSCRingBuffer.available() == 0);
    }

    SECTION("write buffer wraps around correctly")
    {
        // Fill partially and drain to advance head
        constexpr uint32_t initialFill = BufferedStream::getBufferSize() - 1;
        for (uint32_t i = 0; i < initialFill; i++)
        {
            REQUIRE(mockSPSCRingBuffer.write(static_cast<uint8_t>(i)) == 1);
        }
        for (uint32_t i = 0; i < initialFill; i++)
        {
            REQUIRE(mockSPSCRingBuffer.popWrite() == static_cast<uint8_t>(i));
        }
        REQUIRE(mockSPSCRingBuffer.availableWrite() == 0);

        // Now push and read again, this time the head/tail indices wrap around
        constexpr uint32_t numBytes = BufferedStream::getBufferSize() / 2;
        for (uint32_t i = 0; i < numBytes; i++)
        {
            REQUIRE(mockSPSCRingBuffer.write(static_cast<uint8_t>(i)) == 1);
        }
        REQUIRE(mockSPSCRingBuffer.availableWrite() == numBytes);
        for (uint32_t i = 0; i < numBytes; i++)
        {
            REQUIRE(mockSPSCRingBuffer.popWrite() == static_cast<uint8_t>(i));
        }
        REQUIRE(mockSPSCRingBuffer.availableWrite() == 0);
    }

    SECTION("clearBuffers() resets all indices")
    {
        REQUIRE(mockSPSCRingBuffer.write('A') == true);
        REQUIRE(mockSPSCRingBuffer.write('B') == true);
        REQUIRE(mockSPSCRingBuffer.write('C') == 1);
        REQUIRE(mockSPSCRingBuffer.write('D') == 1);

        mockSPSCRingBuffer.clearBuffers();

        REQUIRE(mockSPSCRingBuffer.available() == 0);
        REQUIRE(mockSPSCRingBuffer.read() == -1);
        REQUIRE(mockSPSCRingBuffer.peek() == -1);
    }

    SECTION("write() single byte to write buffer")
    {
        REQUIRE(mockSPSCRingBuffer.write('W') == 1);
    }

    SECTION("write buffer fills up to BUFFER_SIZE - 1")
    {
        constexpr uint32_t maxElements = BufferedStream::getBufferSize() - 1;
        for (uint32_t i = 0; i < maxElements; i++)
        {
            REQUIRE(mockSPSCRingBuffer.write(static_cast<uint8_t>(i)) == 1);
        }
        REQUIRE(mockSPSCRingBuffer.write(0xff) == 0); // buffer is full, so writing fails
    }

    SECTION("read and write buffers are independent")
    {
        REQUIRE(mockSPSCRingBuffer.pushRead('R') == true);
        REQUIRE(mockSPSCRingBuffer.write('W') == 1);

        REQUIRE(mockSPSCRingBuffer.available() == 1);
        REQUIRE(mockSPSCRingBuffer.peek() == 'R');
        REQUIRE(mockSPSCRingBuffer.read() == 'R');
        REQUIRE(mockSPSCRingBuffer.available() == 0);

        REQUIRE(mockSPSCRingBuffer.availableWrite() == 1);
        REQUIRE(mockSPSCRingBuffer.peekWrite() == 'W');
        REQUIRE(mockSPSCRingBuffer.popWrite() == 'W');
        REQUIRE(mockSPSCRingBuffer.availableWrite() == 0);
    }

    SECTION("interleaved push and read operations")
    {
        REQUIRE(mockSPSCRingBuffer.pushRead('A') == true);
        REQUIRE(mockSPSCRingBuffer.read() == 'A');

        REQUIRE(mockSPSCRingBuffer.pushRead('B') == true);
        REQUIRE(mockSPSCRingBuffer.pushRead('C') == true);
        REQUIRE(mockSPSCRingBuffer.read() == 'B');

        REQUIRE(mockSPSCRingBuffer.pushRead('D') == true);
        REQUIRE(mockSPSCRingBuffer.available() == 2);
        REQUIRE(mockSPSCRingBuffer.read() == 'C');
        REQUIRE(mockSPSCRingBuffer.read() == 'D');
        REQUIRE(mockSPSCRingBuffer.available() == 0);
    }

    SECTION("continuous fill and drain cycle across multiple wraps")
    {
        // Simulate a producer-consumer pattern across multiple buffer wraps
        uint32_t produced = 0;
        uint32_t consumed = 0;
        constexpr uint32_t totalItems = BufferedStream::getBufferSize() * 3;

        while (consumed < totalItems)
        {
            // Produce a batch
            for (uint32_t i = 0; i < 10 && produced < totalItems; i++)
            {
                REQUIRE(mockSPSCRingBuffer.pushRead(static_cast<uint8_t>(produced & 0xff)) == true);
                produced++;
            }

            // Consume a batch
            for (uint32_t i = 0; i < 7; i++)
            {
                int32_t val = mockSPSCRingBuffer.read();
                if (val == -1)
                {
                    break;
                }
                REQUIRE(val == static_cast<int32_t>(consumed & 0xff));
                consumed++;
            }
        }
        REQUIRE(consumed == totalItems);
    }
}