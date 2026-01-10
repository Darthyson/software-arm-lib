/******************************************************************************
 * @addtogroup SBLIB_SENSOR Selfbus library sensors
 * @defgroup SBLIB_SENSOR_SHT1x SHT1x Temperature/humidity sensor
 * @ingroup SBLIB_SENSOR
 * @brief   Implementation for the usage of the Sensirion SHT1x series temperature and humidity sensor.
 *
 * @{
 *
 * @file   SHT1x.h
 * @bug Command @ref sht1xReadStatusRegister doesn't work.
 ******************************************************************************/

/*
 * SHT1x Library
 *
 * https://github.com/practicalarduino/SHT1x (04/01/2022) commit sha: be7042c3e3cb32c778b9c9ca270b3df57f769ea5
 * Copyright 2009 Jonathan Oxer <jon@oxer.com.au> / <www.practicalarduino.com>
 * Based on previous work by:
 *    Maurice Ribble: <www.glacialwanderer.com/hobbyrobotics/?p=5>
 *    Wayne ?: <ragingreality.blogspot.com/2008/01/ardunio-and-sht15.html>
 *
 * Manages communication with SHT1x series (SHT10, SHT11, SHT15)
 * temperature / humidity sensors from Sensirion (www.sensirion.com).
 */

#ifndef SBLIB_I2C_SHT1X_H_
#define SBLIB_I2C_SHT1X_H_

#include <sblib/sensors/units.h>
#include <cstdint>


/**
 * Invalid temperature value (-273.00°C)
 */
constexpr int16_t INVALID_TEMPERATURE = -27300;

/**
 * Invalid humidity value (0%rH)
 */
constexpr uint16_t INVALID_HUMIDITY = 0;

/**
 * Invalid dew point value (0 Td)
 */
constexpr float INVALID_DEW_POINT = 0.f;

/**
 * Class for accessing the SHT1x sensor series for temperature and humidity
 * 
 * @details The SHT1x series (SHT10, SHT11, SHT15) are temperature and humidity sensors
 *          from Sensirion that use a proprietary two-wire interface (not standard I2C).
 *          This class provides methods for reading temperature, humidity, dew point,
 *          and managing the sensor's status register.
 * 
 * @note The SHT1x series uses an own version of an I2C start sequence.
 *       Requires a 10K0 pull-up resistor on the DATA line (see SHT1x Datasheet page 5)
 * @warning Many methods are blocking and can take up to 340 ms (@ref MAX_WAIT_MS) to return
 */
class SHT1x
{
public:
    SHT1x() = delete;

    /**
     * Constructor for the SHT1x class to
     * @param dataP     Pin to use as a DATA line
     * @param clockP    Pin to use as a CLOCK line
     * @note DATA line must have a 10K0 pull-up resistor. (See SHT1x Datasheet page 5)
     */
    SHT1x(uint32_t dataP, uint32_t clockP);

    ~SHT1x() = default;

    /**
     * Read temperature-corrected relative humidity
     * @param humidity - pointer to buffer for the new humidity
     * @return True if successful, otherwise false
     * @warning Function is blocking and can take up to twice of @ref MAX_WAIT_MS to return
     */
    bool readHumidity(float* humidity);

    /**
     * Read temperature in degree Celsius
     * @param newTemperature - pointer to buffer for the new temperature
     * @return True if successful, otherwise false
     * @warning Function is blocking and can take up to @ref MAX_WAIT_MS to return
     */
    bool readTemperatureC(float* newTemperature);

    /**
     * Read temperature in degree Fahrenheit
     * @param newTemperature - pointer to buffer for the new temperature
     * @return True if successful, otherwise false
     * @warning Function is blocking and can take up to @ref MAX_WAIT_MS to return
     */
    bool readTemperatureF(float* newTemperature);

    /**
     * Initialize the SHT1x sensor
     * @return True if successful, otherwise false
     */
    [[nodiscard]] bool init();

    /**
     * Soft-reset the SHT1x sensor, resets the interface, clears the status register to default
     * @return True if successful, otherwise false
     * @note The sensor needs ~11ms to reset
     */
    bool softReset();

    /**
     * Get humidity as integer from sensor in %rH
     * @return The humidity with factor 100 (2045 = 20,45%rH)
     * @note only implemented for compatibility with @ref SHT2x class
     * @warning Function is blocking and can take up to twice of @ref MAX_WAIT_MS to return
     */
    uint16_t getHumidity();

    /**
     * Get the last measured humidity
     * @return The last humidity with factor 100 (2045 = 20,45%rH)
     */
    [[nodiscard]] uint16_t getLastHumidity() const;

    /**
    * Get temperature as integer from the sensor in degree Celsius
    * @return Temperature with factor 100 (2045 = 20,45°C)
    * @note only implemented for compatibility with @ref SHT2x class
    * @warning Function is blocking and can take up to @ref MAX_WAIT_MS to return
    */
    int16_t getTemperature();

    /**
     * Get the last measured temperature in degree Celsius
     * @return The last measured temperature with factor 100 (2045 = 20,45°C)
     */
    [[nodiscard]] int16_t getLastTemperature() const;

    /**
     * Gets the current dew point based on the current humidity and temperature
     * @return The dew point in TD
     * @warning Function is blocking and can take up to twice of @ref MAX_WAIT_MS to return
     */
    float getDewPoint();

    /**
     * Get the SHT1x status register e.g., resolution, heater and OTP
     * @param status    Content of the status register
     * @return True if successful, otherwise false
     * @warning Function is blocking and can take up to 340 ms to return (@ref MAX_WAIT_MS)
     */
    bool getStatusRegister(uint8_t* status);

    /**
     * Set the content of the SHT1x status register e.g., resolution, heater and OTP
     * @param newStatus New value of the status register
     * @return True if successful, otherwise false
     * @warning ADC-Resolution change (bit 0) is not supported.
     */
    [[nodiscard]] bool setStatusRegister(const uint8_t& newStatus);

private:
    /**
     * Unique SHT1x address
     * @note SHT1x address is always 000 (bits 7-5 of command byte)
     */
    static constexpr uint8_t Sht1xUniqueAddress = 0b000 << 5;

    /**
     * SHT1x control commands
     * @details Command byte structure: address (3 bits) + command (5 bits)
     */
    enum class SHT1xCommand : uint8_t
    {
        measureTemperature      = Sht1xUniqueAddress | 0x03, //!< Request Temperature from SHT1x
        measureRelativeHumidity = Sht1xUniqueAddress | 0x05, //!< Request relative humidity from SHT1x
        readStatusRegister      = Sht1xUniqueAddress | 0x07, //!< Read the status register of the SHT1x
        writeStatusRegister     = Sht1xUniqueAddress | 0x06, //!< Write to the status register of the SHT1x
        softReset               = Sht1xUniqueAddress | 0x1e, //!< Reset the interface, clears the status register to default values
    };

    /**
     * Read raw temperature value
     * @param   temperatureRaw - pointer to buffer for the new temperature
     * @return  True if successful, otherwise false
     * @warning Function is blocking and can take up to 340 ms to return (@ref MAX_WAIT_MS)
     */
    bool readTemperatureRaw(uint16_t *temperatureRaw);

    /**
     * Convert raw temperature value to Celsius or Fahrenheit
     * @param temperature Raw temperature value from sensor
     * @param type Temperature scale (@ref CELCIUS or @ref FARENHEIT)
     * @return Converted temperature value in the specified scale
     */
    static float convertRawTemperature(const float& temperature, eScale type);

    /**
     * Verifies the acknowledgment bit from the SHT1x sensor
     *
     * @details This method checks if the SHT1x sensor correctly acknowledges a command
     *          by reading the data line during the 9th clock cycle. The sensor pulls
     *          the data line low to acknowledge.
     *
     * @return True if the acknowledgment is correct (data line is low), false otherwise
     */
    [[nodiscard]] bool ackIsOK() const;

    /**
     * Send a command to the SHT1x sensor
     * @param command The command to send
     * @return True if the command was acknowledged, false otherwise
     */
    [[nodiscard]] bool sendCommandToSHT(SHT1xCommand command) const;

    /**
     * Send a data byte to the SHT1x sensor
     * @param dataByte The data byte to send
     * @return True if data was acknowledged and the DATA line went high, false otherwise
     */
    [[nodiscard]] bool sendDataByteToSHT(uint8_t dataByte) const;

    /**
     * Wait for a measurement to complete.
     * It can take up to a maximum of 20/80/320ms for an 8/12/14bit measurement.
     * @return true if successful, otherwise false
     */
    [[nodiscard]] bool waitForResultSHT() const;

    /**
     * Read a single byte from the SHT1x sensor and send acknowledgment
     * @return The byte received from the sensor
     */
    [[nodiscard]] uint8_t getByteSHT() const;

    /**
     * Get 8-bit data from the SHT1x sensor with CRC verification
     * @param command The command that was sent to request the data
     * @param data Pointer to store the received 8-bit data
     * @return True if data received and CRC is valid, false otherwise
     */
    [[nodiscard]] bool getData8SHT(SHT1xCommand command, uint8_t* data);

    /**
     * Get 16-bit data from the SHT1x sensor with CRC verification
     * @param command The command that was sent to request the data
     * @param data Pointer to store the received 16-bit data
     * @return True if data received and CRC is valid, false otherwise
     */
    [[nodiscard]] bool getData16SHT(SHT1xCommand command, uint16_t* data);

    /**
     * Initialize the data and clock pins
     */
    void initPins() const;

    /**
     * Activate the data pin as output with open-drain configuration
     */
    void activateDataPin() const;

    /**
     * Release the data pin to input mode
     */
    void releaseDataPin() const;

    /**
     * Perform one clock cycle (toggle clock pin high then low)
     */
    void clockCycle() const;

    /**
     * Send the transmission start sequence to the SHT1x sensor
     * @details Implements the SHT1x specific start sequence (not standard I2C)
     */
    void sendTransmissionStart() const;

    /**
     * Resets the connections to the SHT11 sensor.
     * @details While leaving @ref dataPin high, we toggle @ref clockPin nine times
     * @note Implements *3.4 Connection reset sequence* of SHT11 (datasheet page 7)
     */
    void connectionReset() const;

    /**
     * Set the internal sensor status value
     * @param newValue The new status value
     * @note Used internally for CRC calculation
     */
    void setSensorStatus(uint8_t newValue);

    /**
     * Get the internal sensor status value
     * @return The current status value
     * @note Used internally for CRC calculation
     */
    [[nodiscard]] uint8_t getSensorStatus() const;

    /**
     * Calculates the CRC8 of given data
     *
     * @param command The @ref SHT1xCommand sent
     * @param status  The current sensor status
     * @param data    Pointer to the data to use when calculating the CRC8.
     * @param length  The number of bytes in 'data'.
     *
     * @return CRC result value
     */
    static uint8_t crc8(SHT1xCommand command, uint8_t status, const uint8_t* data, uint8_t length);

    /**
     * Process a single byte for CRC8 calculation
     * @param byteToProcess The byte to process
     * @param crc Pointer to the current CRC value (will be updated)
     * @details Uses polynomial x8 + x5 + x4 + 1 (0x31)
     */
    static void crcOnSingleByte(uint8_t byteToProcess, uint8_t *crc);

    uint32_t dataPin;         //!< GPIO pin for DATA line
    uint32_t clockPin;        //!< GPIO pin for CLOCK line
    int16_t lastTemperatureC; //!< Last measured temperature in °C * 100
    uint16_t lastHumidity;    //!< Last measured humidity in %rH * 100
    uint8_t sensorStatus;     //!< Internal sensor status register value
};

#endif /* SBLIB_I2C_SHT1X_H_ */
