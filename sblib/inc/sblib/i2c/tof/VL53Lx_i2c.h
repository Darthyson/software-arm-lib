/*
 *  VL53Lx_i2c.h - Basic i2c read/write functions for the ToF Sensor family VL53Lx.
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 3 as
 *  published by the Free Software Foundation.
 */

#ifndef _SB_LIB_VL53LX_I2C_H_
#define _SB_LIB_VL53LX_I2C_H_

#include <stdint.h>

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
 * Reads data from the VL53Lx sensor.
 *
 * @details This function reads data from a specified register of the VL53Lx sensor.
 *          It initializes the i2c communication, sets up the transfer structure,
 *          and performs the i2c read operation.
 *
 * @param i2cAddress      The i2c address of the sensor.
 * @param registerAddress The register address from where data needs to be read.
 * @param value           A pointer to a buffer where the read data will be stored.
 * @param size            The size of the data to be read.
 *
 * @return @ref VL53LX_ERROR_NONE on success, otherwise @ref VL53LX_ERROR_TIMEOUT
 */
uint8_t VL53Lx_Read(Dev_t i2cAddress, uint16_t registerAddress, void *value, uint8_t size);

/**
 * Writes data to the VL53Lx sensor.
 *
 * @details This function writes data to a specified register of the VL53Lx sensor.
 *          It initializes the i2c communication, prepares the transmission buffer with
 *          the register address and data, and performs the i2c write operation.
 *
 * @param i2cAddress      The i2c address of the sensor.
 * @param registerAddress The register address where data needs to be written.
 * @param value           A pointer to the data that needs to be written.
 * @param size            The size of the data to be written.
 *
 * @return @ref VL53LX_ERROR_NONE on success, otherwise @ref VL53LX_ERROR_TIMEOUT
 */
uint8_t VL53Lx_Write(Dev_t i2cAddress, uint16_t registerAddress, void *value, uint8_t size);

/**
 * Reads an 8-bit value from a specified register via I2C.
 *
 * @param i2cAddress      The device address.
 * @param registerAddress The register address from which to read.
 * @param value           Pointer to the 8-bit value buffer.
 *
 * @return @ref VL53LX_ERROR_NONE if successful, otherwise @ref VL53LX_ERROR_TIMEOUT
 */
uint8_t VL53Lx_RdByte(Dev_t i2cAddress, uint16_t registerAddress, uint8_t *value);

/**
 * Reads a 16-bit value from a specified register via I2C.
 *
 * @param i2cAddress      The device address.
 * @param registerAddress The register address from which to read.
 * @param value           Pointer to the 16-bit value buffer.
 *
 * @return @ref VL53LX_ERROR_NONE if successful, otherwise @ref VL53LX_ERROR_TIMEOUT
 */
uint8_t VL53Lx_RdWord(Dev_t i2cAddress, uint16_t registerAddress, uint16_t *value);

/**
 * Reads a 32-bit value from a specified register via I2C.
 *
 * @param i2cAddress      The device address.
 * @param registerAddress The register address from which to read.
 * @param value           Pointer to the 32-bit value buffer.
 *
 * @return @ref VL53LX_ERROR_NONE if successful, otherwise @ref VL53LX_ERROR_TIMEOUT
 */
uint8_t VL53Lx_RdDWord(Dev_t i2cAddress, uint16_t registerAddress, uint32_t *value);

/**
 * Writes an 8-bit value to a specified register via I2C.
 *
 * @param i2cAddress      The device instance.
 * @param registerAddress The register address to which to write.
 * @param value           The 8-bit value to write.
 *
 * @return @ref VL53LX_ERROR_NONE if successful, otherwise @ref VL53LX_ERROR_TIMEOUT
 */
uint8_t VL53Lx_WrByte(Dev_t i2cAddress, uint16_t registerAddress, uint8_t value);

/**
 * Writes a 16-bit value to a specified register via I2C.
 *
 * @param i2cAddress      The device instance.
 * @param registerAddress The register address to which to write.
 * @param value           The 16-bit value to write.
 *
 * @return @ref VL53LX_ERROR_NONE if successful, otherwise @ref VL53LX_ERROR_TIMEOUT
 */
uint8_t VL53Lx_WrWord(Dev_t i2cAddress, uint16_t registerAddress, uint16_t value);

/**
 * Writes a 32-bit value to a specified register via I2C.
 *
 * @param i2cAddress      The device instance.
 * @param registerAddress The register address to which to write.
 * @param value           The 32-bit value to write.
 *
 * @return @ref VL53LX_ERROR_NONE if successful, otherwise @ref VL53LX_ERROR_TIMEOUT
 */
uint8_t VL53Lx_WrDWord(Dev_t i2cAddress, uint16_t registerAddress, uint32_t value);

 /**
  * Waits for the specified number of milliseconds.
  *
  * @param i2cAddress The device instance.
  * @param timeMs     The number of milliseconds to wait.
  *
  * @return Always @ref VL53LX_ERROR_NONE
  */
uint8_t VL53Lx_WaitMs(Dev_t i2cAddress, uint32_t timeMs);

#endif // _SB_LIB_VL53LX_I2C_H_