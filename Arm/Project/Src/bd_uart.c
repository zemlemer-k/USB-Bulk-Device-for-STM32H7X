/**
 ***********************************************************************************************************************
 * \file        bd_uart.c
 * \author      Kirill Gribovskiy
 * \version     V 1.0
 * \date        08-Oct-2025
 * \details     Debug UART handler. UART1 PA9 and PA10
 *
 ***********************************************************************************************************************
**/
/**
 ***********************************************************************************************************************
 * Include section
 ***********************************************************************************************************************
**/
// System includes

// Device library includes
#include "stm32h7xx_hal.h"
#include "stm32h7xx_hal_gpio.h"
#include "stm32h7xx_hal_uart.h"
//#include "stm32g4xx_hal_def.h"
//#include "stm32g4xx_hal_rcc.h"

// Local includes
#include "bd_uart.h"

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
static GPIO_InitTypeDef         GPIO_InitStruct;
static RCC_PeriphCLKInitTypeDef PeriphClkInitStruct;
static UART_HandleTypeDef       DebugUart;

/**
 ***********************************************************************************************************************
 * Function prototypes
 ***********************************************************************************************************************
**/

/**
 ***********************************************************************************************************************
 * Functions
 ***********************************************************************************************************************
**/

/**
 ***********************************************************************************************************************
 * \brief   initDbgUart
 * \details Debug uart initialization; UART1 PA9 - TX;  PA10 - RX
 * \param   nothing
 * \retval  Error code:     0 -- Ok
 ***********************************************************************************************************************
**/
int initDbgUart(void)
{
/*
__HAL_RCC_GPIOA_CLK_ENABLE();
GPIO_InitStruct.Pin = GPIO_PIN_9 | GPIO_PIN_10;
GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
GPIO_InitStruct.Pull = GPIO_NOPULL;
GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
//GPIO_InitStruct.Alternate = GPIO_AF7_USART1;
HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
for(;;) {
HAL_GPIO_TogglePin(GPIOA, GPIO_PIN_9);
HAL_GPIO_TogglePin(GPIOA, GPIO_PIN_10);
HAL_Delay(10);
HAL_GPIO_TogglePin(GPIOA, GPIO_PIN_9);
HAL_GPIO_TogglePin(GPIOA, GPIO_PIN_10);
HAL_Delay(10);
}

*/

    // Clock initialization
    PeriphClkInitStruct.PeriphClockSelection = RCC_PERIPHCLK_USART1;
    PeriphClkInitStruct.Usart16ClockSelection = RCC_USART16CLKSOURCE_D2PCLK2;
    if(HAL_RCCEx_PeriphCLKConfig(&PeriphClkInitStruct) != HAL_OK) {
        return -1;
    }

    __HAL_RCC_USART1_CLK_ENABLE();
    __HAL_RCC_GPIOA_CLK_ENABLE();

    // Pins initialization 
    GPIO_InitStruct.Pin = GPIO_PIN_9 | GPIO_PIN_10;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    GPIO_InitStruct.Alternate = GPIO_AF7_USART1;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

    DebugUart.Instance = USART1;
    DebugUart.Init.BaudRate = 115200;
    DebugUart.Init.WordLength = UART_WORDLENGTH_8B;
    DebugUart.Init.StopBits = UART_STOPBITS_1;
    DebugUart.Init.Parity = UART_PARITY_NONE;
    DebugUart.Init.Mode = UART_MODE_TX_RX;
    DebugUart.Init.HwFlowCtl = UART_HWCONTROL_NONE;
    DebugUart.Init.OverSampling = UART_OVERSAMPLING_16;
    DebugUart.Init.OneBitSampling = UART_ONE_BIT_SAMPLE_DISABLE;
    DebugUart.Init.ClockPrescaler = UART_PRESCALER_DIV1;
    DebugUart.AdvancedInit.AdvFeatureInit = UART_ADVFEATURE_NO_INIT;

    if(HAL_UART_Init(&DebugUart) != HAL_OK) {
        return -2;
    }
    if (HAL_UARTEx_SetTxFifoThreshold(&DebugUart, UART_TXFIFO_THRESHOLD_1_8) != HAL_OK){
        return -2;
    }
    if (HAL_UARTEx_SetRxFifoThreshold(&DebugUart, UART_RXFIFO_THRESHOLD_1_8) != HAL_OK) {
        return -2;
    }
    if (HAL_UARTEx_DisableFifoMode(&DebugUart) != HAL_OK) {
        return -2;
    }

    return 0;
}

/**
 ***********************************************************************************************************************
 * \brief   trnDebugData
 * \details Debug uart initialization; UART1 PA9 - TX;  PA10 - RX
 * \param   chat *st_TerminalStr -- pointer to the transmitted char string
 *          uint16_t  msglng -- transmitted message size``
 * \retval  nothing
 ***********************************************************************************************************************
**/
void trnDebugData(char *st_TerminalStr, uint16_t  msglng)
{
    uint8_t *outPtr = (uint8_t *)st_TerminalStr;
    HAL_UART_Transmit(&DebugUart, outPtr, msglng, HAL_MAX_DELAY);
}

