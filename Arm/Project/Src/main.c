/**
 ***********************************************************************************************************************
 * \file        main.c
 * \author      Kirill Gribovskiy
 * \version     V 1.0
 * \date        30-Sep-2025
 * \brief       Main program source
 * \details     Clock initialization and main cycle
 *
 ***********************************************************************************************************************
**/
/**
 ***********************************************************************************************************************
 * Include section
 ***********************************************************************************************************************
**/
// System includes
#include <string.h>
#include <stdbool.h>

// Device library includes
#include "stm32h7xx_hal.h"


// Local includes
#include "bd_leds.h"
#include "bd_uart.h"
#include "usbd_def.h"
#include "usbd_init.h"
#include "lib_terminal.h"
#include "bd_usbexch.h"
#include "usbd_bd.h"
/**
 ***********************************************************************************************************************
 * Definitions
 ***********************************************************************************************************************
**/
/**
 ***********************************************************************************************************************
 * Types
 ***********************************************************************************************************************
**/
/**
 ***********************************************************************************************************************
 * Variables
 ***********************************************************************************************************************
**/
static  RCC_OscInitTypeDef          RCC_OscInitStruct = {0};
static  RCC_ClkInitTypeDef          RCC_ClkInitStruct = {0};
static  MPU_Region_InitTypeDef      MPU_InitStruct = {0};

/**
 ***********************************************************************************************************************
 * Function prototypes
 ***********************************************************************************************************************
**/
void MPU_Config(void);
void SystemClock_Config(void);
void Error_Handler(void);

/**
 ***********************************************************************************************************************
 * \brief   main
 * \details Program initialisation sequence and main cycle.
 ***********************************************************************************************************************
**/
int main(void)
{
    MPU_Config();

    SCB_EnableICache();
    SCB_EnableDCache();

    HAL_Init();

    SystemClock_Config();

    //SCB_CleanInvalidateDCache();
    
    initLeds();
    setLedState(dbgLedOff);
    initDbgUart();
    setTerminalMessage(info, "Primary initialization done");
 
    setLedState(dbgLedOn);

    // int dec = 1234567;
    // int hex = 0x1234567;
    // float flot = 12.345678;
    // setTerminalMessage(info, "Test dec:   %d", dec);
    // setTerminalMessage(info, "Test hex:   %x", hex);
    // setTerminalMessage(info, "Test hex:   %X", hex);
    // setTerminalMessage(info, "float = 12.345678");
    // setTerminalMessage(info, "float address: %x", &flot);
    // setTerminalMessage(info, "Test float: %f", flot);
    // setTerminalMessage(info, "Test float: %f", -45.666);



    initUsbDevice();
    setTerminalMessage(info, "USB started");

    for(;;) {
        if(true == newHostCommand()) {
            setTerminalMessage(info, "received new usb packet");
            uint16_t dataSz = getRcvHostDataSz();
            uint8_t *dataPtr = getRcvHostData();
            for(int i = 0; i < dataSz; i++) {
                setTerminalMessage(info, "received byte %d: %02x", i, dataPtr[i]);
            }
            setTerminalMessage(info, "sending usb packet");
            uint8_t testData[3] = {3,4,5};

            trnUsbData(3, testData, true, BD_EPIN_ADDR);
        }
    }
}

/**
 ***********************************************************************************************************************
 * \brief   MPU_Config
 * \details Memory progtection unit.
 ***********************************************************************************************************************
**/
void MPU_Config(void)
{
   memset(&MPU_InitStruct, 0, sizeof(MPU_Region_InitTypeDef));
/* 
    __HAL_RCC_D2SRAM1_CLK_ENABLE();
    __HAL_RCC_D2SRAM2_CLK_ENABLE();
    __HAL_RCC_D2SRAM3_CLK_ENABLE();
 */    
    //__HAL_RCC_D3SRAM1_CLK_ENABLE();
    //__HAL_RCC_D3SRAM1_CLK_ENABLE();

    //LL_AHB4_GRP1_EnableClock(LL_AHB4_GRP1_PERIPH_SRAM4);


    // Disables the MPU
    HAL_MPU_Disable();

    // Initializes and configures the Region and the memory to be protected
    MPU_InitStruct.Enable = MPU_REGION_ENABLE;
    MPU_InitStruct.Number = MPU_REGION_NUMBER0;
    MPU_InitStruct.BaseAddress = 0x0;
    MPU_InitStruct.Size = MPU_REGION_SIZE_4GB;
    MPU_InitStruct.SubRegionDisable = 0x87;
    MPU_InitStruct.TypeExtField = MPU_TEX_LEVEL0;
    MPU_InitStruct.AccessPermission = MPU_REGION_NO_ACCESS;
    MPU_InitStruct.DisableExec = MPU_INSTRUCTION_ACCESS_DISABLE;
    MPU_InitStruct.IsShareable = MPU_ACCESS_SHAREABLE;
    MPU_InitStruct.IsCacheable = MPU_ACCESS_NOT_CACHEABLE;
    MPU_InitStruct.IsBufferable = MPU_ACCESS_NOT_BUFFERABLE;
    HAL_MPU_ConfigRegion(&MPU_InitStruct);

    MPU_InitStruct.Number = MPU_REGION_NUMBER1;
    MPU_InitStruct.BaseAddress = 0x30000000;
    MPU_InitStruct.Size = MPU_REGION_SIZE_256KB;
    MPU_InitStruct.SubRegionDisable = 0x00;
    MPU_InitStruct.TypeExtField = MPU_TEX_LEVEL0;
    MPU_InitStruct.AccessPermission = MPU_REGION_FULL_ACCESS;
    MPU_InitStruct.DisableExec = MPU_INSTRUCTION_ACCESS_ENABLE;
    MPU_InitStruct.IsShareable = MPU_ACCESS_SHAREABLE; //MPU_ACCESS_NOT_SHAREABLE; //MPU_ACCESS_SHAREABLE;
    MPU_InitStruct.IsCacheable = MPU_ACCESS_NOT_CACHEABLE;
    MPU_InitStruct.IsBufferable = MPU_ACCESS_BUFFERABLE; //MPU_ACCESS_NOT_BUFFERABLE; //MPU_ACCESS_BUFFERABLE;
    HAL_MPU_ConfigRegion(&MPU_InitStruct);

    MPU_InitStruct.Number = MPU_REGION_NUMBER2;
    MPU_InitStruct.BaseAddress = 0x38000000;
    MPU_InitStruct.Size = MPU_REGION_SIZE_64KB;
    MPU_InitStruct.SubRegionDisable = 0x00;
    MPU_InitStruct.TypeExtField = MPU_TEX_LEVEL1;
    MPU_InitStruct.AccessPermission = MPU_REGION_FULL_ACCESS;
    MPU_InitStruct.DisableExec = MPU_INSTRUCTION_ACCESS_ENABLE;
    MPU_InitStruct.IsShareable = MPU_ACCESS_SHAREABLE;
    MPU_InitStruct.IsCacheable = MPU_ACCESS_NOT_CACHEABLE;
    MPU_InitStruct.IsBufferable = MPU_ACCESS_BUFFERABLE; //MPU_ACCESS_NOT_BUFFERABLE;
    HAL_MPU_ConfigRegion(&MPU_InitStruct);

    // Enables the MPU
    HAL_MPU_Enable(MPU_PRIVILEGED_DEFAULT);
}


/**
 ***********************************************************************************************************************
 * \brief   SystemClock_Config
 * \details System clock configuration.
 * input clock - 25 MHz. 25 / 5 * 192 / 2 = 480MHz core clock
 ***********************************************************************************************************************
**/
void SystemClock_Config(void)
{
    // Supply configuration update enable
    HAL_PWREx_ConfigSupply(PWR_LDO_SUPPLY);

    // Configure the main internal regulator output voltage
    __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE0);
    while(!__HAL_PWR_GET_FLAG(PWR_FLAG_VOSRDY)); // {}

    // Initializes the RCC Oscillators according to the specified parameters
    // in the RCC_OscInitTypeDef structure.
    RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
    RCC_OscInitStruct.HSEState = RCC_HSE_ON;
    RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
    RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
    RCC_OscInitStruct.PLL.PLLM = 5;
    RCC_OscInitStruct.PLL.PLLN = 192;
    RCC_OscInitStruct.PLL.PLLP = 2;
    RCC_OscInitStruct.PLL.PLLQ = 2;
    RCC_OscInitStruct.PLL.PLLR = 2;
    RCC_OscInitStruct.PLL.PLLRGE = RCC_PLL1VCIRANGE_2;
    RCC_OscInitStruct.PLL.PLLVCOSEL = RCC_PLL1VCOWIDE;
    RCC_OscInitStruct.PLL.PLLFRACN = 0;
    if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK) {
        Error_Handler();
    }

    // Initializes the CPU, AHB and APB buses clocks
    RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                            |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2
                            |RCC_CLOCKTYPE_D3PCLK1|RCC_CLOCKTYPE_D1PCLK1;
    RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
    RCC_ClkInitStruct.SYSCLKDivider = RCC_SYSCLK_DIV1;
    RCC_ClkInitStruct.AHBCLKDivider = RCC_HCLK_DIV2;
    RCC_ClkInitStruct.APB3CLKDivider = RCC_APB3_DIV2;
    RCC_ClkInitStruct.APB1CLKDivider = RCC_APB1_DIV2;
    RCC_ClkInitStruct.APB2CLKDivider = RCC_APB2_DIV2;
    RCC_ClkInitStruct.APB4CLKDivider = RCC_APB4_DIV2;

    if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_4) != HAL_OK) {
        Error_Handler();
    }

    /// test output clock PA8 MCO1; 480 / 12 = 40MHz
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    GPIO_InitStruct.Pin = GPIO_PIN_8;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    GPIO_InitStruct.Alternate = GPIO_AF0_MCO;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

    __HAL_RCC_PLLCLKOUT_ENABLE(RCC_PLL1_DIVQ);
    HAL_RCC_MCOConfig(RCC_MCO1, RCC_MCO1SOURCE_PLL1QCLK, RCC_MCODIV_12);
}

/**
 ***********************************************************************************************************************
 * \brief    Error_Handler
 * \details  Program halt
 ***********************************************************************************************************************
**/
void Error_Handler(void)
{
    __disable_irq();
    while (1);
}

