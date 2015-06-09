/*
 *  This example application shows the use of the serial port.
 *  Connect a terminal program to the ARM's serial port:
 *  RXD to ... and TXD to ....
 *
 *  The example's port settings are 19200 baud, no parity, 1 stop bit.
 *
 *  Copyright (c) 2014 Stefan Taferner <stefan.taferner@gmx.at>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 3 as
 *  published by the Free Software Foundation.
 */

#include <sblib/core.h>
#include <sblib/serial.h>
#include <sblib/internal/iap.h>

/*
 * Initialize the application.
 */
void setup()
{
    serial.begin(19200);

    serial.println("Selfbus serial port example");

    serial.print("Target MCU has ");
    serial.print(iapFlashSize() / 1024);
    serial.println("k flash");
    serial.println();
}

/*
 * The main processing loop.
 */
void loop()
{
    static int value = 0;

    // Write some text to the serial port
    serial.print("Counter value: B");
    serial.println(++value, BIN, 8);

    delay(1000);
}
