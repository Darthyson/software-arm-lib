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
#include <cstring>
#include <limits>

// The size of the internal buffer in print()
constexpr size_t PRINT_BUFFER_SIZE = 8 * sizeof(uintmax_t);


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

uint32_t Print::printUnsignedInteger(uintmax_t value, Base base, int8_t digits)
{
    uint8_t buf[PRINT_BUFFER_SIZE]; // need the maximum size for binary printing

    if (base < BIN) base = BIN;
    if (base > HEX) base = HEX;

    uint8_t pos = PRINT_BUFFER_SIZE; // buf will be filled from last to first index
    do
    {
        pos--;
        const auto ch = static_cast<uint8_t>(value % base);
        buf[pos] = static_cast<uint8_t>((ch < 10 ? '0' : 'A' - 10) + ch);
        value /= base;
        digits--;
    }
    while (digits > 0 || value);

    return write(&buf[pos], PRINT_BUFFER_SIZE - pos);
}

uint32_t Print::printUnsignedInteger(const char* str, const uintmax_t value, const Base base, const int8_t digits)
{
    uint32_t wlen = print(str);
    wlen += printUnsignedInteger(value, base, digits);
    return wlen;
}

uint32_t Print::print(const float value, uint8_t precision)
{
    // Sanitize precision
    if (precision > PRINT_FLOAT_MAX_PRECISION)
    {
        precision = PRINT_FLOAT_MAX_PRECISION;
    }

    // Type-punning using union to access bits
    union
    {
        float f;
        uint32_t u;
    } floatBits{};

    // IEEE 754 binary32 (single) precision float bit manipulation
    // Sign: bit 31
    // Exponent: bits 30-23 (8 bits, biased by 127 (0x7f))
    // Mantissa: bits 22-0 (23 bits, implicit leading 1)
    constexpr uint8_t SizeInBits = 32;
    constexpr uint8_t SizeExponentInBits = 8;
    constexpr uint8_t SizeMantissaInBits = 23;

    constexpr uint8_t MaskSign = 0x1;
    constexpr uint16_t MaskExponent = (1 << SizeExponentInBits) - 1;
    constexpr uint32_t MaskMantissa = (1 << SizeMantissaInBits) - 1;

    constexpr uint8_t ExponentBias = 0x7f; // 127;
    constexpr uint8_t MaxExponent = MaskExponent;
    constexpr uint32_t MaxMantissa = MaskMantissa;

    floatBits.f = value;
    const uint32_t bits = floatBits.u;
    const uint8_t sign = bits >> (SizeInBits - 1) & MaskSign;
    const uint8_t exponent = bits >> SizeMantissaInBits & MaskExponent;
    uint32_t mantissa = bits & MaskMantissa;

    uint32_t wlen = 0;

    // Print sign
    if (sign)
    {
        wlen += write('-');
    }

    // Handle NaN and inf
    if (exponent == MaxExponent)
    {
        if (mantissa != 0)
        {
            return wlen + write("NaN");
        }

        return wlen + write("inf");
    }

    // Handle zero
    if (exponent == 0 && mantissa == 0)
    {
        wlen += write('0');
        if (precision > 0)
        {
            wlen += write('.');
            for (uint8_t i = 0; i < precision; i++)
            {
                wlen += write('0');
            }
        }
        return wlen;
    }

    int16_t exp;
    if (exponent != 0)
    {
        // Add implicit leading bit for normalized numbers
        exp = static_cast<int16_t>(exponent - ExponentBias);
        mantissa |= MaxMantissa + 1;
    }
    else
    {
        // Denormalized numbers
        exp = 0;
        mantissa = 0;
    }

    // Convert mantissa to integer by shifting based on exponent
    // Mantissa represents 1.fraction where fraction is 23 bits
    // We need to compute: mantissa * 2^(exp - 23)
    uintmax_t integerPart = 0;
    uintmax_t fractionalPart = 0;
    if (exp >= SizeMantissaInBits)
    {
        // Large number: all mantissa bits are in integer part
        // Check for potential overflow:
        // if shift amount exceeds bit width, clamp to max
        const int32_t shiftAmount = exp - SizeMantissaInBits;
        constexpr auto shiftMax = static_cast<int16_t>(8 * sizeof(uintmax_t) -
                                  (SizeMantissaInBits + 1));
        if (shiftAmount > shiftMax)
        {
            // Shift would overflow
            return wlen + write("overflow");
        }

        integerPart = static_cast<uintmax_t>(mantissa) << shiftAmount;
    }
    else if (exp >= 0)
    {
        // Mixed: some bits in integer, some in fraction
        integerPart = mantissa >> (SizeMantissaInBits - exp);
        fractionalPart = (mantissa & ((1ULL << (SizeMantissaInBits - exp)) - 1));
        // Scale fractional part to use full 64-bit range for precision
        fractionalPart = fractionalPart << (41 + exp);
    }
    else
    {
        // Small number: all in fractional part
        // Shift mantissa left into the fractional part
        if (exp >= -40) // Avoid underflow
            fractionalPart = static_cast<uintmax_t>(mantissa) << (41 + exp);
        else
            fractionalPart = 0; // Too small, treat as zero
    }

    // Print integer part
    wlen += print(integerPart);

    if (precision == 0)
    {
        return wlen;
    }

    // Print fractional part
    wlen += write('.');

    // Convert binary fractional part to decimal digits
    // fractionalPart is scaled to use upper bits of 64-bit value
    // We extract decimal digits by repeatedly multiplying by 10
    for (uint8_t i = 0; i < precision; i++)
    {
        // Multiply by 10 using 128-bit arithmetic simulation
        const uintmax_t low = (fractionalPart & 0xffffffff) * 10;
        uintmax_t high = (fractionalPart >> 32) * 10;
        high += low >> 32; // Add carry from the low part

        // Extract the digit from the high part
        const auto digit = static_cast<uint8_t>(high >> 32);
        wlen += write('0' + digit);

        // Keep only the fractional part
        fractionalPart = ((high & 0xffffffff) << 32) | (low & 0xffffffff);
    }
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
