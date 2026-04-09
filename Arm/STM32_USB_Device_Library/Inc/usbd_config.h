/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : usbd_config.h
  * @version        : v1.0_Cube
  * @brief          : Header for usbd_conf.c file.
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2024 STMicroelectronics.
  * All rights reserved.</center></h2>
  *
  * This software component is licensed by ST under Ultimate Liberty license
  * SLA0044, the "License"; You may not use this file except in compliance with
  * the License. You may obtain a copy of the License at:
  *                             www.st.com/SLA0044
  *
  ******************************************************************************
  */
/* USER CODE END Header */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __USBD_CONF__H__
#define __USBD_CONF__H__

#ifdef __cplusplus
 extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "stm32h7xx.h"
#include "stm32h7xx_hal.h"

#define USBD_MAX_NUM_INTERFACES        1U
/*---------- -----------*/
#define USBD_MAX_NUM_CONFIGURATION     1U
/*---------- -----------*/
#define USBD_MAX_STR_DESC_SIZ          512U
/*---------- -----------*/
#define USBD_DEBUG_LEVEL               0U
/*---------- -----------*/
#define USBD_LPM_ENABLED               0U
/*---------- -----------*/
#define USBD_SELF_POWERED              1U
/*---------- -----------*/
//#define MSC_MEDIA_PACKET     32768U
#define MSC_MEDIA_PACKET               64U


/****************************************/
/* #define for FS and HS identification */
#define DEVICE_FS 		1
//#define DEVICE_HS 		0


/* Exported functions -------------------------------------------------------*/
//void *USBD_static_malloc(uint32_t size);
// void USBD_static_free(void *p);

#ifdef __cplusplus
}
#endif

#endif /* __USBD_CONF__H__ */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/

