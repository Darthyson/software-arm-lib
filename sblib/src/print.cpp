/*
 *  print.cpp - Base class that provides print() and println()
 *
 *  Copyright (c) 2014 Stefan Taferner <stefan.taferner@gmx.at>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 3 as
 *  published by the Free Software Foundation.
 */

#include <sblib/print.h>
#include <algorithm>
#include <cmath>
#include <cstring>

// The size of the internal buffer in print()
#define PRINT_BUFFER_SIZE (8 * sizeof(intmax_t) + 1)


uint32_t Print::printInteger(intmax_t value, const Base base, int8_t digits)
{
    uint32_t wlen = 0;
    if (value < 0)
    {
        wlen += write('-');
        value = -value;
        --digits;
    }

    return printUnsignedInteger(static_cast<uintmax_t>(value), base, digits) + wlen;
}

uint32_t Print::printInteger(const char* str, const intmax_t value, const Base base, const int8_t digits)
{
    uint32_t wlen = print(str);
    wlen += printInteger(value, base, digits);
    return wlen;
}

uint32_t Print::printUnsignedInteger(uintmax_t value, const Base base, int8_t digits)
{
    byte buf[PRINT_BUFFER_SIZE]; // need the maximum size for binary printing

    auto b = static_cast<uint8_t>(base);
    if (b < 2)
        b = 2;

    byte* pos = buf + PRINT_BUFFER_SIZE;
    do
    {
        const byte ch = value % b;
        *--pos = (ch < 10 ? '0' : 'A' - 10) + ch;

        value /= b;
    }
    while (--digits > 0 || value);

    return write(pos, buf + PRINT_BUFFER_SIZE - pos);
}

uint32_t Print::printUnsignedInteger(const char* str, const uintmax_t value, const Base base, const int8_t digits)
{
    uint32_t wlen = print(str);
    wlen += printUnsignedInteger(value, base, digits);
    return wlen;
}

uint32_t Print::print(const float value, uint8_t precision)
{
    const auto number = static_cast<intmax_t>(value);
    float fraction = fabsf(value - static_cast<float>(number));
    uint32_t wlen = print(number);

    if (precision < 1)
    {
        return wlen;
    }

    precision = std::min<uint8_t>(7, precision);

    wlen += print(".");
    for (uint8_t i = 0; i < precision; i++)
    {
        fraction *= 10.0f;
    }
    wlen += print(static_cast<uintmax_t>(fraction), DEC, static_cast<int8_t>(precision));
    return wlen;
}

uint32_t Print::print(const char* str, const float value, const uint8_t precision)
{
    uint32_t wlen = print(str);
    wlen += print(value, precision);
    return wlen;
}

uint32_t Print::println()
{
    return write('\r') + write('\n');
}

uint32_t Print::write(const byte* data, uint32_t count)
{
    uint32_t wlen = 0;
    while (count--)
    {
        wlen += write(*data++);
    }

    return wlen;
}

uint32_t Print::write(const char* str)
{
    if (str == nullptr)
        return 0;

    return write(reinterpret_cast<const byte*>(str), strlen(str));
}

uint32_t Print::printUnsignedIntegerLn(const char* str, const uintmax_t value, const Base base, const int8_t digits)
{
    uint32_t wlen = printUnsignedInteger(str, value, base, digits);
    wlen += println();
    return wlen;
}

uint32_t Print::printIntegerLn(const char* str, const intmax_t value, const Base base, const int8_t digits)
{
    uint32_t wlen = printInteger(str, value, base, digits);
    wlen += println();
    return wlen;
}

uint32_t Print::println(const float value, const uint8_t precision)
{
    uint32_t wlen = print(value, precision);
    wlen += println();
    return wlen;
}

uint32_t Print::println(const char* str, const float value, const uint8_t precision)
{
    uint32_t wlen = print(str, value, precision);
    wlen += println();
    return wlen;
}
