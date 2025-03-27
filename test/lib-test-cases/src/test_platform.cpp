/*
 * Tests for the platform.h
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 3 as
 * published by the Free Software Foundation.
 */

#include <catch.hpp>
#include <sblib/platform.h>
#include <sblib/ioports.h>

TEST_CASE("gpioPorts[4]", "[platform]")
{
    REQUIRE(gpioPorts[0] == LPC_GPIO0);
    REQUIRE(gpioPorts[1] == LPC_GPIO1);
    REQUIRE(gpioPorts[2] == LPC_GPIO2);
    REQUIRE(gpioPorts[3] == LPC_GPIO3);
}

TEST_CASE("LPC_FLASH_BASE", "[platform]")
{
    REQUIRE(&LPC_FLASH_BASE[0] == &FLASH[0]);
}

TEST_CASE("FLASH_BASE_ADDRESS", "[platform]")
{
    REQUIRE(FLASH_BASE_ADDRESS == LPC_FLASH_BASE);
}

TEST_CASE("FLASH_SECTOR_SIZE", "[platform]")
{
    REQUIRE(FLASH_SECTOR_SIZE == 0x1000);
}

TEST_CASE("FLASH_PAGE_SIZE", "[platform]")
{
    REQUIRE(FLASH_PAGE_SIZE == 0x100);
}

TEST_CASE("FLASH_PAGE_ALIGNMENT", "[platform]")
{
    REQUIRE(FLASH_PAGE_ALIGNMENT == 0xff);
}

TEST_CASE("FLASH_RAM_BUFFER_ALIGNMENT", "[platform]")
{
    REQUIRE(FLASH_RAM_BUFFER_ALIGNMENT == 4);
}

TEST_CASE("ioconPointer(pin)", "[platform]")
{
#warning "TODO fix sblib and enable ioconPointer tests"
    ///\todo All below tests with nullptr are failing with current sblib
    SECTION("Invalid pin")
    {
        // CHECK(ioconPointer(0) == nullptr);
        // CHECK(ioconPointer(0xffff) == nullptr);
    }

    SECTION("Port 0")
    {
        REQUIRE(ioconPointer(PIO0_0) == &_LPC_IOCON.RESET_PIO0_0);
        REQUIRE(ioconPointer(PIO0_1) == &_LPC_IOCON.PIO0_1);
        REQUIRE(ioconPointer(PIO0_2) == &_LPC_IOCON.PIO0_2);
        REQUIRE(ioconPointer(PIO0_3) == &_LPC_IOCON.PIO0_3);
        REQUIRE(ioconPointer(PIO0_4) == &_LPC_IOCON.PIO0_4);
        REQUIRE(ioconPointer(PIO0_5) == &_LPC_IOCON.PIO0_5);
        REQUIRE(ioconPointer(PIO0_6) == &_LPC_IOCON.PIO0_6);
        REQUIRE(ioconPointer(PIO0_7) == &_LPC_IOCON.PIO0_7);
        REQUIRE(ioconPointer(PIO0_8) == &_LPC_IOCON.PIO0_8);
        REQUIRE(ioconPointer(PIO0_9) == &_LPC_IOCON.PIO0_9);
        REQUIRE(ioconPointer(PIO0_10) == &_LPC_IOCON.SWCLK_PIO0_10);
        REQUIRE(ioconPointer(PIO0_11) == &_LPC_IOCON.R_PIO0_11);
    }

    SECTION("Port 1")
    {
        REQUIRE(ioconPointer(PIO1_0) == &_LPC_IOCON.R_PIO1_0);
        REQUIRE(ioconPointer(PIO1_1) == &_LPC_IOCON.R_PIO1_1);
        REQUIRE(ioconPointer(PIO1_2) == &_LPC_IOCON.R_PIO1_2);
        REQUIRE(ioconPointer(PIO1_3) == &_LPC_IOCON.SWDIO_PIO1_3);
        REQUIRE(ioconPointer(PIO1_4) == &_LPC_IOCON.PIO1_4);
        REQUIRE(ioconPointer(PIO1_5) == &_LPC_IOCON.PIO1_5);
        REQUIRE(ioconPointer(PIO1_6) == &_LPC_IOCON.PIO1_6);
        REQUIRE(ioconPointer(PIO1_7) == &_LPC_IOCON.PIO1_7);
        REQUIRE(ioconPointer(PIO1_8) == &_LPC_IOCON.PIO1_8);
        REQUIRE(ioconPointer(PIO1_9) == &_LPC_IOCON.PIO1_9);
        REQUIRE(ioconPointer(PIO1_10) == &_LPC_IOCON.PIO1_10);
        REQUIRE(ioconPointer(PIO1_11) == &_LPC_IOCON.PIO1_11);
    }

    SECTION("Port 2")
    {
        REQUIRE(ioconPointer(PIO2_0) == &_LPC_IOCON.PIO2_0);
        REQUIRE(ioconPointer(PIO2_1) == &_LPC_IOCON.PIO2_1);
        REQUIRE(ioconPointer(PIO2_2) == &_LPC_IOCON.PIO2_2);
        REQUIRE(ioconPointer(PIO2_3) == &_LPC_IOCON.PIO2_3);
        REQUIRE(ioconPointer(PIO2_4) == &_LPC_IOCON.PIO2_4);
        REQUIRE(ioconPointer(PIO2_5) == &_LPC_IOCON.PIO2_5);
        REQUIRE(ioconPointer(PIO2_6) == &_LPC_IOCON.PIO2_6);
        REQUIRE(ioconPointer(PIO2_7) == &_LPC_IOCON.PIO2_7);
        REQUIRE(ioconPointer(PIO2_8) == &_LPC_IOCON.PIO2_8);
        REQUIRE(ioconPointer(PIO2_9) == &_LPC_IOCON.PIO2_9);
        REQUIRE(ioconPointer(PIO2_10) == &_LPC_IOCON.PIO2_10);
        REQUIRE(ioconPointer(PIO2_11) == &_LPC_IOCON.PIO2_11);
    }

    SECTION("Port 3")
    {
        REQUIRE(ioconPointer(PIO3_0) == &_LPC_IOCON.PIO3_0);
        REQUIRE(ioconPointer(PIO3_1) == &_LPC_IOCON.PIO3_1);
        REQUIRE(ioconPointer(PIO3_2) == &_LPC_IOCON.PIO3_2);
        REQUIRE(ioconPointer(PIO3_3) == &_LPC_IOCON.PIO3_3);
        REQUIRE(ioconPointer(PIO3_4) == &_LPC_IOCON.PIO3_4);
        REQUIRE(ioconPointer(PIO3_5) == &_LPC_IOCON.PIO3_5);
    }
}

TEST_CASE("ioconPointer(port, pinNum)", "[platform]")
{
#warning "TODO fix sblib and enable remaining ioconPointer tests"
    ///\todo All below tests with nullptr are failing with sblib 2.10
    SECTION("Port 0")
    {
        REQUIRE(ioconPointer(PIO0, 0) == &_LPC_IOCON.RESET_PIO0_0);
        REQUIRE(ioconPointer(PIO0, 1) == &_LPC_IOCON.PIO0_1);
        REQUIRE(ioconPointer(PIO0, 2) == &_LPC_IOCON.PIO0_2);
        REQUIRE(ioconPointer(PIO0, 3) == &_LPC_IOCON.PIO0_3);
        REQUIRE(ioconPointer(PIO0, 4) == &_LPC_IOCON.PIO0_4);
        REQUIRE(ioconPointer(PIO0, 5) == &_LPC_IOCON.PIO0_5);
        REQUIRE(ioconPointer(PIO0, 6) == &_LPC_IOCON.PIO0_6);
        REQUIRE(ioconPointer(PIO0, 7) == &_LPC_IOCON.PIO0_7);
        REQUIRE(ioconPointer(PIO0, 8) == &_LPC_IOCON.PIO0_8);
        REQUIRE(ioconPointer(PIO0, 9) == &_LPC_IOCON.PIO0_9);
        REQUIRE(ioconPointer(PIO0, 10) == &_LPC_IOCON.SWCLK_PIO0_10);
        REQUIRE(ioconPointer(PIO0, 11) == &_LPC_IOCON.R_PIO0_11);
        // CHECK(ioconPointer(PIO0, 12) == nullptr);
    }

    SECTION("Port 1")
    {
        REQUIRE(ioconPointer(PIO1, 0) == &_LPC_IOCON.R_PIO1_0);
        REQUIRE(ioconPointer(PIO1, 1) == &_LPC_IOCON.R_PIO1_1);
        REQUIRE(ioconPointer(PIO1, 2) == &_LPC_IOCON.R_PIO1_2);
        REQUIRE(ioconPointer(PIO1, 3) == &_LPC_IOCON.SWDIO_PIO1_3);
        REQUIRE(ioconPointer(PIO1, 4) == &_LPC_IOCON.PIO1_4);
        REQUIRE(ioconPointer(PIO1, 5) == &_LPC_IOCON.PIO1_5);
        REQUIRE(ioconPointer(PIO1, 6) == &_LPC_IOCON.PIO1_6);
        REQUIRE(ioconPointer(PIO1, 7) == &_LPC_IOCON.PIO1_7);
        REQUIRE(ioconPointer(PIO1, 8) == &_LPC_IOCON.PIO1_8);
        REQUIRE(ioconPointer(PIO1, 9) == &_LPC_IOCON.PIO1_9);
        REQUIRE(ioconPointer(PIO1, 10) == &_LPC_IOCON.PIO1_10);
        REQUIRE(ioconPointer(PIO1, 11) == &_LPC_IOCON.PIO1_11);
        // CHECK(ioconPointer(PIO1, 12) == nullptr);
    }

    SECTION("Port 2")
    {
        REQUIRE(ioconPointer(PIO2, 0) == &_LPC_IOCON.PIO2_0);
        REQUIRE(ioconPointer(PIO2, 1) == &_LPC_IOCON.PIO2_1);
        REQUIRE(ioconPointer(PIO2, 2) == &_LPC_IOCON.PIO2_2);
        REQUIRE(ioconPointer(PIO2, 3) == &_LPC_IOCON.PIO2_3);
        REQUIRE(ioconPointer(PIO2, 4) == &_LPC_IOCON.PIO2_4);
        REQUIRE(ioconPointer(PIO2, 5) == &_LPC_IOCON.PIO2_5);
        REQUIRE(ioconPointer(PIO2, 6) == &_LPC_IOCON.PIO2_6);
        REQUIRE(ioconPointer(PIO2, 7) == &_LPC_IOCON.PIO2_7);
        REQUIRE(ioconPointer(PIO2, 8) == &_LPC_IOCON.PIO2_8);
        REQUIRE(ioconPointer(PIO2, 9) == &_LPC_IOCON.PIO2_9);
        REQUIRE(ioconPointer(PIO2, 10) == &_LPC_IOCON.PIO2_10);
        REQUIRE(ioconPointer(PIO2, 11) == &_LPC_IOCON.PIO2_11);
        // CHECK(ioconPointer(PIO2, 12) == nullptr);
    }

    SECTION("Port 3")
    {
        REQUIRE(ioconPointer(PIO3, 0) == &_LPC_IOCON.PIO3_0);
        REQUIRE(ioconPointer(PIO3, 1) == &_LPC_IOCON.PIO3_1);
        REQUIRE(ioconPointer(PIO3, 2) == &_LPC_IOCON.PIO3_2);
        REQUIRE(ioconPointer(PIO3, 3) == &_LPC_IOCON.PIO3_3);
        REQUIRE(ioconPointer(PIO3, 4) == &_LPC_IOCON.PIO3_4);
        REQUIRE(ioconPointer(PIO3, 5) == &_LPC_IOCON.PIO3_5);
    }

    SECTION("Invalid port 3 pins 6-12")
    {
        // CHECK(ioconPointer(PIO3, 6) == nullptr);
        // CHECK(ioconPointer(PIO3, 7) == nullptr);
        // CHECK(ioconPointer(PIO3, 8) == nullptr);
        // CHECK(ioconPointer(PIO3, 9) == nullptr);
        // CHECK(ioconPointer(PIO3, 10) == nullptr);
        // CHECK(ioconPointer(PIO3, 11) == nullptr);
        // CHECK(ioconPointer(PIO3, 12) == nullptr);
    }
}
