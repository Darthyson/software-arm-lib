/*
 *  print.h - Base class that provides print() and println()
 *
 *  Copyright (c) 2014 Stefan Taferner <stefan.taferner@gmx.at>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 3 as
 *  published by the Free Software Foundation.
 */
#ifndef sblib_print_h
#define sblib_print_h

#include <sblib/types.h>

/**
 * Base for printing numbers.
 */
enum Base
{
    /** A decimal number. */
    DEC = 10,

    /** A hexadecimal number. */
    HEX = 16,

    /** A octal number. */
    OCT = 8,

    /** A binary number. */
    BIN = 2
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
     * @param ch - the character to print.
     *
     * @return The number of bytes that were written.
     */
    int print(const char ch);

    /**
     * Print a zero terminated string.
     *
     * @param str - the string to print.
     *
     * @return The number of bytes that were written.
     */
    int print(const char* str);

    /**
     * Print a number.
     *
     * @param value - the number to print
     * @param base - the base of the number, default: DEC
     * @param digits - output at least this number of digits (optional)
     *
     * @return The number of bytes that were written.
     */
    int print(int value, Base base = DEC, int digits = -1);

    /**
     * Print a float with a given precision.
     * @param value The float to print
     * @param precision The precision to print, default 2
     * @return The number of bytes that were written.
     * @warning Maximum precision supported is 7
     */
    int print(float value, int precision = 2);

    /**
     * Print a zero terminated string followed by a number.
     *
     * @param str - the string to print
     * @param value - the number to print
     * @param base - the base of the number, default: DEC
     * @param digits - output at least this number of digits (optional)
     *
     * @return The number of bytes that were written.
     */
    int print(const char* str, int value, Base base = DEC, int digits = -1);

    /**
     * Print an unsigned number.
     *
     * @param value - the number to print
     * @param base - the base of the number, default: DEC
     * @param digits - output at least this number of digits (optional)
     *
     * @return The number of bytes that were written.
     */
    int print(uintptr_t value, Base base = DEC, int digits = -1);

    /**
     * Print a pointer.
     *
     * @param ptr - the pointer to print
     *
     * @return The number of bytes that were written.
     */
    int print(const void* ptr);

    /**
     * Print a zero terminated string followed by an unsigned number.
     *
     * @param str - the string to print
     * @param value - the number to print
     * @param base - the base of the number, default: DEC
     * @param digits - output at least this number of digits (optional)
     *
     * @return The number of bytes that were written.
     */
    int print(const char* str, uintptr_t value, Base base = DEC, int digits = -1);

    /**
     * Print a zero terminated string followed by a pointer.
     *
     * @param str - the string to print
     * @param ptr - the pointer to print
     *
     * @return The number of bytes that were written.
     */
    int print(const char* str, const void* ptr);

    /**
     * Print a zero terminated string followed by a float with a given precision.
     * @param str   The string to print
     * @param value The float to print
     * @param precision The precision to print, default 2
     * @return The number of bytes that were written.
     * @warning Maximum precision supported is 9
     */
    int print(const char* str, float value, int precision = 2);

    /**
     * Print a new line by sending a carriage return '\r' (ASCII 13) followed
     * by a newline '\n' (ASCII 10).
     *
     * @return the number of bytes written.
     */
    int println();

    /**
     * Print a zero terminated string followed by a new line.
     *
     * @param str - the string to print.
     *
     * @return The number of bytes that were written.
     */
    int println(const char* str);

    /**
     * Print a number followed by a new line.
     *
     * @param value - the number to print
     * @param base - the base of the number, default: DEC
     * @param digits - output at least this number of digits (optional)
     *
     * @return The number of bytes that were written.
     */
    int println(int value, Base base = DEC, int digits = -1);

    /**
     * Print a zero terminated string followed by a number and a new line.
     *
     * @param str - the string to print
     * @param value - the number to print
     * @param base - the base of the number, default: DEC
     * @param digits - output at least this number of digits (optional)
     *
     * @return The number of bytes that were written.
     */
    int println(const char* str, int value, Base base = DEC, int digits = -1);

    /**
     * Print an unsigned number followed by a new line.
     *
     * @param value - the number to print
     * @param base - the base of the number, default: DEC
     * @param digits - output at least this number of digits (optional)
     *
     * @return The number of bytes that were written.
     */
    int println(uintptr_t value, Base base = DEC, int digits = -1);

    /**
     * Print a pointer followed by a new line.
     *
     * @param ptr - the pointer to print
     *
     * @return The number of bytes that were written.
     */
    int println(const void* ptr);

    /**
     * Print a float with a given precision followed by a new line.
     * @param value The float to print
     * @param precision The precision to print, default 2
     * @return The number of bytes that were written.
     * @warning Maximum precision supported is 7
     */
    int println(float value, int precision = 2);

    /**
     * Print a zero terminated string followed by an unsigned number and a new line.
     *
     * @param str - the string to print
     * @param value - the number to print
     * @param base - the base of the number, default: DEC
     * @param digits - output at least this number of digits (optional)
     *
     * @return The number of bytes that were written.
     */
    int println(const char* str, uintptr_t value, Base base = DEC, int digits = -1);

    /**
     * Print a zero terminated string followed by a pointer and a new line.
     *
     * @param str - the string to print
     * @param ptr - the pointer to print
     *
     * @return The number of bytes that were written.
     */
    int println(const char* str, const void* ptr);

    /**
     * Print a zero terminated string followed by a float with a given precision and a new line.
     * @param str   The string to print
     * @param value The float to print
     * @param precision The precision to print, default is 2
     * @return The number of bytes that were written.
     * @warning Maximum precision supported is 9
     */
    int println(const char* str, float value, int precision = 2);

    /**
     * Write a zero terminated string.
     *
     * @param str - the string to write.
     * @return The number of bytes that were written.
     */
    int write(const char* str);

    /**
     * Write a number of bytes.
     *
     * @param data - the bytes to write.
     * @param count - the number of bytes to write.
     *
     * @return The number of bytes that were written.
     */
    virtual int write(const byte* data, int count);

    /**
     * Write a single byte.
     *
     * @param ch - the byte to write.
     *
     * @return 1 if the byte was written, 0 if not.
     */
    virtual int write(byte ch) = 0;
};


//
// Inline functions
//

inline int Print::print(const char ch)
{
    return this->write((const byte*)&ch, 1);
}

inline int Print::print(const char* str)
{
    return this->write(str);
}

inline int Print::print(const void* ptr)
{
    return print((uintptr_t)ptr, HEX, sizeof(void*));
}

inline int Print::print(const char* str, const void* ptr)
{
    return print(str, (uintptr_t)ptr, HEX, sizeof(void*));
}

inline int Print::println(const char* str)
{
    return this->write(str) + println();
}

inline int Print::println(int value, Base base, int digits)
{
    return print(value, base, digits) + println();
}

inline int Print::println(uintptr_t value, Base base, int digits)
{
    return print(value, base, digits) + println();
}

inline int Print::println(const void* ptr)
{
    return print(ptr) + println();
}

inline int Print::println(const char* str, const void* ptr)
{
    return print(str, ptr) + println();
}

#endif /*sblib_print_h*/
