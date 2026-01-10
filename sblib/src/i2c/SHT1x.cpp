/******************************************************************************
 * @addtogroup SBLIB_SENSOR Selfbus library sensors
 * @defgroup SBLIB_SENSOR_SHT1x SHT1x Temperature/humidity sensor
 * @ingroup SBLIB_SENSOR
 * @brief   Implementation for the usage of the Sensirion SHT1x temperature and humidity sensor
 *
 * @{
 *
 * @file   SHT1x.cpp
 * @bug No known bugs.
 ******************************************************************************/

/*
 * SHT1x Library
 *
 * https://github.com/practicalarduino/SHT1x (04/01/2022) commit sha: be7042c3e3cb32c778b9c9ca270b3df57f769ea5
 * Copyright 2009 Jonathan Oxer <jon@oxer.com.au> / <www.practicalarduino.com>
 * Based on previous work by:
 *    Maurice Ribble: <www.glacialwanderer.com/hobbyrobotics/?p=5>
 *    Wayne ?: <ragingreality.blogspot.com/2008/01/ardunio-and-sht15.html>
 *
 * Manages communication with SHT1x series (SHT10, SHT11, SHT15)
 * temperature / humidity sensors from Sensirion (www.sensirion.com).
 */

#include "sblib/i2c/SHT1x.h"
#include "sblib/digital_pin.h"
#include "sblib/timeout.h"
#include "sblib/utils.h"
#include "sblib/bits.h"

#include <cmath> // for logf


constexpr uint16_t WAIT_STEP_MS = 20;
constexpr uint16_t MAX_WAIT_MS = 340;
constexpr uint8_t RESET_MIN_CLOCK_TOGGLE_COUNT = 9;

/**
 * After power-up, the sensor needs 11 ms to get to Sleep State.
 * No commands must be sent before that time. (from SHT1x datasheet)
 */
constexpr uint8_t SensorPowerUpDelayMs = 11;


SHT1x::SHT1x(const uint32_t dataP, const uint32_t clockP) :
    dataPin(dataP),
    clockPin(clockP),
    lastTemperatureC(INVALID_TEMPERATURE),
    lastHumidity(INVALID_HUMIDITY),
    sensorStatus(0)
{
    initPins();
}

void SHT1x::initPins() const
{
    releaseDataPin();
    pinMode(clockPin, OUTPUT);
    digitalWrite(clockPin, false);
}

void SHT1x::activateDataPin() const
{
    digitalWrite(dataPin, true);
    pinMode(dataPin, OUTPUT | OPEN_DRAIN);
    digitalWrite(dataPin, true);
    __NOP();
}

void SHT1x::releaseDataPin() const
{
    digitalWrite(dataPin, true);
    pinMode(dataPin, INPUT);
    __NOP();
}

void SHT1x::connectionReset() const
{
    activateDataPin();
    for (uint8_t i = 0; i < RESET_MIN_CLOCK_TOGGLE_COUNT; i++)
    {
        clockCycle();
    }
    sendTransmissionStart();
}

void SHT1x::clockCycle() const
{
    digitalWrite(clockPin, true);
    __NOP();
    digitalWrite(clockPin, false);
}

float SHT1x::convertRawTemperature(const float& temperature, const eScale type)
{
    // Conversion coefficients D1 and D2 from SHT1x datasheet
    float D2;
    switch (type)
    {
        case CELCIUS:
            D2 = 0.01f; // for 14 Bit DEGC
            break;

        case FARENHEIT:
            D2 = 0.018f; // for 14 Bit DEGF
            break;
        default:
            D2 = 0.0f; // Don't use KELVIN or other unsupported scales
            fatalError();
            break;
    }
    constexpr float D1 = -39.7f; // @ 3.5V VDD (-39.6 @ 3.0V)
    return temperature * D2 + D1;
}

bool SHT1x::readTemperatureC(float* newTemperature)
{
    uint16_t val; // raw value returned from sensor
    if (!readTemperatureRaw(&val))
    {
        return false;
    }
    *newTemperature = convertRawTemperature(val, CELCIUS);
    lastTemperatureC = static_cast<int16_t>(*newTemperature * 100.f);
    return true;
}

bool SHT1x::readTemperatureF(float* newTemperature)
{
    uint16_t val; // raw value returned from sensor
    if (!readTemperatureRaw(&val))
    {
        return false;
    }
    *newTemperature = convertRawTemperature(val, CELCIUS); // stupid hack to store the last temp in C
    lastTemperatureC = static_cast<int16_t>(*newTemperature * 100.f);

    *newTemperature = convertRawTemperature(val, FARENHEIT);
    return true;
}

bool SHT1x::readHumidity(float* humidity)
{
    lastHumidity = INVALID_HUMIDITY;

    // Get current temperature for humidity correction
    float temperatureC;
    if (!readTemperatureC(&temperatureC))
    {
        return false;
    }

    constexpr auto measureRelativeHumidity = SHT1xCommand::measureRelativeHumidity;

    // Fetch the value from the sensor
    if (!sendCommandToSHT(measureRelativeHumidity))
    {
        return false;
    }

    if (!waitForResultSHT())
    {
        return false;
    }

    // Conversion coefficients from SHT1x datasheet
    constexpr float T1 = 0.01f;    // for 14 Bit @ 5V
    constexpr float T2 = 0.00008f; // for 14 Bit @ 5V

    constexpr float C1 = -2.0468f;    // for 12 Bit
    constexpr float C2 = 0.0367f;     // for 12 Bit
    constexpr float C3 = -1.5955E-6f; // for 12 Bit

    // Raw humidity value returned from the sensor
    uint16_t rawHumidity;
    if (!getData16SHT(measureRelativeHumidity, &rawHumidity))
    {
        return false;
    }
    const auto rawValue = static_cast<float>(rawHumidity);

    // Humidity with linear correction applied
    const float linearHumidity = C1 + C2 * rawValue + C3 * rawValue * rawValue;

    // Correct humidity value for current temperature
    *humidity = (temperatureC - 25.f) * (T1 + T2 * rawValue) + linearHumidity;

    // Check for valid humidity
    if (*humidity <= 0.0f)
    {
        return false;
    }

    // Values higher than 99% RH indicate fully saturated air and
    // must be processed and displayed as 100%RH13
    if (*humidity > 99.0f)
    {
        *humidity = 100.0f;
    }

    lastHumidity = static_cast<uint16_t>(*humidity * 100.f);
    return true;
}

bool SHT1x::readTemperatureRaw(uint16_t* temperatureRaw)
{
    lastTemperatureC = INVALID_TEMPERATURE;
    constexpr auto measureTemperature = SHT1xCommand::measureTemperature;
    if (!sendCommandToSHT(measureTemperature))
    {
        return false;
    }

    if (!waitForResultSHT())
    {
        return false;
    }

    if (!getData16SHT(measureTemperature, temperatureRaw))
    {
        return false;
    }

    return true;
}

void SHT1x::sendTransmissionStart() const
{
    // "idle" dataPin
    digitalWrite(dataPin, true);

    // Transmission Start sequence (datasheet 3.2 *Sending a command* (page 6):
    // 1) Lowering of the dataPin while clockPin is high
    digitalWrite(clockPin, true);
    __NOP();
    digitalWrite(dataPin, false);
    __NOP();

    // 2) followed by a low pulse on clockPin
    digitalWrite(clockPin, false);
    __NOP();
    digitalWrite(clockPin, true);
    __NOP();

    // 3) and raising dataPin again while clockPin is still high
    digitalWrite(dataPin, true);
    __NOP();

    // "idle" clockPin
    digitalWrite(clockPin, false);
    __NOP();
}

bool SHT1x::ackIsOK() const
{
    // Verify we get an ack (9th clock cycle)
    digitalWrite(clockPin, true);
    __NOP();
    const bool ackIsCorrect = !digitalRead(dataPin);
    digitalWrite(clockPin, false); // (end of 9th clock cycle)
    __NOP();
    return ackIsCorrect;
}

bool SHT1x::sendCommandToSHT(SHT1xCommand command) const
{
    // connectionReset();
    activateDataPin();
    sendTransmissionStart();

    shiftOut(dataPin, clockPin, MSBFIRST, static_cast<uint8_t>(command));
    releaseDataPin();
    return ackIsOK();
}

bool SHT1x::sendDataByteToSHT(const uint8_t dataByte) const
{
    activateDataPin();
    shiftOut(dataPin, clockPin, MSBFIRST, dataByte);
    releaseDataPin();

    if (!ackIsOK())
    {
        return false;
    }

    uint8_t i = 0;
    do
    {
        if (digitalRead(dataPin) == true)
        {
            return true;
        }
        // need to wait, it seems like DATA-pin needs ~8-100 ms to rise to high
        delayMicroseconds(MAX_DELAY_MICROSECONDS);
        i++;
    }
    while (i < 25);
    //serial.println("Ack Error 2, DATA not high");
    return false;
}

bool SHT1x::waitForResultSHT() const
{
    releaseDataPin();
    unsigned int i = 0;
    do
    {
        if (digitalRead(dataPin) == false)
        {
            // DATA line low -> measurement complete
            return true;
        }

        i++;
        delay(WAIT_STEP_MS);
    }
    while (i * WAIT_STEP_MS <= MAX_WAIT_MS);
    //serial.println("Error, wait for DATA low failed");
    return false;
}

uint8_t SHT1x::getByteSHT() const
{
    releaseDataPin();
    // Get the most significant bits
    const uint8_t rawByte = shiftIn(dataPin, clockPin, MSBFIRST);

    // Send the required ack
    activateDataPin();
    digitalWrite(dataPin, false);
    clockCycle();
    releaseDataPin();
    return rawByte;
}

bool SHT1x::getData8SHT(const SHT1xCommand command, uint8_t* data)
{
    const uint8_t rawValue = getByteSHT();
    uint8_t crc = getByteSHT();

    if (crc != crc8(command, getSensorStatus(), &rawValue, sizeof(rawValue)))
    {
        softReset(); // Reset connection on crc error
        return false;
    }

    *data = rawValue;
    return true;
}

bool SHT1x::getData16SHT(const SHT1xCommand command, uint16_t* data)
{

    uint8_t rawValue[2];
    rawValue[0] = getByteSHT(); // Get the most significant byte
    rawValue[1] = getByteSHT(); // Get the least significant byte
    uint8_t crc = getByteSHT();

    if (crc != crc8(command, getSensorStatus(), rawValue, sizeof(rawValue)))
    {
        softReset(); // Reset connection on crc error
        return false;
    }

    *data = makeWord(rawValue[0], rawValue[1]);
    return true;
}

 // ReSharper disable once CppMemberFunctionMayBeStatic
bool SHT1x::init()
{
    initPins();
    delay(SensorPowerUpDelayMs);
    return softReset();
}

uint16_t SHT1x::getHumidity()
{
    float humidity;
    if (!readHumidity(&humidity))
    {
        return INVALID_HUMIDITY;
    }
    return static_cast<uint16_t>(humidity * 100.f);
}

int16_t SHT1x::getTemperature()
{
    float temperature;
    if (!readTemperatureC(&temperature))
    {
        return INVALID_TEMPERATURE;
    }
    return static_cast<int16_t>(temperature * 100.f);
}

float SHT1x::getDewPoint()
{
    float humidity;
    if (!readHumidity(&humidity))
    {
        return INVALID_DEW_POINT;
    }

    float temperature;
    if (!readTemperatureC(&temperature))
    {
        return INVALID_DEW_POINT;
    }

    // Specify the constants for water vapor and barometric pressure.
    constexpr float WATER_VAPOR = 17.62f;
    constexpr float BAROMETRIC_PRESSURE = 243.5f;

    // Check for denominator for near-zero to avoid division by zero
    const float denominator = BAROMETRIC_PRESSURE + temperature;
    if (fabsf(denominator) < 0.001f)
    {
        return INVALID_DEW_POINT;
    }

    // Calculate the intermediate value 'gamma'
    const float gamma = logf(humidity / 100.f) + WATER_VAPOR * temperature / (BAROMETRIC_PRESSURE + temperature);
    // Calculate dew point in Celsius
    const float dewPoint = BAROMETRIC_PRESSURE * gamma / (WATER_VAPOR - gamma);

    return dewPoint;
}

bool SHT1x::getStatusRegister(uint8_t* status)
{
    constexpr auto readStatusRegister = SHT1xCommand::readStatusRegister;
    if (!sendCommandToSHT(readStatusRegister))
    {
        return false;
    }

    if (!getData8SHT(readStatusRegister, status))
    {
        return false;
    }

    setSensorStatus(*status);
    return true;
}

bool SHT1x::setStatusRegister(const uint8_t& newStatus)
{
    constexpr uint8_t ReadOnlyBitMask = 0b11111000;
    constexpr uint8_t UnSupportedFlags = 0b001; // ADC-Resolution change is not supported
    if (newStatus & ReadOnlyBitMask)
    {
        return false;
    }

    if (newStatus & UnSupportedFlags)
    {
        return false;
    }

    if (!sendCommandToSHT(SHT1xCommand::writeStatusRegister))
    {
        return false;
    }

    if (!sendDataByteToSHT(newStatus))
    {
        return false;
    }
    setSensorStatus(newStatus); // Update internal status for correct crc calculation

    uint8_t actualSensorStatus;
    if (!getStatusRegister(&actualSensorStatus))
    {
        return false;
    }

    return getSensorStatus() == newStatus;
}

void SHT1x::setSensorStatus(const uint8_t newValue)
{
    sensorStatus = newValue;
}

uint8_t SHT1x::getSensorStatus() const
{
    return sensorStatus;
}

bool SHT1x::softReset()
{
    connectionReset();

    if (!sendCommandToSHT(SHT1xCommand::softReset))
    {
        return false;
    }
    setSensorStatus(0);
    return true;
}

uint16_t SHT1x::getLastHumidity() const
{
    return lastHumidity;
}

int16_t SHT1x::getLastTemperature() const
{
    return lastTemperatureC;
}

void SHT1x::crcOnSingleByte(const uint8_t byteToProcess, uint8_t *crc)
{
    for (int8_t bit = 7; bit >= 0; bit--)
    {
        const uint8_t feedback = ((*crc >> 7) ^ ((byteToProcess >> bit) & 1)) & 1;
        *crc <<= 1;
        if (feedback)
        {
            *crc ^= 0x31;  // Polynomial: bit5, bit4, and bit0 (x8 + x5 + x4 + 1)
        }
    }
}

uint8_t SHT1x::crc8(const SHT1xCommand command, const uint8_t status, const uint8_t* data, const uint8_t length)
{
    // from Sensirion_AppNotes_Humidity_Sensors_SHT1x_SHT7x_CRC_Calculation.pdf
    // Step 1: Initialize CRC register to reversed low nibble of status register (s0s1s2s3 0000)
    uint8_t crc = (status & 0x01) << 7 |
                  (status & 0x02) << 5 |
                  (status & 0x04) << 3 |
                  (status & 0x08) << 1;

    // Process command byte (Steps 2 to 5)
    crcOnSingleByte(static_cast<uint8_t>(command), &crc);

    // Process data byte(s) (Steps 2 to 5)
    for (uint8_t i = 0; i < length; i++)
    {
        crcOnSingleByte(data[i], &crc);
    }

    // Step 6: Reverse crc
    crc = (crc & 0x01) << 7 |
          (crc & 0x02) << 5 |
          (crc & 0x04) << 3 |
          (crc & 0x08) << 1 |
          (crc & 0x10) >> 1 |
          (crc & 0x20) >> 3 |
          (crc & 0x40) >> 5 |
          (crc & 0x80) >> 7;
    return crc;
}
