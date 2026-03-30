/*
 *  buffered_stream.h - Base class for character-based streams.
 *
 *  Copyright (c) 2015 Stefan Taferner <stefan.taferner@gmx.at>
 *  Copyright (c) 2026 Darthyson <darth@maptrack.de>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 3 as
 *  published by the Free Software Foundation.
 */
#ifndef SBLIB_BUFFERED_STREAM_H_
#define SBLIB_BUFFERED_STREAM_H_

#include "sblib/ring_buffer.h"
#include <sblib/stream.h>


/**
 * A simple SPSC (Single Producer Single Consumer) Ring Buffer 
 * that has a read and a write byte buffer.
 */
class BufferedStream : public Stream
{
public:
    explicit BufferedStream();
    ~BufferedStream() override = default;

    /**
     * Read a single byte.
     *
     * @return The read byte (0..255) or -1 if no byte was received.
     */
    int16_t read() override;

    /**
     * Query the next byte to be read, without reading it.
     *
     * @return The next byte (0..255) or -1 if no byte is available
     *         for reading.
     */
    int16_t peek() override;

    /**
     * Query the next byte from the writeBuffer, without reading it.
     *
     * @return The next byte (0..255) or -1 if no byte is available
     */
    [[nodiscard]] int32_t peekWrite() const;

    /**
     * @return The number of bytes that are available for reading.
     */
    uint32_t available() override;

    /**
     * @return The number of bytes that are available in writeBuffer.
     */
    [[nodiscard]] int32_t availableWrite() const;

    static constexpr uint16_t getBufferSize() { return static_cast<uint16_t>(fixedBufferSize128bytes); }

protected:
    /**
     * @brief Test if the read buffer is full.
     */
    [[nodiscard]] bool readFull() const;

    /**
     * @brief Test if the write buffer is full.
     */
    [[nodiscard]] bool writeFull() const;

    /**
     * @brief Test if the read buffer is empty.
     */
    [[nodiscard]] bool readEmpty() const;

    /**
     * @brief Test if the write buffer is empty.
     */
    [[nodiscard]] bool writeEmpty() const;

    /**
     * @brief Read a single byte from the writeBuffer.
     *
     * @return The read byte (0..255) or -1 if no byte was received.
     */
    [[nodiscard]] int16_t popWrite();

    /**
     * @brief Push a byte into the read buffer.
     *
     * @param byteToPush The byte to push into the read buffer.
     */
    bool pushRead(uint8_t byteToPush);

    /**
     * @brief Push a byte into the write buffer.
     *
     * @param byteToPush The byte to push into the write buffer.
     */
    bool pushWrite(uint8_t byteToPush);

    /**
     * @brief Clear the read and write buffers.
     *
     * This method shall be called at least once in begin() by subclasses.
     */
    void clearBuffers();

private:
    RingBuffer readBuffer;
    RingBuffer writeBuffer;

    /**
     * @brief The fixed size of the internal read/write SPSC buffers in bytes.
     */
    static constexpr auto fixedBufferSize128bytes = RingBuffer::Size::bytes_128;
};


//
//  Inline functions
//
ALWAYS_INLINE void BufferedStream::clearBuffers()
{
    readBuffer.clear();
    writeBuffer.clear();
}

ALWAYS_INLINE bool BufferedStream::readFull() const
{
    return readBuffer.full();
}

ALWAYS_INLINE bool BufferedStream::writeFull() const
{
    return writeBuffer.full();
}

ALWAYS_INLINE bool BufferedStream::readEmpty() const
{
    return readBuffer.empty();
}

ALWAYS_INLINE bool BufferedStream::writeEmpty() const
{
    return writeBuffer.empty();
}

ALWAYS_INLINE bool BufferedStream::pushRead(const uint8_t byteToPush)
{
    return readBuffer.push(byteToPush);
}

ALWAYS_INLINE bool BufferedStream::pushWrite(const uint8_t byteToPush)
{
    return writeBuffer.push(byteToPush);
}

ALWAYS_INLINE int16_t BufferedStream::popWrite()
{
    return writeBuffer.pop();
}

#endif /* SBLIB_BUFFERED_STREAM_H_ */
