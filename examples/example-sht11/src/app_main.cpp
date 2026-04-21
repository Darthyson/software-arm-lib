/**************************************************************************//**
 * @addtogroup SBLIB_EXAMPLES Selfbus library usage examples
 * @defgroup SBLIB_EXAMPLE_SHT1x temperature/humidity sensor SHT1x example
 * @ingroup SBLIB_EXAMPLES
 * @brief   A simple application which shows the usage of the SHT1x class.
 * @details The mask version of this example is 0x0701.
 *
 * @{
 *
 * @file   app_main.cpp
 * @author Darthyson <darth@maptrack.de> Copyright (c) 2022
 * @bug No known bugs.
 ******************************************************************************/

/*
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 3 as
 *  published by the Free Software Foundation.
 */

#include <sblib/eibMASK0701.h>
#include <sblib/i2c/SHT1x.h>
#include <sblib/io_pin_names.h>
#include <sblib/serial.h>
#include <sblib/timeout.h>

MASK0701 bcu;
//   TS_ARM: IO3   , IO2
SHT1x sensor(PIO0_9, PIO2_2); // don't use i2c (SDA, SCL) pins. They have worse edge steepness.

Timeout readTimeout;
#define READ_TIMER_MS (500)

constexpr uint8_t SensorStatusToSet = 0b00000000; // heater off
//constexpr uint8_t SensorStatusToSet = 0b00000100; // heater on
//constexpr uint8_t SensorStatusToSet = 0b00000111; // heater on, reload from OTP, 8bit RH / 12bit Temp

//#define SOFTRESET_SHT1x


/**
 * This function is called by the Selfbus's library main
 * when the processor is started or reset.
 *
 * @note  You must implement this function in your code.
 */
BcuBase* setup()
{
    serial.setTxPin(PIO1_7);
    serial.setRxPin(PIO1_6);
    serial.begin(SERIAL_BAUD_RATE_115200);
    bcu.begin(0,0,0);
    serial.println("example-sht11 started.");
    serial.flush();

    serial.print("SHT1x sensor ");
    if (sensor.init())
    {
        serial.println("initialized.");
    }
    else
    {
        serial.println("initialization FAILED!");
    }
    serial.flush();

    readTimeout.start(READ_TIMER_MS);
    return &bcu;
}

/**
 * The main processing loop while a KNX-application is loaded.
 */
void loop()
{
    if (!readTimeout.expired())
    {
        return;
    }

    float dewPoint = sensor.getDewPoint(); // this triggers temperature and humidity measurement

    int16_t temperature = sensor.getLastTemperature();
    if (temperature != INVALID_TEMPERATURE)
    {
        serial.print(temperature / 100.f, 2);
        serial.print("C ");
    }
    else
    {
        serial.println("Reading temperature FAILED!");
    }

    uint16_t humidity = sensor.getLastHumidity();
    if (humidity != INVALID_HUMIDITY)
    {
        serial.print(humidity / 100.f, 2);
        serial.print("%rH ");
    }
    else
    {
        serial.println("Reading relative humidity FAILED!");
    }

    if (dewPoint > INVALID_DEW_POINT)
    {
        serial.print(dewPoint);
        serial.print("Td ");
    }
    else
    {
        serial.println("Reading dew point FAILED!");
    }

    float temperatureFahrenheit;
    if (sensor.readTemperatureF(&temperatureFahrenheit))
    {
        serial.print(temperatureFahrenheit, 2);
        serial.print("F ");
    }
    else
    {
        serial.println("Reading temperature in Fahrenheit FAILED!");
    }

    uint8_t statusReceived;
    if (sensor.getStatusRegister(&statusReceived))
    {
        serial.print("status 0b", statusReceived, BIN, 8);
    }
    else
    {
        serial.println("Reading status register FAILED!");
    }

    if (sensor.setStatusRegister(SensorStatusToSet))
    {
        serial.print(" status set to 0b", SensorStatusToSet, BIN, 8);
    }
    else
    {
        serial.println(" Writing status register FAILED!");
    }


#ifdef SOFTRESET_SHT1x
    if (sensor.softReset())
    {
        serial.print(" sensor reset successful");
    }
    else
    {
        serial.println(" softReset FAILED!");
    }
#endif

    serial.println();
    readTimeout.start(READ_TIMER_MS);
}

/**
 * The processing loop while no KNX-application is loaded.
 */
void loop_noapp()
{
    loop();
}

/** @}*/
