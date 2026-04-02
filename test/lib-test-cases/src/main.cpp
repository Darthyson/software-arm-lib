/*
 *  Copyright (c) 2014 Martin Glueck <martin@mangari.org>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 3 as
 *  published by the Free Software Foundation.
 */

#define CATCH_CONFIG_MAIN

#ifdef INCLUDE_SERIAL
#    include <sblib/serial.h>
#endif

// ReSharper disable once CppUnusedIncludeDirective
#include <catch.hpp> // If possible, include catch.hpp as last header


/* The test framework expects that the application provides a setup function
 * since the lib tests don't have an application we need to provide an
 * empty setup function
 */
void setup()
{
    ///\todo check valid userRamData and userEepromData before we even start something REQUIRE(...);
}

void beforeTestStarts()
{
#ifdef INCLUDE_SERIAL
    // Clear the serial buffers before each test to avoid interference between tests.
    Serial::testClearSentBytes();
#endif
}

void afterTestFinished()
{
#ifdef INCLUDE_SERIAL
    Serial::testGetSentBytes();
#endif
}

