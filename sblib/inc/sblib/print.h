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
#include <cstdint>


// Automatically inserted macro by VS Code 1.108.0
// Compatibility for embedded toolchains that don't provide C++14/17 type trait aliases
#ifndef __cpp_lib_type_trait_variable_templates
namespace std {
    // C++14 type trait aliases (_t suffix)
    template<bool B, typename T = void>
    using enable_if_t = typename enable_if<B, T>::type;
    
    template<typename T>
    using underlying_type_t = typename underlying_type<T>::type;
    
    // C++17 type trait variable templates (_v suffix)
    template<typename T>
    constexpr bool is_integral_v = is_integral<T>::value;
    
    template<typename T>
    constexpr bool is_enum_v = is_enum<T>::value;
    
    template<typename T, typename U>
    constexpr bool is_same_v = is_same<T, U>::value;
    
    template<typename T>
    constexpr bool is_signed_v = is_signed<T>::value;
}
#endif

/**
 * Base for printing numbers.
 */
enum Base : uint8_t
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

    /** Default precision for floating point printing */
    static constexpr uint8_t PRINT_FLOAT_PRECISION_DEFAULT = 2;

    /** Maximum precision supported for floating point printing */
    static constexpr uint8_t PRINT_FLOAT_MAX_PRECISION = 7;

    /**
     * Print a character.
     *
     * @param ch    The character to print.
     * @return The number of bytes that were written.
     */
    uint32_t print(char ch);

    /**
     * Print a zero-terminated string.
     *
     * @param str   The string to print.
     * @return The number of bytes that were written.
     */
    uint32_t print(const char* str);

    /**
     * Print an IEEE 754 single precision float with given precision.
     * 
     * @param value         The float to print
     * @param precision     The precision to print, default @ref PRINT_FLOAT_PRECISION_DEFAULT
     * @return The number of bytes that were written.
     * @note The maximum precision supported is @ref PRINT_FLOAT_MAX_PRECISION
     */
    uint32_t print(float value, uint8_t precision = PRINT_FLOAT_PRECISION_DEFAULT);

    /**
     * Print any integer type (template overload).
     * Handles all integer types and enums.
     *
     * @param value     The number to print
     * @param base      The base of the number, default: DEC
     * @param digits    Output at least this number of digits (optional)
     * @return The number of bytes that were written.
     */
    template<typename T>
    std::enable_if_t<
        (std::is_integral_v<T> || std::is_enum_v<T>) &&
        !std::is_same_v<T, char>,
    uint32_t>
    print(T value, const Base base = DEC, const int8_t digits = -1)
    {
        if constexpr (std::is_enum_v<T>)
        {
            // Check if the enum is signed
            typedef std::underlying_type_t<T> underlying_t;
            if constexpr (std::is_signed_v<underlying_t>)
                return printInteger(static_cast<intmax_t>(value), base, digits);
            else
                return printUnsignedInteger(static_cast<uintmax_t>(value), base, digits);
        }
        else
        {
            if constexpr (std::is_signed_v<T>)
                return printInteger(static_cast<intmax_t>(value), base, digits);
            else
                return printUnsignedInteger(static_cast<uintmax_t>(value), base, digits);
        }
    }

    /**
     * Print a pointer.
     *
     * @param ptr   The pointer to print
     * @return The number of bytes that were written.
     */
    uint32_t print(const void* ptr);

    /**
     * Print a zero-terminated string followed by any integer type.
     * Handles all integer types and enums.
     *
     * @param str       The string to print
     * @param value     The number to print
     * @param base      The base of the number, default: DEC
     * @param digits    Output at least this number of digits (optional)
     * @return The number of bytes that were written.
     */
    template<typename T>
    std::enable_if_t<
        (std::is_integral_v<T> || std::is_enum_v<T>) &&
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
     * Print a zero-terminated string followed by an IEEE 754 single precision float with given precision.
     * 
     * @param str           The string to print
     * @param value         The float to print
     * @param precision     The precision to print, default @ref PRINT_FLOAT_PRECISION_DEFAULT
     * @return The number of bytes that were written.
     * @note The maximum precision supported is @ref PRINT_FLOAT_MAX_PRECISION
     */
    uint32_t print(const char* str, float value, uint8_t precision = PRINT_FLOAT_PRECISION_DEFAULT);

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
     * Print any integer type followed by a new line.
     * Handles all integer types and enums.
     * 
     * @param value     The number to print
     * @param base      The base of the number, default: DEC
     * @param digits    Output at least this number of digits (optional)
     * @return The number of bytes that were written.
     */
    template<typename T>
    std::enable_if_t<
        (std::is_integral_v<T> || std::is_enum_v<T>) &&
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
     * Print an IEEE 754 single precision float with given precision followed by a new line.
     * 
     * @param value         The float to print
     * @param precision     The precision to print, default @ref PRINT_FLOAT_PRECISION_DEFAULT
     * @return The number of bytes that were written.
     * @note The maximum precision supported is @ref PRINT_MAX_FLOAT_PRECISION
     */
    uint32_t println(float value, uint8_t precision = PRINT_FLOAT_PRECISION_DEFAULT);
    /**
     * Print a zero-terminated string followed by any integer type and a new line.
     * Handles all integer types and enums.
     * 
     * @param str       The string to print
     * @param value     The number to print
     * @param base      The base of the number, default: DEC
     * @param digits    Output at least this number of digits (optional)
     * @return The number of bytes that were written.
     */
    template<typename T>
    std::enable_if_t<
        (std::is_integral_v<T> || std::is_enum_v<T>) &&
        !std::is_same_v<T, char>,
    uint32_t>
    println(const char* str, T value, Base base = DEC, int8_t digits = -1)
    {
        return print(str, value, base, digits) + println();
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
     * Print a zero-terminated string followed by an IEEE 754 single precision float with a given precision and a new line.
     * 
     * @param str           The string to print
     * @param value         The float to print
     * @param precision     The precision to print, default is @ref PRINT_FLOAT_PRECISION_DEFAULT
     * @return The number of bytes that were written.
     * @note The maximum precision supported is @ref PRINT_FLOAT_MAX_PRECISION
     */
    uint32_t println(const char* str, float value, uint8_t precision = PRINT_FLOAT_PRECISION_DEFAULT);
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

/**
 * I'm only here for static_asserts of the template
 */
namespace test_print
{
    // =====================================================================
    // Template test: can_print_template_v<T>
    // Detects at compile-time if type T can be passed to Print::print<T>()
    // =====================================================================

    // Default template: defaults to false (type cannot be printed)
    // Second parameter is used for SFINAE detection
    template<typename T, typename = void>
    struct can_print_template : std::false_type {};

    // Matched when the print expression is valid.
    // If print<T>() doesn't exist or can't be called,
    // this template is discarded and the default template is used
    template<typename T>
        struct can_print_template<T, std::void_t<decltype(
            std::declval<Print>().print<T>(std::declval<T>())
    )>> : std::true_type {};

    // Alias for shorter syntax
    // Usage: static_assert(can_print_template_v<T>);
    template<typename T>
    constexpr bool can_print_template_v = can_print_template<T>::value;

    // Print template should accept integrals
    static_assert(can_print_template_v<int>);
    static_assert(can_print_template_v<unsigned int>);
    static_assert(can_print_template_v<long>);
    static_assert(can_print_template_v<unsigned long>);
    static_assert(can_print_template_v<short>);
    static_assert(can_print_template_v<unsigned short>);
    static_assert(can_print_template_v<long long>);
    static_assert(can_print_template_v<unsigned long long>);

    // Print template should accept fixed-width integer
    static_assert(can_print_template_v<uint8_t>);
    static_assert(can_print_template_v<int8_t>);
    static_assert(can_print_template_v<uint16_t>);
    static_assert(can_print_template_v<int16_t>);
    static_assert(can_print_template_v<uint32_t>);
    static_assert(can_print_template_v<int32_t>);
    static_assert(can_print_template_v<uint64_t>);
    static_assert(can_print_template_v<int64_t>);
    static_assert(can_print_template_v<uintptr_t>);
    static_assert(can_print_template_v<intmax_t>);
    static_assert(can_print_template_v<uintmax_t>);

    // Should NOT accept char
    static_assert(!can_print_template_v<char>);

    // Test enums
    enum TestEnum : uint8_t
    {
        VALUE1 = 1,
        VALUE2 = 2
    };

    enum class TestEnumClass : uint16_t
    {
        VALUE1 = 1,
        VALUE2 = 2
    };

    enum class TestEnumClassUint8 : uint8_t
    {
        VALUE1 = 1,
        VALUE2 = 2
    };
    // Should accept enum and enum class
    static_assert(can_print_template_v<TestEnum>);
    static_assert(can_print_template_v<TestEnumClass>);
    static_assert(can_print_template_v<TestEnumClassUint8>);
}

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
