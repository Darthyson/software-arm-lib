/*
 * Tests for the digital_pin.h
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 3 as
 * published by the Free Software Foundation.
 */

#include <catch.hpp> // If possible, include catch.hpp as last header

#include "test_digital_pin.h"
#include "test_ioports.h"
#include <sblib/digital_pin.h>


void printPinMask()
{
    for (const PortPinInfo& pin : allPins)
    {
        printf("REQUIRE(digitalPinToBitMask(%s) == 0x%.8x);\n", pin.name, digitalPinToBitMask(pin.pin));
    }
    printf("\n");
}

TEST_CASE("digitalPinToPort", "[digital_pin]")
{
    for (const PortPinInfo& pin :  allPins)
    {
        uint8_t portNumber = digitalPinToPort(pin.pin);
        REQUIRE(portNumber == pin.portNumber);
    }
}

TEST_CASE("digitalPinToPinNum", "[digital_pin]")
{
    for (const PortPinInfo& pin :  allPins)
    {
        uint8_t pinNumber = digitalPinToPinNum(pin.pin);
        REQUIRE(pinNumber == pin.pinNumber);
    }
}

TEST_CASE("digitalPinToBitMask", "[digital_pin]")
{
    //printPinMask();
    // Port 0
    REQUIRE(digitalPinToBitMask(PIO0_0) == 0x00000001);
    REQUIRE(digitalPinToBitMask(PIO0_1) == 0x00000002);
    REQUIRE(digitalPinToBitMask(PIO0_2) == 0x00000004);
    REQUIRE(digitalPinToBitMask(PIO0_3) == 0x00000008);
    REQUIRE(digitalPinToBitMask(PIO0_4) == 0x00000010);
    REQUIRE(digitalPinToBitMask(PIO0_5) == 0x00000020);
    REQUIRE(digitalPinToBitMask(PIO0_6) == 0x00000040);
    REQUIRE(digitalPinToBitMask(PIO0_7) == 0x00000080);
    REQUIRE(digitalPinToBitMask(PIO0_8) == 0x00000100);
    REQUIRE(digitalPinToBitMask(PIO0_9) == 0x00000200);
    REQUIRE(digitalPinToBitMask(PIO0_10) == 0x00000400);
    REQUIRE(digitalPinToBitMask(PIO0_11) == 0x00000800);

    // Port 1
    REQUIRE(digitalPinToBitMask(PIO1_0) == 0x00000001);
    REQUIRE(digitalPinToBitMask(PIO1_1) == 0x00000002);
    REQUIRE(digitalPinToBitMask(PIO1_2) == 0x00000004);
    REQUIRE(digitalPinToBitMask(PIO1_3) == 0x00000008);
    REQUIRE(digitalPinToBitMask(PIO1_4) == 0x00000010);
    REQUIRE(digitalPinToBitMask(PIO1_5) == 0x00000020);
    REQUIRE(digitalPinToBitMask(PIO1_6) == 0x00000040);
    REQUIRE(digitalPinToBitMask(PIO1_7) == 0x00000080);
    REQUIRE(digitalPinToBitMask(PIO1_8) == 0x00000100);
    REQUIRE(digitalPinToBitMask(PIO1_9) == 0x00000200);
    REQUIRE(digitalPinToBitMask(PIO1_10) == 0x00000400);
    REQUIRE(digitalPinToBitMask(PIO1_11) == 0x00000800);

    // Port 2
    REQUIRE(digitalPinToBitMask(PIO2_0) == 0x00000001);
    REQUIRE(digitalPinToBitMask(PIO2_1) == 0x00000002);
    REQUIRE(digitalPinToBitMask(PIO2_2) == 0x00000004);
    REQUIRE(digitalPinToBitMask(PIO2_3) == 0x00000008);
    REQUIRE(digitalPinToBitMask(PIO2_4) == 0x00000010);
    REQUIRE(digitalPinToBitMask(PIO2_5) == 0x00000020);
    REQUIRE(digitalPinToBitMask(PIO2_6) == 0x00000040);
    REQUIRE(digitalPinToBitMask(PIO2_7) == 0x00000080);
    REQUIRE(digitalPinToBitMask(PIO2_8) == 0x00000100);
    REQUIRE(digitalPinToBitMask(PIO2_9) == 0x00000200);
    REQUIRE(digitalPinToBitMask(PIO2_10) == 0x00000400);
    REQUIRE(digitalPinToBitMask(PIO2_11) == 0x00000800);

    // Port 3
    REQUIRE(digitalPinToBitMask(PIO3_0) == 0x00000001);
    REQUIRE(digitalPinToBitMask(PIO3_1) == 0x00000002);
    REQUIRE(digitalPinToBitMask(PIO3_2) == 0x00000004);
    REQUIRE(digitalPinToBitMask(PIO3_3) == 0x00000008);
    REQUIRE(digitalPinToBitMask(PIO3_4) == 0x00000010);
    REQUIRE(digitalPinToBitMask(PIO3_5) == 0x00000020);
}

void resetRegisters(const PortPin pin)
{
    LPC_GPIO_TypeDef* port = gpioPorts[digitalPinToPort(pin)];
    port->DIR = 0;
}

uint32_t getDigitalPinFunctionNumber(const PortPin pin, const PinFunc pinFunction)
{
    if ((pin & PFL_ADMODE) == PFL_ADMODE) // If pin has ADC support we need to set ADMODE = Digital functional mode bit 7
    {
        return getPinFunctionNumber(pin, pinFunction) | 0x80;
    }

    return getPinFunctionNumber(pin, pinFunction);
}

struct IOConRegister
{
    uint32_t RESERVED0[1];
    uint32_t SSEL1_LOC;
    uint32_t SCK_LOC;
    uint32_t DSR_LOC;
    uint32_t DCD_LOC;
    uint32_t RI_LOC;
    uint32_t CT16B0_CAP0_LOC;
    uint32_t SCK1_LOC;
    uint32_t MISO1_LOC;
    uint32_t MOSI1_LOC;
    uint32_t CT32B0_CAP0_LOC;
    uint32_t RXD_LOC;
};

void readIOConRegisters(IOConRegister* reg)
{
    reg->RESERVED0[0] = LPC_IOCON->RESERVED0[0];
    reg->SSEL1_LOC = LPC_IOCON->SSEL1_LOC;
    reg->SCK_LOC = LPC_IOCON->SCK_LOC;
    reg->DSR_LOC = LPC_IOCON->DSR_LOC;
    reg->DCD_LOC = LPC_IOCON->DCD_LOC;
    reg->RI_LOC = LPC_IOCON->RI_LOC;
    reg->CT16B0_CAP0_LOC = LPC_IOCON->CT16B0_CAP0_LOC;
    reg->SCK1_LOC = LPC_IOCON->SCK1_LOC;
    reg->MISO1_LOC = LPC_IOCON->MISO1_LOC;
    reg->MOSI1_LOC = LPC_IOCON->MOSI1_LOC;
    reg->CT32B0_CAP0_LOC = LPC_IOCON->CT32B0_CAP0_LOC;
    reg->RXD_LOC = LPC_IOCON->RXD_LOC;
}

void requireReservedRegistersUnchanged(const IOConRegister* reg)
{
    REQUIRE(reg->RESERVED0[0] == LPC_IOCON->RESERVED0[0]);
}

void requireSerialRegistersUnchanged(const IOConRegister* reg)
{
    REQUIRE(reg->DSR_LOC == LPC_IOCON->DSR_LOC);
    REQUIRE(reg->DCD_LOC == LPC_IOCON->DCD_LOC);
    REQUIRE(reg->RI_LOC == LPC_IOCON->RI_LOC);
    REQUIRE(reg->RXD_LOC == LPC_IOCON->RXD_LOC);
}

void requireAllIOConRegistersUnchanged(IOConRegister* reg)
{
    requireReservedRegistersUnchanged(reg);
    requireSerialRegistersUnchanged(reg);
    REQUIRE(reg->SSEL1_LOC == LPC_IOCON->SSEL1_LOC);
    REQUIRE(reg->SCK_LOC == LPC_IOCON->SCK_LOC);
    REQUIRE(reg->CT16B0_CAP0_LOC == LPC_IOCON->CT16B0_CAP0_LOC);
    REQUIRE(reg->SCK1_LOC == LPC_IOCON->SCK1_LOC);
    REQUIRE(reg->MISO1_LOC == LPC_IOCON->MISO1_LOC);
    REQUIRE(reg->MOSI1_LOC == LPC_IOCON->MOSI1_LOC);
    REQUIRE(reg->CT32B0_CAP0_LOC == LPC_IOCON->CT32B0_CAP0_LOC);
}

TEST_CASE("pinMode(pin, OUTPUT) / pinMode(pin, OUTPUT_MATCH)", "[digital_pin]")
{
    for (const PortPinInfo& pin : allPins)
    {
        IOConRegister savedRegister{};
        readIOConRegisters(&savedRegister);

        const LPC_GPIO_TypeDef* port = gpioPorts[digitalPinToPort(pin.pin)];
        const uint32_t* iocon = ioconPointer(pin.pin);
        const uint32_t bitOutMask = 1 << digitalPinToPinNum(pin.pin);

        uint32_t pinPIOfunctionNumber = getDigitalPinFunctionNumber(pin.pin, PF_PIO);

        // Set pin mode to output
        pinMode(pin.pin, OUTPUT);
        REQUIRE((port->DIR & bitOutMask) == bitOutMask); // port direction bit for pin set

        REQUIRE(*iocon == pinPIOfunctionNumber); // IOCON_PIO_x_y set to correct pin function
        requireAllIOConRegistersUnchanged(&savedRegister);

        pinMode(pin.pin, INPUT);  // set to input
        pinMode(pin.pin, OUTPUT); // and again to output
        REQUIRE((port->DIR & bitOutMask) == bitOutMask); // port direction bit still set?

        if (getPinFunctionNumber(pin.pin, PF_MAT) < 0)
        {
            ///\todo it would be better, if pinMode had a return error value with [[nounused]]
            continue; // pin does not support output match, so continue
        }

        // Set pin mode to output match
        pinMode(pin.pin, OUTPUT_MATCH);
        REQUIRE((port->DIR & bitOutMask) == bitOutMask); // port direction bit for pin set

        pinPIOfunctionNumber = getDigitalPinFunctionNumber(pin.pin, PF_MAT);

        REQUIRE(*iocon == pinPIOfunctionNumber); // IOCON_PIO_x_y set to correct pin function
        requireAllIOConRegistersUnchanged(&savedRegister);
    }
}

TEST_CASE("pinMode(pin, INPUT)", "[digital_pin]")
{
    for (const PortPinInfo& pin : allPins)
    {
        IOConRegister savedRegister{};
        readIOConRegisters(&savedRegister);

        const LPC_GPIO_TypeDef* port = gpioPorts[digitalPinToPort(pin.pin)];
        const uint32_t* iocon = ioconPointer(pin.pin);
        const uint32_t bitOutMask = 1 << digitalPinToPinNum(pin.pin);

        uint32_t pinPIOfunctionNumber = getDigitalPinFunctionNumber(pin.pin, PF_PIO);

        pinMode(pin.pin, INPUT); // Set pin mode to input
        REQUIRE((port->DIR & bitOutMask) == 0); // port direction bit for pin NOT set

        REQUIRE(*iocon == pinPIOfunctionNumber); // IOCON_PIO_x_y set to correct pin function
        requireAllIOConRegistersUnchanged(&savedRegister);

        pinMode(pin.pin, OUTPUT); // set to output
        pinMode(pin.pin, INPUT);  // and again to input
        REQUIRE((port->DIR & bitOutMask) == 0); // port direction bit still NOT set?
        requireAllIOConRegistersUnchanged(&savedRegister);
    }
}

TEST_CASE("pinMode(pin, INPUT_CAPTURE)", "[digital_pin]")
{
    // we run this test twice
    // to check correct changes in CT16B0_CAP0_LOC and CT32B0_CAP0_LOC
    for (auto i = 0; i < 2; i++)
    {
        for (const PortPinInfo& pin : allPins)
        {
            if (getPinFunctionNumber(pin.pin, PF_CAP) < 0)
            {
                ///\todo it would be better, if pinMode had a return error value with [[nounused]]
                continue; // pin does not support INPUT_CAPTURE, so continue
            }

            IOConRegister savedRegister{};
            readIOConRegisters(&savedRegister);

            const LPC_GPIO_TypeDef* port = gpioPorts[digitalPinToPort(pin.pin)];
            const uint32_t* iocon = ioconPointer(pin.pin);
            const uint32_t bitOutMask = 1 << digitalPinToPinNum(pin.pin);

            uint32_t pinPIOfunctionNumber = getDigitalPinFunctionNumber(pin.pin, PF_CAP);

            pinMode(pin.pin, INPUT_CAPTURE); // Set pin mode to input capture
            REQUIRE((port->DIR & bitOutMask) == 0); // port direction bit for pin NOT set

            REQUIRE(*iocon == pinPIOfunctionNumber); // IOCON_PIO_x_y set to correct pin function

            switch (pin.pin)
            {
                case PIO0_2:
                    REQUIRE(LPC_IOCON->CT16B0_CAP0_LOC == 0);
                    savedRegister.CT16B0_CAP0_LOC = 0;
                    break;
                case PIO1_5:
                    REQUIRE(LPC_IOCON->CT32B0_CAP0_LOC == 0);
                    savedRegister.CT32B0_CAP0_LOC = 0;
                    break;
                case PIO2_9:
                    REQUIRE(LPC_IOCON->CT32B0_CAP0_LOC == 1);
                    savedRegister.CT32B0_CAP0_LOC = 1;
                    break;
                case PIO3_3:
                    REQUIRE(LPC_IOCON->CT16B0_CAP0_LOC == 1);
                    savedRegister.CT16B0_CAP0_LOC = 1;
                    break;
                case PIO1_0:  // CT32B1_CAP0, has no location register
                case PIO1_8:  // CT16B1_CAP0, has no location register
                case PIO1_11: // CT32B1_CAP1, has no location register
                case PIO2_11: // CT32B0_CAP1, has no location register
                case PIO3_4:  // CT16B0_CAP1, has no location register
                case PIO3_5:  // CT16B1_CAP1, has no location register
                    break;
                default:
                    FAIL("Unknown pin for INPUT_CAPTURE.");
                    break;
            }

            requireAllIOConRegistersUnchanged(&savedRegister);
        }
    }
}

TEST_CASE("pinMode(pin, INPUT_ANALOG)", "[digital_pin]")
{
    for (const PortPinInfo& pin : allPins)
    {
        if (getPinFunctionNumber(pin.pin, PF_AD) < 0)
        {
            ///\todo it would be better, if pinMode had a return error value with [[nounused]]
            continue; // pin does not support INPUT_ANALOG, so continue
        }
        IOConRegister savedRegister{};
        readIOConRegisters(&savedRegister);

        const LPC_GPIO_TypeDef* port = gpioPorts[digitalPinToPort(pin.pin)];
        const uint32_t* iocon = ioconPointer(pin.pin);
        const uint32_t bitOutMask = 1 << digitalPinToPinNum(pin.pin);

        int8_t pinPIOfunctionNumber = getPinFunctionNumber(pin.pin, PF_AD);

        pinMode(pin.pin, INPUT_ANALOG);         // Set pin mode to input
        REQUIRE((port->DIR & bitOutMask) == 0); // port direction bit for pin NOT set

        REQUIRE(*iocon == static_cast<uint32_t>(pinPIOfunctionNumber)); // IOCON_PIO_x_y set to correct pin function
        requireAllIOConRegistersUnchanged(&savedRegister);
    }
}

TEST_CASE("pinMode(pin, SERIAL_RXD)", "[digital_pin]")
{
    for (const PortPinInfo& pin : allPins)
    {
        if (getPinFunctionNumber(pin.pin, PF_RXD) < 0)
        {
            ///\todo it would be better, if pinMode had a return error value with [[nounused]]
            continue; // pin does not support PF_RXD, so continue
        }

        IOConRegister savedRegister{};
        readIOConRegisters(&savedRegister);

        const LPC_GPIO_TypeDef* port = gpioPorts[digitalPinToPort(pin.pin)];
        const uint32_t* iocon = ioconPointer(pin.pin);
        const uint32_t bitOutMask = 1 << digitalPinToPinNum(pin.pin);

        uint32_t pinPIOfunctionNumber = getDigitalPinFunctionNumber(pin.pin, PF_RXD);

        pinMode(pin.pin, SERIAL_RXD); // Set pin mode to input capture
        REQUIRE((port->DIR & bitOutMask) == 0); // port direction bit for pin NOT set

        REQUIRE(*iocon == pinPIOfunctionNumber); // IOCON_PIO_x_y set to correct pin function

        switch (pin.pin)
        {
            case PIO1_6:
                REQUIRE(LPC_IOCON->RXD_LOC == 0);
                savedRegister.RXD_LOC = 0;
                break;
            case PIO2_7:
                REQUIRE(LPC_IOCON->RXD_LOC == 1);
                savedRegister.RXD_LOC = 1;
                break;
            case PIO3_1:
                REQUIRE(LPC_IOCON->RXD_LOC == 2);
                savedRegister.RXD_LOC = 2;
                break;
            case PIO3_4:
                REQUIRE(LPC_IOCON->RXD_LOC == 3);
                savedRegister.RXD_LOC = 3;
                break;
            default:
                FAIL("Unknown pin for SERIAL_RXD.");
                break;
        }

        requireAllIOConRegistersUnchanged(&savedRegister);
    }
}

TEST_CASE("pinMode(pin, SPI_MISO)", "[digital_pin]")
{
    // we run this test twice
    // to check correct changes in MISO1_LOC
    for (auto i = 0; i < 2; i++)
    {
        for (const PortPinInfo& pin : allPins)
        {
            if (getPinFunctionNumber(pin.pin, PF_MISO) < 0)
            {
                ///\todo it would be better, if pinMode had a return error value with [[nounused]]
                continue; // pin does not support PF_MISO, so continue
            }

            IOConRegister savedRegister{};
            readIOConRegisters(&savedRegister);

            const LPC_GPIO_TypeDef* port = gpioPorts[digitalPinToPort(pin.pin)];
            const uint32_t* iocon = ioconPointer(pin.pin);
            const uint32_t bitOutMask = 1 << digitalPinToPinNum(pin.pin);

            uint32_t pinPIOfunctionNumber = getDigitalPinFunctionNumber(pin.pin, PF_MISO);

            pinMode(pin.pin, SPI_MISO); // Set pin mode to input capture
            REQUIRE((port->DIR & bitOutMask) == 0); // port direction bit for pin NOT set

            REQUIRE(*iocon == pinPIOfunctionNumber); // IOCON_PIO_x_y set to correct pin function

            switch (pin.pin)
            {
                case PIO0_8: // has no location register
                    break;
                case PIO1_10:
                    REQUIRE(LPC_IOCON->MISO1_LOC == 1);
                    savedRegister.MISO1_LOC = 1;
                    break;
                case PIO2_2:
                    REQUIRE(LPC_IOCON->MISO1_LOC == 0);
                    savedRegister.MISO1_LOC = 0;
                    break;

                default:
                    FAIL("Unknown pin for SPI_MISO.");
                    break;
            }

            requireAllIOConRegistersUnchanged(&savedRegister);
        }
    }
}

TEST_CASE("pinMode(pin, SPI_MOSI)", "[digital_pin]")
{
    // we run this test twice
    // to check correct changes in MOSI1_LOC
    for (auto i = 0; i < 2; i++)
    {
        for (const PortPinInfo& pin : allPins)
        {
            if (getPinFunctionNumber(pin.pin, PF_MOSI) < 0)
            {
                ///\todo it would be better, if pinMode had a return error value with [[nounused]]
                continue; // pin does not support PF_MISO, so continue
            }

            IOConRegister savedRegister{};
            readIOConRegisters(&savedRegister);

            const LPC_GPIO_TypeDef* port = gpioPorts[digitalPinToPort(pin.pin)];
            const uint32_t* iocon = ioconPointer(pin.pin);
            const uint32_t bitOutMask = 1 << digitalPinToPinNum(pin.pin);

            uint32_t pinPIOfunctionNumber = getDigitalPinFunctionNumber(pin.pin, PF_MOSI);

            pinMode(pin.pin, SPI_MOSI); // Set pin mode to input capture
            REQUIRE((port->DIR & bitOutMask) == 0); // port direction bit for pin NOT set

            REQUIRE(*iocon == pinPIOfunctionNumber); // IOCON_PIO_x_y set to correct pin function

            switch (pin.pin)
            {
                case PIO0_9: // has no location register
                    break;
                case PIO1_9:
                    REQUIRE(LPC_IOCON->MOSI1_LOC == 1);
                    savedRegister.MOSI1_LOC = 1;
                    break;
                case PIO2_3:
                    REQUIRE(LPC_IOCON->MOSI1_LOC == 0);
                    savedRegister.MOSI1_LOC = 0;
                    break;

                default:
                    FAIL("Unknown pin for SPI_MOSI.");
                    break;
            }

            requireAllIOConRegistersUnchanged(&savedRegister);
        }
    }
}

TEST_CASE("pinMode(pin, SPI_CLOCK)", "[digital_pin]")
{
    for (const PortPinInfo& pin : allPins)
    {
        if (getPinFunctionNumber(pin.pin, PF_SCK) < 0)
        {
            ///\todo it would be better, if pinMode had a return error value with [[nounused]]
            continue; // pin does not support PF_SCK, so continue
        }

        IOConRegister savedRegister{};
        readIOConRegisters(&savedRegister);

        const LPC_GPIO_TypeDef* port = gpioPorts[digitalPinToPort(pin.pin)];
        const uint32_t* iocon = ioconPointer(pin.pin);
        const uint32_t bitOutMask = 1 << digitalPinToPinNum(pin.pin);

        uint32_t pinPIOfunctionNumber = getDigitalPinFunctionNumber(pin.pin, PF_SCK);

        pinMode(pin.pin, SPI_CLOCK); // Set pin mode to input capture
        REQUIRE((port->DIR & bitOutMask) == 0); // port direction bit for pin NOT set

        REQUIRE(*iocon == pinPIOfunctionNumber); // IOCON_PIO_x_y set to correct pin function

        switch (pin.pin)
        {
            case PIO0_6:
                REQUIRE(LPC_IOCON->SCK_LOC == 2);
                savedRegister.SCK_LOC = 2;
                break;
            case PIO0_10:
                REQUIRE(LPC_IOCON->SCK_LOC == 0);
                savedRegister.SCK_LOC = 0;
                break;
            case PIO2_1:
                REQUIRE(LPC_IOCON->SCK1_LOC == 0);
                savedRegister.SCK1_LOC = 0;
                break;
            case PIO2_11:
                REQUIRE(LPC_IOCON->SCK_LOC == 1);
                savedRegister.SCK_LOC = 1;
                break;
            case PIO3_2:
                REQUIRE(LPC_IOCON->SCK1_LOC == 1);
                savedRegister.SCK1_LOC = 1;
                break;
            default:
                FAIL("Unknown pin for SPI_CLOCK.");
                break;
        }

        requireAllIOConRegistersUnchanged(&savedRegister);
    }
}

TEST_CASE("pinMode(pin, SPI_SSEL)", "[digital_pin]")
{
    for (const PortPinInfo& pin : allPins)
    {
        if (getPinFunctionNumber(pin.pin, PF_SSEL) < 0)
        {
            ///\todo it would be better, if pinMode had a return error value with [[nounused]]
            continue; // pin does not support PF_SSEL, so continue
        }

        IOConRegister savedRegister{};
        readIOConRegisters(&savedRegister);

        const LPC_GPIO_TypeDef* port = gpioPorts[digitalPinToPort(pin.pin)];
        const uint32_t* iocon = ioconPointer(pin.pin);
        const uint32_t bitOutMask = 1 << digitalPinToPinNum(pin.pin);

        uint32_t pinPIOfunctionNumber = getDigitalPinFunctionNumber(pin.pin, PF_SSEL);

        pinMode(pin.pin, SPI_SSEL); // Set pin mode to input capture
        REQUIRE((port->DIR & bitOutMask) == 0); // port direction bit for pin NOT set

        REQUIRE(*iocon == pinPIOfunctionNumber); // IOCON_PIO_x_y set to correct pin function

        switch (pin.pin)
        {
            case PIO0_2: // has no location register
                break;
            case PIO2_0:
                REQUIRE(LPC_IOCON->SSEL1_LOC == 0);
                savedRegister.SSEL1_LOC = 0;
                break;
            case PIO2_4:
                REQUIRE(LPC_IOCON->SSEL1_LOC == 1);
                savedRegister.SSEL1_LOC = 1;
                break;
            default:
                FAIL("Unknown pin for SPI_SSEL.");
                break;
        }

        requireAllIOConRegistersUnchanged(&savedRegister);
    }
}

TEST_CASE("pinMode(pin, INPUT | ...)", "[digital_pin]")
{
    // Bits 3:4 mode select
    //      0x00 no pull-down/pull-up
    //      0x01 pull-down
    //      0x10 pull-up
    //      0x11 repeater mode
    // Bit 5: hysteresis
    //      0x0 Disabled hysteresis
    //      0x1 Enabled hysteresis
    struct ModeTestCase
    {
        uint32_t mode;
        uint32_t expectedResult;
    };
    const std::vector<ModeTestCase> modeTestCases = {
        {(INPUT | OPEN_DRAIN), 0b000},
        {(INPUT | OPEN_DRAIN | HYSTERESIS), 0b100},
        {(INPUT | PULL_DOWN), 0b001},
        {(INPUT | PULL_DOWN | HYSTERESIS), 0b101},
        {(INPUT | PULL_UP), 0b010},
        {(INPUT | PULL_UP | HYSTERESIS), 0b110},
        {(INPUT | REPEATER_MODE), 0b011},
        {(INPUT | REPEATER_MODE | HYSTERESIS), 0b111},
    };

    for (const PortPinInfo& pin : allPins)
    {
        for (const auto& [mode, expectedResult] : modeTestCases)
        {
            constexpr uint32_t bitMask = 0b111;
            const uint32_t* iocon = ioconPointer(pin.pin);
            pinMode(pin.pin, mode);
            // ignore function bits 0:2
            REQUIRE(((*iocon >> 3) & bitMask) == expectedResult);
        }
    }
}

TEST_CASE("pinDirection(pin, INPUT / OUTPUT)", "[digital_pin]")
{
    for (const PortPinInfo& pin : allPins)
    {
        LPC_GPIO_TypeDef* port = gpioPorts[digitalPinToPort(pin.pin)];
        const uint8_t pinNumber = digitalPinToPinNum(pin.pin);
        const uint32_t bitTestMaskIsOutput = 1 << pinNumber;

        // Test pinMode set to output
        port->DIR = 0; // set all pins to input in port direction register
        pinMode(pin.pin, OUTPUT);
        REQUIRE(port->DIR == bitTestMaskIsOutput);

        // Test only pin.pin changes to INPUT
        constexpr uint32_t allPinsAsOutput = 0xffffffff; // 12 pins on a port, but also check bits 31:12 are unchanged
        port->DIR = allPinsAsOutput; // set all pins to output in port direction register
        pinDirection(pin.pin, INPUT);
        REQUIRE(port->DIR == (allPinsAsOutput & ~bitTestMaskIsOutput));

        // Test only pin.pin changes to OUTPUT
        port->DIR = 0; // set all pins to input in port direction register
        pinDirection(pin.pin, OUTPUT);
        REQUIRE(port->DIR == bitTestMaskIsOutput);

        // Test again only pin.pin changes to INPUT
        port->DIR = allPinsAsOutput; // set all pins to output in port direction register
        pinDirection(pin.pin, INPUT);
        REQUIRE(port->DIR == (allPinsAsOutput & ~bitTestMaskIsOutput));
    }
}

TEST_CASE("pinInterruptMode(pin, INTERRUPT_* | ...)", "[digital_pin]")
{
    struct InterruptModeTestCase
    {
        uint16_t interruptMode;
        bool isISset;  // Interrupt sense register (UM10398 12.3.3) 0 = edge sensitive, 1 = level sensitive
        bool isIBEset; // Interrupt both edges sense register (UM10398 12.3.4) 0 = controlled with IEV, 1 = both edges
        bool isIEVset; // Interrupt event register (UM10398 12.3.5) 0 = falling edge or low, 1 = rising edge or high
        bool isIEset;  // Interrupt mask register (UM10398 12.3.6) 0 = interrupt disabled, 1 = interrupt enabled
    };
    const std::vector<InterruptModeTestCase> modeTestCases = {
        // Nothing configured
        {0, false, false, false, false},

        // Level sensitive test cases
        {INTERRUPT_LEVEL_LOW, true, false, false, false},
        {INTERRUPT_LEVEL_HIGH, true, false, true, false},
        {(INTERRUPT_LEVEL_LOW | INTERRUPT_ENABLED), true, false, false, true},
        {(INTERRUPT_LEVEL_HIGH | INTERRUPT_ENABLED), true, false, true, true},

        // Edge sensitive test cases
        {INTERRUPT_EDGE_FALLING, false, false, false, false},
        {INTERRUPT_EDGE_RISING, false, false, true, false},
        {INTERRUPT_EDGE_BOTH, false, true, false, false},
        {(INTERRUPT_EDGE_FALLING | INTERRUPT_ENABLED), false, false, false, true},
        {(INTERRUPT_EDGE_RISING | INTERRUPT_ENABLED), false, false, true, true},
        {(INTERRUPT_EDGE_BOTH  | INTERRUPT_ENABLED), false, true, false, true},
    };

    for (const PortPinInfo& pin : allPins)
    {
        for (const InterruptModeTestCase testCase : modeTestCases)
        {
            // Reset port interrupt registers
            LPC_GPIO_TypeDef* port = gpioPorts[digitalPinToPort(pin.pin)];
            port->IS = 0;
            port->IBE = 0;
            port->IEV = 0;
            port->IE = 0;
            const uint8_t pinNumber = digitalPinToPinNum(pin.pin);

            // Simple test if nothing is configured
            pinInterruptMode(pin.pin, testCase.interruptMode);
            REQUIRE(port->IS == (static_cast<uint32_t>(testCase.isISset) << pinNumber));
            REQUIRE(port->IBE == (static_cast<uint32_t>(testCase.isIBEset) << pinNumber));
            REQUIRE(port->IEV == (static_cast<uint32_t>(testCase.isIEVset) << pinNumber));
            REQUIRE(port->IE == (static_cast<uint32_t>(testCase.isIEset) << pinNumber));

            const uint32_t testMask = 1 << pinNumber;
            constexpr uint32_t allPinsSet = 0xffffffff; // 12 pins on a port, but also check bits 31:12 are unchanged
            port->IS = allPinsSet;
            port->IBE = allPinsSet;
            port->IEV = allPinsSet;
            port->IE = allPinsSet;

            // Test correct configuration of pin.pin, if all other pins are already set
            pinInterruptMode(pin.pin, testCase.interruptMode);
            REQUIRE((port->IS & testMask) == (static_cast<uint32_t>(testCase.isISset) << pinNumber));
            REQUIRE((port->IBE & testMask) == (static_cast<uint32_t>(testCase.isIBEset) << pinNumber));
            REQUIRE((port->IEV & testMask) == (static_cast<uint32_t>(testCase.isIEVset) << pinNumber));
            REQUIRE((port->IE & testMask) == (static_cast<uint32_t>(testCase.isIEset) << pinNumber));

            // Ensure other pins are unchanged
            const uint32_t pinsUnchangedMask = allPinsSet & ~testMask;
            REQUIRE((port->IS & ~testMask) == pinsUnchangedMask);
            REQUIRE((port->IBE & ~testMask) == pinsUnchangedMask);
            REQUIRE((port->IEV & ~testMask) == pinsUnchangedMask);
            REQUIRE((port->IE & ~testMask) == pinsUnchangedMask);
        }
    }
}

TEST_CASE("pinEnableInterrupt(pin)", "[digital_pin]")
{
    for (const PortPinInfo& pin : allPins)
    {
        // Reset port interrupt registers
        LPC_GPIO_TypeDef* port = gpioPorts[digitalPinToPort(pin.pin)];
        port->IE = 0;
        const uint8_t pinNumber = digitalPinToPinNum(pin.pin);
        const uint32_t testMask = 1 << pinNumber;

        pinEnableInterrupt(pin.pin);
        REQUIRE(port->IE == testMask);

        constexpr uint32_t allPinsSet = 0xffffffff; // 12 pins on a port, but also check bits 31:12 are unchanged
        port->IE = allPinsSet;

        // Test correct configuration of pin.pin, if all other pins are already set
        pinEnableInterrupt(pin.pin);
        REQUIRE((port->IE & testMask) == testMask);

        // Ensure other pins are unchanged
        REQUIRE((port->IE & ~testMask) == (allPinsSet & ~testMask));
    }
}

TEST_CASE("pinDisableInterrupt(pin)", "[digital_pin]")
{
    for (const PortPinInfo& pin : allPins)
    {
        // Reset port interrupt registers
        LPC_GPIO_TypeDef* port = gpioPorts[digitalPinToPort(pin.pin)];
        port->IE = 0;
        const uint8_t pinNumber = digitalPinToPinNum(pin.pin);

        pinEnableInterrupt(pin.pin);
        pinDisableInterrupt(pin.pin);
        REQUIRE(port->IE == 0);

        pinDisableInterrupt(pin.pin); // ensure it stays disabled
        REQUIRE(port->IE == 0);

        // Test correct configuration of pin.pin, if all other pins are already set
        constexpr uint32_t allPinsSet = 0xffffffff; // 12 pins on a port, but also check bits 31:12 are unchanged
        port->IE = allPinsSet;

        const uint32_t testMask = 1 << pinNumber;
        pinDisableInterrupt(pin.pin);
        REQUIRE((port->IE & testMask) == 0);

        // Ensure other pins are unchanged
        REQUIRE((port->IE & ~testMask) == (allPinsSet & ~testMask));
    }
}

bool checkIOConfigRegister(const Port portNum, uint32_t pinMask, const uint16_t mode)
{
    // This is a stupid test,
    // because it executes the same logic as the actual function `portMode`
    const uint32_t toTestIOConfigRegister = mode & 0xfff;
    for (uint16_t pinNum = 0; pinMask != 0; ++pinNum, pinMask >>= 1)
    {
        if (pinMask & 1)
        {
            if (*(ioconPointer(portNum, pinNum)) != toTestIOConfigRegister)
            {
                return false;
            }
        }
    }
    return true;
}

TEST_CASE("portMode(...)", "[digital_pin]")
{
    for (const PortInfo& testPort : allPorts)
    {
        LPC_GPIO_TypeDef* port = gpioPorts[testPort.port];
        port->DIR = 0;
        for (uint32_t i = 0; i < 0x10000; i++)
        {
            uint32_t lastPortDIR = port->DIR;
            portMode(testPort.port, i, OUTPUT);
            REQUIRE(port->DIR == (lastPortDIR | i));
            REQUIRE(checkIOConfigRegister(testPort.port, i, OUTPUT) == true);

            lastPortDIR = port->DIR;
            portMode(testPort.port, i, INPUT);
            REQUIRE(port->DIR == (lastPortDIR & ~i));
            REQUIRE(checkIOConfigRegister(testPort.port, i, INPUT) == true);

            lastPortDIR = port->DIR;
            portMode(testPort.port, i, OUTPUT_MATCH);
            REQUIRE(port->DIR == (lastPortDIR | i));
            REQUIRE(checkIOConfigRegister(testPort.port, i, OUTPUT_MATCH) == true);
        }
    }
}

TEST_CASE("portDirection(...)", "[digital_pin]")
{
    for (const PortInfo& testPort : allPorts)
    {
        LPC_GPIO_TypeDef* port = gpioPorts[testPort.port];
        port->DIR = 0;
        for (uint32_t i = 0; i < 0x10000; i++)
        {
            uint32_t lastPortDIR = port->DIR;
            portDirection(testPort.port, i, OUTPUT);
            REQUIRE(port->DIR == (lastPortDIR | i));

            lastPortDIR = port->DIR;
            portDirection(testPort.port, i, INPUT);
            REQUIRE(port->DIR == (lastPortDIR & ~i));
        }
    }
}

TEST_CASE("digitalWrite(...)", "[digital_pin]")
{
    for (const PortPinInfo& pin : allPins)
    {
        // This is a stupid test,
        // because it executes mostly the same logic as the actual function `digitalWrite`
        const LPC_GPIO_TypeDef* port = gpioPorts[digitalPinToPort(pin.pin)];
        const uint32_t mask = digitalPinToBitMask(pin.pin);

        pinMode(pin.pin, OUTPUT);

        digitalWrite(pin.pin, false);
        REQUIRE(port->MASKED_ACCESS[mask] == false);
        digitalWrite(pin.pin, true);
        REQUIRE(port->MASKED_ACCESS[mask] == mask);
        digitalWrite(pin.pin, false);
        REQUIRE(port->MASKED_ACCESS[mask] == false);
    }
}

TEST_CASE("digitalRead(...)", "[digital_pin]")
{
    for (const PortPinInfo& pin : allPins)
    {
        pinMode(pin.pin, INPUT);

        digitalWrite(pin.pin, false);
        REQUIRE(digitalRead(pin.pin) == false);

        digitalWrite(pin.pin, true);
        REQUIRE(digitalRead(pin.pin) == true);

        digitalWrite(pin.pin, false);
        REQUIRE(digitalRead(pin.pin) == false);
    }
}

///\todo Find a way to implement tests for functions shiftOut(..), shiftIn(..) and pulseIn(..) of digital_pin.h