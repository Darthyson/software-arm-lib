/*
 *  serial.cpp - Serial port access. This file defines the default serial port.
 *
 *  Copyright (c) 2015 Stefan Taferner <stefan.taferner@gmx.at>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 3 as
 *  published by the Free Software Foundation.
 */

#include "sblib/serial.h"
#include "sblib/digital_pin.h"

// ReSharper disable once CppUnusedIncludeDirective
#include "sblib/libconfig.h"


#if !defined(SERIAL_TX_PIN)
#   define SERIAL_TX_PIN PIO1_7     //!> Default serial Tx-Pin PIO1.7 (LPC11xx)
#endif
#if !defined(SERIAL_RX_PIN)
#   define SERIAL_RX_PIN PIO1_6     //!> Default serial Rx-Pin PIO1.6 (LPC11xx)
#endif

Serial serial(SERIAL_RX_PIN, SERIAL_TX_PIN);

extern "C" void UART_IRQHandler()
{
    serial.uartInterruptHandler();
}
