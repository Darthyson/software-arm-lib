/*
 *  Implementation of class BufferedStream for character-based IO-streams.
 *
 *  Copyright (c) 2015 Stefan Taferner <stefan.taferner@gmx.at>
 *  Copyright (c) 2026 Darthyson <darth@maptrack.de>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 3 as
 *  published by the Free Software Foundation.
 */

#include <sblib/buffered_stream.h>


BufferedStream::BufferedStream()
    :
    readBuffer(fixedBufferSize128bytes),
    writeBuffer(fixedBufferSize128bytes)
{
}

int16_t BufferedStream::read()
{
    return readBuffer.pop();
}

int16_t BufferedStream::peek()
{
    return readBuffer.peek();
}

int32_t BufferedStream::peekWrite() const
{
    return writeBuffer.peek();
}

uint32_t BufferedStream::available()
{
    return readBuffer.available();
}

int32_t BufferedStream::availableWrite() const
{
    return writeBuffer.available();
}
