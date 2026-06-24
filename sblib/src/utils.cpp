/*
 *  utils.cpp - Utility functions.
 *
 *  Copyright (c) 2014 Stefan Taferner <stefan.taferner@gmx.at>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 3 as
 *  published by the Free Software Foundation.
 */

#include <sblib/utils.h>

#include <sblib/digital_pin.h>
#include <sblib/platform.h>
#include <sblib/io_pin_names.h>
#include <cstring>

static int fatalErrorPin = PIN_PROG;
static int eibTxPin = PIN_EIB_TX; ///\todo make it universal

void reverseCopy(uint8_t* dest, const uint8_t* src, int len)
{
    src += len - 1;
    while (len > 0)
    {
        *dest++ = *src--;
        --len;
    }
}

void setPinsInSecureModes()
{
    pinMode(fatalErrorPin, OUTPUT);
    pinMode(eibTxPin, INPUT);
}

void fatalError()
{
    // We use only low level functions here as a fatalError() could happen
    // anywhere in the lib and we want to ensure that the function works
    SysTick_Config(0x1000000);
    setPinsInSecureModes();
    while (1)
    {
        // Blink the fatalErrorLED
        digitalWrite(fatalErrorPin, (SysTick->VAL & 0x400000) == 0 ? 1 : 0);
    }
}

void HardFault_Handler()
{
    // We use only low level functions here as a HardFault could happen
    // anywhere and we want to ensure that the function works
    SysTick_Config(0x1000000);
    setPinsInSecureModes();
    while (1)
    {
        // Blink the fatalErrorLED
        digitalWrite(fatalErrorPin, (SysTick->VAL & 0x200000) == 0 ? 1 : 0);
    }
}

void setFatalErrorPin(const int newPin)
{
    fatalErrorPin = newPin;
}

void setKNX_TX_Pin(const int newTxPin)
{
    eibTxPin = newTxPin;
}

