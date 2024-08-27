/*
 *  VL53L4CD_i2c.h - Basic i2c read/write functions for the ToF Sensor VL54L4CD.
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 3 as
 *  published by the Free Software Foundation.
 */

#ifndef _SB_LIB_VL53L4CD_I2C_H_
#define _SB_LIB_VL53L4CD_I2C_H_

#include <stdint.h>
#include <sblib/i2c/tof/VL53Lx_i2c.h>

/**
 * @def VL53L4CD_I2C_FAST_MODE_PLUS
 * Configures the i2c to run in Fast Mode Plus (up to 1MHz).
 * Default max value is 400kHz if this macro is not defined.
 */
//#define VL53L4CD_I2C_FAST_MODE_PLUS

uint8_t VL53L4CD_Read(Dev_t i2cAddress, uint16_t registerAddress, void *value, uint8_t size);
uint8_t VL53L4CD_Write(Dev_t i2cAddress, uint16_t registerAddress, void *value, uint8_t size);
uint8_t VL53L4CD_RdByte(Dev_t i2cAddress, uint16_t registerAddress, uint8_t *value);
uint8_t VL53L4CD_RdWord(Dev_t i2cAddress, uint16_t registerAddress, uint16_t *value);
uint8_t VL53L4CD_RdDWord(Dev_t i2cAddress, uint16_t registerAddress, uint32_t *value);
uint8_t VL53L4CD_WrByte(Dev_t i2cAddress, uint16_t registerAddress, uint8_t value);
uint8_t VL53L4CD_WrWord(Dev_t i2cAddress, uint16_t registerAddress, uint16_t value);
uint8_t VL53L4CD_WrDWord(Dev_t i2cAddress, uint16_t registerAddress, uint32_t value);
uint8_t VL53L4CD_WaitMs(Dev_t i2cAddress, uint32_t timeMs);

#endif /* _SB_LIB_VL53L4CD_I2C_H_ */
