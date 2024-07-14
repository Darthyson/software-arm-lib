#include "LPC11xx.h"
#include <string.h>
#include "iap_emu.h"

SCB_Type           _SCB;
SysTick_Type       _SysTick;
NVIC_Type          _NVIC;

LPC_I2C_TypeDef    _LPC_I2C;
LPC_WDT_TypeDef    _LPC_WDT;
LPC_UART_TypeDef   _LPC_UART;
LPC_TMR_TypeDef    _LPC_TMR16B0;
LPC_TMR_TypeDef    _LPC_TMR16B1;
LPC_TMR_TypeDef    _LPC_TMR32B0;
LPC_TMR_TypeDef    _LPC_TMR32B1;
LPC_ADC_TypeDef    _LPC_ADC;
LPC_PMU_TypeDef    _LPC_PMU;
LPC_FLASHCTRL_Type _LPC_FLASHCTRL;
LPC_SSP_TypeDef    _LPC_SSP0;
LPC_SSP_TypeDef    _LPC_SSP1;
LPC_CAN_TypeDef    _LPC_CAN;
LPC_IOCON_TypeDef  _LPC_IOCON;
LPC_SYSCON_TypeDef _LPC_SYSCON;
LPC_GPIO_TypeDef   _LPC_GPIO0;
LPC_GPIO_TypeDef   _LPC_GPIO1;
LPC_GPIO_TypeDef   _LPC_GPIO2;
LPC_GPIO_TypeDef   _LPC_GPIO3;


// Flash emulation array
unsigned char FLASH[FLASH_SIZE];

// System core clock
uint32_t SystemCoreClock = 48000000;
unsigned int wfiSystemTimeInc = 0;


typedef enum
{
    IAP_PREPARE = 50,         // Prepare sector(s) for write
    IAP_COPY_RAM2FLASH,     // Copy RAM to Flash
    IAP_ERASE,                 // Erase sector(s)
    IAP_BLANK_CHECK,         // Blank check sector(s)
    IAP_READ_PART_ID,         // Read chip part ID
    IAP_READ_BOOT_VER,         // Read chip boot code version
    IAP_COMPARE,             // Compare memory areas
    IAP_REINVOKE_ISP,         // Reinvoke ISP
    IAP_READ_UID,             // Read unique ID
    IAP_ERASE_PAGE             // Erase page(s)
} IAP_Commands;

typedef enum
{
    CMD_SUCCESS,
    INVALID_COMMAND,
    SRC_ADDR_ERROR,
    DST_ADDR_ERROR,
    SRC_ADDR_NOT_MAPPED,
    DST_ADDR_NOT_MAPPED,
    COUNT_ERROR,
    INVALID_SECTOR,
    SECTOR_NOT_BLANK,
    SECTOR_NOT_PREPARED_FOR_WRITE_OPERATION,
    COMPARE_ERROR,
    BUSY
} IAP_Status;

int iap_calls [6] = {0, 0, 0, 0, 0, 0};

void IAP_Init_Flash(unsigned char value)
{
    memset(FLASH, value, FLASH_SIZE);
}

void IAP_Call (uintptr_t * cmd, uintptr_t * stat)
{
    unsigned int i;
    unsigned int end;
    uint8_t * rom;
    uint8_t * ram;

    * stat = CMD_SUCCESS;
    switch (* cmd)
    {
    case IAP_PREPARE :
        iap_calls [I_PREPARE]++;
        break;
    case IAP_ERASE :
        iap_calls [I_ERASE]++;
        i    =  * (cmd + 1)      * SECTOR_SIZE;
        end  = (* (cmd + 2) + 1) * SECTOR_SIZE;
        for (; i < end; i++)
        {
            FLASH [i] = 0xFF;
        }
        break;
    case IAP_BLANK_CHECK :
        iap_calls [I_BLANK_CHECK]++;
        i    =  * (cmd + 1)      * SECTOR_SIZE;
        end  = (* (cmd + 2) + 1) * SECTOR_SIZE;
        if (i >= end)
        {
            * stat = INVALID_COMMAND;
            break;
        }
        for (; i < end; i++)
        {
            if (i >= FLASH_SIZE)
            {
                * stat = INVALID_SECTOR;
                break;
            }
            if (FLASH [i] != 0xFF)
            {
                * stat = SECTOR_NOT_BLANK;
                break;
            }
        }
        break;
    case IAP_COPY_RAM2FLASH :
        iap_calls [I_RAM2FLASH]++;
        rom = (uint8_t *) (* (cmd + 1));
        ram = (uint8_t *) (* (cmd + 2));
        i   = * (cmd + 3);
        memcpy (rom, ram, i);
        break;
    case IAP_COMPARE :
        iap_calls [I_COMPARE]++;
        rom = (uint8_t *) (* (cmd + 1));
        ram = (uint8_t *) (* (cmd + 2));
        i   = * (cmd + 3);
        if (memcmp (rom, ram, i) != 0)
        {
            * stat = COMPARE_ERROR;
        }
        break;

    case IAP_READ_UID:
    {
        iap_calls [I_READ_UID]++;
        uint8_t guid[16] = {0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16};
        cmd += 6; //points now to p.res
        memcpy(cmd, &guid, 16);
        break;
    }

    default:
        * stat = INVALID_COMMAND;
    }
}

extern unsigned int millis();
extern void setMillis(unsigned int newSystemTime);

void _test_wfi(void)
{
    setMillis(millis() + wfiSystemTimeInc);
}
