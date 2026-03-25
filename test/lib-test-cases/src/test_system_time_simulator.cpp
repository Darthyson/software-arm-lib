/*
 *  Unit tests for SystemTimeSimulator class
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 3 as
 *  published by the Free Software Foundation.
 */

#include "system_time_simulator.h"
#include <sblib/timer.h>

#include <catch.hpp>


TEST_CASE("SystemTimeSimulator::", "[system_time_simulator]") {
    SECTION("Simulate time increments as expected")
    {
        setMillis(0);
        SystemTimeSimulator sim(1);  // 1ns tick interval
        sim.start();
        delay(50);  // wait till 50ns elapsed
        REQUIRE(millis() >= 50);
        sim.stop();
    }
}