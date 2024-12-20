/**************************************************************************//**
 *
 * @author Doumanix <doumanix@gmx.de> Copyright (c) 2023
 * @bug No known bugs.
 ******************************************************************************/

/*
 This program is free software; you can redistribute it and/or modify
 it under the terms of the GNU General Public License version 3 as
 published by the Free Software Foundation.
 ---------------------------------------------------------------------------*/


#ifndef SGP4X_H
#define SGP4X_H

#include <stdint.h>
#include <sblib/i2c/sensirion_gas_index_algorithm.h>

enum class SGP4xResult : int8_t {
  invalidByteCount = -6,
  invalidCommandBuffer = -5,
  vocPixelError = -4,
  noxPixelError = -3,
  crc8Mismatch = -2,
  sendError = -1,
  readError = 0,
  success = 1,
};

class SGP4xClass
{
private:
  enum class Sgp4xCommand : uint16_t {
      selfConditioning = 0x2612,      // sgp41_execute_conditioning
      getSerial       = 0x3682 ,      // sgp4x_get_serial_number 0x3682
      measureRaw      = 0x2619,       // sgp41_measure_raw_signals
      heaterOff       = 0x3615,       // sgp4x_turn_heater_off
      selfTest        = 0x280E,       // sgp41_execute_self_test

      /**
       * featureSet is not documented in sgp4x datasheet.\n
       * Got it from\n
       * https://github.com/winkj/sensirion-ess/blob/e506f5dda0d28bd46e7f6eebf7d2b06bde41699c/sensirion_ess.cpp#L187 \n
       * https://github.com/adafruit/Adafruit_SGP40/blob/b31a6a56da76588073de977e68ea09421e507cda/src/Adafruit_SGP40.cpp#L63 \n
       * looks like product type is encoded in the high nibble of the high byte of featuresSet as \n
       * productType = featureSet >> 12 \n
       * productType: \n
       * - 0 => SGP30, SGP40 ??
       * - 1 => SGPC3
       *
       * featureSetVersion = featureSet & 0xff \n
       * - 0x40 => SGP40
       *
       * @ref readFeatureSet returns 0x0240 for my SGP40
       */
      featureSet      = 0x202f  };

  int32_t rawVocTics;
  int32_t rawNoxTics;
  int32_t vocIndexValue;
  int32_t noxIndexValue;
  uint16_t featureSet;

  GasIndexAlgorithmParams voc_algorithm_params;
  GasIndexAlgorithmParams nox_algorithm_params;

  /**
   * Send a command to the sensor, wait for it to be processed and retrieve the sensor's response
   *
   * @param command             The @ref Sgp4xCommand to send
   * @param commandBuffer       Buffer containing the command to send and its optional parameters
   * @param commandBufferSize   Size in bytes of the command buffer
   * @param readBuffer          Buffer to store the response of the sensor
   * @param readBufferSize      Size in bytes of the response buffer
   * @param processDelayMs      Time in milliseconds to wait for command processing by the sensor
   *
   * @return @ref SGP4xResult::success if successful, otherwise a @ref SGP4xResult
   */
  SGP4xResult readSensor(Sgp4xCommand command, uint8_t * commandBuffer, uint8_t commandBufferSize, uint8_t * readBuffer, uint8_t readBufferSize, uint16_t processDelayMs);

  /**
   * Do a CRC validation of result
   *
   * @param data  Pointer to the data to use when calculating the CRC8.
   * @param len   The number of bytes in 'data'.
   *
   * @return CRC result value
   */
  uint8_t crc8(const uint8_t* data, int len);

public:
  SGP4xClass();
  ~SGP4xClass() = default;

  /**
   * Initialize the SGP4x. Calls @ref executeConditioning() to condition the sensor.
   * @param samplingIntervalMs Sampling interval for the gas index algorithm in milliseconds.\n
   *                           1s (1000ms) is recommend by sensirion, see @ref GasIndexAlgorithm_init_with_sampling_interval in @ref sensirion_gas_index_algorithm.h)
   *
   * @return @ref SGP4xResult::success if successful, otherwise a @ref SGP4xResult
   */
  SGP4xResult init(uint32_t samplingIntervalMs);

  /**
   * Triggers the built-in self-test checking
   * for integrity of both hotplate and MOX material and checks the 2 byte test result.
   *
   * @return @ref SGP4xResult::success if successful, otherwise a @ref SGP4xResult
   */
  SGP4xResult executeSelfTest();

  /**
   * Executes the @ref Sgp4xCommand::measureRaw command to measure VOC and NOX
   * with optional temperature and relative humidity compensation
   *
   * @param relativeHumidity    Relative humidity in percent
   * @param temperature         Temperature in degree celsius
   * @param useCompensation     Set true to use SGP4x internal temperature/humidity compensation
   *
   * @return @ref SGP4xResult::success if successful, otherwise a @ref SGP4xResult
   */
  SGP4xResult measureRawSignal(float relativeHumidity, float temperature, bool useCompensation);

  /**
   * Executes the @ref Sgp4xCommand::measureRaw command to measure VOC and NOX
   * without temperature or relative humidity compensation
   *
   * @return @ref SGP4xResult::success if successful, otherwise a @ref SGP4xResult
   */
  SGP4xResult measureRawSignal();

  /**
   * Shall be executed after each re-start of the SGP4x
   * sensor returns a VOC value - but this is discarded here.
   * We just need the conditioning functionality.
   *
   * @return @ref SGP4xResult::success if successful, otherwise a @ref SGP4xResult
   */
  SGP4xResult executeConditioning();

  /**
   * Executes the @ref Sgp4xCommand::heaterOff command which
   * - turns the hotplate off
   * - stops the measurement
   * - sets the sensor to idle mode
   *
   * @return @ref SGP4xResult::success if successful, otherwise a @ref SGP4xResult
   */
  SGP4xResult turnHeaterOffAndReturnToIdle();

  SGP4xResult readFeatureSet();

  int32_t getVocIndexValue();
  int32_t getNoxIndexValue();
  int32_t getRawVocValue();
  int32_t getRawNoxValue();
  uint16_t getFeatureSet();

  /**
   * Read the sensor's unique serial number
   *
   * @param serialNumber    Buffer to store the serial number of the sensor
   * @param length          Length in bytes of the provided buffer
   * @return @ref SGP4xResult::success if successful, otherwise a @ref SGP4xResult
   */
  SGP4xResult getSerialnumber(uint8_t * serialNumber, uint8_t length);

  static constexpr uint8_t maxSerialNumberLength = 6;
};

#endif
