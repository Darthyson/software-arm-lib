/*
 *  VL53L1_i2c.h - Basic i2c read/write functions for the ToF Sensor VL54L1X.
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 3 as
 *  published by the Free Software Foundation.
 */
 
#ifndef _SB_LIB_VL53L1X_I2C_H_
#define _SB_LIB_VL53L1X_I2C_H_

#include <stdint.h>

/**
 * @param count Number of bytes to read (max. 255)
 * @warning Even count is declared as uint32_t the function only supports a maximum of 255.
 */
int8_t VL53L1_WriteMulti(uint16_t dev, uint16_t index, uint8_t *pdata, uint32_t count);

/**
 * @param count Number of bytes to read (max. 255)
 * @warning Even count is declared as uint32_t the function only supports a maximum of 255.
 */
int8_t VL53L1_ReadMulti(uint16_t dev, uint16_t index, uint8_t *pdata, uint32_t count);

int8_t VL53L1_WrByte(uint16_t dev, uint16_t index, uint8_t data);
int8_t VL53L1_WrWord(uint16_t dev, uint16_t index, uint16_t data);
int8_t VL53L1_WrDWord(uint16_t dev, uint16_t index, uint32_t data);
int8_t VL53L1_RdByte(uint16_t dev, uint16_t index, uint8_t *pdata);
int8_t VL53L1_RdWord(uint16_t dev, uint16_t index, uint16_t *pdata);
int8_t VL53L1_RdDWord(uint16_t dev, uint16_t index, uint32_t *pdata);
int8_t VL53L1_WaitMs(uint16_t dev, int32_t wait_ms);

#endif /* _SB_LIB_VL53L1X_I2C_H_ */
