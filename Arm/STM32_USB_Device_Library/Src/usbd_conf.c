/**
 ***********************************************************************************************************************
 * \file        usbd_conf.c
 * \author      Kirill Gribovskiy
 * \version     V 1.0
 * \date        19-Nov-2025
 * \brief       Peripheral Controller Driver (PCD) level
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
#include "usbd_config.h"
#include "usbd_core.h"
#include "usbd_bd.h"

// Local includes
#include "usbd_conf.h"


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
//PCD_HandleTypeDef   hpcd_USB_OTG_FS;
__attribute__((section(".noncache"))) PCD_HandleTypeDef   hpcd_USB_OTG;


/**
 ***********************************************************************************************************************
 * Function prototypes
 ***********************************************************************************************************************
**/
static USBD_StatusTypeDef USBD_Get_USB_Status(HAL_StatusTypeDef hal_status);

/**
 ***********************************************************************************************************************
 * @brief  Setup stage callback
 * @param  hpcd: PCD handle
 * @retval None
 ***********************************************************************************************************************
**/
#if (USE_HAL_PCD_REGISTER_CALLBACKS == 1U)
static void PCD_SetupStageCallback(PCD_HandleTypeDef *hpcd)
#else
void HAL_PCD_SetupStageCallback(PCD_HandleTypeDef *hpcd)
#endif // USE_HAL_PCD_REGISTER_CALLBACKS 
{
// #include "lib_terminal.h"
// setTerminalMessage(info, "HAL_PCD_SetupStageCallback->USBD_LL_SetupStage");

    USBD_LL_SetupStage((USBD_HandleTypeDef*)hpcd->pData, (uint8_t *)hpcd->Setup);
}

/**
 ***********************************************************************************************************************
 * @brief  Data Out stage callback.
 * @param  hpcd: PCD handle
 * @param  epnum: Endpoint number
 * @retval None
 ***********************************************************************************************************************
**/
#if (USE_HAL_PCD_REGISTER_CALLBACKS == 1U)
static void PCD_DataOutStageCallback(PCD_HandleTypeDef *hpcd, uint8_t epnum)
#else
void HAL_PCD_DataOutStageCallback(PCD_HandleTypeDef *hpcd, uint8_t epnum)
#endif /* USE_HAL_PCD_REGISTER_CALLBACKS */
{
    USBD_LL_DataOutStage((USBD_HandleTypeDef*)hpcd->pData, epnum, hpcd->OUT_ep[epnum].xfer_buff);
}

/**
 ***********************************************************************************************************************
 * @brief  Data In stage callback.
 * @param  hpcd: PCD handle
 * @param  epnum: Endpoint number
 * @retval None
 ***********************************************************************************************************************
**/
#if (USE_HAL_PCD_REGISTER_CALLBACKS == 1U)
static void PCD_DataInStageCallback(PCD_HandleTypeDef *hpcd, uint8_t epnum)
#else
void HAL_PCD_DataInStageCallback(PCD_HandleTypeDef *hpcd, uint8_t epnum)
#endif /* USE_HAL_PCD_REGISTER_CALLBACKS */
{
    USBD_LL_DataInStage((USBD_HandleTypeDef*)hpcd->pData, epnum, hpcd->IN_ep[epnum].xfer_buff);
}

/**
 ***********************************************************************************************************************
 * @brief  SOF callback.
 * @param  hpcd: PCD handle
 * @retval None
 ***********************************************************************************************************************
**/
#if (USE_HAL_PCD_REGISTER_CALLBACKS == 1U)
static void PCD_SOFCallback(PCD_HandleTypeDef *hpcd)
#else
void HAL_PCD_SOFCallback(PCD_HandleTypeDef *hpcd)
#endif /* USE_HAL_PCD_REGISTER_CALLBACKS */
{
    USBD_LL_SOF((USBD_HandleTypeDef*)hpcd->pData);
}

/**
 ***********************************************************************************************************************
 * @brief  Reset callback.
 * @param  hpcd: PCD handle
 * @retval None
 ***********************************************************************************************************************
**/
#if (USE_HAL_PCD_REGISTER_CALLBACKS == 1U)
static void PCD_ResetCallback(PCD_HandleTypeDef *hpcd)
#else
void HAL_PCD_ResetCallback(PCD_HandleTypeDef *hpcd)
#endif // USE_HAL_PCD_REGISTER_CALLBACKS
{
    USBD_SpeedTypeDef speed = USBD_SPEED_FULL;
    if ( hpcd->Init.speed == PCD_SPEED_HIGH) {
        speed = USBD_SPEED_HIGH;
    }
    else if ( hpcd->Init.speed == PCD_SPEED_FULL) {
        speed = USBD_SPEED_FULL;
    }
    else {
        // setTerminalMessage(error, "HAL_PCD_ResetCallback with wrong PCD_SPEED");
    }
    // Set Speed
    USBD_LL_SetSpeed((USBD_HandleTypeDef*)hpcd->pData, speed);

    // Reset Device
    USBD_LL_Reset((USBD_HandleTypeDef*)hpcd->pData);
}

/**
 ***********************************************************************************************************************
 * @brief  Suspend callback.
 * When Low power mode is enabled the debug cannot be used (IAR, Keil doesn't support it)
 * @param  hpcd: PCD handle
 * @retval None
 ***********************************************************************************************************************
**/
#if (USE_HAL_PCD_REGISTER_CALLBACKS == 1U)
static void PCD_SuspendCallback(PCD_HandleTypeDef *hpcd)
#else
void HAL_PCD_SuspendCallback(PCD_HandleTypeDef *hpcd)
#endif // USE_HAL_PCD_REGISTER_CALLBACKS
{
    // Inform USB library that core enters in suspend Mode.
    USBD_LL_Suspend((USBD_HandleTypeDef*)hpcd->pData);
    __HAL_PCD_GATE_PHYCLOCK(hpcd);
    // Enter in STOP mode.
    if (hpcd->Init.low_power_enable) {
        // Set SLEEPDEEP bit and SleepOnExit of Cortex System Control Register.
        SCB->SCR |= (uint32_t)((uint32_t)(SCB_SCR_SLEEPDEEP_Msk | SCB_SCR_SLEEPONEXIT_Msk));
    }
}

/**
 ***********************************************************************************************************************
 * @brief  Resume callback.
 * When Low power mode is enabled the debug cannot be used (IAR, Keil doesn't support it)
 * @param  hpcd: PCD handle
 * @retval None
 ***********************************************************************************************************************
**/
#if (USE_HAL_PCD_REGISTER_CALLBACKS == 1U)
static void PCD_ResumeCallback(PCD_HandleTypeDef *hpcd)
#else
void HAL_PCD_ResumeCallback(PCD_HandleTypeDef *hpcd)
#endif // USE_HAL_PCD_REGISTER_CALLBACKS
{
    USBD_LL_Resume((USBD_HandleTypeDef*)hpcd->pData);
}

/**
 ***********************************************************************************************************************
 * @brief  ISOOUTIncomplete callback.
 * @param  hpcd: PCD handle
 * @param  epnum: Endpoint number
 * @retval None
 ***********************************************************************************************************************
**/
#if (USE_HAL_PCD_REGISTER_CALLBACKS == 1U)
static void PCD_ISOOUTIncompleteCallback(PCD_HandleTypeDef *hpcd, uint8_t epnum)
#else
void HAL_PCD_ISOOUTIncompleteCallback(PCD_HandleTypeDef *hpcd, uint8_t epnum)
#endif // USE_HAL_PCD_REGISTER_CALLBACKS
{
    USBD_LL_IsoOUTIncomplete((USBD_HandleTypeDef*)hpcd->pData, epnum);
}

/**
 ***********************************************************************************************************************
 * @brief  ISOINIncomplete callback.
 * @param  hpcd: PCD handle
 * @param  epnum: Endpoint number
 * @retval None
 ***********************************************************************************************************************
**/
#if (USE_HAL_PCD_REGISTER_CALLBACKS == 1U)
static void PCD_ISOINIncompleteCallback(PCD_HandleTypeDef *hpcd, uint8_t epnum)
#else
void HAL_PCD_ISOINIncompleteCallback(PCD_HandleTypeDef *hpcd, uint8_t epnum)
#endif // USE_HAL_PCD_REGISTER_CALLBACKS
{
    USBD_LL_IsoINIncomplete((USBD_HandleTypeDef*)hpcd->pData, epnum);
}

/**
 ***********************************************************************************************************************
 * @brief  Connect callback.
 * @param  hpcd: PCD handle
 * @retval None
 ***********************************************************************************************************************
**/
#if (USE_HAL_PCD_REGISTER_CALLBACKS == 1U)
static void PCD_ConnectCallback(PCD_HandleTypeDef *hpcd)
#else
void HAL_PCD_ConnectCallback(PCD_HandleTypeDef *hpcd)
#endif // USE_HAL_PCD_REGISTER_CALLBACKS 
{
    USBD_LL_DevConnected((USBD_HandleTypeDef*)hpcd->pData);
}

/**
 ***********************************************************************************************************************
 * @brief  Disconnect callback.
 * @param  hpcd: PCD handle
 * @retval None
 ***********************************************************************************************************************
**/
#if (USE_HAL_PCD_REGISTER_CALLBACKS == 1U)
static void PCD_DisconnectCallback(PCD_HandleTypeDef *hpcd)
#else
void HAL_PCD_DisconnectCallback(PCD_HandleTypeDef *hpcd)
#endif /* USE_HAL_PCD_REGISTER_CALLBACKS */
{
    USBD_LL_DevDisconnected((USBD_HandleTypeDef*)hpcd->pData);
}

/**
 ***********************************************************************************************************************
                       LL Driver Interface (USB Device Library --> PCD)
 ***********************************************************************************************************************
**/

/**
 ***********************************************************************************************************************
 * @brief  Initializes the low level portion of the device driver.
 * @param  pdev: Device handle
 * @retval USBD status
 ***********************************************************************************************************************
**/
USBD_StatusTypeDef USBD_LL_Init(USBD_HandleTypeDef *pdev)
{

    memset(&hpcd_USB_OTG, 0, sizeof(PCD_HandleTypeDef));
    // Init USB Ip
    // if (pdev->id == DEVICE_FS) {
    // Link the driver to the stack.
        hpcd_USB_OTG.pData = pdev;
        pdev->pData = &hpcd_USB_OTG;
#if DEVICE_FS == 1
        hpcd_USB_OTG.Instance = USB_OTG_FS;
#elif DEVICE_HS == 1
        hpcd_USB_OTG.Instance = USB_OTG_HS;
#endif // DEVICE_FS
        hpcd_USB_OTG.Init.dev_endpoints = 2;
        hpcd_USB_OTG.Init.speed = USBD_SPEED_FULL;
        hpcd_USB_OTG.Init.dma_enable = ENABLE;
#if DEVICE_FS == 1
        hpcd_USB_OTG.Init.phy_itface = PCD_PHY_EMBEDDED; //USB_OTG_ULPI_PHY;
#elif DEVICE_HS == 1
        hpcd_USB_OTG.Init.phy_itface = USB_OTG_ULPI_PHY;
#endif // DEVICE_FS
        hpcd_USB_OTG.Init.Sof_enable = DISABLE;
        hpcd_USB_OTG.Init.low_power_enable = DISABLE;
        hpcd_USB_OTG.Init.lpm_enable = DISABLE;
        hpcd_USB_OTG.Init.vbus_sensing_enable = DISABLE;
        hpcd_USB_OTG.Init.use_dedicated_ep1 = DISABLE;
        hpcd_USB_OTG.Init.use_external_vbus = DISABLE;

        if (HAL_PCD_Init(&hpcd_USB_OTG) != HAL_OK) {
            //setTerminalMessage(error, "USBD_LL_Init failed at call of HAL_PCD_Init");
            for(;;);
        }

        // TODO: Clear interrupts 
        hpcd_USB_OTG.Instance->GINTMSK &= ~(0x04008008);
        //  Pullup delay
        /* USB_OTG_GlobalTypeDef *USBx = hpcd_USB_OTG.Instance;
        USBx->DCTL |= USB_OTG_DCTL_SFTDISCON;   // off
        HAL_Delay(30);
        USBx->DCTL &= ~USB_OTG_DCTL_SFTDISCON;  // On */

#if (USE_HAL_PCD_REGISTER_CALLBACKS == 1U)
        // Register USB PCD CallBacks 
        HAL_PCD_RegisterCallback(&hpcd_USB_OTG, HAL_PCD_SOF_CB_ID, PCD_SOFCallback);
        HAL_PCD_RegisterCallback(&hpcd_USB_OTG, HAL_PCD_SETUPSTAGE_CB_ID, PCD_SetupStageCallback);
        HAL_PCD_RegisterCallback(&hpcd_USB_OTG, HAL_PCD_RESET_CB_ID, PCD_ResetCallback);
        HAL_PCD_RegisterCallback(&hpcd_USB_OTG, HAL_PCD_SUSPEND_CB_ID, PCD_SuspendCallback);
        HAL_PCD_RegisterCallback(&hpcd_USB_OTG, HAL_PCD_RESUME_CB_ID, PCD_ResumeCallback);
        HAL_PCD_RegisterCallback(&hpcd_USB_OTG, HAL_PCD_CONNECT_CB_ID, PCD_ConnectCallback);
        HAL_PCD_RegisterCallback(&hpcd_USB_OTG, HAL_PCD_DISCONNECT_CB_ID, PCD_DisconnectCallback);

        HAL_PCD_RegisterDataOutStageCallback(&hpcd_USB_OTG, PCD_DataOutStageCallback);
        HAL_PCD_RegisterDataInStageCallback(&hpcd_USB_OTG, PCD_DataInStageCallback);
        HAL_PCD_RegisterIsoOutIncpltCallback(&hpcd_USB_OTG, PCD_ISOOUTIncompleteCallback);
        HAL_PCD_RegisterIsoInIncpltCallback(&hpcd_USB_OTG, PCD_ISOINIncompleteCallback);
#endif // USE_HAL_PCD_REGISTER_CALLBACKS


#if DEVICE_FS == 1
        HAL_PCDEx_SetRxFiFo(&hpcd_USB_OTG, 0x80);      // 128 words = 512 bytes
        HAL_PCDEx_SetTxFiFo(&hpcd_USB_OTG, 0, 0x10);   // EP0 Tx — 16 words (64 bytes) —  control
        HAL_PCDEx_SetTxFiFo(&hpcd_USB_OTG, 1, 0x100);  // EP1 IN bulk — 256 words = 1024 bytes
#elif DEVICE_HS == 1
        HAL_PCDEx_SetRxFiFo(&hpcd_USB_OTG, 0x120);
        HAL_PCDEx_SetTxFiFo(&hpcd_USB_OTG, 0, 0x20);
        HAL_PCDEx_SetTxFiFo(&hpcd_USB_OTG, 1, 0x200);
#endif // hpcd_USB

    //}
    return USBD_OK;
}

/**
 ***********************************************************************************************************************
 * @brief  De-Initializes the low level portion of the device driver.
 * @param  pdev: Device handle
 * @retval USBD status
 ***********************************************************************************************************************
**/
USBD_StatusTypeDef USBD_LL_DeInit(USBD_HandleTypeDef *pdev)
{
    HAL_StatusTypeDef hal_status = HAL_OK;
    USBD_StatusTypeDef usb_status = USBD_OK;

    hal_status = HAL_PCD_DeInit(pdev->pData);
    usb_status =  USBD_Get_USB_Status(hal_status);

    return usb_status;
}

/**
 ***********************************************************************************************************************
 * @brief  Starts the low level portion of the device driver.
 * @param  pdev: Device handle
 * @retval USBD status
 ***********************************************************************************************************************
**/
USBD_StatusTypeDef USBD_LL_Start(USBD_HandleTypeDef *pdev)
{
    HAL_StatusTypeDef hal_status = HAL_OK;
    USBD_StatusTypeDef usb_status = USBD_OK;

    hal_status = HAL_PCD_Start(pdev->pData);
    usb_status =  USBD_Get_USB_Status(hal_status);


    return usb_status;
}

/**
 ***********************************************************************************************************************
 * @brief  Stops the low level portion of the device driver.
 * @param  pdev: Device handle
 * @retval USBD status
 ***********************************************************************************************************************
**/
USBD_StatusTypeDef USBD_LL_Stop(USBD_HandleTypeDef *pdev)
{
    HAL_StatusTypeDef hal_status = HAL_OK;
    USBD_StatusTypeDef usb_status = USBD_OK;

    hal_status = HAL_PCD_Stop(pdev->pData);
    usb_status =  USBD_Get_USB_Status(hal_status);

    return usb_status;
}

/**
 ***********************************************************************************************************************
 * @brief  Opens an endpoint of the low level driver.
 * @param  pdev: Device handle
 * @param  ep_addr: Endpoint number
 * @param  ep_type: Endpoint type
 * @param  ep_mps: Endpoint max packet size
 * @retval USBD status
 ***********************************************************************************************************************
**/
USBD_StatusTypeDef USBD_LL_OpenEP(USBD_HandleTypeDef *pdev, uint8_t ep_addr, uint8_t ep_type, uint16_t ep_mps)
{
    HAL_StatusTypeDef hal_status = HAL_OK;
    USBD_StatusTypeDef usb_status = USBD_OK;

    hal_status = HAL_PCD_EP_Open(pdev->pData, ep_addr, ep_mps, ep_type);
    usb_status =  USBD_Get_USB_Status(hal_status);

    return usb_status;
}

/**
 ***********************************************************************************************************************
 * @brief  Closes an endpoint of the low level driver.
 * @param  pdev: Device handle
 * @param  ep_addr: Endpoint number
 * @retval USBD status
 ***********************************************************************************************************************
**/
USBD_StatusTypeDef USBD_LL_CloseEP(USBD_HandleTypeDef *pdev, uint8_t ep_addr)
{
    HAL_StatusTypeDef hal_status = HAL_OK;
    USBD_StatusTypeDef usb_status = USBD_OK;

    hal_status = HAL_PCD_EP_Close(pdev->pData, ep_addr);
    usb_status =  USBD_Get_USB_Status(hal_status);

    return usb_status;
}

/**
 ***********************************************************************************************************************
 * @brief  Flushes an endpoint of the Low Level Driver.
 * @param  pdev: Device handle
 * @param  ep_addr: Endpoint number
 * @retval USBD status
 ***********************************************************************************************************************
**/
USBD_StatusTypeDef USBD_LL_FlushEP(USBD_HandleTypeDef *pdev, uint8_t ep_addr)
{
  HAL_StatusTypeDef hal_status = HAL_OK;
  USBD_StatusTypeDef usb_status = USBD_OK;

  hal_status = HAL_PCD_EP_Flush(pdev->pData, ep_addr);
  usb_status =  USBD_Get_USB_Status(hal_status);

  return usb_status;
}

/**
 ***********************************************************************************************************************
 * @brief  Sets a Stall condition on an endpoint of the Low Level Driver.
 * @param  pdev: Device handle
 * @param  ep_addr: Endpoint number
 * @retval USBD status
 ***********************************************************************************************************************
**/
USBD_StatusTypeDef USBD_LL_StallEP(USBD_HandleTypeDef *pdev, uint8_t ep_addr)
{
    HAL_StatusTypeDef hal_status = HAL_OK;
    USBD_StatusTypeDef usb_status = USBD_OK;

    hal_status = HAL_PCD_EP_SetStall(pdev->pData, ep_addr);
    usb_status =  USBD_Get_USB_Status(hal_status);

    return usb_status;
}

/**
 ***********************************************************************************************************************
 * @brief  Clears a Stall condition on an endpoint of the Low Level Driver.
 * @param  pdev: Device handle
 * @param  ep_addr: Endpoint number
 * @retval USBD status
 ***********************************************************************************************************************
**/
USBD_StatusTypeDef USBD_LL_ClearStallEP(USBD_HandleTypeDef *pdev, uint8_t ep_addr)
{
    HAL_StatusTypeDef hal_status = HAL_OK;
    USBD_StatusTypeDef usb_status = USBD_OK;

    hal_status = HAL_PCD_EP_ClrStall(pdev->pData, ep_addr);
    usb_status =  USBD_Get_USB_Status(hal_status);

    return usb_status;
}

/**
 ***********************************************************************************************************************
 * @brief  Returns Stall condition.
 * @param  pdev: Device handle
 * @param  ep_addr: Endpoint number
 * @retval Stall (1: Yes, 0: No)
 ***********************************************************************************************************************
**/
uint8_t USBD_LL_IsStallEP(USBD_HandleTypeDef *pdev, uint8_t ep_addr)
{
    PCD_HandleTypeDef *hpcd = (PCD_HandleTypeDef*) pdev->pData;

    if((ep_addr & 0x80) == 0x80) {
        return hpcd->IN_ep[ep_addr & 0x7F].is_stall;
    }
    else {
        return hpcd->OUT_ep[ep_addr & 0x7F].is_stall;
    }
}

/**
 ***********************************************************************************************************************
 * @brief  Assigns a USB address to the device.
 * @param  pdev: Device handle
 * @param  dev_addr: Device address
 * @retval USBD status
 ***********************************************************************************************************************
**/
USBD_StatusTypeDef USBD_LL_SetUSBAddress(USBD_HandleTypeDef *pdev, uint8_t dev_addr)
{
    HAL_StatusTypeDef hal_status = HAL_OK;
    USBD_StatusTypeDef usb_status = USBD_OK;

    hal_status = HAL_PCD_SetAddress(pdev->pData, dev_addr);
    usb_status =  USBD_Get_USB_Status(hal_status);

    return usb_status;
}

/**
 ***********************************************************************************************************************
 * @brief  Transmits data over an endpoint.
 * @param  pdev: Device handle
 * @param  ep_addr: Endpoint number
 * @param  pbuf: Pointer to data to be sent
 * @param  size: Data size
 * @retval USBD status
 ***********************************************************************************************************************
**/
USBD_StatusTypeDef USBD_LL_Transmit(USBD_HandleTypeDef *pdev, uint8_t ep_addr, uint8_t *pbuf, uint32_t size)
{
    HAL_StatusTypeDef hal_status = HAL_OK;
    USBD_StatusTypeDef usb_status = USBD_OK;

    hal_status = HAL_PCD_EP_Transmit(pdev->pData, ep_addr, pbuf, size);
    usb_status =  USBD_Get_USB_Status(hal_status);

    return usb_status;
}

/**
 ***********************************************************************************************************************
 * @brief  Prepares an endpoint for reception.
 * @param  pdev: Device handle
 * @param  ep_addr: Endpoint number
 * @param  pbuf: Pointer to data to be received
 * @param  size: Data size
 * @retval USBD status
 ***********************************************************************************************************************
**/
USBD_StatusTypeDef USBD_LL_PrepareReceive(USBD_HandleTypeDef *pdev, uint8_t ep_addr, uint8_t *pbuf, uint32_t size)
{
    HAL_StatusTypeDef hal_status = HAL_OK;
    USBD_StatusTypeDef usb_status = USBD_OK;

    hal_status = HAL_PCD_EP_Receive(pdev->pData, ep_addr, pbuf, size);
    usb_status =  USBD_Get_USB_Status(hal_status);

    return usb_status;
}

/**
 ***********************************************************************************************************************
 * @brief  Returns the last transferred packet size.
 * @param  pdev: Device handle
 * @param  ep_addr: Endpoint number
 * @retval Received Data Size
 ***********************************************************************************************************************
**/
uint32_t USBD_LL_GetRxDataSize(USBD_HandleTypeDef *pdev, uint8_t ep_addr)
{
    return HAL_PCD_EP_GetRxCount((PCD_HandleTypeDef*) pdev->pData, ep_addr);
}

/**
 ***********************************************************************************************************************
 * @brief  Static single allocation.
 * @param  size: Size of allocated memory
 * @retval None
 ***********************************************************************************************************************
**/
// void *USBD_static_malloc(uint32_t size)
// {
//     static uint32_t mem[(sizeof(USBD_BD_HandleTypeDef)/4)+1];// On 32-bit boundary
//     return mem;
// }

/**
 ***********************************************************************************************************************
 * @brief  Dummy memory free
 * @param  p: Pointer to allocated  memory address
 * @retval None
 ***********************************************************************************************************************
**/
// void USBD_static_free(void *p)
// {
// }

/**
 ***********************************************************************************************************************
 * @brief  Delays routine for the USB device library.
 * @param  Delay: Delay in ms
 * @retval None
 ***********************************************************************************************************************
**/
void USBD_LL_Delay(uint32_t Delay)
{
    HAL_Delay(Delay);
}

/**
 ***********************************************************************************************************************
 * @brief  Returns the USB status depending on the HAL status:
 * @param  hal_status: HAL status
 * @retval USB status
 ***********************************************************************************************************************
**/
static USBD_StatusTypeDef USBD_Get_USB_Status(HAL_StatusTypeDef hal_status)
{
    USBD_StatusTypeDef usb_status = USBD_OK;

    switch (hal_status) {
        case HAL_OK :
            usb_status = USBD_OK;
            break;
        case HAL_ERROR :
            usb_status = USBD_FAIL;
            break;
        case HAL_BUSY :
            usb_status = USBD_BUSY;
           break;
        case HAL_TIMEOUT :
            usb_status = USBD_FAIL;
            break;
        default :
            usb_status = USBD_FAIL;
            break;
    }
    return usb_status;
}

