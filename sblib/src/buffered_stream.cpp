/*
 *  buffered_stream.cpp - Base class for character-based streams.
 *
 *  Copyright (c) 2015 Stefan Taferner <stefan.taferner@gmx.at>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 3 as
 *  published by the Free Software Foundation.
 */

#include <sblib/buffered_stream.h>


int BufferedStream::read()
{
    if (readTail == readHead)
        return -1;

    const int ch = readBuffer[readHead];
    readHead = (readHead + 1) & BufferedStream::BUFFER_SIZE_MASK;

    return ch;
}

int BufferedStream::peek()
{
    if (readTail == readHead)
        return -1;
    return readBuffer[readHead];
}

int32_t BufferedStream::peekWrite() const
{
    if (writeTail == writeHead)
    {
        return -1;
    }

    return writeBuffer[writeHead];
}

int BufferedStream::available()
{
    int num = readTail - readHead;
    if (num < 0)
        num += BUFFER_SIZE;

    return num;
}

int32_t BufferedStream::availableWrite() const
{
    int32_t num = writeTail - writeHead;
    if (num < 0)
    {
        num += BUFFER_SIZE;
    }

    return num;
}
