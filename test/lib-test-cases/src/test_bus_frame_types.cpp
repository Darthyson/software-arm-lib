/**************************************************************************//**
 * @addtogroup SBLIB_MAIN_GROUP Selfbus KNX-Library
 * @defgroup SBLIB_SUB_GROUP_TEST Bus frame type Unit Test
 * @ingroup SBLIB_MAIN_GROUP
 * @brief   Tests which received frames Bus::handleTelegram() passes to the upper layers
 * @details
 *
 *
 * @{
 *
 * @file   test_bus_frame_types.cpp
 * @bug No known bugs.
 ******************************************************************************/

/*
 This program is free software; you can redistribute it and/or modify
 it under the terms of the GNU General Public License version 3 as
 published by the Free Software Foundation.
 ---------------------------------------------------------------------------*/

#include <cstring>

// catch.hpp has to be included before private and protected are redefined, see protocol.h
#include <catch.hpp>

#define private   public
#define protected public
#   include <sblib/eib/bus.h>
#undef private
#undef protected

#include <sblib/eib/bcu_const.h>
#include <sblib/io_pin_names.h>
#include <sblib/timer.h>

/**
 * Callback providing the layer status of a BCU
 */
class TestCallbackBus : public CallbackBus
{
public:
    void finishedSendingTelegram([[maybe_unused]] bool successful) override {}
    [[nodiscard]] uint8_t getLayerStatus() const override { return layerStatus; }

    uint8_t layerStatus = BCU_STATUS_LINK_LAYER;
};

/**
 * Put a frame into the receive buffer of the bus, as the receive process of Bus::timerInterruptHandler() does.
 * The checksum byte is not calculated, Bus::handleTelegram() gets the checksum result as parameter.
 */
static void receiveFrame(Bus& bus, const uint8_t* frame, const uint16_t length)
{
    bus.telegramLen = 0;
    bus.rx_error = RX_OK;
    memcpy(bus.rx_telegram, frame, length);
    bus.nextByteIndex = length;
    bus.handleTelegram(true);
}

// group value write to 0/0/0 (broadcast) from 1.1.1, including the checksum byte (not calculated)
static const uint8_t standardFrame[] = {0xBC, 0x11, 0x01, 0x00, 0x00, 0xE1, 0x00, 0x80, 0x00};
static const uint8_t extendedFrame[] = {0x3C, 0xE0, 0x11, 0x01, 0x00, 0x00, 0x01, 0x00, 0x80, 0x00};
constexpr uint16_t standardFrameLength = sizeof(standardFrame) / sizeof(standardFrame[0]);
constexpr uint16_t extendedFrameLength = sizeof(extendedFrame) / sizeof(extendedFrame[0]);

TEST_CASE("Bus frame types with disabled transport layer","[SBLIB][KNX][BUS]")
{
    TestCallbackBus callback;
    callback.layerStatus = BCU_STATUS_LINK_LAYER;
    Bus bus(nullptr, timer16_1, PIN_EIB_RX, PIN_EIB_TX, CAP0, TIMER_MATCH_MAT0, &callback, 64);

    SECTION("standard frame is passed on and acknowledged")
    {
        receiveFrame(bus, standardFrame, standardFrameLength);
        REQUIRE(bus.telegramLen == standardFrameLength);
        REQUIRE(bus.sendAck == SB_BUS_ACK);
        REQUIRE(memcmp(bus.telegram, standardFrame, standardFrameLength) == 0);
    }

    SECTION("extended frame is passed on and acknowledged")
    {
        receiveFrame(bus, extendedFrame, extendedFrameLength);
        REQUIRE(bus.telegramLen == extendedFrameLength);
        REQUIRE(bus.sendAck == SB_BUS_ACK);
        REQUIRE(memcmp(bus.telegram, extendedFrame, extendedFrameLength) == 0);
    }

    SECTION("extended frame with wrong length byte is rejected")
    {
        uint8_t frame[extendedFrameLength];
        memcpy(frame, extendedFrame, extendedFrameLength);
        frame[6] = 2;
        receiveFrame(bus, frame, extendedFrameLength);
        REQUIRE(bus.telegramLen == 0);
        REQUIRE(bus.sendAck == 0);
        REQUIRE((bus.rx_error & RX_INVALID_TELEGRAM_ERROR) != 0);
    }

    SECTION("extended frame longer than the receive buffer is rejected")
    {
        // The receive process stops storing bytes at the end of the buffer, so only the
        // first bytes of a frame announcing 60 APDU bytes (69 bytes in total) arrive.
        uint8_t frame[64] = {};
        memcpy(frame, extendedFrame, extendedFrameLength);
        frame[6] = 60;
        receiveFrame(bus, frame, 64);
        REQUIRE(bus.telegramLen == 0);
        REQUIRE(bus.sendAck == 0);
    }
}

TEST_CASE("Bus frame types with enabled transport layer","[SBLIB][KNX][BUS]")
{
    TestCallbackBus callback;
    callback.layerStatus = BCU_STATUS_LINK_LAYER | BCU_STATUS_TRANSPORT_LAYER;
    Bus bus(nullptr, timer16_1, PIN_EIB_RX, PIN_EIB_TX, CAP0, TIMER_MATCH_MAT0, &callback);

    SECTION("standard frame is passed on and acknowledged")
    {
        receiveFrame(bus, standardFrame, standardFrameLength);
        REQUIRE(bus.telegramLen == standardFrameLength);
        REQUIRE(bus.sendAck == SB_BUS_ACK);
    }

    SECTION("standard frame with wrong length is rejected")
    {
        uint8_t frame[standardFrameLength];
        memcpy(frame, standardFrame, standardFrameLength);
        frame[5] = 0xE2;
        receiveFrame(bus, frame, standardFrameLength);
        REQUIRE(bus.telegramLen == 0);
        REQUIRE((bus.rx_error & RX_INVALID_TELEGRAM_ERROR) != 0);
    }

    SECTION("extended frame is rejected, the transport layer handles standard frames only")
    {
        receiveFrame(bus, extendedFrame, extendedFrameLength);
        REQUIRE(bus.telegramLen == 0);
        REQUIRE(bus.sendAck == 0);
        REQUIRE((bus.rx_error & RX_INVALID_TELEGRAM_ERROR) != 0);
    }
}

TEST_CASE("Bus sets the sender address of extended frames","[SBLIB][KNX][BUS]")
{
    TestCallbackBus callback;
    Bus bus(nullptr, timer16_1, PIN_EIB_RX, PIN_EIB_TX, CAP0, TIMER_MATCH_MAT0, &callback, 64);
    bus.ownAddress = 0x1234;

    uint8_t frame[extendedFrameLength];
    memcpy(frame, extendedFrame, extendedFrameLength);
    bus.prepareTelegram(frame, extendedFrameLength - 1);

    REQUIRE(frame[1] == 0xE0); // extended control field unchanged
    REQUIRE(frame[2] == 0x12);
    REQUIRE(frame[3] == 0x34);
    REQUIRE(frame[4] == 0x00); // destination address unchanged

    uint8_t checksum = 0xFF;
    for (uint16_t i = 0; i < extendedFrameLength - 1; i++)
    {
        checksum ^= frame[i];
    }
    REQUIRE(frame[extendedFrameLength - 1] == checksum);
}

/** @}*/
