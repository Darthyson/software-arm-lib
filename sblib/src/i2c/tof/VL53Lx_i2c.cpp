/*
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 3 as
 *  published by the Free Software Foundation.
 */

#include <sblib/i2c/tof/VL53Lx_i2c.h>
#include <sblib/timer.h>
#include <sblib/i2c.h>

/**
 * Initializes I2C communication and sets the clock rate.
 *
 * @details This function initializes the I2C communication interface and sets the clock rate to 400kHz.
 *          It must be called before performing any other I2C operations.
 */
static void VL53Lx_Init()
{
    i2c_lpcopen_init();
    Chip_I2C_SetClockRate(I2C0, 400000);
}

uint8_t VL53Lx_Read(Dev_t i2cAddress, uint16_t registerAddress, void *value, uint8_t size, bool convertToLittleEndianess)
{
    VL53Lx_Init();
    static_assert(sizeof(registerAddress) == 2);
    uint8_t reg[sizeof(registerAddress)];
    reg[0] = static_cast<uint8_t>((registerAddress >> 8) & 0xff); // high byte
    reg[1] = static_cast<uint8_t>(registerAddress & 0xff);        // low byte
    I2C_XFER_T xfer = {0};
    xfer.slaveAddr = static_cast<uint8_t>(i2cAddress);
    xfer.txBuff = reg;
    xfer.txSz = sizeof(reg)/sizeof(reg[0]);
    xfer.rxBuff = reinterpret_cast<uint8_t*>(value);
    xfer.rxSz = size;
    while (Chip_I2C_MasterTransfer(I2C0, &xfer) == I2C_STATUS_ARBLOST) {}

    if (xfer.txSz != 0)
    {
        return VL53LX_ERROR_TIMEOUT;
    }

    if (xfer.rxSz != 0)
    {
        return VL53LX_ERROR_TIMEOUT;
    }

    if (!convertToLittleEndianess)
    {
        return VL53LX_ERROR_NONE;
    }

    for (uint8_t i = 0; i < size / 2; ++i)
    {
        uint8_t temp = reinterpret_cast<uint8_t*>(value)[i];
        reinterpret_cast<uint8_t*>(value)[i] = reinterpret_cast<uint8_t*>(value)[size - i - 1];
        reinterpret_cast<uint8_t*>(value)[size - i - 1] = temp;
    }
    return VL53LX_ERROR_NONE;
}

uint8_t VL53Lx_Write(Dev_t i2cAddress, uint16_t registerAddress, void *value, uint8_t size, bool convertToBigEndianess)
{
    VL53Lx_Init();
    static_assert(sizeof(registerAddress) == 2);
    uint8_t txBufferSize = sizeof(registerAddress) + size;
    uint8_t txBuffer[txBufferSize];
    txBuffer[0] = static_cast<uint8_t>((registerAddress >> 8) & 0xff);
    txBuffer[1] = static_cast<uint8_t>(registerAddress & 0xff);
    uint8_t *pTxBuffer = &txBuffer[sizeof(registerAddress)];

    if (convertToBigEndianess)
    {
        // Reverse value bytes
        for (uint8_t i = 0; i < size; ++i)
        {
            pTxBuffer[i] = ((uint8_t *)value)[size - 1 - i];
        }
    }

    int32_t sent = Chip_I2C_MasterSend(I2C0, static_cast<uint8_t>(i2cAddress), txBuffer, txBufferSize);

    if (sent != txBufferSize)
    {
        return VL53LX_ERROR_TIMEOUT;
    }
    return VL53LX_ERROR_NONE;
}

uint8_t VL53Lx_RdByte(Dev_t i2cAddress, uint16_t registerAddress, uint8_t *value)
{
    return VL53Lx_Read(i2cAddress, registerAddress, value, sizeof(uint8_t), true);
}

uint8_t VL53Lx_RdWord(Dev_t i2cAddress, uint16_t registerAddress, uint16_t *value)
{
    return VL53Lx_Read(i2cAddress, registerAddress, value, sizeof(uint16_t), true);
}

uint8_t VL53Lx_RdDWord(Dev_t i2cAddress, uint16_t registerAddress, uint32_t *value)
{
    return VL53Lx_Read(i2cAddress, registerAddress, value, sizeof(uint32_t), true);
}

uint8_t VL53Lx_WrByte(Dev_t i2cAddress, uint16_t registerAddress, uint8_t value)
{
    return VL53Lx_Write(i2cAddress, registerAddress, &value, sizeof(uint8_t), true);
}

uint8_t VL53Lx_WrWord(Dev_t i2cAddress, uint16_t registerAddress, uint16_t value)
{
    return VL53Lx_Write(i2cAddress, registerAddress, &value, sizeof(uint16_t), true);
}

uint8_t VL53Lx_WrDWord(Dev_t i2cAddress, uint16_t registerAddress, uint32_t value)
{
    return VL53Lx_Write(i2cAddress, registerAddress, &value, sizeof(uint32_t), true);
}

uint8_t VL53Lx_WaitMs(Dev_t i2cAddress, uint32_t TimeMs)
{
    if (TimeMs > MAX_DELAY_MILLISECONDS)
    {
        delay(TimeMs);
        return VL53LX_ERROR_NONE;
    }
    delayMicroseconds(TimeMs * 1000);
    return VL53LX_ERROR_NONE;
}
