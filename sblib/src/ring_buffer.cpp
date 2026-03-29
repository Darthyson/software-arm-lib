/*
 *  Implementation of SPSC (Single Producer Single Consumer) ring buffer.
 *
 *  Copyright (c) 2015 Stefan Taferner <stefan.taferner@gmx.at>
 *  Copyright (c) 2026 Darthyson <darth@maptrack.de>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 3 as
 *  published by the Free Software Foundation.
 */

#include "sblib/ring_buffer.h"

RingBuffer::RingBuffer(Size size)
    :
    head(0),
    tail(0),
    bufferSize(static_cast<uint16_t>(size)),
    bufferSizeMask(bufferSize - 1),
    buffer(new uint8_t[bufferSize]{})
{
}

RingBuffer::~RingBuffer()
{
    delete[] buffer;
}

void RingBuffer::clear()
{
    head = 0;
    tail = 0;
}

uint16_t RingBuffer::available() const
{
    return static_cast<uint16_t>((tail - head) & bufferSizeMask);
}

int16_t RingBuffer::peek() const
{
    if (empty())
    {
        return -1;
    }
    return buffer[head];
}

uint16_t RingBuffer::getBufferSize() const
{
    // The usable capacity of the buffer is one byte less than the actual size,
    // because head == tail is used to indicate an empty buffer
    return bufferSize - 1;
}