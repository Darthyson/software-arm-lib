/**************************************************************************//**
 * @addtogroup SBLIB_EXAMPLES Selfbus library usage examples
 * @defgroup SBLIB_EXAMPLE_I2C_TOF_VL43L4CD i2c VL53L4CD Time of Flight (ToF) sensor example
 * @ingroup SBLIB_EXAMPLES
 * @brief   Configures and reads VL53L4CD range
 * @note Use
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
#include <sblib/i2c/tof/VL53L4CD_api.h>
#include <sblib/i2c/tof/VL53L4CD_calibration.h>

constexpr uint8_t VL53L4CD_DEFAULT_I2C_ADDRESS = 0x29;
BCU1 bcu = BCU1();

void failHardInDebug(VL53L4CD_Error status)
{
#ifndef DEBUG
    return;
#endif

    if (status != VL53L4CD_ERROR_NONE)
    {
        fatalError();
    }
}

void calibrateOffset()
{
    int16_t measuredOffset;
    failHardInDebug(VL53L4CD_CalibrateOffset(VL53L4CD_DEFAULT_I2C_ADDRESS, 100, &measuredOffset, 255));
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
    serial.println("Selfbus I2C VL53L4CD Time of Flight (ToF) sensor example");
    failHardInDebug(VL53L4CD_SensorInit(VL53L4CD_DEFAULT_I2C_ADDRESS));
    failHardInDebug(VL53L4CD_SetRangeTiming(VL53L4CD_DEFAULT_I2C_ADDRESS, 200, 0));
    failHardInDebug(VL53L4CD_SetSigmaThreshold(VL53L4CD_DEFAULT_I2C_ADDRESS, 2));
    failHardInDebug(VL53L4CD_ClearInterrupt(VL53L4CD_DEFAULT_I2C_ADDRESS));
    //calibrateOffset();
    failHardInDebug(VL53L4CD_StartRanging(VL53L4CD_DEFAULT_I2C_ADDRESS));
    failHardInDebug(VL53L4CD_StartTemperatureUpdate(VL53L4CD_DEFAULT_I2C_ADDRESS));
    failHardInDebug(VL53L4CD_StartRanging(VL53L4CD_DEFAULT_I2C_ADDRESS));

    return (&bcu);
}

void printResult(const VL53L4CD_ResultsData_t *result)
{
    serial.print("state: ", result->range_status, DEC, 3);
    serial.print(" dist (mm): ", result->distance_mm, DEC, 5);
    serial.print(" sigma (mm): ", result->sigma_mm, DEC, 5);
    serial.print(" ambient (kcps): ", result->ambient_rate_kcps, DEC, 5);
    serial.print(" ambient (kcps/SPAD): ", result->ambient_per_spad_kcps, DEC, 5);
    serial.print(" target sig (kcps): ", result->signal_rate_kcps, DEC, 5);
    serial.print(" target sig (kcps/SPAD): ", result->signal_per_spad_kcps, DEC, 5);
    serial.print(" #SPADs enabled: ", result->number_of_spad, DEC, 5);
    serial.println();
}

/**
 * The main processing loop while no KNX-application is loaded.
 */
void loop_noapp()
{
    uint8_t dataReady = 0;
    uint8_t status = VL53L4CD_CheckForDataReady(VL53L4CD_DEFAULT_I2C_ADDRESS, &dataReady);
    if (status != VL53L4CD_ERROR_NONE)
    {
        return;
    }

    if (dataReady == 0)
    {
        return;
    }

    // Read measured distance. RangeStatus = 0 means valid data
    VL53L4CD_ResultsData_t result;
    failHardInDebug(VL53L4CD_GetResult(VL53L4CD_DEFAULT_I2C_ADDRESS, &result));
    failHardInDebug(VL53L4CD_ClearInterrupt(VL53L4CD_DEFAULT_I2C_ADDRESS));
    printResult(&result);
    digitalWrite(PIN_INFO, !digitalRead(PIN_INFO));
    //failHardInDebug(VL53L4CD_StartRanging(VL53L4CD_DEFAULT_I2C_ADDRESS));
}

/**
 * The main processing loop.
 */
void loop()
{
    // will never be called in this example
}
/** @}*/
