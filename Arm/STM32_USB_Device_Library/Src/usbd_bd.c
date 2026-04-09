#include <stdint.h>
#include <stdbool.h>

#include "stm32h7xx_hal.h"
#include "usbd_def.h"
#include "usbd_config.h"
#include "bd_usbexch.h"
#include "usbd_bd.h"


uint8_t USBD_BD_Init(USBD_HandleTypeDef *pdev, uint8_t cfgidx);
uint8_t USBD_BD_DeInit(USBD_HandleTypeDef *pdev, uint8_t cfgidx);
uint8_t USBD_BD_Setup(USBD_HandleTypeDef *pdev, USBD_SetupReqTypedef *req);
uint8_t USBD_BD_DataIn(USBD_HandleTypeDef *pdev, uint8_t epnum);
uint8_t USBD_BD_DataOut(USBD_HandleTypeDef *pdev, uint8_t epnum);

uint8_t *USBD_BD_GetHSCfgDesc(uint16_t *length);
uint8_t *USBD_BD_GetFSCfgDesc(uint16_t *length);
uint8_t *USBD_BD_GetOtherSpeedCfgDesc(uint16_t *length);
uint8_t *USBD_BD_GetDeviceQualifierDescriptor(uint16_t *length);

int flg;

USBD_ClassTypeDef  USBD_BD =
{
    USBD_BD_Init,
    USBD_BD_DeInit,
    USBD_BD_Setup,
    NULL, /*EP0_TxSent*/
    NULL, /*EP0_RxReady*/
    USBD_BD_DataIn,
    USBD_BD_DataOut,
    NULL, /*SOF */
    NULL,
    NULL,
    USBD_BD_GetHSCfgDesc,
    USBD_BD_GetFSCfgDesc,
    USBD_BD_GetOtherSpeedCfgDesc,
    USBD_BD_GetDeviceQualifierDescriptor,
};

/* USB bulk device Configuration Descriptor */
/*   All Descriptors (Configuration, Interface, Endpoint, Class, Vendor */
__ALIGN_BEGIN static uint8_t USBD_BD_CfgHSDesc[USB_BD_CONFIG_DESC_SIZ]  __ALIGN_END =
{
    0x09,                                            /* bLength: Configuration Descriptor size */
    USB_DESC_TYPE_CONFIGURATION,                     /* bDescriptorType: Configuration */
    USB_BD_CONFIG_DESC_SIZ,
    0x00,
    0x01,                                            /* bNumInterfaces: 1 interface */
    0x01,                                            /* bConfigurationValue: */
    0x04,                                            /* iConfiguration: */
#if (USBD_SELF_POWERED == 1U)
    0xC0,                                            /* bmAttributes: Bus Powered according to user configuration */
#else
    0x80,                                            /* bmAttributes: Bus Powered according to user configuration */
#endif
    USBD_MAX_POWER,                                  /* MaxPower 100 mA */

    //*******************  Bulk interface ********************
    0x09,                                            /* bLength: Interface Descriptor size */
    0x04,                                            /* bDescriptorType: */
    0x00,                                            /* bInterfaceNumber: Number of Interface */
    0x00,                                            /* bAlternateSetting: Alternate setting */
    0x02,                                            /* bNumEndpoints */
    0xFF,                                            /* bInterfaceClass: BD Class */
    0x00,                                            /* bInterfaceSubClass : SCSI transparent */
    0x00,                                            /* nInterfaceProtocol */
    0x05,                                            /* iInterface: */
    //********************  Bulk Endpoints ********************
    0x07,                                            /* Endpoint descriptor length = 7 */
    0x05,                                            /* Endpoint descriptor type */
    BD_EPIN_ADDR,                                   /* Endpoint address (IN, address 1) */
    0x02,                                            /* Bulk endpoint type */
    LOBYTE(BD_MAX_HS_PACKET),
    HIBYTE(BD_MAX_HS_PACKET),
    0x00,                                            /* Polling interval in milliseconds */
    0x07,                                            /* Endpoint descriptor length = 7 */
    0x05,                                            /* Endpoint descriptor type */
    BD_EPOUT_ADDR,                                  /* Endpoint address (OUT, address 1) */
    0x02,                                            /* Bulk endpoint type */
    LOBYTE(BD_MAX_HS_PACKET),
    HIBYTE(BD_MAX_HS_PACKET),
    0x00                                             /* Polling interval in milliseconds */
};

/* USB Mass storage device Configuration Descriptor */
/* All Descriptors (Configuration, Interface, Endpoint, Class, Vendor */
__ALIGN_BEGIN static uint8_t USBD_BD_CfgFSDesc[USB_BD_CONFIG_DESC_SIZ]  __ALIGN_END =
{
    0x09,                                            /* bLength: Configuration Descriptor size */
    USB_DESC_TYPE_CONFIGURATION,                     /* bDescriptorType: Configuration */
    USB_BD_CONFIG_DESC_SIZ,
    0x00,
    0x01,                                            /* bNumInterfaces: 1 interface */
    0x01,                                            /* bConfigurationValue: */
    0x04,                                            /* iConfiguration: */
#if (USBD_SELF_POWERED == 1U)
    0xC0,                                            /* bmAttributes: Bus Powered according to user configuration */
#else
    0x80,                                            /* bmAttributes: Bus Powered according to user configuration */
#endif
    USBD_MAX_POWER,                                  /* MaxPower 100 mA */

    0x09,                                            /* bLength: Interface Descriptor size */
    0x04,                                            /* bDescriptorType: */
    0x00,                                            /* bInterfaceNumber: Number of Interface */
    0x00,                                            /* bAlternateSetting: Alternate setting */
    0x02,                                            /* bNumEndpoints*/
    0xFF,                                            /* bInterfaceClass: BD Class */
    0x00,                                            /* bInterfaceSubClass : SCSI transparent*/
    0x00,                                            /* nInterfaceProtocol */
    0x05,                                            /* iInterface: */

    0x07,                                            /* Endpoint descriptor length = 7 */
    0x05,                                            /* Endpoint descriptor type */
    BD_EPIN_ADDR,                                    /* Endpoint address (IN, address 1) */
    0x02,                                            /* Bulk endpoint type */
    LOBYTE(BD_MAX_FS_PACKET),
    HIBYTE(BD_MAX_FS_PACKET),
    0x00,                                            /* Polling interval in milliseconds */

    0x07,                                            /* Endpoint descriptor length = 7 */
    0x05,                                            /* Endpoint descriptor type */
    BD_EPOUT_ADDR,                                   /* Endpoint address (OUT, address 1) */
    0x02,                                            /* Bulk endpoint type */
    LOBYTE(BD_MAX_FS_PACKET),
    HIBYTE(BD_MAX_FS_PACKET),
    0x00                                             /* Polling interval in milliseconds */
};

__ALIGN_BEGIN static uint8_t USBD_BD_OtherSpeedCfgDesc[USB_BD_CONFIG_DESC_SIZ]   __ALIGN_END  =
{
    0x09,                                           /* bLength: Configuration Descriptor size */
    USB_DESC_TYPE_OTHER_SPEED_CONFIGURATION,
    USB_BD_CONFIG_DESC_SIZ,
    0x00,
    0x01,                                           /* bNumInterfaces: 1 interface */
    0x01,                                           /* bConfigurationValue: */
    0x04,                                           /* iConfiguration: */
#if (USBD_SELF_POWERED == 1U)
    0xC0,                                           /* bmAttributes: Bus Powered according to user configuration */
#else
    0x80,                                           /* bmAttributes: Bus Powered according to user configuration */
#endif
    USBD_MAX_POWER,                                 /* MaxPower 100 mA */

    0x09,                                           /* bLength: Interface Descriptor size */
    0x04,                                           /* bDescriptorType: */
    0x00,                                           /* bInterfaceNumber: Number of Interface */
    0x00,                                           /* bAlternateSetting: Alternate setting */
    0x02,                                           /* bNumEndpoints */
    0xFF,                                           /* bInterfaceClass: BD Class */
    0x00,                                           /* bInterfaceSubClass : SCSI transparent command set */
    0x00,                                           /* nInterfaceProtocol */
    0x05,                                           /* iInterface: */

    0x07,                                           /* Endpoint descriptor length = 7 */
    0x05,                                           /* Endpoint descriptor type */
    BD_EPIN_ADDR,                                  /* Endpoint address (IN, address 1) */
    0x02,                                           /* Bulk endpoint type */
    0x40,
    0x00,
    0x00,                                           /* Polling interval in milliseconds */

    0x07,                                           /* Endpoint descriptor length = 7 */
    0x05,                                           /* Endpoint descriptor type */
    BD_EPOUT_ADDR,                                 /* Endpoint address (OUT, address 1) */
    0x02,                                           /* Bulk endpoint type */
    0x40,
    0x00,
    0x00                                            /* Polling interval in milliseconds */
};

/* USB Standard Device Descriptor */
__ALIGN_BEGIN static uint8_t USBD_BD_DeviceQualifierDesc[USB_LEN_DEV_QUALIFIER_DESC]  __ALIGN_END =
{
    USB_LEN_DEV_QUALIFIER_DESC,
    USB_DESC_TYPE_DEVICE_QUALIFIER,
    0x00,
    0x02,
    0xFF,		// Class
    0x00,		// Subclass
    0x00,		// Protocol
    BD_MAX_FS_PACKET,
    0x01,		// Num configuration
    0x00,		// bInterval
};

// Making static USBD_BD_HandleTypeDef structure
__attribute__((section(".noncache")))  USBD_BD_HandleTypeDef usbd_bd_Handle; 

/**
  * @brief  USBD_BD_Init
  *         Initialize  the mass storage configuration
  * @param  pdev: device instance
  * @param  cfgidx: configuration index
  * @retval status
  */
uint8_t USBD_BD_Init(USBD_HandleTypeDef *pdev, uint8_t cfgidx)
{
    UNUSED(cfgidx);

    // USBD_BD_HandleTypeDef *hmsc;
    // hmsc = USBD_malloc(sizeof(USBD_BD_HandleTypeDef));

    // if (hmsc == NULL)  {
    //     pdev->pClassData = NULL;
    //     return (uint8_t)USBD_EMEM;
    // }
    
    USBD_BD_HandleTypeDef *hmsc = &usbd_bd_Handle;

    pdev->pClassData = (void *)hmsc;

    if (pdev->dev_speed == USBD_SPEED_HIGH) {
        /* Open EP OUT */
        (void)USBD_LL_OpenEP(pdev, BD_EPOUT_ADDR, USBD_EP_TYPE_BULK, BD_MAX_HS_PACKET);
        pdev->ep_out[BD_EPOUT_ADDR & 0xFU].is_used = 1U;

        /* Open EP IN */
        (void)USBD_LL_OpenEP(pdev, BD_EPIN_ADDR, USBD_EP_TYPE_BULK, BD_MAX_HS_PACKET);
        pdev->ep_in[BD_EPIN_ADDR & 0xFU].is_used = 1U;
    }
    else {
        /* Open EP OUT */
        (void)USBD_LL_OpenEP(pdev, BD_EPOUT_ADDR, USBD_EP_TYPE_BULK, BD_MAX_FS_PACKET);
        pdev->ep_out[BD_EPOUT_ADDR & 0xFU].is_used = 1U;

        /* Open EP IN */
        (void)USBD_LL_OpenEP(pdev, BD_EPIN_ADDR, USBD_EP_TYPE_BULK, BD_MAX_FS_PACKET);
        pdev->ep_in[BD_EPIN_ADDR & 0xFU].is_used = 1U;
    }

    return (uint8_t)USBD_OK;
}

/**
  * @brief  USBD_BD_DeInit
  *         DeInitialize  the mass storage configuration
  * @param  pdev: device instance
  * @param  cfgidx: configuration index
  * @retval status
  */
uint8_t USBD_BD_DeInit(USBD_HandleTypeDef *pdev, uint8_t cfgidx)
{
  UNUSED(cfgidx);

  /* Close BD EPs */
  (void)USBD_LL_CloseEP(pdev, BD_EPOUT_ADDR);
  pdev->ep_out[BD_EPOUT_ADDR & 0xFU].is_used = 0U;

  /* Close EP IN */
  (void)USBD_LL_CloseEP(pdev, BD_EPIN_ADDR);
  pdev->ep_in[BD_EPIN_ADDR & 0xFU].is_used = 0U;

  /* Free BD Class Resources */
  if (pdev->pClassData != NULL)
  {
    // (void)USBD_free(pdev->pClassData);
    pdev->pClassData = NULL;
  }

  return (uint8_t)USBD_OK;
}
/**
  * @brief  USBD_BD_Setup
  *         Handle the BD specific requests
  * @param  pdev: device instance
  * @param  req: USB request
  * @retval status
  */
uint8_t USBD_BD_Setup(USBD_HandleTypeDef *pdev, USBD_SetupReqTypedef *req)
{
    USBD_BD_HandleTypeDef *hmsc = (USBD_BD_HandleTypeDef *)pdev->pClassData;
    USBD_StatusTypeDef ret = USBD_OK;
    uint16_t status_info = 0U;

    if (hmsc == NULL) {
        return (uint8_t)USBD_FAIL;
    }

    switch (req->bmRequest & USB_REQ_TYPE_MASK)
    {
        // Interface & Endpoint request
        case USB_REQ_TYPE_STANDARD:
            switch (req->bRequest)
            {
                case USB_REQ_GET_STATUS:
                if (pdev->dev_state == USBD_STATE_CONFIGURED)                {
                    (void)USBD_CtlSendData(pdev, (uint8_t *)&status_info, 2U);
                }
                else {
                    USBD_CtlError(pdev, req);
                    ret = USBD_FAIL;
                }
                break;

            case USB_REQ_GET_INTERFACE:
                if (pdev->dev_state == USBD_STATE_CONFIGURED) {
                    (void)USBD_CtlSendData(pdev, (uint8_t *)&hmsc->interface, 1U);
                }
                else {
                    USBD_CtlError(pdev, req);
                    ret = USBD_FAIL;
                }
                break;

            case USB_REQ_SET_INTERFACE:
                if (pdev->dev_state == USBD_STATE_CONFIGURED) {
                    hmsc->interface = (uint8_t)(req->wValue);
                }
                else {
                    USBD_CtlError(pdev, req);
                    ret = USBD_FAIL;
                }
                break;

            case USB_REQ_CLEAR_FEATURE:
                if (pdev->dev_state == USBD_STATE_CONFIGURED) {
                    if (req->wValue == USB_FEATURE_EP_HALT) {
                        // Flush the FIFO
                        (void)USBD_LL_FlushEP(pdev, (uint8_t)req->wIndex);
                    }
                }
                break;

            case USB_REQ_SET_CONFIGURATION:
                USBD_LL_PrepareReceive(pdev, BD_EPOUT_ADDR, (uint8_t *)&hmsc->cbw, USBD_CBW_LENGTH);
                break;

            default:
                USBD_CtlError(pdev, req);
                ret = USBD_FAIL;
                break;
            }
            break;

        default:
        USBD_CtlError(pdev, req);
        ret = USBD_FAIL;
        break;
    }

    return (uint8_t)ret;
}

/**
  * @brief  USBD_BD_DataIn
  *         handle data IN Stage
  * @param  pdev: device instance
  * @param  epnum: endpoint index
  * @retval status
  */

uint8_t USBD_BD_DataIn(USBD_HandleTypeDef *pdev, uint8_t epnum)
{
    //trnFrameData();
    return (uint8_t)USBD_OK;
}

/**
  * @brief  USBD_BD_DataOut
  *         handle data OUT Stage
  * @param  pdev: device instance
  * @param  epnum: endpoint index
  * @retval status
  */
//extern void rcvUsbData(USBD_HandleTypeDef *pdev);
uint8_t USBD_BD_DataOut(USBD_HandleTypeDef *pdev, uint8_t epnum)
{

    USBD_BD_HandleTypeDef *hmsc = (USBD_BD_HandleTypeDef *)pdev->pClassData;
    rcvUsbData(pdev, epnum);
    USBD_LL_PrepareReceive(pdev, BD_EPOUT_ADDR, (uint8_t *)&hmsc->cbw, USBD_CBW_LENGTH);
    return (uint8_t)USBD_OK;
}

/**
  * @brief  USBD_BD_GetHSCfgDesc
  *         return configuration descriptor
  * @param  length : pointer data length
  * @retval pointer to descriptor buffer
  */
uint8_t *USBD_BD_GetHSCfgDesc(uint16_t *length)
{
  *length = (uint16_t)sizeof(USBD_BD_CfgHSDesc);

  return USBD_BD_CfgHSDesc;
}

/**
  * @brief  USBD_BD_GetFSCfgDesc
  *         return configuration descriptor
  * @param  length : pointer data length
  * @retval pointer to descriptor buffer
  */
uint8_t *USBD_BD_GetFSCfgDesc(uint16_t *length)
{
  *length = (uint16_t)sizeof(USBD_BD_CfgFSDesc);

  return USBD_BD_CfgFSDesc;
}

/**
  * @brief  USBD_BD_GetOtherSpeedCfgDesc
  *         return other speed configuration descriptor
  * @param  length : pointer data length
  * @retval pointer to descriptor buffer
  */
uint8_t *USBD_BD_GetOtherSpeedCfgDesc(uint16_t *length)
{
  *length = (uint16_t)sizeof(USBD_BD_OtherSpeedCfgDesc);

  return USBD_BD_OtherSpeedCfgDesc;
}
/**
  * @brief  DeviceQualifierDescriptor
  *         return Device Qualifier descriptor
  * @param  length : pointer data length
  * @retval pointer to descriptor buffer
  */
uint8_t *USBD_BD_GetDeviceQualifierDescriptor(uint16_t *length)
{
  *length = (uint16_t)sizeof(USBD_BD_DeviceQualifierDesc);

  return USBD_BD_DeviceQualifierDesc;
}
