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

    uint32_t write(const uint8_t ch) override
    {
        if (insertInWriteBuffer(ch))
        {
            return 1;
        }

        return 0;
    }


    void flush() override
    {
        clearBuffers();
    }

    /**
     * Simulate receiving data into the read buffer (producer side).
     */
    bool insertInReadBuffer(const uint8_t ch)
    {
        if (readBufferFull())
        {
            return false;
        }
        readBuffer[readTail] = ch;
        readTail = (readTail + 1) & BUFFER_SIZE_MASK;
        return true;
    }

    /**
     * Simulate writing data into the write buffer (consumer side).
     */
    bool insertInWriteBuffer(const uint8_t ch)
    {
        if (writeBufferFull())
        {
            return false;
        }
        writeBuffer[writeTail] = ch;
        writeTail = (writeTail + 1) & BUFFER_SIZE_MASK;
        return true;
    }

    using BufferedStream::readHead;
    using BufferedStream::readTail;
    using BufferedStream::writeHead;
    using BufferedStream::writeTail;
};


TEST_CASE("BufferedStream::", "[buffered_stream]")
{
    MockBufferedStream mockSPSCRingBuffer;
    mockSPSCRingBuffer.clearBuffers();

    SECTION("read() returns -1 when buffer is empty")
    {
        REQUIRE(mockSPSCRingBuffer.read() == -1);
    }

    SECTION("peek() returns -1 when buffer is empty")
    {
        REQUIRE(mockSPSCRingBuffer.peek() == -1);
    }

    SECTION("available() returns 0 when buffer is empty")
    {
        REQUIRE(mockSPSCRingBuffer.available() == 0);
    }

    SECTION("read() single byte")
    {
        REQUIRE(mockSPSCRingBuffer.insertInReadBuffer('A') == true);
        REQUIRE(mockSPSCRingBuffer.available() == 1);
        REQUIRE(mockSPSCRingBuffer.read() == 'A');
        REQUIRE(mockSPSCRingBuffer.available() == 0);
        REQUIRE(mockSPSCRingBuffer.read() == -1);
    }

    SECTION("peek() does not consume the byte")
    {
        REQUIRE(mockSPSCRingBuffer.insertInReadBuffer('B') == true);
        REQUIRE(mockSPSCRingBuffer.peek() == 'B');
        REQUIRE(mockSPSCRingBuffer.available() == 1);
        REQUIRE(mockSPSCRingBuffer.peek() == 'B');
        REQUIRE(mockSPSCRingBuffer.read() == 'B');
        REQUIRE(mockSPSCRingBuffer.available() == 0);
    }

    SECTION("read() multiple bytes in FIFO order")
    {
        REQUIRE(mockSPSCRingBuffer.insertInReadBuffer('X') == true);
        REQUIRE(mockSPSCRingBuffer.insertInReadBuffer('Y') == true);
        REQUIRE(mockSPSCRingBuffer.insertInReadBuffer('Z') == true);
        REQUIRE(mockSPSCRingBuffer.available() == 3);
        REQUIRE(mockSPSCRingBuffer.read() == 'X');
        REQUIRE(mockSPSCRingBuffer.read() == 'Y');
        REQUIRE(mockSPSCRingBuffer.read() == 'Z');
        REQUIRE(mockSPSCRingBuffer.available() == 0);
    }

    SECTION("available() tracks count correctly")
    {
        constexpr uint8_t numBytes = 10;
        for (uint8_t i = 0; i < numBytes; i++)
        {
            REQUIRE(mockSPSCRingBuffer.insertInReadBuffer(i) == true);
        }
        REQUIRE(mockSPSCRingBuffer.available() == numBytes);

        mockSPSCRingBuffer.read();
        mockSPSCRingBuffer.read();
        REQUIRE(mockSPSCRingBuffer.available() == numBytes - 2);
    }

    SECTION("read buffer fills up to BUFFER_SIZE - 1")
    {
        constexpr uint32_t maxElements = BufferedStream::BUFFER_SIZE - 1;
        for (uint32_t i = 0; i < maxElements; i++)
        {
            REQUIRE(mockSPSCRingBuffer.insertInReadBuffer(static_cast<uint8_t>(i)) == true);
        }
        REQUIRE(mockSPSCRingBuffer.available() == maxElements);
        REQUIRE(mockSPSCRingBuffer.insertInReadBuffer('E') == false);
        REQUIRE(mockSPSCRingBuffer.available() == maxElements);
    }

    SECTION("read buffer wraps around correctly")
    {
        // Fill partially and drain to advance head
        constexpr uint32_t initialFill = BufferedStream::BUFFER_SIZE - 1;
        for (uint32_t i = 0; i < initialFill; i++)
        {
            REQUIRE(mockSPSCRingBuffer.insertInReadBuffer(static_cast<uint8_t>(i)) == true);
        }
        for (uint32_t i = 0; i < initialFill; i++)
        {
            REQUIRE(mockSPSCRingBuffer.read() == static_cast<uint8_t>(i));
        }
        REQUIRE(mockSPSCRingBuffer.available() == 0);

        // Now push and read again, this time the head/tail indices wrap around
        constexpr uint32_t numBytes = BufferedStream::BUFFER_SIZE / 2;
        for (uint32_t i = 0; i < numBytes; i++)
        {
            REQUIRE(mockSPSCRingBuffer.insertInReadBuffer(static_cast<uint8_t>(i)) == true);
        }
        REQUIRE(mockSPSCRingBuffer.available() == numBytes);
        for (uint32_t i = 0; i < numBytes; i++)
        {
            REQUIRE(mockSPSCRingBuffer.read() == static_cast<uint8_t>(i));
        }
        REQUIRE(mockSPSCRingBuffer.available() == 0);
    }

    SECTION("clearBuffers() resets all indices")
    {
        REQUIRE(mockSPSCRingBuffer.insertInReadBuffer('A') == true);
        REQUIRE(mockSPSCRingBuffer.insertInReadBuffer('B') == true);
        REQUIRE(mockSPSCRingBuffer.write('C') == 1);
        REQUIRE(mockSPSCRingBuffer.write('D') == 1);

        mockSPSCRingBuffer.clearBuffers();

        REQUIRE(mockSPSCRingBuffer.available() == 0);
        REQUIRE(mockSPSCRingBuffer.read() == -1);
        REQUIRE(mockSPSCRingBuffer.peek() == -1);
        REQUIRE(mockSPSCRingBuffer.readHead == 0);
        REQUIRE(mockSPSCRingBuffer.readTail == 0);
        REQUIRE(mockSPSCRingBuffer.writeHead == 0);
        REQUIRE(mockSPSCRingBuffer.writeTail == 0);
    }

    SECTION("write() single byte to write buffer")
    {
        REQUIRE(mockSPSCRingBuffer.write('W') == 1);
    }

    SECTION("write buffer fills up to BUFFER_SIZE - 1")
    {
        constexpr uint32_t maxElements = BufferedStream::BUFFER_SIZE - 1;
        for (uint32_t i = 0; i < maxElements; i++)
        {
            REQUIRE(mockSPSCRingBuffer.write(static_cast<uint8_t>(i)) == 1);
        }
        REQUIRE(mockSPSCRingBuffer.write(0xff) == 0); // buffer is full, so writing fails
    }

    SECTION("read and write buffers are independent")
    {
        REQUIRE(mockSPSCRingBuffer.insertInReadBuffer('R') == true);
        REQUIRE(mockSPSCRingBuffer.write('W') == 1);

        REQUIRE(mockSPSCRingBuffer.available() == 1);
        REQUIRE(mockSPSCRingBuffer.peek() == 'R');
        REQUIRE(mockSPSCRingBuffer.read() == 'R');
        REQUIRE(mockSPSCRingBuffer.available() == 0);
    }

    SECTION("interleaved push and read operations")
    {
        REQUIRE(mockSPSCRingBuffer.insertInReadBuffer('A') == true);
        REQUIRE(mockSPSCRingBuffer.read() == 'A');

        REQUIRE(mockSPSCRingBuffer.insertInReadBuffer('B') == true);
        REQUIRE(mockSPSCRingBuffer.insertInReadBuffer('C') == true);
        REQUIRE(mockSPSCRingBuffer.read() == 'B');

        REQUIRE(mockSPSCRingBuffer.insertInReadBuffer('D') == true);
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
        constexpr uint32_t totalItems = BufferedStream::BUFFER_SIZE * 3;

        while (consumed < totalItems)
        {
            // Produce a batch
            for (uint32_t i = 0; i < 10 && produced < totalItems; i++)
            {
                REQUIRE(mockSPSCRingBuffer.insertInReadBuffer(static_cast<uint8_t>(produced & 0xff)) == true);
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