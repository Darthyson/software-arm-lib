/**************************************************************************//**
 * @addtogroup SBLIB_EXAMPLES Selfbus library usage examples
 * @defgroup SBLIB_EXAMPLE_I2C_TOF_VL53L1X i2c VL53L1X Time of Flight (ToF) sensor example
 * @ingroup SBLIB_EXAMPLES
 * @brief   Configures and reads VL53L1X range
 * @{
 ******************************************************************************/

/*
 This program is free software; you can redistribute it and/or modify
 it under the terms of the GNU General Public License version 3 as
 published by the Free Software Foundation.
 ---------------------------------------------------------------------------*/

#include <sblib/types.h>
#include <sblib/io_pin_names.h>
#include <sblib/eibBCU1.h>
#include <sblib/serial.h>
#include <sblib/i2c/tof/VL53L1X_api.h>
#include <sblib/i2c/tof/VL53L1X_calibration.h>
#include <sblib/i2c/tof/VL53Lx_i2c.h>

constexpr uint8_t VL53L1X_DEFAULT_I2C_ADDRESS = 0x29;
constexpr uint16_t VL53L1X_DEFAULT_DISTANCE_MODE = 1; // short distance = 1, long distance = 2
constexpr uint16_t VL53L1X_DEFAULT_TIMING_BUDGET_MS = 500;
constexpr uint16_t VL53L1X_DEFAULT_INTER_MEASUREMENT_TIMING_BUDGET = VL53L1X_DEFAULT_TIMING_BUDGET_MS;

BCU1 bcu = BCU1();

void failHardInDebug(VL53L1X_ERROR status)
{
#ifndef DEBUG
    return;
#endif

    if (status != VL53LX_ERROR_NONE)
    {
        fatalError();
    }
}

void calibrateOffset()
{
    int16_t measuredOffset;
    failHardInDebug(VL53L1X_CalibrateOffset(VL53L1X_DEFAULT_I2C_ADDRESS, 100, &measuredOffset));
    serial.println("calibrateOffset measuredOffset=", measuredOffset, DEC, 6);
}

/**
 * Initialize the application.
 */
BcuBase* setup()
{
    // LED Initialize
    pinMode(PIN_INFO, OUTPUT);
    digitalWrite(PIN_INFO, false);

    serial.setRxPin(PIN_RX);
    serial.setTxPin(PIN_TX);
    serial.setRxPin(PIO1_6);
    serial.setTxPin(PIO1_7);

    serial.begin(115200);
    serial.println("Selfbus I2C VL53L1X Time of Flight (ToF) sensor example");

    uint8_t bootState;
    VL53L1X_BootState(VL53L1X_DEFAULT_I2C_ADDRESS, &bootState);
    while (bootState == 0)
    {
        VL53L1X_BootState(VL53L1X_DEFAULT_I2C_ADDRESS, &bootState);
        delay(100);
    }

    failHardInDebug(VL53L1X_SensorInit(VL53L1X_DEFAULT_I2C_ADDRESS));
    failHardInDebug(VL53L1X_SetDistanceMode(VL53L1X_DEFAULT_I2C_ADDRESS, VL53L1X_DEFAULT_DISTANCE_MODE));
    failHardInDebug(VL53L1X_SetTimingBudgetInMs(VL53L1X_DEFAULT_I2C_ADDRESS, VL53L1X_DEFAULT_TIMING_BUDGET_MS));
    failHardInDebug(VL53L1X_SetInterMeasurementInMs(VL53L1X_DEFAULT_I2C_ADDRESS, VL53L1X_DEFAULT_INTER_MEASUREMENT_TIMING_BUDGET));
    //failHardInDebug(VL53L1X_SetSigmaThreshold(VL53L1X_DEFAULT_I2C_ADDRESS, 2));
    failHardInDebug(VL53L1X_ClearInterrupt(VL53L1X_DEFAULT_I2C_ADDRESS));
    //calibrateOffset();
    failHardInDebug(VL53L1X_StartRanging(VL53L1X_DEFAULT_I2C_ADDRESS));
    //failHardInDebug(VL53L1X_StartTemperatureUpdate(VL53L1X_DEFAULT_I2C_ADDRESS));
    //failHardInDebug(VL53L1X_StartRanging(VL53L1X_DEFAULT_I2C_ADDRESS));

    return (&bcu);
}

void printResult(const VL53L1X_Result_t *result)
{
    serial.print("state: ", result->Status, DEC, 3);
    serial.print(" dist (mm): ", result->Distance, DEC, 5);
    //serial.print(" sigma (mm): ", result->sigma_mm, DEC, 5);
    serial.print(" ambient (kcps): ", result->Ambient, DEC, 5);
    //serial.print(" ambient (kcps/SPAD): ", result->ambient_per_spad_kcps, DEC, 5);
    //serial.print(" target sig (kcps): ", result->signal_rate_kcps, DEC, 5);
    serial.print(" target sig (kcps/SPAD): ", result->SigPerSPAD, DEC, 5);
    serial.print(" #SPADs enabled: ", result->NumSPADs, DEC, 5);
    serial.println();
}

/**
 * The main processing loop while no KNX-application is loaded.
 */
void loop_noapp()
{
    uint8_t dataReady = 0;
    uint8_t status = VL53L1X_CheckForDataReady(VL53L1X_DEFAULT_I2C_ADDRESS, &dataReady);
    if (status != VL53LX_ERROR_NONE)
    {
        return;
    }

    if (dataReady == 0)
    {
        return;
    }

    // Read measured distance. RangeStatus = 0 means valid data
    VL53L1X_Result_t result;
    failHardInDebug(VL53L1X_GetResult(VL53L1X_DEFAULT_I2C_ADDRESS, &result));
    failHardInDebug(VL53L1X_ClearInterrupt(VL53L1X_DEFAULT_I2C_ADDRESS));
    printResult(&result);
    digitalWrite(PIN_INFO, !digitalRead(PIN_INFO));
    //failHardInDebug(VL53L1X_StartRanging(VL53L1X_DEFAULT_I2C_ADDRESS));
}

/**
 * The main processing loop.
 */
void loop()
{
    // will never be called in this example
}
/** @}*/
