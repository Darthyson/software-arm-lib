/*
 *  VL53Lx_i2c.h - Basic i2c read/write functions for the ToF Sensor VL54L4CD.
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 3 as
 *  published by the Free Software Foundation.
 */

#ifndef _SB_LIB_VL53LX_I2C_H_
#define _SB_LIB_VL53LX_I2C_H_

#include <stdint.h>

/**
 * @def VL53L4CD_I2C_FAST_MODE_PLUS
 * Configures the i2c to run in Fast Mode Plus (up to 1MHz).
 * Default max value is 400kHz if this macro is not defined.
 */
//#define VL53L4CD_I2C_FAST_MODE_PLUS

/**
 * Indicates no error in the i2c operation.
 * @note Stupid "redefinition" of @ref VL53L4CD_ERROR_NONE
 */
#define VL53LX_ERROR_NONE       ((uint8_t)0)

/**
 * Indicates a timeout error in the I2C operation
 * @note Stupid "redefinition" of @ref VL53L4CD_ERROR_TIMEOUT
 */
#define VL53LX_ERROR_TIMEOUT    ((uint8_t)255)

/**
* Type definition for device address/instance
*/
typedef uint16_t Dev_t;

/**
 * Reads an 8-bit value from a specified register via I2C.
 *
 * @param i2cAddress      The device address.
 * @param registerAddress The register address from which to read.
 * @param value           Pointer to the 8-bit value buffer.
 *
 * @return @ref VL53LX_ERROR_NONE if successful, otherwise @ref VL53LX_ERROR_TIMEOUT
 */
uint8_t VL53L4CD_RdByte(Dev_t i2cAddress, uint16_t registerAddress, uint8_t *value);

/**
 * Reads a 16-bit value from a specified register via I2C.
 *
 * @param i2cAddress      The device address.
 * @param registerAddress The register address from which to read.
 * @param value           Pointer to the 16-bit value buffer.
 *
 * @return @ref VL53LX_ERROR_NONE if successful, otherwise @ref VL53LX_ERROR_TIMEOUT
 */
uint8_t VL53L4CD_RdWord(Dev_t i2cAddress, uint16_t registerAddress, uint16_t *value);

/**
 * Reads a 32-bit value from a specified register via I2C.
 *
 * @param i2cAddress      The device address.
 * @param registerAddress The register address from which to read.
 * @param value           Pointer to the 32-bit value buffer.
 *
 * @return @ref VL53LX_ERROR_NONE if successful, otherwise @ref VL53LX_ERROR_TIMEOUT
 */
uint8_t VL53L4CD_RdDWord(Dev_t i2cAddress, uint16_t registerAddress, uint32_t *value);

/**
 * Writes an 8-bit value to a specified register via I2C.
 *
 * @param i2cAddress      The device instance.
 * @param registerAddress The register address to which to write.
 * @param value           The 8-bit value to write.
 *
 * @return @ref VL53LX_ERROR_NONE if successful, otherwise @ref VL53LX_ERROR_TIMEOUT
 */
uint8_t VL53L4CD_WrByte(Dev_t i2cAddress, uint16_t registerAddress, uint8_t value);

/**
 * Writes a 16-bit value to a specified register via I2C.
 *
 * @param i2cAddress      The device instance.
 * @param registerAddress The register address to which to write.
 * @param value           The 16-bit value to write.
 *
 * @return @ref VL53LX_ERROR_NONE if successful, otherwise @ref VL53LX_ERROR_TIMEOUT
 */
uint8_t VL53L4CD_WrWord(Dev_t i2cAddress, uint16_t registerAddress, uint16_t value);

/**
 * Writes a 32-bit value to a specified register via I2C.
 *
 * @param i2cAddress      The device instance.
 * @param registerAddress The register address to which to write.
 * @param value           The 32-bit value to write.
 *
 * @return @ref VL53LX_ERROR_NONE if successful, otherwise @ref VL53LX_ERROR_TIMEOUT
 */
uint8_t VL53L4CD_WrDWord(Dev_t i2cAddress, uint16_t registerAddress, uint32_t value);

 /**
  * Waits for the specified number of milliseconds.
  *
  * @param i2cAddress The device instance.
  * @param timeMs     The number of milliseconds to wait.
  *
  * @return Always @ref VL53LX_ERROR_NONE
  */
uint8_t VL53L4CD_WaitMs(Dev_t i2cAddress, uint32_t timeMs);

#endif // _SB_LIB_VL53LX_I2C_H_
