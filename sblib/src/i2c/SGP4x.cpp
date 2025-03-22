/******************************************************************************
 *
 * @author Doumanix <doumanix@gmx.de> Copyright (c) 2023
 * @bug No known bugs.
 ******************************************************************************/

/*
 This program is free software; you can redistribute it and/or modify
 it under the terms of the GNU General Public License version 3 as
 published by the Free Software Foundation.
 ---------------------------------------------------------------------------*/

#include <cstdint>
#include <sblib/i2c/SGP4x.h>
#include <sblib/i2c.h>
#include <sblib/timer.h>
#include <sblib/bits.h>

typedef enum
{
    eSGP4xAddress = 0x59, // SGP40 (VOC) and SGP41 (VOC & NOx)
} HUM_SENSOR_T;

SGP4xClass::SGP4xClass():
    rawVocTics(0),
    rawNoxTics(0),
    vocIndexValue(-1),
    noxIndexValue(-1),
    featureSet(0)
{
    // init VOC index algorithm with default sampling interval
    GasIndexAlgorithm_init_with_sampling_interval(&voc_algorithm_params, GasIndexAlgorithm_ALGORITHM_TYPE_VOC, GasIndexAlgorithm_DEFAULT_SAMPLING_INTERVAL);
    // init NOx index algorithm with default sampling interval
    GasIndexAlgorithm_init_with_sampling_interval(&nox_algorithm_params, GasIndexAlgorithm_ALGORITHM_TYPE_NOX, GasIndexAlgorithm_DEFAULT_SAMPLING_INTERVAL);
}

SGP4xResult SGP4xClass::readSensor(Sgp4xCommand command, uint8_t* commandBuffer, uint8_t commandBufferSize,
                                   uint8_t* readBuffer, uint8_t readBufferSize, uint16_t processDelayMs)
{
    if ((commandBuffer == nullptr) || (commandBufferSize < 2))
    {
        return SGP4xResult::invalidCommandBuffer;
    }

    commandBuffer[0] = highByte((uint16_t)command);
    commandBuffer[1] = lowByte((uint16_t)command);

    int32_t bytesProcessed = Chip_I2C_MasterSend(I2C0, eSGP4xAddress, commandBuffer, commandBufferSize);
    if (bytesProcessed != commandBufferSize)
    {
        i2c_lpcopen_init();
        return SGP4xResult::sendError;
    }

    if (processDelayMs > 0)
    {
        delay(processDelayMs);
    }

    if ((readBuffer == nullptr) || (readBufferSize == 0))
    {
        // command expects no response
        return SGP4xResult::success;
    }

    bytesProcessed = Chip_I2C_MasterRead(I2C0, eSGP4xAddress, readBuffer, readBufferSize);
    if (bytesProcessed != readBufferSize)
    {
        i2c_lpcopen_init();
        return SGP4xResult::readError;
    }

    // crc8 checksum is transmitted after every word (2 bytes)
    // including the checksum we have to receive a multiple of 3 bytes
    if ((bytesProcessed == 0) || ((bytesProcessed % 3) != 0))
    {
        return SGP4xResult::invalidByteCount;
    }

    for (uint8_t i = 0; i < bytesProcessed; i = i + 3)
    {
        if (crc8(&readBuffer[i], 2) != readBuffer[i + 2])
        {
            return SGP4xResult::crc8Mismatch;
        }
    }

    return SGP4xResult::success;
}

SGP4xResult SGP4xClass::init(uint32_t samplingIntervalMs)
{
    i2c_lpcopen_init();
    rawVocTics = 0;
    rawNoxTics = 0;
    vocIndexValue = -1;
    noxIndexValue = -1;
    // init VOC index algorithm with provided sampling interval
    GasIndexAlgorithm_init_with_sampling_interval(&voc_algorithm_params, GasIndexAlgorithm_ALGORITHM_TYPE_VOC, (float)(samplingIntervalMs / 1000.f));
    // init NOx index algorithm with provided sampling interval
    GasIndexAlgorithm_init_with_sampling_interval(&nox_algorithm_params, GasIndexAlgorithm_ALGORITHM_TYPE_NOX, (float)(samplingIntervalMs / 1000.f));
    return executeConditioning();
}

SGP4xResult SGP4xClass::executeConditioning()
{
    uint8_t readBuffer[3];
    uint8_t readBufferSize = sizeof(readBuffer) / sizeof(*readBuffer);

    uint8_t cmdBuffer[8] = {0x00, 0x00, 0x80, 0x00, 0xA2, 0x66, 0x66, 0x93};
    uint8_t commandBufferSize = sizeof(cmdBuffer) / sizeof(*cmdBuffer);

    // max. duration for processing sgp41_execute_conditioning is 50ms
    SGP4xResult result = readSensor(Sgp4xCommand::selfConditioning, cmdBuffer, commandBufferSize, readBuffer, readBufferSize, 50);
    return result;
}


SGP4xResult SGP4xClass::executeSelfTest()
{
    uint8_t cmdBuffer[2];
    uint8_t commandBufferSize = sizeof(cmdBuffer) / sizeof(*cmdBuffer);

    uint8_t readBuffer[3];
    uint8_t readBufferSize = sizeof(readBuffer) / sizeof(*readBuffer);

    // max. duration for processing sgp41_execute_self_test is 320ms (we add a margin of 30ms to be on the safe side)
    SGP4xResult result = readSensor(Sgp4xCommand::selfTest, cmdBuffer, commandBufferSize, readBuffer, readBufferSize, 350);
    if (result != SGP4xResult::success)
    {
        return result;
    }

    // Datasheet:
    // The most significant byte shall be ignored and check only nibble of least significant byte
    // ignore bits 2,3

    // bit 0 set -> VOC pixel error
    if ((readBuffer[1] & 0x1) != 0)
    {
        return SGP4xResult::vocPixelError;
    }

    // bit 1 set -> NOx pixel error
    if ((readBuffer[1] & 0x2) != 0)
    {
        return SGP4xResult::noxPixelError;
    }

    return SGP4xResult::success;
}

SGP4xResult SGP4xClass::measureRawSignal(float relativeHumidity, float temperature, bool useCompensation)
{
    uint8_t readBuffer[6];
    uint8_t readBufferSize = sizeof(readBuffer) / sizeof(*readBuffer);
    // default static command without temperature/humidity correction
    // (same parameter byte values as with 50% relative humidity at 25 degree celsius)
    //                      0x26, 0x19, 0x80, 0x00, 0xA2, 0x66, 0x66, 0x93
    uint8_t cmdBuffer[8] = {0x00, 0x00, 0x80, 0x00, 0xA2, 0x66, 0x66, 0x93};
    uint8_t commandBufferSize = sizeof(cmdBuffer) / sizeof(*cmdBuffer);

    if (useCompensation)
    {
        const uint16_t relativeHumidityTicks = relativeHumidity * 65535 / 100;
        const uint16_t temperatureTicks = (temperature + 45.f) * 65535 / 175;
        // set provided relative humidity and temperature in command parameters
        // 2 bytes of data with most significant bit first!
        // 1 byte crc8 checksum
        // e.g. 50% relative humidity => bytes 0x80 0x00 0xA2
        //      25 degree celsius     => bytes 0x66 0x66 0x93
        cmdBuffer[2] = highByte(relativeHumidityTicks);
        cmdBuffer[3] = lowByte(relativeHumidityTicks);
        cmdBuffer[4] = crc8(&cmdBuffer[2], 2);
        cmdBuffer[5] = highByte(temperatureTicks);
        cmdBuffer[6] = lowByte(temperatureTicks);
        cmdBuffer[7] = crc8(&cmdBuffer[5], 2);
    }

    // max. duration for processing sgp41_measure_raw_signals is 50ms
    SGP4xResult result = readSensor(Sgp4xCommand::measureRaw, cmdBuffer, commandBufferSize, readBuffer, readBufferSize, 50);
    if (result != SGP4xResult::success)
    {
        return result;
    }

    rawVocTics = makeWord(readBuffer[0], readBuffer[1]);
    GasIndexAlgorithm_process(&voc_algorithm_params, rawVocTics, &vocIndexValue);

    rawNoxTics = makeWord(readBuffer[3], readBuffer[4]);
    GasIndexAlgorithm_process(&nox_algorithm_params, rawNoxTics, &noxIndexValue);

    return SGP4xResult::success;
}

SGP4xResult SGP4xClass::measureRawSignal()
{
    return measureRawSignal(50.f, 25.f, false);
}

SGP4xResult SGP4xClass::getSerialnumber(uint8_t* serialNumber, uint8_t length)
{
    uint8_t readBuffer[9];
    uint8_t readBufferSize = sizeof(readBuffer) / sizeof(*readBuffer);
    uint8_t cmdBuffer[2];
    uint8_t commandBufferSize = sizeof(cmdBuffer) / sizeof(*cmdBuffer);

    // max. duration for processing sgp4x_get_serial_number is 1 second
    SGP4xResult result = readSensor(Sgp4xCommand::getSerial, cmdBuffer, commandBufferSize, readBuffer, readBufferSize, 1000);
    if (result != SGP4xResult::success)
    {
        return result;
    }

    uint8_t idxBuffer = 0;
    uint8_t idxSerial = 0;
    while ((idxBuffer < readBufferSize) && (idxSerial < length))
    {
        *serialNumber = readBuffer[idxBuffer];
        serialNumber++;
        idxSerial++;
        idxBuffer++;
        if ((idxBuffer != 0) && ((idxBuffer % 3) == 2))
        {
            idxBuffer++; // skip crc8 byte in readBuffer
        }
    }

    return SGP4xResult::success;
}

SGP4xResult SGP4xClass::turnHeaterOffAndReturnToIdle()
{
    uint8_t cmdBuffer[2];
    uint8_t commandBufferSize = sizeof(cmdBuffer) / sizeof(*cmdBuffer);

    // max. duration for processing sgp4x_turn_heater_off is 1 second
    SGP4xResult result = readSensor(Sgp4xCommand::heaterOff, cmdBuffer, commandBufferSize, nullptr, 0, 1000);
    return result;
}

SGP4xResult SGP4xClass::readFeatureSet()
{
    uint8_t cmdBuffer[2];
    uint8_t commandBufferSize = sizeof(cmdBuffer) / sizeof(*cmdBuffer);

    uint8_t readBuffer[3];
    uint8_t readBufferSize = sizeof(readBuffer) / sizeof(*readBuffer);

    // max. duration 10ms
    SGP4xResult result = readSensor(Sgp4xCommand::featureSet, cmdBuffer, commandBufferSize, readBuffer, readBufferSize, 10);
    if (result != SGP4xResult::success)
    {
        return result;
    }

    featureSet = makeWord(readBuffer[0], readBuffer[1]);
    return SGP4xResult::success;
}

int32_t SGP4xClass::getVocIndexValue()
{
    return vocIndexValue;
}

int32_t SGP4xClass::getNoxIndexValue()
{
    return noxIndexValue;
}

int32_t SGP4xClass::getRawVocValue()
{
    return rawVocTics;
}

int32_t SGP4xClass::getRawNoxValue()
{
    return rawNoxTics;
}

uint16_t SGP4xClass::getFeatureSet()
{
    return featureSet;
}

uint8_t SGP4xClass::crc8(const uint8_t* data, int len)
{
    /*
     *
     * CRC-8 formula from page 14 of SHT spec pdf
     *
     * Test data 0xBE, 0xEF should yield 0x92
     *
     * Initialization data 0xFF
     * Polynomial 0x31 (x8 + x5 +x4 +1)
     * Final XOR 0x00
     */

    uint8_t crc(0xFF);

    for (int j = len; j; --j)
    {
        crc ^= *data++;

        for (int i = 8; i; --i)
        {
            constexpr uint8_t POLYNOMIAL(0x31);
            crc = (crc & 0x80) ? (crc << 1) ^ POLYNOMIAL : (crc << 1);
        }
    }
    return crc;
}
