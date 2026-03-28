/*
 *  buffered_stream.h - Base class for character-based streams.
 *
 *  Copyright (c) 2015 Stefan Taferner <stefan.taferner@gmx.at>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 3 as
 *  published by the Free Software Foundation.
 */
#ifndef SBLIB_BUFFERED_STREAM_H_
#define SBLIB_BUFFERED_STREAM_H_

#include <sblib/stream.h>


/**
 * A simple SPSC (Single Producer Single Consumer) Ring Buffer 
 * that has a read and a write byte buffer.
 */
class BufferedStream : public Stream
{
public:
    /**
     * Read a single byte.
     *
     * @return The read byte (0..255) or -1 if no byte was received.
     */
    int read() override;

    /**
     * Query the next byte to be read, without reading it.
     *
     * @return The next byte (0..255) or -1 if no byte is available
     *         for reading.
     */
    int peek() override;

    /**
     * Query the next byte from the writeBuffer, without reading it.
     *
     * @return The next byte (0..255) or -1 if no byte is available
     */
    [[nodiscard]] int32_t peekWrite() const;

    /**
     * @return The number of bytes that are available for reading.
     */
    int available() override;

    /**
     * @return The number of bytes that are available in writeBuffer.
     */
    [[nodiscard]] int32_t availableWrite() const;

    enum
    {
        BUFFER_SIZE = 128, //!< The size of the internal read/write buffers in bytes.
        BUFFER_SIZE_MASK = BUFFER_SIZE - 1
    };

    static_assert((BUFFER_SIZE & BUFFER_SIZE_MASK) == 0, "BufferedStream::BUFFER_SIZE must be a power of 2 (..16, 32, 64, 128, 256, 512..");
    static_assert(BUFFER_SIZE > 1, "BufferedStream::BUFFER_SIZE must be at least 2");
    static_assert(BUFFER_SIZE <= 1024, "BufferedStream::BUFFER_SIZE should not exceed 1024 bytes");

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
    volatile int readHead = 0;  //!< head index for the read buffer
    volatile int readTail = 0;  //!< tail index for the read buffer
    volatile int writeHead = 0; //!< head index for the write buffer
    volatile int writeTail = 0; //!< tail index for the write buffer

    byte readBuffer[BUFFER_SIZE] = {0};  //!< the read buffer
    byte writeBuffer[BUFFER_SIZE] = {0}; //!< the write buffer
};


//
//  Inline functions
//
ALWAYS_INLINE void BufferedStream::clearBuffers()
{
    readHead = 0;
    readTail = 0;
    writeHead = 0;
    writeTail = 0;
}

ALWAYS_INLINE bool BufferedStream::readFull() const
{
    return ((readTail + 1) & BufferedStream::BUFFER_SIZE_MASK) == readHead;
}

ALWAYS_INLINE bool BufferedStream::writeFull() const
{
    return ((writeTail + 1) & BufferedStream::BUFFER_SIZE_MASK) == writeHead;
}

ALWAYS_INLINE bool BufferedStream::readEmpty() const
{
    return readHead == readTail;
}

ALWAYS_INLINE bool BufferedStream::writeEmpty() const
{
    return writeHead == writeTail;
}

ALWAYS_INLINE bool BufferedStream::pushRead(const uint8_t byteToPush)
{
    if (readFull())
    {
        return false;
    }
    const int32_t newReadTail = (readTail + 1) & BufferedStream::BUFFER_SIZE_MASK;
    readBuffer[readTail] = byteToPush;
    readTail = newReadTail;
    return true;
}

ALWAYS_INLINE bool BufferedStream::pushWrite(const uint8_t byteToPush)
{
    if (writeFull())
    {
        return false;
    }
    const int32_t newWriteTail = (writeTail + 1) & BufferedStream::BUFFER_SIZE_MASK;
    writeBuffer[writeTail] = byteToPush;
    writeTail = newWriteTail;
    return true;
}

ALWAYS_INLINE int16_t BufferedStream::popWrite()
{
    if (writeEmpty())
    {
        return -1;
    }

    const int32_t newWriteHead = (writeHead + 1) & BUFFER_SIZE_MASK;
    const int16_t bufferedValue = writeBuffer[writeHead];
    writeHead = newWriteHead;
    return bufferedValue;
}

#endif /* SBLIB_BUFFERED_STREAM_H_ */
