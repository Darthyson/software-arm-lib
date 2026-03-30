/*
 *  stream.h - Base class for character-based streams.
 *
 *  Copyright (c) 2014 Stefan Taferner <stefan.taferner@gmx.at>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 3 as
 *  published by the Free Software Foundation.
 */
#ifndef SBLIB_STREAM_H_
#define SBLIB_STREAM_H_

#include <sblib/print.h>
#include <cstring>
#include <cstdint>


/**
 * Base class for character-based streams.
 * Streams can be read from and written to.
 */
class Stream : public Print
{
public:
    /**
     * Set the maximum number of milliseconds that are waited for a
     * character to arrive. Default is 1000 msec.
     *
     * @param newTimeout  The number of milliseconds to wait.
     */
    void setTimeout(uint32_t newTimeout);

    /**
     * Read a single byte.
     *
     * @return The read byte (0..255) or -1 if no byte was received.
     */
    virtual int16_t read() = 0;

    /**
     * Query the next byte to be read, without reading it.
     *
     * @return The next byte (0..255) or -1 if no byte is available
     *         for reading.
     */
    virtual int16_t peek() = 0;

    /**
     * @brief Get the number of bytes available for reading.
     * @return The number of bytes that are available for reading.
     */
    virtual uint32_t available() = 0;

    /**
     * Wait until all bytes are written.
     */
    virtual void flush() = 0;

    /**
     * Read the first valid integer value from the current position. Initial characters
     * that are not digits (or the minus sign) are skipped. The integer is terminated
     * by the first character that is not a digit.
     *
     * @return The read integer, or 0 if no integer could be read due to timeout.
     */
    int32_t parseInt();

    /**
     * Read the first valid integer value from the current position. Initial characters
     * that are not digits (or the minus sign) are skipped. The integer is terminated
     * by the first character that is not a digit. If the skip character is read, it
     * is ignored and skipped. Digits are no valid skip character, they are never ignored.
     *
     * @param skipChar  The character to ignore when read (do not use a digit)
     *
     * @return The read integer, or 0 if no integer could be read due to timeout.
     */
    int32_t parseInt(char skipChar);

    /**
     * Read characters from the stream into the buffer. Reading stops if length
     * characters have been read or a timeout occurs.
     *
     * @param buffer  The buffer to read into
     * @param length  The maximum number of bytes to read
     *
     * @return The number of bytes read, or 0 if timeout occurred.
     */
    uint32_t readBytes(char* buffer, uint32_t length);

    /**
     * Read characters from the stream into the buffer. Reading stops if length
     * characters have been read or a timeout occurs.
     *
     * @param buffer  The buffer to read into
     * @param length  The maximum number of bytes to read
     *
     * @return The number of bytes read, or 0 if timeout occurred.
     */
    uint32_t readBytes(uint8_t* buffer, uint32_t length);

    /**
     * Read characters from the stream into the buffer. Reading stops if length
     * characters have been read, the terminator character is read, or a timeout occurs.
     *
     * @param terminator  The terminator character
     * @param buffer      The buffer to read into
     * @param length      The maximum number of bytes to read
     *
     * @return The number of bytes read, or 0 if timeout occurred.
     */
    uint32_t readBytesUntil(char terminator, char* buffer, uint32_t length);

    /**
     * Read characters from the stream into the buffer. Reading stops if length
     * characters have been read, the terminator character is read, or a timeout occurs.
     *
     * @param terminator  The terminator character
     * @param buffer      The buffer to read into
     * @param length      The maximum number of bytes to read
     *
     * @return The number of bytes read, or 0 if timeout occurred.
     */
    uint32_t readBytesUntil(char terminator, uint8_t* buffer, uint32_t length);

    /**
     * Reads data from the stream until the target string is read.
     *
     * @param target  The target string to find, zero terminated.
     * @return True if target string is found, false if timed out.
     */
    bool find(const char* target);

    /**
     * Reads data from the stream until the target string is read.
     *
     * @param target   The target string to find, zero terminated.
     * @return True if target string is found, false if timed out.
     */
    bool find(const uint8_t* target);

    /**
     * Reads data from the stream until the target string is read.
     *
     * @param target  The target string to find.
     * @param length  The length of the target string to find.
     *
     * @return True if target string is found, false if timed out.
     */
    bool find(const char* target, uint32_t length);

    /**
     * Reads data from the stream until the target string is read.
     *
     * @param target  The target string to find.
     * @param length  The length of the target string to find.
     *
     * @return True if target string is found, false if timed out.
     */
    bool find(const uint8_t* target, uint32_t length);

    /**
     * Reads data from the stream until the target string is read. Reading stops
     * if the target string is read, the terminator string is read, or a timeout
     * occurs.
     *
     * @param target      The target string to find, zero terminated.
     * @param terminator  The terminator string, zero terminated.
     *
     * @return true if target string is found, false if the terminator string was
     *         read or a timeout occurred.
     */
    bool findUntil(const char* target, const char* terminator);

    /**
     * Reads data from the stream until the target string is read. Reading stops
     * if the target string is read, the terminator string is read, or a timeout
     * occurs.
     *
     * @param target - the target string to find, zero terminated.
     * @param terminator - the terminator string, zero terminated.
     *
     * @return true if target string is found, false if the terminator string was
     *         read or a timeout occurred.
     */
    bool findUntil(const uint8_t* target, const char* terminator);

    /**
     * Reads data from the stream until the target string is read. Reading stops
     * if the target string is read, the terminator string is read, or a timeout
     * occurs.
     *
     * @param target - the target string to find.
     * @param targetLen - the length of the target string.
     * @param terminator - the terminator string.
     * @param termLen - the length of the terminator string.
     *
     * @return true if target string is found, false if the terminator string was
     *         read or a timeout occurred.
     */
    bool findUntil(const char* target, uint32_t targetLen, const char* terminator, uint32_t termLen);

    /**
     * Reads data from the stream until the target string is read. Reading stops
     * if the target string is read, the terminator string is read, or a timeout
     * occurs.
     *
     * @param target - the target string to find.
     * @param targetLen - the length of the target string.
     * @param terminate - the terminator string.
     * @param termLen - the length of the terminator string.
     *
     * @return true if target string is found, false if the terminator string was
     *         read or a timeout occurred.
     */
    bool findUntil(const uint8_t* target, uint32_t targetLen, const char* terminate, uint32_t termLen);

protected:
    uint32_t timeout; //!< timeout for timed reads in milliseconds

    /**
     * Create a stream with the default timeout of 1 second.
     */
    Stream();

    /**
     * Read the next byte. Wait up to the number of milliseconds that are configured
     * as timeout.
     *
     * @return The read byte (0..255) or -1 if no byte was read within the timeout.
     */
    int16_t timedRead();

    /**
     * Query the next byte to be read, without reading it. Wait up to the number of
     * milliseconds that are configured as timeout.
     *
     * @return The peeked byte (0..255) or -1 if no byte was peeked within the timeout.
     */
    int16_t timedPeek();

    /**
     * Query the next numeric digit (or minus) from the stream. Non-digit characters
     * are discarded.
     *
     * @return The next digit or -1 if a timeout occurs.
     */
    int16_t peekNextDigit();

private:
    /**
     * Read characters from the stream into the buffer. Reading stops if length
     * characters have been read, the terminator character is read, or a timeout occurs.
     *
     * @param terminator - the terminator character or -1 if none
     * @param buffer - the buffer to read into
     * @param length - the maximum number of bytes to read
     *
     * @return The number of bytes read.
     */
    uint32_t _readBytesUntil(int32_t terminator, char* buffer, uint32_t length);
};


//
//  Inline functions
//

inline Stream::Stream()
    :
    timeout(1000)
{
}

inline void Stream::setTimeout(const uint32_t newTimeout)
{
    timeout = newTimeout;
}

inline int32_t Stream::parseInt()
{
    return parseInt('0');
}

inline uint32_t Stream::readBytes(char* buffer, const uint32_t length)
{
    return _readBytesUntil(-1, buffer, length);
}

inline uint32_t Stream::readBytes(uint8_t* buffer, const uint32_t length)
{
    return _readBytesUntil(-1, reinterpret_cast<char*>(buffer), length);
}

inline uint32_t Stream::readBytesUntil(const char terminator, char* buffer, const uint32_t length)
{
    return _readBytesUntil(terminator, buffer, length);
}

inline uint32_t Stream::readBytesUntil(const char terminator, uint8_t* buffer, const uint32_t length)
{
    return _readBytesUntil(terminator, reinterpret_cast<char*>(buffer), length);
}

inline bool Stream::find(const char* target)
{
    return findUntil(target, strlen(target), nullptr, 0);
}

inline bool Stream::find(const uint8_t* target)
{
    return find(reinterpret_cast<const char*>(target));
}

inline bool Stream::find(const char* target, const uint32_t length)
{
    return findUntil(target, length, nullptr, 0);
}

inline bool Stream::find(const uint8_t* target, const uint32_t length)
{
    return find(reinterpret_cast<const char*>(target), length);
}

inline bool Stream::findUntil(const char* target, const char* terminator)
{
    return findUntil(target, strlen(target), terminator, strlen(terminator));
}

inline bool Stream::findUntil(const uint8_t* target, const char* terminator)
{
    return findUntil(reinterpret_cast<const char*>(target), terminator);
}

inline bool Stream::findUntil(const uint8_t* target, const uint32_t targetLen, const char* terminate,
    const uint32_t termLen)
{
    return findUntil(reinterpret_cast<const char*>(target), targetLen, terminate, termLen);
}

#endif /* SBLIB_STREAM_H_ */
