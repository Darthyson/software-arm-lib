/*
 *  buffered_stream.h - Base class for character-based streams.
 *
 *  Copyright (c) 2015 Stefan Taferner <stefan.taferner@gmx.at>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 3 as
 *  published by the Free Software Foundation.
 */
#ifndef sblib_buffered_stream_h
#define sblib_buffered_stream_h

#include <sblib/stream.h>

/**
 * A stream class that has a read and a write buffer.
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
     * @return The number of bytes that are available for reading.
     */
    int available() override;

    /**
     * Clear the read and write buffers.
     *
     * This method shall be called at least once in begin() by subclasses.
     */
    void clearBuffers();

    enum
    {
        BUFFER_SIZE = 128, //!< The size of the internal read/write buffers in bytes.
        BUFFER_SIZE_MASK = BUFFER_SIZE - 1
    };

protected:
    volatile int readHead = 0;  //!< head index for the read buffer
    volatile int readTail = 0;  //!< tail index for the read buffer
    volatile int writeHead = 0; //!< head index for the write buffer
    volatile int writeTail = 0; //!< tail index for the write buffer

    byte readBuffer[BUFFER_SIZE] = {0};  //!< the read buffer
    byte writeBuffer[BUFFER_SIZE] = {0}; //!< the write buffer

    /**
     * Test if the read buffer is full.
     */
    bool readBufferFull();

    /**
     * Test if the write buffer is full.
     */
    bool writeBufferFull();
};


//
//  Inline functions
//

inline void BufferedStream::clearBuffers()
{
    readHead = 0;
    readTail = 0;
    writeHead = 0;
    writeTail = 0;
}

ALWAYS_INLINE bool BufferedStream::readBufferFull()
{
    return ((readTail + 1) & BufferedStream::BUFFER_SIZE_MASK) == readHead;
}

ALWAYS_INLINE bool BufferedStream::writeBufferFull()
{
    return ((writeTail + 1) & BufferedStream::BUFFER_SIZE_MASK) == writeHead;
}

#endif /* sblib_buffered_stream_h */
