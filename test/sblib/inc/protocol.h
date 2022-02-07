/*
 *  protocol.h - Main include file for defining protocol related tests
 *
 *  Copyright (c) 2014 Martin Glueck <martin@mangari.org>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 3 as
 *  published by the Free Software Foundation.
 */


#ifndef PROTOCOL_H_
#define PROTOCOL_H_

#include "catch.hpp"

#define private   public
#define protected public
#include "sblib/eib/bus.h"
#include "sblib/eib/bcu.h"
#undef private
#undef protected
#include "iap_emu.h"

#include <string.h>
#include <stdio.h>

typedef void (TestCaseSetup) (void);

typedef void (StateFunction) (void * state, void * refState);
typedef void (StepFunction)  (void * state, unsigned int var);

typedef enum
{
  TEL_RX            //!> simulated telegram received from the bus
, TEL_TX            //!> simulated telegram to transmit to the bus
, TIMER_TICK        //!> simulated timer tick by increasing system time
, CHECK_TX_BUFFER
, LOOP              //!> simulates the bcu.loop()
, BREAK
, END               //!> test case end
} TelegramType;

typedef struct
{
    TelegramType     type;
    int              length;
    unsigned int     variable;
    StepFunction   * stepFunction;
    unsigned char    bytes[23];
} Telegram;

typedef struct
{
    unsigned int dummy;
} TestCaseState;

typedef struct
{
    const char            * name;
          int               manufacturer;
          int               deviceType;
          int               version;
          unsigned int      powerOnDelay;
          TestCaseSetup   * eepromSetup;
          TestCaseSetup   * setup;
          StateFunction   * gatherState;
          TestCaseState   * refState;
          TestCaseState   * stepState;
          Telegram        * telegram;
} Test_Case;

void executeTest(Test_Case * tc);

#endif /* PROTOCOL_H_ */
