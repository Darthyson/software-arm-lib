/*
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 3 as
 *  published by the Free Software Foundation.
 */
#ifndef SB_LIB_SYSTEM_TIME_SIMULATOR_H_
#define SB_LIB_SYSTEM_TIME_SIMULATOR_H_

#include <atomic>
#include <thread>

/**
 *  Simulates systemTime incrementing in a background thread for unit tests.
 *
 *  Usage:
 *  @code
 *      SystemTimeSimulator sim(1);  // increment every 1 nanosecond (real time)
 *      sim.start();
 *      // ... code under test that uses millis() / delay() ...
 *      sim.stop();
 *  @endcode
 *
 *  @note Requires @ref IAP_EMULATION to be defined (for setMillis()).
 */
class SystemTimeSimulator
{
public:
    /**
     * Constructor for SystemTimeSimulator.
     *
     * @param intervalNanoSeconds  Real-time interval in nanoseconds between each systemTime increment.
     * @param initIncrement        How much to add to systemTime on each tick (default 1).
     */
    explicit SystemTimeSimulator(uint32_t intervalNanoSeconds = 1, uint32_t initIncrement = 1);

    ~SystemTimeSimulator();

    /// Start the background ticker thread.
    void start();

    /// Stop the background ticker thread.
    void stop();

    /// Check whether the simulator is running.
    bool isRunning() const;

    /// Change the increment value while running.
    void setIncrement(uint32_t newIncrement);

    /// Change the interval while running (takes effect on next tick).
    void setInterval(uint32_t nanoSeconds);

private:
    uint32_t intervalNano;
    uint32_t increment;
    std::atomic<bool> thRunning;
    std::thread workerThread;
};

#endif /* SB_LIB_SYSTEM_TIME_SIMULATOR_H_ */

