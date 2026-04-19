/*
 *  digital_pin.h - Functions for digital I/O
 *
 *  Copyright (c) 2014 Stefan Taferner <stefan.taferner@gmx.at>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 3 as
 *  published by the Free Software Foundation.
 */
#ifndef SBLIB_DIGITAL_PIN_H_
#define SBLIB_DIGITAL_PIN_H_

#include <cstdint>
#include <sblib/ioports.h>
#include <sblib/platform.h>
#include <sblib/types.h>


/**
 * Modes for I/O pin configuration with pinMode().
 */
enum PinMode : uint32_t
{
    /**
     * Configure the pin as standard digital input.
     */
    INPUT = 0x1000,

    /**
     * Configure the pin as capture input for a timer. This only works for the following timer,
     * digital pin, and timer capture register combinations (for LPC11xx):
     *
     * 16 Bit timer #0: capture register CR0: PIO0_2 or PIO3_3.
     *                  capture register CR1: PIO3_4.
     * 16 Bit timer #1: capture register CR0: PIO1_8.
     *                  capture register CR1: PIO3_5.
     * 32 Bit timer #0: capture register CR0: PIO1_5 or PIO2_9.
     *                  capture register CR1: PIO2_11.
     * 32 Bit timer #1: capture register CR0: PIO1_0.
     *                  capture register CR1: PIO1_11.
     *
     * Configuring digital pins as capture input that do not have this function will cause
     * unexpected behavior.
     *
     * If there are multiple pins possible, then the last pin that is configured as
     * INPUT_CAPTURE will have the capture register assigned.
     *
     * For example pin PIO1_8 captures to the capture register CR0 of the 16 bit timer #1
     * if activated.
     */
    INPUT_CAPTURE = 0x2000,

    /**
     * Configure the pin as analog input.
     */
    INPUT_ANALOG = 0x3000,

    /**
     * Enable the pull-up resistor for input.
     */
    PULL_UP = 0x10,

    /**
     * Enable the pull-down resistor for input.
     */
    PULL_DOWN = 0x08,

    /**
     * Enable the input hysteresis.
     */
    HYSTERESIS = 0x20,

    /**
     * Configure the pin as standard digital output.
     */
    OUTPUT = 0x6000,

    /**
     * Configure the pin as match output for a timer. This option is used to configure
     * a digital pin for PWM output. Match output is available for the following combinations
     * of timer, match register, and digital pin:
     *
     * 16 Bit timer #0:  match register MR0 for pin PIO0_8 or PIO3_0,
     *                   match register MR1 for pin PIO0_9,
     *                   match register MR2 for pin PIO0_10 or PIO3_2.
     * 16 Bit timer #1:  match register MR0 for pin PIO1_9,
     *                   match register MR1 for pin PIO1_10 or PIO2_4.
     * 32 Bit timer #0:  match register MR0 for pin PIO1_6 or PIO2_5,
     *                   match register MR1 for pin PIO1_7 or PIO2_6,
     *                   match register MR2 for pin PIO0_1 or PIO2_7,
     *                   match register MR3 for pin PIO0_11 or PIO2_8.
     * 32 Bit timer #1:  match register MR0 for pin PIO1_1,
     *                   match register MR1 for pin PIO1_2,
     *                   match register MR2 for pin PIO1_3,
     *                   match register MR3 for pin PIO1_4.
     *
     * Configuring digital pins as output match that do not have this function will cause
     * unexpected behavior.
     *
     * For example pin PIO1_9 is driven by the match register MR0 of the 16 bit
     * timer #1 if activated.
     */
    OUTPUT_MATCH = 0x7000,

    /**
     * Enable open-drain output mode. If not enabled, the standard output mode is used.
     * In open-drain output mode, a 1 disables the high-drive transistor.
     */
    OPEN_DRAIN = 0x400,

    /**
     * Enable the repeater mode.
     */
    REPEATER_MODE = 0x18,

    /**
     * Configure the pin as serial data input (RxD).
     */
    SERIAL_RXD = INPUT | PinModeFunc(PF_RXD),

    /**
     * Configure the pin as serial data output (TxD).
     */
    SERIAL_TXD = OUTPUT | PinModeFunc(PF_TXD),

    /**
     * Configure the pin as serial request to send (RTS).
     */
    SERIAL_RTS = OUTPUT | PinModeFunc(PF_RTS),

    /**
     * Configure the pin as output for SPI clock (SCK) in SPI master mode or input
     * for SPI clock (SCK) in SPI slave mode. Shall be combined with OUTPUT or INPUT.
     */
    SPI_CLOCK = PinModeFunc(PF_SCK),

    /**
     * Configure the pin for SPI master-in-slave-out (MISO). Combine with INPUT for
     * SPI master mode or OUTPUT for SPI slave mode.
     */
    SPI_MISO = PinModeFunc(PF_MISO),

    /**
     * Configure the pin for SPI master-out-slave-in (MOSI). Combine with OUTPUT for
     * SPI master mode or INPUT for SPI slave mode.
     */
    SPI_MOSI = PinModeFunc(PF_MOSI),

    /**
     * Configure the pin for SPI slave select (SSEL). Combine with OUTPUT for
     * SPI master mode or INPUT for SPI slave mode.
     */
    SPI_SSEL = PinModeFunc(PF_SSEL)
};

enum PinInterruptMode : uint16_t
{
    /**
     * Configure the interrupt to be level triggered activated by a LOW level.
     */
    INTERRUPT_LEVEL_LOW = 0x0100,

    /**
     * Configure the interrupt to be level triggered activated by a HIGH level.
     */
    INTERRUPT_LEVEL_HIGH = 0x0101,

    /**
     * Configure the interrupt to be triggered by a falling edge on the I/O pin.
     */
    INTERRUPT_EDGE_FALLING = 0x0000,

    /**
     * Configure the interrupt to be triggered by a rising edge on the I/O pin.
     */
    INTERRUPT_EDGE_RISING = 0x0001,

    /**
     * Configure the interrupt to be triggered by a both edges on the I/O pin.
     */
    INTERRUPT_EDGE_BOTH = 0x0010,

    /**
     * After the configuration enable the interrupt.
     */
    INTERRUPT_ENABLED = 0x1000,
};

/**
 * Configure the mode of an I/O pin.
 *
 * @param pin   The pin to configure: PIO0_0, PIO0_1, ... @ref PortPin
 * @param mode  The I/O mode to set. Use a combination of the PinMode values (see below)
 *
 * Examples:
 * Configure PIO0_2 to digital input: pinMode(PIO0_2, INPUT);
 * Configure PIO1_7 to serial data output: pinMode(PIO1_7, OUTPUT | PINMODE_FUNC(PF_TXD));
 *
 * @see PinMode in digital_pin.h for the pin modes
 * @see PinFunc in ioports.h for the pin functions for PINMODE_FUNC()
 */
void pinMode(uint32_t pin, uint32_t mode);

/**
 * Configure the direction of an I/O pin. This does not change the other configuration
 * settings of the pin.
 *
 * @param pin   The pin to configure: PIO0_0, PIO0_1, ... @ref PortPin
 * @param dir   The direction: INPUT or OUTPUT
 */
void pinDirection(uint32_t pin, uint32_t dir);

/**
 * Configure the interrupt for the I/O port
 *
 * @param pin   The pin to configure: PIO0_0, PIO0_1, ... @ref PortPin
 * @param mode  The interrupt mode. Use a combination of @ref PinInterruptMode values
 */
void pinInterruptMode(uint32_t pin,  uint16_t mode);

/**
 * Enable the interrupt for this I/O pin
 */
void pinEnableInterrupt(uint32_t pin);

/**
 * Disable the interrupt for this I/O pin
 */
void pinDisableInterrupt(uint32_t pin);

/**
 * Configure the mode of the pins of an I/O port.
 *
 * This function can only handle a sub-set of the available pin configurations.
 * The following PinMode values are supported: INPUT, OUTPUT, PULL_UP, PULL_DOWN,
 * REPEATER_MODE, HYSTERESIS, OPEN_DRAIN.
 *
 * @param portNum   The port to configure: PIO0, PIO1, PIO2, PIO3  (see sblib/ioports.h)
 * @param pinMask   The bit mask for the port pins that shall be configured.
 * @param mode      The I/O mode to set. Use a combination of the PinMode values.
 *
 * Example: to configure pins 0,1,2 of port 0 to open drain output:
 *          portMode(PIO0, 7, OUTPUT|OPEN_DRAIN);
 *
 * @see PinMode in digital_pin.h for the pin modes
 */
void portMode(uint8_t portNum, uint32_t pinMask, uint32_t mode);

/**
 * Configure the direction of an I/O pin. This does not change the other configuration
 * settings of the port pins.
 *
 * @param portNum   The port to configure: PIO0, PIO1, PIO2, PIO3  (see sblib/ioports.h)
 * @param pinMask   The bit mask for the port pins that shall be configured.
 * @param dir       The direction: INPUT or OUTPUT
 */
void portDirection(uint8_t portNum, uint32_t pinMask, uint32_t dir);

/**
 * Set the value of a digital output pin.
 *
 * @param pin   The pin to set: PIO0_0, PIO0_1, ... @ref PortPin
 * @param value The value to set: true or false.
 */
void digitalWrite(uint32_t pin, bool value);

/**
 * Read the value of a digital input pin.
 *
 * @param pin   The pin to read: PIO0_0, PIO0_1, ... @ref PortPin
 * @return The value of the pin: true (1) or false (0).
 */
bool digitalRead(uint32_t pin);

/**
 * Output a byte on a digital pin. The output is done bit by bit. The clock pin
 * pulses the output. Output of a bit happens when the clock pin is high. This
 * is a software function. For hardware supported output of data, see SPI or I2C.
 *
 * @param dataPin   The data pin to output the byte to
 * @param clockPin  The clock pin
 * @param bitOrder  The bit order: LSBFIRST or MSBFIRST.
 * @param val       The value to output.
 */
void shiftOut(uint32_t dataPin, uint32_t clockPin, BitOrder bitOrder, uint8_t val);

/**
 * Read a byte from a digital pin. The byte is read bit by bit. The clock pin
 * pulses the reading. Please note that the clock pin is used as output and outputs
 * the pulses for reading. This is a software function. For hardware supported input
 * of data, see SPI or I2C.
 *
 * @param dataPin   The data pin to read the byte from
 * @param clockPin  The clock pin
 * @param bitOrder  The bit order: LSBFIRST or MSBFIRST.
 *
 * @return The read byte.
 */
uint8_t shiftIn(uint32_t dataPin, uint32_t clockPin, BitOrder bitOrder);

/**
 * Measures the length (in microseconds) of a pulse on the pin; state is HIGH
 * or LOW, the type of pulse to measure. Works on pulses from 2-3 microseconds
 * to 3 minutes in length, but must be called at least a few dozen microseconds
 * before the start of the pulse.
 *
 * @param pin       The pin to measure.
 * @param state     The state of the pin to measure: true measures a high pulse, false measures a low pulse.
 * @param timeout   The timeout to wait for the pulse to end, in microseconds.
 *
 * @return The length of the pulse in microseconds.
 */
uint32_t pulseIn(uint32_t pin, bool state, uint32_t timeout);

/**
 * Get the port number of the pin.
 *
 * @param pin   The pin to process, e.g. PIO1_9
 * @return The port number of the pin, e.g. 1
 */
constexpr Port digitalPinToPort(const uint32_t pin) {return static_cast<Port>((pin >> 5) & 3);} // 3 = 0b0000 0011

/**
 * Get the number of the pin.
 *
 * @param pin   The pin to process, e.g. PIO1_9
 * @return The number of the pin, e.g. 9
 */
constexpr uint8_t digitalPinToPinNum(const uint32_t pin) {return (pin & 31);} // 31 = 0b0001 1111

/**
 * Get the bit mask for the pin.
 *
 * @param pin   The pin to process, e.g. PIO1_9
 * @return The bit mask for the pin, e.g. 0x200
 */
constexpr uint32_t digitalPinToBitMask(const uint32_t pin) {return (1 << digitalPinToPinNum(pin));}

//
//  Inline functions
//

ALWAYS_INLINE void digitalWrite(const uint32_t pin, const bool value)
{
    const uint32_t mask = digitalPinToBitMask(pin);
    gpioPorts[digitalPinToPort(pin)]->MASKED_ACCESS[mask] = value ? mask : 0;
}

ALWAYS_INLINE bool digitalRead(const uint32_t pin)
{
    return gpioPorts[digitalPinToPort(pin)]->MASKED_ACCESS[digitalPinToBitMask(pin)] != 0;
}

ALWAYS_INLINE void pinEnableInterrupt(const uint32_t pin)
{
    LPC_GPIO_TypeDef* port = gpioPorts[digitalPinToPort(pin)];
    const uint32_t mask = digitalPinToBitMask(pin);

    port->IE |= mask;
}

ALWAYS_INLINE void pinDisableInterrupt(const uint32_t pin)
{
    LPC_GPIO_TypeDef* port = gpioPorts[digitalPinToPort(pin)];
    const uint32_t mask = digitalPinToBitMask(pin);

    port->IE &= ~mask;
}

#endif /* SBLIB_DIGITAL_PIN_H_ */
