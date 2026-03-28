/*
 *  digital_pin.cpp - Functions for digital I/O
 *
 *  Copyright (c) 2014 Stefan Taferner <stefan.taferner@gmx.at>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 3 as
 *  published by the Free Software Foundation.
 */
#include <sblib/digital_pin.h>
#include <sblib/platform.h>
#include <sblib/utils.h>

// The location value for the IO configuration of the RXD pin
static constexpr uint32_t rxPinLocation[] = {PIO1_6, PIO2_7, PIO3_1, PIO3_4};

// Find a pin in a pin location array. Fail if not found
static uint8_t pinLocation(const uint32_t pin, const uint32_t* arr, const uint8_t count)
{
    for (uint8_t idx = 0; idx < count; ++idx)
    {
        if (arr[idx] == pin)
            return idx;
    }

    fatalError(); // pin not found
    return -1;
}

void pinMode(const uint32_t pin, const uint32_t mode)
{
    LPC_GPIO_TypeDef* port = gpioPorts[digitalPinToPort(pin)];
    const uint32_t mask = digitalPinToBitMask(pin);
    const uint16_t type = mode & 0xf000;
    uint32_t iocon = mode & 0xfff;

    auto func = getPinFunction(mode);
    if (func == 0)
    {
        func = PF_PIO;
    }

    if (type == OUTPUT_MATCH)
    {
        func = PF_MAT;
    }
    else // INPUT modes
    {
        if (type == INPUT_CAPTURE)
        {
            func = PF_CAP;

            // Configure the location of the timer's capture pin
            if (pin == PIO0_2)
                LPC_IOCON->CT16B0_CAP0_LOC = 0;
            else if (pin == PIO3_3)
                LPC_IOCON->CT16B0_CAP0_LOC = 1;
            else if (pin == PIO1_5)
                LPC_IOCON->CT32B0_CAP0_LOC = 0;
            else if (pin == PIO2_9)
                LPC_IOCON->CT32B0_CAP0_LOC = 1;
        }
        else if (type == INPUT_ANALOG)
        {
            func = PF_AD;
        }
        else if (func == PF_RXD)
        {
            // Configure the location of the RXD pin
            LPC_IOCON->RXD_LOC = pinLocation(pin, rxPinLocation, 4);
        }
    }

    if (func == PF_MISO)
    {
        // Configure the location of the MISO pin
        if (pin == PIO2_2)
            LPC_IOCON->MISO1_LOC = 0;
        else if (pin == PIO1_10)
            LPC_IOCON->MISO1_LOC = 1;
    }
    else if (func == PF_MOSI)
    {
        // Configure the location of the MOSI pin
        if (pin == PIO2_3)
            LPC_IOCON->MOSI1_LOC = 0;
        else if (pin == PIO1_9)
            LPC_IOCON->MOSI1_LOC = 1;
    }
    else if (func == PF_SCK)
    {
        // Configure the location of the SCK pin
        if (pin == PIO0_10)
            LPC_IOCON->SCK_LOC = 0;
        else if (pin == PIO2_11)
            LPC_IOCON->SCK_LOC = 1;
        else if (pin == PIO0_6)
            LPC_IOCON->SCK_LOC = 2;
        else if (pin == PIO2_1)
            LPC_IOCON->SCK1_LOC = 0;
        else if (pin == PIO3_2)
            LPC_IOCON->SCK1_LOC = 1;
    }
    else if (func == PF_SSEL)
    {
        // Configure the location of the SSEL pin
        if (pin == PIO2_2)
            LPC_IOCON->SSEL1_LOC = 0;
        else if (pin == PIO2_4)
            LPC_IOCON->SSEL1_LOC = 1;
    }

    if ((pin & PFL_ADMODE) && func != PF_AD)
    {
        iocon |= 0x80;
    }

    const int8_t funcNum = getPinFunctionNumber(pin, func);
    if (funcNum >= 0)
    {
        iocon |= funcNum;
    }
    else
    {
        fatalError(); // the pin does not have the desired function
    }

    // Write the IO configuration to the IOCON register
    *ioconPointer(static_cast<PortPin>(pin)) = iocon;

    // As last step, set pin direction after IOCON is configured
    const bool isOutput = (type == OUTPUT) || (type == OUTPUT_MATCH);
    if (isOutput)
    {
        port->DIR |= mask;
    }
    else
    {
        port->DIR &= ~mask;
    }
}

void pinDirection(const uint32_t pin, const uint32_t dir)
{
    LPC_GPIO_TypeDef* port = gpioPorts[digitalPinToPort(pin)];
    const uint32_t mask = digitalPinToBitMask(pin);

    if (dir == OUTPUT)
        port->DIR |= mask;
    else
        port->DIR &= ~mask;
}

void pinInterruptMode(const uint32_t pin, const uint16_t mode)
{
    LPC_GPIO_TypeDef* port = gpioPorts[digitalPinToPort(pin)];
    const uint32_t mask = digitalPinToBitMask(pin);

    /* Configure the pin as input */
    pinMode(pin, INPUT);

    /* Set the level/edge configuration */
    if (mode & 0x0100)
        port->IS |= mask;
    else
        port->IS &= ~mask;

    /* Set the both edge configuration */
    if (mode & 0x0010)
        port->IBE |= mask;
    else
        port->IBE &= ~mask;

    /* Set the edge/level type configuration */
    if (mode & 0x0001)
        port->IEV |= mask;
    else
        port->IEV &= ~mask;

    /* Enable the interrupt for this pin */
    if (mode & 0x1000)
        port->IE |= mask;
    else
        port->IE &= ~mask;
}
