/**************************************************************************//**
 * @addtogroup SBLIB_EXAMPLES Selfbus library usage examples
 * @defgroup SBLIB_EXAMPLE_SERIAL_SOFT_UART_TEST Serial software UART speed tests example
 * @ingroup SBLIB_EXAMPLES
 * @brief   Shows the use of the serial software UART.
 * @details This example contains very basis performance tests for the software UART,
 *          e.g. burst sending, periodic sending and echoing with and without a buffer.
 *
 * @{
 * @author Darthyson <darth@maptrack.de> Copyright (c) 2026
 ******************************************************************************/

/*
 This program is free software; you can redistribute it and/or modify
 it under the terms of the GNU General Public License version 3 as
 published by the Free Software Foundation.
 ---------------------------------------------------------------------------*/

#include <sblib/eibBCU1.h>
#include <sblib/soft_uart.h>
#include <sblib/timer.h>

/** The Baudrate to use for the software UART */
constexpr auto baudRate = SoftUART::BaudRate::Baud19200;

/** The serial Rx Pin to use */
constexpr uint32_t rxPin = PIO1_6; // @ swd/jtag connector

/** The serial Tx Pin to use */
constexpr uint32_t txPin = PIO1_7; // @ swd/jtag connector

//constexpr uint32_t rxPin = PIO2_7); // @ 4TE-ARM Controller pin 1 on connector SV3 (ID_SEL)
//constexpr uint32_t txPin = PIO2_8); // @ 4TE-ARM Controller pin 2 on connector SV3 (ID_SEL)

auto  bcu = BCU1();
SoftUART softUART(rxPin, txPin, timer16_0, baudRate, SystemCoreClock);

// User-provided Timer ISR handler
extern "C" void TIMER16_0_IRQHandler()
{
    softUART.timerInterruptHandler();
}

// User-provided PIO1 ISR handler
extern "C" void PIOINT1_IRQHandler()
{
    softUART.handleGpioInterrupt();
}

// forward declaration for usage in setup()
void sendBurst(uint32_t loopCount);

/**
 * @def SEND_BURST
 * @brief Sends a burst of characters in a loop.
 * @note Tested from Baud9600 up to Baud115200 in release build, Baud230400 was failing
 */
//#define SEND_BURST

/**
 * @def SEND_PERIODIC
 * @brief Sends a bulk of characters periodically.
 * @note Tested with Baud115200 in release build, Baud230400 was failing
 */
//#define SEND_PERIODIC

/**
 * @def ECHO_LOOP_BUFFER_READ_WRITE
 * @brief Echos back received bytes using a buffer.
 * @note Tested with Baud115200 in release build, Baud230400 was failing
 */
#define ECHO_LOOP_BUFFER_READ_WRITE

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
    // Manufacturer ID     : MDT
    // Device Type         : Binary input 16f
    // Application Version : 2.0 ?
    bcu.begin(0x83, 0x0030, 0x20);
    softUART.begin();
#ifdef SEND_BURST
    sendBurst(5000);
#endif
    softUART.println("Selfbus serial software UART speedtests example");
    return &bcu;
}

/**
 * @brief Sends a burst of characters in a loop.
 *
 * @param loopCount The number of iterations for the burst.
 * @note Should work fine up to SoftUART::BaudRate:: ///\todo benchmark the max. stable baudrate
 */
void sendBurst(const uint32_t loopCount)
{
    uint32_t count = 0;
    for (uint32_t j = 0; j < loopCount; j++)
    {
        count += softUART.print(j, DEC, 4);
        count += softUART.print(" ");
        for (auto i = 'A'; i <= 'z'; i++)
        {
            count += softUART.print(i);
        }
        count += softUART.println();
    }
    softUART.println("count: ", count);
}

/**
 * @brief Sends a bulk of characters periodically.
 *
 * @note Should work fine up to SoftUART::BaudRate:: ///\todo benchmark the max. stable baudrate
 */
void sendPeriodic()
{
    static uint32_t lastSysTick = 0;
    static char lastSend = 'A';
    if (elapsed(lastSysTick) >= 10)
    {
        softUART.write(lastSend);
        lastSend++;
        if (lastSend > 'z')
        {
            softUART.println();
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
 * @note Should work fine up to SoftUART::BaudRate::Baud ///\todo benchmark the max. stable baudrate
 */
void echoLoopBuffer()
{
    constexpr uint16_t bufferLength = 512;
    static uint8_t buffer[bufferLength] = {};
    // new variant reading into buffer
    uint32_t count = 0;
    while(true)
    {
        const int16_t readByte = softUART.read();
        if (readByte < 0)
        {
            break;
        }
        buffer[count] = static_cast<uint8_t>(readByte);
        count++;
    }

    totalBytesRead += count;
    totalBytesWrite += softUART.write(buffer, count);
}

/**
 * @brief Echos back received bytes one by one.
 *
 * @note Should work fine up to SoftUART::BaudRate::Baud ///\todo benchmark the max. stable baudrate
 */
void echoLoopSingleByteReadWrite()
{
    // old style single byte reading
    int16_t serialByte = softUART.read();
    while (serialByte != -1)
    {
        totalBytesWrite += softUART.write(serialByte);
//        softUART.print(serialByte, HEX, 2);
        serialByte = softUART.read();
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
