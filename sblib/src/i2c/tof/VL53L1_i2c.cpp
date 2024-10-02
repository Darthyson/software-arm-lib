/*
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 3 as
 *  published by the Free Software Foundation.
 */

#include <sblib/i2c/tof/VL53L1_i2c.h>
#include <sblib/i2c/tof/VL53Lx_i2c.h>

int8_t VL53L1_WriteMulti(uint16_t dev, uint16_t index, uint8_t *pdata, uint32_t count)
{
    return static_cast<int8_t>(VL53Lx_Write(dev, index, pdata, static_cast<uint8_t>(count), false));
}

int8_t VL53L1_ReadMulti(uint16_t dev, uint16_t index, uint8_t *pdata, uint32_t count)
{
    return static_cast<int8_t>(VL53Lx_Read(dev, index, pdata, static_cast<uint8_t>(count), false));
}

int8_t VL53L1_WrByte(uint16_t dev, uint16_t index, uint8_t data)
{
    return static_cast<int8_t>(VL53Lx_WrByte(dev, index, data));
}

int8_t VL53L1_WrWord(uint16_t dev, uint16_t index, uint16_t data)
{
    return static_cast<int8_t>(VL53Lx_WrWord(dev, index, data));
}

int8_t VL53L1_WrDWord(uint16_t dev, uint16_t index, uint32_t data)
{
    return static_cast<int8_t>(VL53Lx_WrDWord(dev, index, data));
}

int8_t VL53L1_RdByte(uint16_t dev, uint16_t index, uint8_t *data)
{
    return static_cast<int8_t>(VL53Lx_RdByte(dev, index, data));
}

int8_t VL53L1_RdWord(uint16_t dev, uint16_t index, uint16_t *data)
{
    return static_cast<int8_t>(VL53Lx_RdWord(dev, index, data));
}

int8_t VL53L1_RdDWord(uint16_t dev, uint16_t index, uint32_t *data)
{
    return static_cast<int8_t>(VL53Lx_RdDWord(dev, index, data));
}

int8_t VL53L1_WaitMs(uint16_t dev, int32_t wait_ms)
{
    return static_cast<int8_t>(VL53Lx_WaitMs(dev, static_cast<uint32_t>(wait_ms)));
}
