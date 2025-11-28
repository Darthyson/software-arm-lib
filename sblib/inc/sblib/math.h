/*
 *  math.h - Mathematical functions and constants.
 *
 *  Copyright (c) 2014 Stefan Taferner <stefan.taferner@gmx.at>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 3 as
 *  published by the Free Software Foundation.
 */
#ifndef SBLIB_MATH_H_
#define SBLIB_MATH_H_

/**
 * The number PI
 */
#define PI 3.1415926535897932384626433832795f

/**
 * PI * 0.5
 */
#define HALF_PI 1.5707963267948966192313216916398f

/**
 * PI * 2
 */
#define TWO_PI 6.283185307179586476925286766559f

/**
 * Conversion factor for converting degree to radian.
 */
#define DEG_TO_RAD 0.017453292519943295769236907684886f

/**
 * Conversion factor for converting radian to degree.
 */
#define RAD_TO_DEG 57.295779513082320876798154814105f

/**
 * Euler
 */
#define EULER 2.718281828459045235360287471352f


// undefine stdlib's abs if encountered
#ifdef abs
# undef abs
#endif

/**
 * The minimum of two values.
 */
#define min(a,b) ((a) < (b) ? (a) : (b))

/**
 * The maximum of two values.
 */
#define max(a,b) ((a) > (b) ? (a) : (b))

/**
 * The absolute value.
 */
#define abs(x) ((x) > 0 ? (x) : -(x))

/**
 * Ensure that the value v is between low and high.
 *
 * @param v - the value
 * @param low - the minimum value
 * @param high - the maximum value
 * @return low if v < low, high if v > high, else v.
 */
#define constrain(v, low, high) ((v) < (low) ? (low) : ((v) > (high) ? (high) : (v)))

/**
 * Round x to the nearest integer.
 */
#define round(x) ((x) >= 0 ? (int)((x) + 0.5f) : (int)((x) - 0.5f))

/**
 * Convert degrees to radians.
 *
 * @param deg - the value in degrees.
 * @return The value in radians.
 */
#define radians(deg) ((deg) * DEG_TO_RAD)

/**
 * Convert radians to degrees.
 *
 * @param rad - the value in radians.
 * @return The value in degrees.
 */
#define degrees(rad) ((rad)*RAD_TO_DEG)

/**
 * The square of a value.
 *
 * @param x - the value to square.
 * @return The squared value: x*x
 */
#define sq(x) ((x) * (x))

#endif /* SBLIB_MATH_H_ */
