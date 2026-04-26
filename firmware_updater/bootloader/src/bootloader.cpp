/**************************************************************************//**
 * @addtogroup SBLIB_BOOTLOADER Selfbus Bootloader
 * @addtogroup SBLIB_BOOTLOADER_MAIN Bootloader
 * @ingroup SBLIB_BOOTLOADER
 * @brief   Bootloader main program
 * @details
 *
 * @{
 *
 * @file   bootloader.cpp
 * @author Martin Glueck <martin@mangari.org> Copyright (c) 2015
 * @author Stefan Haller Copyright (c) 2021
 * @author Darthyson <darth@maptrack.de> Copyright (c) 2021
 * @bug No known bugs.
 ******************************************************************************/

/*
 This program is free software; you can redistribute it and/or modify
 it under the terms of the GNU General Public License version 3 as
 published by the Free Software Foundation.
 -----------------------------------------------------------------------------*/

#include "boot_descriptor_block.h"
#include "bcu_updater.h"
#include "dump.h"
#include <sblib/main.h>
#include <sblib/interrupt.h>
#include <sblib/io_pin_names.h>
#include <sblib/digital_pin.h>
#include <sblib/hardware_descriptor.h>
#include <sblib/eib/knx_lpdu.h>
#include <sblib/internal/bootloader_commands.h>
#include <cstring>

#ifdef DEBUG
#   include "flash.h"
#   include "version.h"
#   include <sblib/serial.h>
#   include <sblib/version.h>
#   include <sblib/bits.h>
#endif


// bootloader specific settings
constexpr uint16_t RUN_MODE_BLINK_CONNECTED_MS = 250; //!< while connected, programming and run led blinking time in milliseconds
constexpr uint16_t RUN_MODE_BLINK_IDLE_MS = 1000;     //!< while idle/disconnected, programming and run led blinking time in milliseconds
constexpr uint32_t BL_RESERVED_RAM_START = 0x10000000UL;       //!< RAM start address for bootloader
constexpr uint8_t BL_DEFAULT_VECTOR_TABLE_SIZE_IN_BYTES = 192; //!< vector table size in bytes
constexpr uint8_t BL_DEFAULT_VECTOR_TABLE_COUNT = BL_DEFAULT_VECTOR_TABLE_SIZE_IN_BYTES / sizeof(uintptr_t); //!< vector count to copy prior application start

constexpr uint16_t APP_START_DELAY_MS = 250; //!< Time in milliseconds the programming LED will light before the app is started

BcuUpdate bcu; //!< @ref BcuUpdate instance used for bus communication of the bootloader

Timeout runModeTimeout; //!< running mode LED blinking timeout
bool blinky = false;

const BootloaderDescriptor* startup();

uint32_t getProgrammingButton()
{
#ifdef GNAX_IO16FM_PROGRAMMING_BUTTON
    return PIO2_11;
#elif defined ALTERNATIVE_PROGRAMMING_BUTTON
    return PIO2_8;
#else
    return hwPinProgButton();
#endif
}

/**
 * @brief Bootloader setup function.
 *        Initializes the bcu and configures the programming button and physical address.
 *        If a valid BootloaderDescriptor is found in RAM, its parameters are used.
 *        Otherwise, default values are used.
 * @note  In debug build also initializes the serial port for debugging output and
 *        sets the @ref PIN_INFO and @ref PIN_RUN LEDs to output.
 * @return Pointer to initialized BcuBase instance
 */
BcuBase* setup()
{
// setup LED's for debug
#if defined(DEBUG) && (!(defined(TS_ARM)))
    pinMode(PIN_INFO, OUTPUT);
    digitalWrite(PIN_INFO, false);  // Turn off info LED
    pinMode(PIN_RUN, OUTPUT);
#endif

// if sblib didn't configure the serial port -> set it up for debugging
#if !defined(INCLUDE_SERIAL) && defined(DEBUG)
#   ifdef TS_ARM
        serial.setRxPin(PIO3_1);
        serial.setTxPin(PIO3_0);
#   else
        serial.setRxPin(PIO1_6); // on swd connector
        serial.setTxPin(PIO1_7); // on swd connector
#   endif
    if (!serial.enabled())
    {
        serial.begin(SERIAL_BAUD_RATE_115200);
    }
#endif

    uint16_t physAddr;
    uint32_t progButton;
    const BootloaderDescriptor* blDescriptor = startup();
    if (blDescriptor != nullptr)
    {
        physAddr = blDescriptor->physicalAddress;
        progButton = blDescriptor->programmingButton;
        ///\todo After reset the application will be started (old behavior).
        ///      We need to find the right place to clear the BootloaderDescriptor.
        ///      Here it's too early. Best place would be, if we are 100% sure that the application is startable.
        ///      But keep in mind, that a simple UPD_REQUEST_UID can also restart the bootloader.
        clearBootloaderDescriptor();
    }
    else
    {
        physAddr = DEFAULT_BL_KNX_ADDRESS;
        progButton = getProgrammingButton();
    }
    bcu.setOwnAddress(physAddr);
    bcu.setProgPin(progButton);
    runModeTimeout.start(1);

    // finally start the bcu
    bcu.begin();

    dump(
        serial.print("Selfbus KNX Bootloader v", BOOTLOADER_MAJOR_VERSION);
        serial.print(".", BOOTLOADER_MINOR_VERSION, DEC);
        serial.println(" DEBUG MODE :-)");
        serial.print("Build                       : ");
        serial.print(__DATE__);
        serial.print(" ");
        serial.println(__TIME__);
        serial.print("Library                     : v", highByte(static_cast<uint16_t>(SBLIB_VERSION)), HEX); // lib version is in hexadecimal
        serial.println(".", lowByte(SBLIB_VERSION), HEX);
        serial.println("Features                    : 0x", BL_FEATURES, HEX);
        serial.println("bootLoaderDescriptor addr.  : 0x", reinterpret_cast<uintptr_t>(debugOnlyBootloaderDescriptor()), HEX);
        serial.print("Flash      (start,end,size) : 0x", flashFirstAddress());
        serial.print(" 0x", flashLastAddress());
        serial.println(" 0x", flashSize(), HEX);
        serial.print("Bootloader (start,end,size) : 0x", bootLoaderFirstAddress());
        serial.print(" 0x", bootLoaderLastAddress());
        serial.println(" 0x", bootLoaderSize(), HEX);
        serial.println("Firmware (start)            : 0x", applicationFirstAddress());
        serial.println("Boot descriptor (start)     : 0x", bootDescriptorBlockAddress());
        serial.println("Boot descriptor page        : 0x", bootDescriptorBlockPage(), HEX);
        serial.println("Boot descriptor size        : 0x", BOOT_BLOCK_DESC_SIZE, HEX);
        uint16_t physicalAddress = bcu.ownAddress();
        serial.print("physical address            : ");
        serial.print(physAddressToArea(physicalAddress));
        serial.print(".", physAddressToLine(physicalAddress));
        serial.println(".", physAddressToDevice(physicalAddress));
        serial.println();
    )

    return &bcu;
}

/**
 * Handles LED status.
 */
void loop()
{
    if (runModeTimeout.expired())
    {
        if (bcu.directConnection())
        {
            runModeTimeout.start(RUN_MODE_BLINK_CONNECTED_MS);
        }
        else
        {
#if defined(DEBUG) && (!(defined(TS_ARM)))
            digitalWrite(PIN_INFO, false);  // Turn Off info LED
#endif
            runModeTimeout.start(RUN_MODE_BLINK_IDLE_MS);
        }
        blinky = !blinky;
    }

    digitalWrite(getProgrammingButton(), blinky);

#if defined(DEBUG) && (!(defined(TS_ARM)))
    digitalWrite(PIN_RUN, blinky);
#endif
}

/**
 * The processing loop while no KNX-application is loaded.
 * @note This function is required by the sblib but not used in the bootloader.
 */
void loop_noapp()
{
}

/**
 * Restores MCU and register changes made by the bootloader (e.g. sysTick).
 */
static void finalize()
{
    Timeout ledTimeout; // don't use delay(), it needs nearly 100% more flash
    pinMode(getProgrammingButton(), OUTPUT);
    digitalWrite(getProgrammingButton(), false);
    ledTimeout.start(APP_START_DELAY_MS);
    while (!ledTimeout.expired())
    {
        waitForInterrupt();
    }
    SysTick->CTRL = 0; // disable sysTick, otherwise other apps may fail to start (e.g. bootloaderupdater)
}

/**
 * Transfers control from bootloader to the application.
 * @details This function performs the critical operation of starting the user application
 *          by setting up the vector table and jumping to the application's reset handler.
 *          The process involves:
 *          1. Finalizing bootloader state (LEDs, timers)
 *          2. Copying the application's vector table from flash to RAM
 *          3. Remapping memory so RAM vectors are used instead of flash
 *          4. Loading the application's stack pointer
 *          5. Jumping to the application's reset vector
 * 
 * @param applicationStartAddress Pointer to the start address of the application in flash. 
 * @note This function does not return. Control is transferred to the application.
 */
[[noreturn]] static void jumpToApplication(uint8_t * applicationStartAddress)
{
    finalize(); // restore changes made and turn the programming led on
    const auto* rom = reinterpret_cast<const uint32_t*>(applicationStartAddress);
    auto* ram = reinterpret_cast<uint32_t*>(BL_RESERVED_RAM_START);
    uint32_t StackTop = rom[0];
    uint32_t ResetVector = rom[1];

    dump(
        serial.print("Vectortable size: ", BL_DEFAULT_VECTOR_TABLE_SIZE_IN_BYTES);
        serial.println(" bytes, #Vectors: ", BL_DEFAULT_VECTOR_TABLE_COUNT);
        serial.flush();
    );

    // copy the first 192 bytes (vector table) of the application
    // into the RAM and then remap the vector table inside the RAM
    memcpy(ram, rom, BL_DEFAULT_VECTOR_TABLE_SIZE_IN_BYTES);

    LPC_SYSCON->SYSMEMREMAP = 0x01;
    // DO NOT use a __DSB here, even if the user manual UM10398 28.4.2.4 states it, otherwise application won't start.

    // Normally during RESET the stack pointer will be loaded
    // with the value stored at location 0x0. Since the vector
    // table of the application is not located at 0x0 we have to do this
    // manually to ensure a correct stack.
    asm volatile ("mov SP, %0" : : "r" (StackTop));
    /* Once the stack is set up we jump to the application reset vector */
    asm volatile ("bx      %0" : : "r" (ResetVector));
    __builtin_unreachable(); // Tell compiler this point is never reached
}

/**
 * Determines the startup mode of the bootloader.
 * @details This function implements the startup logic with the following priority:
 *          1. Checks if a BootloaderDescriptor was set by the application to request updater mode
 *          2. Checks if the programming button is pressed at power-up to enter updater mode
 *          3. Attempts to start the main application if it exists and is valid
 *          4. Falls back to updater mode if no valid application is found
 * 
 * @return Pointer to BootloaderDescriptor if updater mode was requested by application, 
 *         nullptr if updater mode should be entered (button pressed or no valid application),
 *         or does not return if a valid application is started (jumps to application)
 * 
 * @note If a valid application is found, this function does not return and instead 
 *       transfers control to the application via @ref jumpToApplication().
 */
const BootloaderDescriptor* startup()
{
    dump(serial.print("Bootloader startup -> ");)
    // Updater request from application by setting BootloaderDescriptor
    const BootloaderDescriptor* blDescriptor = getBootloaderDescriptor();
    if (blDescriptor != nullptr)
    {
        dump(serial.println("BootloaderDescriptor valid");)
        return blDescriptor;
    }

    // Enter Updater when programming button was pressed at power up
    pinMode(getProgrammingButton(), INPUT | PULL_UP);
    if (!digitalRead(getProgrammingButton()))
    {
        dump(serial.println("Programming Button pressed");)
        return nullptr;
    }

    // Start main application at address
    const auto* block = reinterpret_cast<const AppDescriptionBlock*>(bootDescriptorBlockAddress());
    if (checkApplication(block))
    {
        dump(serial.println("Application valid");)
        jumpToApplication(block->startAddress);
    }
    // Start updater in case of error
    dump(serial.println("Application INVALID");)
    return nullptr;
}

/** @}*/
