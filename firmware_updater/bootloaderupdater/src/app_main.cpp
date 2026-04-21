#include <sblib/internal/iap.h>
#include <sblib/io_pin_names.h>
#include <sblib/digital_pin.h>
#include <sblib/version.h>
#include <sblib/internal/bootloader_commands.h>
#include <sblib/platform.h>
#include <cstdint>
#include <cstring>

#ifdef DEBUG
#   include <sblib/serial.h>
#endif

#ifdef DEBUG
#   define dump(x) {x}
#else
#   define dump(x)
#endif

// Remember to change build-variable sw_version in the .cproject file
constexpr uint8_t BOOTLOADERUPDATER_MAJOR_VERSION = 1;  //!< BootloaderUpdater major version @note change also in @ref APP_VERSION
constexpr uint8_t BOOTLOADERUPDATER_MINOR_VERSION = 20; //!< BootloaderUpdater minor Version @note change also in @ref APP_VERSION

// changes of the app version string must also be done in BootloaderUpdater.java of the Selfbus-Updater
APP_VERSION("SBblu   ", "1", "20");

extern const __attribute__((aligned(16))) uint8_t incbin_bl_start[];
extern const uint8_t incbin_bl_end[];

uint32_t gpioProgButton = PIN_PROG;

void setup()
{
    dump(
        if (!serial.enabled())
        {
            serial.setRxPin(PIO1_6);
            serial.setTxPin(PIO1_7);
            serial.begin(SERIAL_BAUD_RATE_115200);
        }
        serial.println();
        serial.print("Selfbus BootloaderUpdater v", BOOTLOADERUPDATER_MAJOR_VERSION);
        serial.println(".", BOOTLOADERUPDATER_MINOR_VERSION);
        serial.print("Build: ");
        serial.print(__DATE__);
        serial.print(" ");
        serial.println(__TIME__);
        serial.flush();
    );

    // Check for BootloaderDescriptor in RAM
    const BootloaderDescriptor* blDescriptor = getBootloaderDescriptor();

    dump(serial.print("BootloaderDescriptor ");)
    if (blDescriptor != nullptr)
    {
        dump(serial.println("valid");)
        gpioProgButton = blDescriptor->programmingButton;
    }
    else
    {
        dump(
            serial.println("INVALID");
            serial.flush();
            const BootloaderDescriptor* debugOnlyDescriptor = debugOnlyBootloaderDescriptor();
            serial.println("debugOnlyDescriptor 0x", &debugOnlyDescriptor);
            if (debugOnlyDescriptor != nullptr)
            {
                serial.println("bootState 0x", debugOnlyDescriptor->bootState, HEX);
                serial.println("reserved 0x", debugOnlyDescriptor->reserved, HEX);
                serial.println("physicalAddress 0x", debugOnlyDescriptor->physicalAddress, HEX);
                serial.println("progButton 0x", debugOnlyDescriptor->programmingButton, HEX);
                serial.println("appId 0x", debugOnlyDescriptor->applicationId, HEX);
                serial.println("appVersion 0x", debugOnlyDescriptor->applicationVersion, HEX);
            }
        )
        gpioProgButton = PIN_PROG;
    }
    pinMode(gpioProgButton, OUTPUT);
    digitalWrite(gpioProgButton, false);
}

void SystemReset()
{
    dump(
        serial.println("RESET");
        serial.flush();
    )
    NVIC_SystemReset();
}

int main()
{
    setup();

    const uint32_t newBlSize = incbin_bl_end - incbin_bl_start;
    const uint8_t* newBlEndAddress = FLASH_BASE_ADDRESS + newBlSize - 1;
    const uint32_t newBlStartSector = iapSectorOfAddress(FLASH_BASE_ADDRESS);
    const uint32_t newBlEndSector = iapSectorOfAddress(newBlEndAddress);

    dump(
        serial.println("newBlSize: 0x", newBlSize, HEX, 4);
        serial.print("Erasing Sectors: ", newBlStartSector);
        serial.print(" - ", newBlEndSector);
    )

    if (iapEraseSectorRange(newBlStartSector, newBlEndSector) != IAP_SUCCESS)
    {
        dump(serial.println(" --> FAILED");)
        SystemReset();
    }

    dump(serial.println(" done");)

    for (const uint8_t * i = incbin_bl_start; i < incbin_bl_end; i += FLASH_SECTOR_SIZE)
    {
        alignas(FLASH_RAM_BUFFER_ALIGNMENT) uint8_t buf[FLASH_SECTOR_SIZE]; // Address of buf must be word aligned, see iapProgram(..) hint.
        memset(buf, 0xFF, FLASH_SECTOR_SIZE);
        uint32_t len = incbin_bl_end - i;
        if (len > FLASH_SECTOR_SIZE)
        {
            len = FLASH_SECTOR_SIZE;
        }
        memcpy(buf, i, len);

        uint8_t * flash = FLASH_BASE_ADDRESS + (i - incbin_bl_start);

        if (flash == nullptr)
        {
            // NXP bootloader uses an Int-Vect as a checksum to see if the application is valid.
            // If the value is not correct, then it does not start the application
            // Vector table starts always at base address. Each entry is 4 bytes.
            uint32_t checksum = 0;
            for (uint8_t j = 0; j < 7; j++) // Checksum is 2's complement of entries 0 through 6
            {
                checksum += *(int*)&buf[j*4];
            }
            checksum = -checksum;
            *(int*)&buf[28] = checksum;
            dump(serial.println("checksum: 0x", checksum, HEX);)
        }

        dump(
            serial.print("flashing 0x", flash);
            serial.print(" - ", flash + FLASH_SECTOR_SIZE - 1);
        )
        if (iapProgram(flash, buf, FLASH_SECTOR_SIZE) != IAP_SUCCESS)
        {
            dump(serial.println(" --> FAILED");)
            SystemReset();
        }
        dump(serial.println(" done");)
        digitalWrite(gpioProgButton, !digitalRead(gpioProgButton));
    }

    // Make sure that the current boot descriptor of the BLU is erased,
    // otherwise the BL will restart the BLU in an endless loop.
    const uint32_t bootDescriptorBlockPage = iapPageOfAddress(newBlEndAddress + FLASH_PAGE_SIZE);
    dump(serial.print("Erasing BootDescriptorPage: 0x", bootDescriptorBlockPage, HEX);)
    if (iapErasePageRange(bootDescriptorBlockPage, bootDescriptorBlockPage) != IAP_SUCCESS)
    {
        dump(serial.println(" --> FAILED");)
    }
    else
    {
        dump(serial.println(" done");)
        BootloaderDescriptor();
    }

    SystemReset();
}
