/**************************************************************************//**
 * @addtogroup SBLIB_EXAMPLES Selfbus library usage examples
 * @defgroup SBLIB_EXAMPLE_SERIAL_PORT_SPEED_TEST Serial port speed tests example
 * @ingroup SBLIB_EXAMPLES
 * @brief   Shows the use of the serial port.
 * @details This example application shows the use of the serial port.<br />
 *          Connect a terminal program to the ARM's serial port.<br />
 *          The serial port is used with 115200 baud, 8 data bits, no parity, 1 stop bit.<br />
 *          Tx-pin is PIO1.7, Rx-pin is PIO1.6<br />
 *
 * @{
 *
 * @file    app_main.cpp
 * @author Stefan Taferner <stefan.taferner@gmx.at> Copyright (c) 2014
 * @author Darthyson <darth@maptrack.de> Copyright (c) 2026
 * @bug No known bugs.
 ******************************************************************************/

/*
 This program is free software; you can redistribute it and/or modify
 it under the terms of the GNU General Public License version 3 as
 published by the Free Software Foundation.
 ---------------------------------------------------------------------------*/

#include <sblib/eibBCU1.h>
#include <sblib/serial.h>

constexpr uint32_t rxPin = PIO1_6; // @ swd/jtag connector
constexpr uint32_t txPin = PIO1_7; // @ swd/jtag connector

//constexpr uint32_t rxPin = PIO2_7); // @ 4TE-ARM Controller pin 1 on connector SV3 (ID_SEL)
//constexpr uint32_t txPin = PIO2_8); // @ 4TE-ARM Controller pin 2 on connector SV3 (ID_SEL)

constexpr auto  triggerLevel = RxTriggerLevel::CHAR_8;

auto  bcu = BCU1();

void sendBurst(uint32_t loopCount);

/**
 * @def SEND_BURST
 * @brief Sends a burst of characters in a loop.
 */
#define SEND_BURST

/**
 * @def SEND_PERIODIC
 * @brief Sends a bulk of characters periodically.
 */
//#define SEND_PERIODIC

/**
 * @def ECHO_LOOP_BUFFER_READ_WRITE
 * @brief Echos back received bytes using a buffer.
 */
//#define ECHO_LOOP_BUFFER_READ_WRITE

/**
 * @def ECHO_LOOP_SINGLE_READ_WRITE
 * @brief Echos back received bytes one by one.
 */
//#define ECHO_LOOP_SINGLE_READ_WRITE

/**
 * Initialize the application.
 */
BcuBase* setup()
{
    serial.setRxPin(rxPin);
    serial.setTxPin(txPin);

//    serial.begin(SERIAL_BAUD_RATE_9600, SERIAL_8N1, triggerLevel);
//    serial.begin(SERIAL_BAUD_RATE_38400, SERIAL_8N1, triggerLevel);
//    serial.begin(SERIAL_BAUD_RATE_115200, SERIAL_8N1, triggerLevel);
//    serial.begin(SERIAL_BAUD_RATE_576000, SERIAL_8N1, triggerLevel);
//    serial.begin(SERIAL_BAUD_RATE_661765, SERIAL_8N1, triggerLevel);
//    serial.begin(SERIAL_BAUD_RATE_1000000, SERIAL_8N1, triggerLevel); // sblib main lost bytes
//    serial.begin(SERIAL_BAUD_RATE_1500000, SERIAL_8N1, triggerLevel); // sblib main lost bytes, realterm null modem connection is data received ok
//    serial.begin(SERIAL_BAUD_RATE_576000, SERIAL_8N1); // perfectly fine with old lib v2.10
//    serial.begin(SERIAL_BAUD_RATE_661765, SERIAL_8N1); // perfectly fine with old lib v2.10
    serial.begin(SERIAL_BAUD_RATE_750000, SERIAL_8N1); // failed with old lib v2.10
//    serial.begin(SERIAL_BAUD_RATE_921600, SERIAL_8N1); // failed with old lib v2.10
//    serial.begin(SERIAL_BAUD_RATE_1000000, SERIAL_8N1); //failed with old lib v2.10
#ifdef SEND_BURST
    sendBurst(5000);// up to SERIAL_BAUD_RATE_1500000
#endif
    serial.println("Selfbus serial port speedtests example");
    return &bcu;
}

/**
 * @brief Sends a burst of characters in a loop.
 *
 * @param loopCount The number of iterations for the burst.
 * @note Should work fine up to SERIAL_BAUD_RATE_1500000
 */
void sendBurst(const uint32_t loopCount)
{
    uint32_t count = 0;
    for (uint32_t j = 0; j < loopCount; j++)
    {
        count += serial.print(j, DEC, 4);
        count += serial.print(" ");
        for (auto i = 'A'; i <= 'z'; i++)
        {
            count += serial.print(i);
        }
        count += serial.println();
    }
    serial.println("count: ", count);
}

/**
 * @brief Sends a bulk of characters periodically.
 *
 * @note Should work fine up to SERIAL_BAUD_RATE_1500000
 */
void sendPeriodic()
{
    static uint32_t lastSysTick = 0;
    static char lastSend = 'A';
    const uint32_t elapsedMs = elapsed(lastSysTick);
    if (elapsedMs >= 10)
    {
        serial.write(lastSend);
        lastSend++;
        if (lastSend > 'z')
        {
            serial.println();
            lastSend = 'A';
        }
        lastSysTick = millis();
    }
}

uint32_t totalBytesRead = 0;
uint32_t totalBytesWrite = 0;

/**
 * @brief Echos back received bytes using a buffer.
 *
 * @note Should work fine up to SERIAL_BAUD_RATE_1000000 in release build
 */
void echoLoopBuffer()
{
    constexpr uint16_t bufferLength = 512;
    static uint8_t buffer[bufferLength] = {};
    // new variant reading into buffer
    uint32_t count = 0;
    while(true)
    {
        const int16_t readByte = serial.read();
        if (readByte < 0)
        {
            break;
        }
        buffer[count] = static_cast<uint8_t>(readByte);
        count++;
    }

    totalBytesRead += count;
    totalBytesWrite += serial.write(buffer, count);
}

/**
 * @brief Echos back received bytes one by one.
 *
 * @note Should work fine up to SERIAL_BAUD_RATE_750000 in release build
 */
void echoLoopSingleByteReadWrite()
{
    // old style single byte reading
    int16_t serialByte = serial.read();
    while (serialByte != -1)
    {
        totalBytesWrite += serial.write(serialByte);
//        serial.print(serialByte, HEX, 2);
        serialByte = serial.read();
        totalBytesRead++;
    }
}

/**
 * The main processing loop while no KNX-application is loaded.
 */
void loop_noapp()
{
#ifdef SEND_PERIODIC
    sendPeriodic();
#endif

#ifdef ECHO_LOOP_BUFFER_READ_WRITE
    echoLoopBuffer();
#endif

#ifdef ECHO_LOOP_SINGLE_READ_WRITE
    echoLoopSingleByteReadWrite();
#endif
}

/**
 * The main processing loop.
 */
void loop()
{
    // will never be called in this example
    loop_noapp();
}
/** @}*/
