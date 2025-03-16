/*
 *  debounce.h - A debouncer.
 *
 *  Copyright (c) 2014 Stefan Taferner <stefan.taferner@gmx.at>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 3 as
 *  published by the Free Software Foundation.
 */
#ifndef sblib_debounce_h
#define sblib_debounce_h
#include <cstdint>

/**
 * A debouncer for debouncing a value. The debouncer ensures that a value
 * stays the same over a period of time before it is used.
 *
 * Example:
 *
 *     Debouncer d;
 *     ...
 *     int32_t value = d.debounce(digitalRead(PIO1_8), 100);
 */
class Debouncer
{
public:
    /**
     * Create a debouncer.
     */
    Debouncer() = default;

    /**
     * Send the current value into the debouncer. When the value is the same for at least
     * the debounce time, the new value becomes the valid value.
     *
     * @param current - the current value.
     * @param timeout - the debounce time in milliseconds. Default: 100 msec
     *
     * @return The debounced value.
     */
    int32_t debounce(int32_t current, uint32_t timeout = 100);

    /**
     * @return The debounced value.
     */
    [[nodiscard]] int32_t value() const;

    /**
     * Set the debounced value without debouncing.
     *
     * @param newValue - the new debounced value.
     */
    void init(int32_t newValue);

    /**
     * @return The last temporary value that was sent to debounce()
     */
    [[nodiscard]] int32_t lastValue() const;

private:
    uint32_t time = 0;
    int32_t valid = 0;
    int32_t last = 0;
};

#endif /*sblib_debounce_h*/
