/*
 *  Class for a SPSC (Single Producer Single Consumer) ring buffer.
 *
 *  Copyright (c) 2015 Stefan Taferner <stefan.taferner@gmx.at>
 *  Copyright (c) 2026 Darthyson <darth@maptrack.de>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 3 as
 *  published by the Free Software Foundation.
 */

#ifndef SBLIB_RINGBUFFER_H
#define SBLIB_RINGBUFFER_H

#include "sblib/always_inline.h"
#include <cstdint>

 /**
 * @brief A simple SPSC (Single Producer Single Consumer) Ring Buffer
 */
class RingBuffer
{
public:
    /**
     * @brief Enum representing the possible sizes of the ring buffer
     * 
     * @note  The size must be a power of 2, because the bufferSizeMask is calculated as (size - 1)
     *        for efficient wrapping of head and tail cursors with logical AND.
     * @warning The buffer in its size is stored in RAM.
                So be careful when choosing the size, especially on memory-constrained microcontrollers.
     */
    enum class Size : uint16_t
    {
        bytes_2 = 2,
        bytes_4 = 4,
        bytes_8 = 8,
        bytes_16 = 16,
        bytes_32 = 32,
        bytes_64 = 64,
        bytes_128 = 128,
        bytes_256 = 256,
        bytes_512 = 512,
        bytes_1024 = 1024
    };

    /**
     * @brief Constructor for the RingBuffer class
     * 
     * @param size The size of the ring buffer in bytes.
     */
    explicit RingBuffer(Size size);

    //Delete the other constructors
    RingBuffer() = delete;
    RingBuffer(const RingBuffer&) = delete;

    /**
     * @brief Destructor for the RingBuffer class
     */
    ~RingBuffer();

    /**
     * @brief Reset the ring buffer.
     */
    void clear();

    /**
     * @brief Test if the buffer is empty.
     * 
     * @return True if the buffer is empty, false otherwise.
     */
    [[nodiscard]] bool empty() const;

    /**
     * @brief Test if the buffer is full.
     * 
     * @return True if the buffer is full, false otherwise.
     */
    [[nodiscard]] bool full() const;

    /**
     * @brief Get the number of bytes that are currently stored in the buffer.
     * 
     * @return Zero if the buffer is empty, otherwise the number of bytes that 
     *         are currently stored in the buffer.
     */
    [[nodiscard]] uint16_t available() const;

    /**
     * @brief Query the next byte, without reading it.
     *
     * @return The next byte (0..255) or -1 if the buffer is empty.
     */
    [[nodiscard]] int16_t peek() const;

    /**
     * @brief Read a single byte.
     * 
     * @return The next byte (0..255) or -1 if the buffer is empty.
     */
    [[nodiscard]] int16_t pop();

    /**
     * @brief Write a single byte into the buffer.
     * 
     * @param byteToPush The byte to write into the buffer.
     * @return True if the byte was successfully written, 
     *         false if the buffer is full and the byte could not be written.
     */
    bool push(uint8_t byteToPush);

    /**
     * @brief Get the size of the buffer.
     * 
     * @return The size of the buffer in bytes.
     */
    [[nodiscard]] uint16_t getBufferSize() const;

private:
    /**
     * @brief The head cursor of the buffer.
     */
    volatile uint16_t head;

    /**
     * @brief The tail cursor of the buffer.
     */
    volatile uint16_t tail;

    /**
     * @brief The size of the buffer in bytes.
     */
    const uint16_t bufferSize;

    /**
     * @brief Mask for wrapping head and tail cursors (equal to buffer size - 1).
     */
    const uint16_t bufferSizeMask;

    /**
     * @brief The actual ring buffer array.
     */
    uint8_t * buffer;

};

#define SBLIB_OPTIMIZE __attribute__((optimize("O3")))

//
//  Inline functions which are performance critical
//  and are hopefully be inlined by the compiler
//
ALWAYS_INLINE SBLIB_OPTIMIZE bool RingBuffer::empty() const
{
    return head == tail;
}

ALWAYS_INLINE SBLIB_OPTIMIZE bool RingBuffer::full() const
{
    return ((tail + 1) & bufferSizeMask) == head;
}

ALWAYS_INLINE SBLIB_OPTIMIZE int16_t RingBuffer::pop()
{
    if (empty())
    {
        return -1;
    }

    const int16_t value = buffer[head];
    head = (head + 1) & bufferSizeMask;
    return value;
}

ALWAYS_INLINE SBLIB_OPTIMIZE bool RingBuffer::push(const uint8_t byteToPush)
{
    if (full())
    {
        return false;
    }

    buffer[tail] = byteToPush;
    tail = (tail + 1) & bufferSizeMask;
    return true;
}

#undef SBLIB_OPTIMIZE

#endif /* SBLIB_RINGBUFFER_H */
