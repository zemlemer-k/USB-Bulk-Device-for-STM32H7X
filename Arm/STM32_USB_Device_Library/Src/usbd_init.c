/**
 ***********************************************************************************************************************
 * \file        bd_usb_init.c
 * \author      Kirill Gribovskiy
 * \version     V 1.0
 * \date        24-Okt-2025
 * \brief       USB initialization listing
 * \details     USB pins initialization, device registration and starting
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

// Device library includes
#include "stm32h7xx_hal.h"
#include "stm32h7xx_hal_rcc.h"
#include "usbd_def.h"
#include "usbd_core.h"
#include "usbd_bd.h"

// Local includes
#include "usbd_config.h"
#include "usbd_desc.h"
#include "usbd_conf.h"
#include "usbd_init.h"
//#include "lib_terminal.h"

/**
 ***********************************************************************************************************************
 * Definitions
 ***********************************************************************************************************************
**/

#define USB_INT_PRIORITY    0
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
USBD_HandleTypeDef  hUsbDevice;

/**
 ***********************************************************************************************************************
 * Function prototypes
 ***********************************************************************************************************************
**/
static int initUsbDevicePins(void);

/**
 ***********************************************************************************************************************
 * @brief  Pins and interrupt initialization``
 * @param  None
 * @retval None
 ***********************************************************************************************************************
**/
static int initUsbDevicePins(void)
{
    GPIO_InitTypeDef GPIO_InitStruct;
    RCC_PeriphCLKInitTypeDef PeriphClkInitStruct;
    memset(&GPIO_InitStruct, 0, sizeof(GPIO_InitTypeDef));
    memset(&PeriphClkInitStruct, 0, sizeof(RCC_PeriphCLKInitTypeDef));

    // Clocks initialization

    PeriphClkInitStruct.PeriphClockSelection = RCC_PERIPHCLK_USB;
    PeriphClkInitStruct.PLL3.PLL3M = 25;

#if DEVICE_FS == 1
    PeriphClkInitStruct.PLL3.PLL3N = 192; // FS -> 48MHz
#elif DEVICE_HS == 1
    PeriphClkInitStruct.PLL3.PLL3N = 192; // HS -> 48MHz
#else 
    #error "specify device type DEVICE_FS or DEVICE_HS"
#endif // DEVICE_FS

    PeriphClkInitStruct.PLL3.PLL3P = 2;
    PeriphClkInitStruct.PLL3.PLL3Q = 4;
    PeriphClkInitStruct.PLL3.PLL3R = 2;
    PeriphClkInitStruct.PLL3.PLL3RGE = RCC_PLL3VCIRANGE_0;
    PeriphClkInitStruct.PLL3.PLL3FRACN = 0;
    PeriphClkInitStruct.UsbClockSelection = RCC_USBCLKSOURCE_PLL3;
    if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInitStruct) != HAL_OK) {
        return -1;
    }

    HAL_PWREx_EnableUSBVoltageDetector();

#if DEVICE_FS == 1
    // Pins initialization
    __HAL_RCC_GPIOA_CLK_ENABLE();

    // USB_OTG_FS GPIO Configuration
    // PA11     ------> USB_OTG_FS_DM
    // PA12     ------> USB_OTG_FS_DP

    GPIO_InitStruct.Pin = GPIO_PIN_11 | GPIO_PIN_12;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    GPIO_InitStruct.Alternate = GPIO_AF10_OTG1_FS;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

    __HAL_RCC_USB_OTG_FS_CLK_ENABLE();
#elif DEVICE_HS == 1
    __HAL_RCC_GPIOC_CLK_ENABLE();
    __HAL_RCC_GPIOA_CLK_ENABLE();
    __HAL_RCC_GPIOB_CLK_ENABLE();

    // USB_OTG_HS GPIO Configuration
    // PC0     ------> USB_OTG_HS_ULPI_STP
    // PC2_C   ------> USB_OTG_HS_ULPI_DIR
    // PC3_C   ------> USB_OTG_HS_ULPI_NXT
    // PA3     ------> USB_OTG_HS_ULPI_D0
    // PA5     ------> USB_OTG_HS_ULPI_CK
    // PB0     ------> USB_OTG_HS_ULPI_D1
    // PB1     ------> USB_OTG_HS_ULPI_D2
    // PB10    ------> USB_OTG_HS_ULPI_D3
    // PB11    ------> USB_OTG_HS_ULPI_D4
    // PB12    ------> USB_OTG_HS_ULPI_D5
    // PB13    ------> USB_OTG_HS_ULPI_D6
    // PB5     ------> USB_OTG_HS_ULPI_D7

    GPIO_InitStruct.Pin = GPIO_PIN_0 | GPIO_PIN_2 | GPIO_PIN_3;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
    GPIO_InitStruct.Alternate = GPIO_AF10_OTG2_HS;
    HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

    GPIO_InitStruct.Pin = GPIO_PIN_3|GPIO_PIN_5;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
    GPIO_InitStruct.Alternate = GPIO_AF10_OTG2_HS;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

    GPIO_InitStruct.Pin = GPIO_PIN_0|GPIO_PIN_1|GPIO_PIN_10|GPIO_PIN_11
                          |GPIO_PIN_12|GPIO_PIN_13|GPIO_PIN_5;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
    GPIO_InitStruct.Alternate = GPIO_AF10_OTG2_HS;
    HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

    __HAL_RCC_USB_OTG_HS_CLK_ENABLE();
    __HAL_RCC_USB_OTG_HS_ULPI_CLK_ENABLE();
#endif // DEVICE_FS or DEVICE_HS

    USB2_OTG_FS->GINTSTS = 0xFFFFFFFF;

    // Interrupts
#if DEVICE_FS == 1
    HAL_NVIC_SetPriority(OTG_FS_IRQn, USB_INT_PRIORITY, 0);
    HAL_NVIC_EnableIRQ(OTG_FS_IRQn);
#elif DEVICE_HS == 1
    HAL_NVIC_SetPriority(OTG_HS_IRQn, USB_INT_PRIORITY, 0);
    HAL_NVIC_EnableIRQ(OTG_HS_IRQn);
#endif // DEVICE_FS or DEVICE_HS

    return 0;

}

/**
 ***********************************************************************************************************************
 * @brief  USB main initialization function
 * @param  None
 * @retval Error code: 0 -- Ok
 *                    -1 -- initUsbDevicePins failed 
 *                    -2 -- USBD_Init failed
 *                    -3 -- USBD_RegisterClass failed
 *                    -4 -- USBD_Start failed
 ***********************************************************************************************************************
**/
int initUsbDevice(void)
{
    if (initUsbDevicePins()) {
        //setTerminalMessage(error, "initUsbDevicePins failed");
        return -1;
    }

#if DEVICE_FS == 1
    if (USBD_Init(&hUsbDevice, &FS_Desc, DEVICE_FS) != USBD_OK) {
#elif DEVICE_HS == 1
    if (USBD_Init(&hUsbDevice, &FS_Desc, DEVICE_HS) != USBD_OK) {
#endif// DEVICE_FS
        //setTerminalMessage(error, "USBD_Init failed");
        return -2;
    }
    
    if (USBD_RegisterClass(&hUsbDevice, &USBD_BD) != USBD_OK) {
        //setTerminalMessage(error, "USBD_RegisterClass failed");
        return -3;
    }

    if (USBD_Start(&hUsbDevice) != USBD_OK) {
        //setTerminalMessage(error, "USBD_Start failed");
        return -3;
    }

    return 0;
}

/**
 ***********************************************************************************************************************
 * @brief  USB main interrupt handler
 * @param  None
 * @retval None
 ***********************************************************************************************************************
**/
#if DEVICE_FS == 1
void OTG_FS_IRQHandler(void)
{
    HAL_PCD_IRQHandler(&hpcd_USB_OTG);
}
#endif // DEVICE_FS
/**
 ***********************************************************************************************************************
 * @brief  USB main interrupt handler
 * @param  None
 * @retval None
 ***********************************************************************************************************************
**/
#if DEVICE_HS == 1
void OTG_HS_IRQHandler(void)
{
    HAL_PCD_IRQHandler(&hpcd_USB_OTG);
}
#endif // DEVICE_HS

