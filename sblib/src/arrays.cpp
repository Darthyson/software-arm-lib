/*
 *  arrays.cpp - Array handling.
 *
 *  Copyright (c) 2014 Stefan Taferner <stefan.taferner@gmx.at>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 3 as
 *  published by the Free Software Foundation.
 */

#include <sblib/arrays.h>


short indexOf(const int val, const int* arr, const short count)
{
    for (short idx = 0; idx < count; ++idx)
    {
        if (arr[idx] == val)
            return idx;
    }

    return -1;
}

short indexOf(const unsigned short val, const unsigned short* arr, const short count)
{
    for (short idx = 0; idx < count; ++idx)
    {
        if (arr[idx] == val)
            return idx;
    }

    return -1;
}

short indexOf(const uint8_t val, const uint8_t* arr, const short count)
{
    for (short idx = 0; idx < count; ++idx)
    {
        if (arr[idx] == val)
            return idx;
    }

    return -1;
}
