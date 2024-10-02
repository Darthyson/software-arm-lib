/*
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 3 as
 *  published by the Free Software Foundation.
 */

#include <sblib/i2c/tof/VL53Lx_i2c.h>

uint8_t VL53L4CD_Read(Dev_t i2cAddress, uint16_t registerAddress, void *value, uint8_t size)
{
    return VL53Lx_Read(i2cAddress, registerAddress, value, size, true);
}

uint8_t VL53L4CD_Write(Dev_t i2cAddress, uint16_t registerAddress, void *value, uint8_t size)
{
    return VL53Lx_Write(i2cAddress, registerAddress, value, size, true);
}

uint8_t VL53L4CD_RdByte(Dev_t i2cAddress, uint16_t registerAddress, uint8_t *value)
{
    return VL53Lx_Read(i2cAddress, registerAddress, value, sizeof(uint8_t), true);
}

uint8_t VL53L4CD_RdWord(Dev_t i2cAddress, uint16_t registerAddress, uint16_t *value)
{
    return VL53Lx_Read(i2cAddress, registerAddress, value, sizeof(uint16_t), true);
}

uint8_t VL53L4CD_RdDWord(Dev_t i2cAddress, uint16_t registerAddress, uint32_t *value)
{
    return VL53Lx_Read(i2cAddress, registerAddress, value, sizeof(uint32_t), true);
}

uint8_t VL53L4CD_WrByte(Dev_t i2cAddress, uint16_t registerAddress, uint8_t value)
{
    return VL53Lx_Write(i2cAddress, registerAddress, &value, sizeof(uint8_t), true);
}

uint8_t VL53L4CD_WrWord(Dev_t i2cAddress, uint16_t registerAddress, uint16_t value)
{
    return VL53Lx_Write(i2cAddress, registerAddress, &value, sizeof(uint16_t), true);
}

uint8_t VL53L4CD_WrDWord(Dev_t i2cAddress, uint16_t registerAddress, uint32_t value)
{
    return VL53Lx_Write(i2cAddress, registerAddress, &value, sizeof(uint32_t), true);
}

uint8_t VL53L4CD_WaitMs(Dev_t i2cAddress, uint32_t timeMs)
{
    return VL53Lx_WaitMs(i2cAddress, timeMs);
}
