/*
 *  print.h - Base class that provides print() and println()
 *
 *  Copyright (c) 2014 Stefan Taferner <stefan.taferner@gmx.at>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 3 as
 *  published by the Free Software Foundation.
 */
#ifndef SBLIB_PRINT_H_
#define SBLIB_PRINT_H_

#include <sblib/types.h>
#include <type_traits>

/**
 * Base for printing numbers.
 */
enum Base
{
    DEC = 10,   //!< A decimal number.
    HEX = 16,   //!< A hexadecimal number.
    OCT = 8,    //!< An octal number.
    BIN = 2     //!< A binary number.
};

/**
 * Base class that provides print() and println().
 */
class Print
{
public:
    virtual ~Print() = default;

    /**
     * Print a character.
     *
     * @param ch    The character to print.
     * @return The number of bytes that were written.
     */
    uint32_t print(const char ch);

    /**
     * Print a zero-terminated string.
     *
     * @param str   The string to print.
     * @return The number of bytes that were written.
     */
    uint32_t print(const char* str);

    /**
     * Print a float with given precision.
     * 
     * @param value         The float to print
     * @param precision     The precision to print, default 2
     * @return The number of bytes that were written.
     * @warning Maximum precision supported is 7
     */
    uint32_t print(float value, uint8_t precision = 2);

    /**
     * Print any integer type (template overload).
     * Handles all integer types including uint8_t, int8_t, uint16_t, int16_t, 
     * uint32_t, int32_t, uint64_t, int64_t, int, long, etc.
     *
     * @param value     The number to print
     * @param base      The base of the number, default: DEC
     * @param digits    Output at least this number of digits (optional)
     * @return The number of bytes that were written.
     */
    template<typename T>
    std::enable_if_t<
        std::is_integral_v<T> &&
        !std::is_same_v<T, char>,
    uint32_t>
    print(T value, const Base base = DEC, const int8_t digits = -1)
    {
        if (std::is_signed_v<T>)
            return printInteger(static_cast<intmax_t>(value), base, digits);

        return printUnsignedInteger(static_cast<uintmax_t>(value), base, digits);
    }

    /**
     * Print a pointer.
     *
     * @param ptr   The pointer to print
     * @return The number of bytes that were written.
     */
    uint32_t print(const void* ptr);

    /**
     * Print a zero-terminated string followed by any integer type (template overload).
     * Handles all integer types including uint8_t, int8_t, uint16_t, int16_t,
     * uint32_t, int32_t, uint64_t, int64_t, int, long, etc.
     *
     * @param str       The string to print
     * @param value     The number to print
     * @param base      The base of the number, default: DEC
     * @param digits    Output at least this number of digits (optional)
     * @return The number of bytes that were written.
     */
    template<typename T>
    std::enable_if_t<
        std::is_integral_v<T> &&
        !std::is_same_v<T, char>,
    uint32_t>
    print(const char* str, T value, Base base = DEC, int8_t digits = -1)
    {
        return print(str) + print(value, base, digits);
    }

    /**
     * Print a zero-terminated string followed by a pointer.
     *
     * @param str   The string to print
     * @param ptr   The pointer to print
     * @return The number of bytes that were written.
     */
    uint32_t print(const char* str, const void* ptr);

    /**
     * Print a zero-terminated string followed by a float with given precision.
     * 
     * @param str           The string to print
     * @param value         The float to print
     * @param precision     The precision to print, default 2
     * @return The number of bytes that were written.
     * @warning Maximum precision supported is 9
     */
    uint32_t print(const char* str, float value, uint8_t precision = 2);

    /**
     * Print a new line by sending a carriage return '\r' (ASCII 13) followed
     * by a newline '\n' (ASCII 10).
     *
     * @return the number of bytes written.
     */
    uint32_t println();

    /**
     * Print a zero-terminated string followed by a new line.
     *
     * @param str   The string to print.
     * @return The number of bytes that were written.
     */
    uint32_t println(const char* str);

    /**
     * Print any integer type followed by a new line (template overload).
     * Handles all integer types including uint8_t, int8_t, uint16_t, int16_t,
     * uint32_t, int32_t, uint64_t, int64_t, int, long, etc.
     * 
     * @param value     The number to print
     * @param base      The base of the number, default: DEC
     * @param digits    Output at least this number of digits (optional)
     * @return The number of bytes that were written.
     */
    template<typename T>
    std::enable_if_t<
        std::is_integral_v<T> &&
        !std::is_same_v<T, char>,
    int32_t>
    println(T value, Base base = DEC, int8_t digits = -1)
    {
        return print(value, base, digits) + println();
    }

    /**
     * Print a pointer followed by a new line.
     *
     * @param ptr   The pointer to print
     * @return The number of bytes that were written.
     */
    uint32_t println(const void* ptr);

    /**
     * Print a float with given precision followed by a new line.
     * 
     * @param value         The float to print
     * @param precision     The precision to print, default 2
     * @return The number of bytes that were written.
     * @warning Maximum precision supported is 7
     */
    uint32_t println(float value, uint8_t precision = 2);

    /**
     * Print a zero-terminated string followed by any integer type and a new line (template overload).
     * Handles all integer types including uint8_t, int8_t, uint16_t, int16_t,
     * uint32_t, int32_t, uint64_t, int64_t, int, long, etc.
     * 
     * @param str       The string to print
     * @param value     The number to print
     * @param base      The base of the number, default: DEC
     * @param digits    Output at least this number of digits (optional)
     * @return The number of bytes that were written.
     */
    template<typename T>
    std::enable_if_t<
        std::is_integral_v<T> &&
        !std::is_same_v<T, char>,
    uint32_t>
    println(const char* str, T value, Base base = DEC, int8_t digits = -1)
    {
        print(str, value, base, digits);
        return println();
    }

    /**
     * Print a zero-terminated string followed by a pointer and a new line.
     *
     * @param str   The string to print
     * @param ptr   The pointer to print
     * @return The number of bytes that were written.
     */
    uint32_t println(const char* str, const void* ptr);

    /**
     * Print a zero-terminated string followed by a float with a given precision and a new line.
     * 
     * @param str           The string to print
     * @param value         The float to print
     * @param precision     The precision to print, default is 2
     * @return The number of bytes that were written.
     * @warning Maximum precision supported is 9
     */
    uint32_t println(const char* str, float value, uint8_t precision = 2);

    /**
     * Write a zero-terminated string.
     *
     * @param str   The string to write.
     * @return The number of bytes that were written.
     */
    uint32_t write(const char* str);

    /**
     * Write a number of bytes.
     *
     * @param data      The bytes to write.
     * @param count     The number of bytes to write.
     * @return The number of bytes that were written.
     */
    virtual uint32_t write(const byte* data, uint32_t count);

    /**
     * Write a single byte.
     *
     * @param ch    The byte to write.
     * @return 1 if the byte was written, 0 if not.
     */
    virtual uint32_t write(byte ch) = 0;

private:
    /**
     * Print a number.
     *
     * @param value     The number to print
     * @param base      The base of the number, default: DEC
     * @param digits    Output at least this number of digits (optional)
     * @return The number of bytes that were written.
     */
    uint32_t printInteger(intmax_t value, Base base = DEC, int8_t digits = -1);

    /**
     * Print an unsigned number.
     *
     * @param value     The number to print
     * @param base      The base of the number, default: DEC
     * @param digits    Output at least this number of digits (optional)
     * @return The number of bytes that were written.
     */
    uint32_t printUnsignedInteger(uintmax_t value, Base base = DEC, int8_t digits = -1);

    /**
     * Print a zero-terminated string followed by a number.
     *
     * @param str       The string to print
     * @param value     The number to print
     * @param base      The base of the number, default: DEC
     * @param digits    Output at least this number of digits (optional)
     * @return The number of bytes that were written.
     */
    uint32_t printInteger(const char* str, intmax_t value, Base base = DEC, int8_t digits = -1);

    /**
     * Print a zero terminated string followed by an unsigned number.
     *
     * @param str       The string to print
     * @param value     The number to print
     * @param base      The base of the number, default: DEC
     * @param digits    Output at least this number of digits (optional)
     * @return The number of bytes that were written.
     */
    uint32_t printUnsignedInteger(const char* str, uintmax_t value, Base base = DEC, int8_t digits = -1);

    /**
     * Print a number followed by a new line.
     *
     * @param value     The number to print
     * @param base      The base of the number, default: DEC
     * @param digits    Output at least this number of digits (optional)
     * @return The number of bytes that were written.
     */
    uint32_t printIntegerLn(intmax_t value, Base base = DEC, int8_t digits = -1);

    /**
     * Print a zero-terminated string followed by a number and a new line.
     *
     * @param str       The string to print
     * @param value     The number to print
     * @param base      The base of the number, default: DEC
     * @param digits    Output at least this number of digits (optional)
     * @return The number of bytes that were written.
     */
    uint32_t printIntegerLn(const char* str, intmax_t value, Base base = DEC, int8_t digits = -1);

    /**
     * Print an unsigned number followed by a new line.
     *
     * @param value     The number to print
     * @param base      The base of the number, default: DEC
     * @param digits    Output at least this number of digits (optional)
     * @return The number of bytes that were written.
     */
    uint32_t printUnsignedIntegerLn(uintmax_t value, Base base = DEC, int8_t digits = -1);

    /**
     * Print a zero-terminated string followed by an unsigned number and a new line.
     *
     * @param str       The string to print
     * @param value     The number to print
     * @param base      The base of the number, default: DEC
     * @param digits    Output at least this number of digits (optional)
     * @return The number of bytes that were written.
     */
    uint32_t printUnsignedIntegerLn(const char* str, uintmax_t value, Base base = DEC, int8_t digits = -1);
};


//
// Inline functions
//

inline uint32_t Print::print(const char ch)
{
    return this->write(reinterpret_cast<const byte*>(&ch), 1);
}

inline uint32_t Print::print(const char* str)
{
    return this->write(str);
}

inline uint32_t Print::print(const void* ptr)
{
    return print(reinterpret_cast<uintmax_t>(ptr), HEX, 2 * sizeof(void*));
}

inline uint32_t Print::print(const char* str, const void* ptr)
{
    return print(str, reinterpret_cast<uintmax_t>(ptr), HEX, 2 * sizeof(void*));
}

inline uint32_t Print::println(const char* str)
{
    return this->write(str) + println();
}

inline uint32_t Print::printIntegerLn(const intmax_t value, const Base base, const int8_t digits)
{
    return printInteger(value, base, digits) + println();
}

inline uint32_t Print::printUnsignedIntegerLn(const uintmax_t value, const Base base, const int8_t digits)
{
    return printUnsignedInteger(value, base, digits) + println();
}

inline uint32_t Print::println(const void* ptr)
{
    return print(ptr) + println();
}

inline uint32_t Print::println(const char* str, const void* ptr)
{
    return print(str, ptr) + println();
}

#endif /* SBLIB_PRINT_H_ */
