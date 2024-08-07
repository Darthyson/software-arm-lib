/**************************************************************************//**
 * @addtogroup SBLIB_EXAMPLES Selfbus library usage examples
 * @defgroup SBLIB_EXAMPLE_SPI_1 SPI example
 * @ingroup SBLIB_EXAMPLES
 * @brief   Configures SPI for output and sends a byte every second.
 * @details A simple example for SPI. This example configures SPI for output and sends a byte every second.
 *          We use SPI port 0 in this example.<br/>
 *          Pinout:
 *              - PIO0_2:  SSEL0
 *              - PIO0_9:  MOSI0
 *              - PIO2_11: SCK0
 *
 * @{
 *
 * @file   app_main.cpp
 * @author Stefan Taferner <stefan.taferner@gmx.at> Copyright (c) 2014
 * @author Darthyson <darth@maptrack.de> Copyright (c) 2022
 * @bug No known bugs.
 ******************************************************************************/

/*
 This program is free software; you can redistribute it and/or modify
 it under the terms of the GNU General Public License version 3 as
 published by the Free Software Foundation.
 ---------------------------------------------------------------------------*/

#include <sblib/eibBCU1.h>
#include <sblib/ioports.h>
#include <sblib/io_pin_names.h>
#include <sblib/spi.h>

SPI spi(SPI_PORT_0);

// #define BLINK_PIN PIN_IO2
#define BLINK_PIN PIN_INFO

BCU1 bcu = BCU1();

/**
 * Initialize the application.
 */
BcuBase* setup()
{
    pinMode(BLINK_PIN,  OUTPUT);

    pinMode(PIO0_2,  OUTPUT | SPI_SSEL);
    pinMode(PIO0_9,  OUTPUT | SPI_MOSI);
    pinMode(PIO2_11, OUTPUT | SPI_CLOCK);

    spi.setClockDivider(128);
    spi.begin();
    return (&bcu);
}

/**
 * The main processing loop while no KNX-application is loaded.
 */
void loop_noapp()
{
    static int val = 0;

    val++;
    val &= 255;

    digitalWrite(BLINK_PIN, true);
    spi.transfer(val);
    delay(200);

    digitalWrite(BLINK_PIN, false);
    delay(800);
}

/**
 * The main processing loop.
 */
void loop()
{
    // will never be called in this example
}
/** @}*/
