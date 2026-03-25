/*
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 3 as
 *  published by the Free Software Foundation.
 */

#include "system_time_simulator.h"
#include <sblib/timer.h>
#include <chrono>

SystemTimeSimulator::SystemTimeSimulator(const uint32_t intervalNanoSeconds, const uint32_t initIncrement)
        : intervalNano(intervalNanoSeconds),
          increment(initIncrement),
          thRunning(false)
{
}

SystemTimeSimulator::~SystemTimeSimulator()
{
    stop();
}

void SystemTimeSimulator::start()
{
    if (thRunning.load())
    {
        return;
    }

    thRunning.store(true);
    workerThread = std::thread([this]()
    {
        while (thRunning.load())
        {
            std::this_thread::sleep_for(std::chrono::nanoseconds(intervalNano));
            if (thRunning.load())
            {
                setMillis(millis() + increment);
            }
        }
    });
}

void SystemTimeSimulator::stop()
{
    thRunning.store(false);
    if (workerThread.joinable())
    {
        workerThread.join();
    }
}

bool SystemTimeSimulator::isRunning() const
{
    return thRunning.load();
}

void SystemTimeSimulator::setIncrement(const uint32_t newIncrement)
{
    increment = newIncrement;
}

void SystemTimeSimulator::setInterval(const uint32_t nanoSeconds)
{
    intervalNano = nanoSeconds;
}
