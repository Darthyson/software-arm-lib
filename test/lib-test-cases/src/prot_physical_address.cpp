/*
 *  prot_parameter.cpp - Test case for programming the parameters
 *
 *  Copyright (c) 2014 Martin Glueck <martin@mangari.org>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 3 as
 *  published by the Free Software Foundation.
 */

#include "protocol.h"

typedef struct
{
    unsigned char  state;
    bool           connected;
    unsigned short ownAddress;
} ProtocolTestState;

static ProtocolTestState protoState[2];

#define VaS(s) ((ProtocolTestState *) (s))

static void tc_setup(void)
{
    bcu.setOwnAddress(0x11C9); // set own address to 1.1.102
    userRam.status ^= 0x81; // set the programming mode
}

static void connect(void * state, unsigned int param)
{
    VaS(state)->connected = true;
}

static void disconnect(void * state, unsigned int param)
{
    VaS(state)->connected = false;
}

static void phy_addr_changed(void * state, unsigned int param)
{
    VaS(state)->ownAddress = userEeprom.addrTab[0] << 8 | userEeprom.addrTab[1];
}

static void clearProgMode(void * state, unsigned int param)
{
    if (userRam.status & BCU_STATUS_PROG)
    {
        userRam.status   ^= 0x81; // clear the programming mode
        VaS(state)->state = userRam.status;
    }
}

static Telegram testCaseTelegrams[] =
{//Step     type   len var. StepFunction        bytes[23];
/*  1 */   {TEL_RX,  8, 1, NULL              , {0xB0, 0x00, 0x01, 0x00, 0x00, 0xE1, 0x01, 0x00}} //
/*  2 */ , {TEL_TX,  8, 0, NULL              , {0xB0, 0x11, 0xC9, 0x00, 0x00, 0xE1, 0x01, 0x40}} //
/*  3 */ , {TEL_RX,  7, 0, NULL              , {0xB0, 0x00, 0x01, 0x11, 0x12, 0x60, 0x80}} //   3
/*  4 */ , {TEL_RX,  8, 0, NULL              , {0xB0, 0x00, 0x01, 0x11, 0x12, 0x61, 0x43, 0x00}} //   4
/*  5 */ , {TEL_RX,  7, 0, NULL              , {0xB0, 0x00, 0x01, 0x11, 0x12, 0x60, 0x81}} //   5
/*  6 */ , {TEL_RX, 10, 0, phy_addr_changed  , {0xB0, 0x00, 0x01, 0x00, 0x00, 0xE3, 0x00, 0xC0, 0x11, 0x12}} //   6
/*  7 */ , {TEL_RX,  8, 1, NULL              , {0xB0, 0x00, 0x01, 0x00, 0x00, 0xE1, 0x01, 0x00}} //   7
/*  8 */ , {TEL_TX,  8, 0, NULL              , {0xB0, 0x11, 0x12, 0x00, 0x00, 0xE1, 0x01, 0x40}} //   8
/*  9 */ , {TEL_RX,  7, 0, connect           , {0xB0, 0x00, 0x01, 0x11, 0x12, 0x60, 0x80}} //  9
/* 10 */ , {TEL_RX,  8, 2, NULL              , {0xB0, 0x00, 0x01, 0x11, 0x12, 0x61, 0x43, 0x00}} //  10
/* 11 */ , {TEL_TX,  7, 0, NULL              , {0xB0, 0x11, 0x12, 0x00, 0x01, 0x60, 0xC2}} //  11
/* 12 */ , {TEL_TX, 10, 0, NULL              , {0xB0, 0x11, 0x12, 0x00, 0x01, 0x63, 0x43, 0x40, 0x00, 0x12}} //  12
/* 13 */ , {TEL_RX,  7, 0, NULL              , {0xB0, 0x00, 0x01, 0x11, 0x12, 0x60, 0xC2}} //  13
/* 14 */ , {TEL_RX,  8, 0, clearProgMode     , {0xB0, 0x00, 0x01, 0x11, 0x12, 0x61, 0x47, 0x80}} //  14
/* 15 */ , {TEL_RX,  7, 0, disconnect        , {0xB0, 0x00, 0x01, 0x11, 0x12, 0x60, 0x81}} //  15
/* 16 */ , {TEL_RX,  8, 0, NULL              , {0xB0, 0x00, 0x01, 0x00, 0x00, 0xE1, 0x01, 0x00}} //  16
/* 17 */ , {END}
};

static void gatherProtocolState(ProtocolTestState * state, ProtocolTestState * refState)
{
    state->state      = userRam.status;
    state->connected  = bcu.directConnection();
    state->ownAddress = userEeprom.addrTab[0] << 8 | userEeprom.addrTab[1];

    if(refState)
    {
        REQUIRE(state->state      == refState->state);
        REQUIRE(state->ownAddress == refState->ownAddress);
        REQUIRE(state->connected  == refState->connected);
    }
}

static Test_Case testCase =
{
  "Phy Addr Prog"
, 0x0004, 0x2060, 0x01
, 0
, NULL
, tc_setup
, (StateFunction *) gatherProtocolState
, (TestCaseState *) &protoState[0]
, (TestCaseState *) &protoState[1]
, testCaseTelegrams
};

TEST_CASE("Programming of the physical address", "[protocol][address]")
{
    executeTest(& testCase);
}


