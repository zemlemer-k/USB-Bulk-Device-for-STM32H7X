/**
 ***********************************************************************************************************************
 * \file        bd_usbd_desc.c
 * \author      Kirill Gribovskiy
 * \version     V 1.0
 * \date        10-Nov-2025
 * \brief       USB descriptors
 ***********************************************************************************************************************
**/
/**
 ***********************************************************************************************************************
 * Include section
 ***********************************************************************************************************************
**/
// System includes
#include <stdint.h>

// Device library includes
#include "stm32h7xx_hal.h"
#include "usbd_def.h"
#include "usbd_ctlreq.h"

// Local includes
#include "usbd_desc.h"
/**
 ***********************************************************************************************************************
 * Definitions
 ***********************************************************************************************************************
**/

#define USBD_VID                  0x1833
#define USBD_PID                  0x4712
#define USBD_DEV_VER              0x0001
#define USBD_LANGID_STRING        1033
#define USBD_MANUFACTURER_STRING  "Test Labs"
#define USBD_PRODUCT_STRING       "Generic Bulk Dev"
#define USBD_CONFIGURATION_STRING "Bulk Transfer Config"
#define USBD_INTERFACE_STRING     "Vendor Specific Interface"


#define USBD_MAX_STR_DESC_SIZ     512U

//#define DEVICE_ID1                0xDEAD  
//#define DEVICE_ID2                0xBEEF  
//#define DEVICE_ID3                0xC0DE

/**
 ***********************************************************************************************************************
 * Types
 ***********************************************************************************************************************
**/

/**
 ***********************************************************************************************************************
 * Function prototypes
 ***********************************************************************************************************************
**/
static uint8_t *USBD_DeviceDescriptor(USBD_SpeedTypeDef speed, uint16_t *length);
static uint8_t *USBD_LangIDStrDescriptor(USBD_SpeedTypeDef speed, uint16_t *length);
static uint8_t *USBD_ManufacturerStrDescriptor(USBD_SpeedTypeDef speed, uint16_t *length);
static uint8_t *USBD_ProductStrDescriptor(USBD_SpeedTypeDef speed, uint16_t *length);
static uint8_t *USBD_SerialStrDescriptor(USBD_SpeedTypeDef speed, uint16_t *length);
static uint8_t *USBD_ConfigStrDescriptor(USBD_SpeedTypeDef speed, uint16_t *length);
static uint8_t *USBD_InterfaceStrDescriptor(USBD_SpeedTypeDef speed, uint16_t *length);

static void Get_SerialNum(void);
static void IntToUnicode(uint32_t value, uint8_t *pbuf, uint8_t len);


/**
 ***********************************************************************************************************************
 * Variables
 ***********************************************************************************************************************
**/

USBD_DescriptorsTypeDef FS_Desc =
{
    USBD_DeviceDescriptor,
    USBD_LangIDStrDescriptor,
    USBD_ManufacturerStrDescriptor,
    USBD_ProductStrDescriptor,
    USBD_SerialStrDescriptor, //0
    USBD_ConfigStrDescriptor,
    USBD_InterfaceStrDescriptor
};


#if defined ( __ICCARM__ ) // IAR Compiler
    #pragma data_alignment=4
#endif // defined ( __ICCARM__ )

// USB standard device descriptor.
//uint8_t USBD_FS_DeviceDesc[USB_LEN_DEV_DESC] __attribute__((aligned(4))) = 
__ALIGN_BEGIN uint8_t USBD_FS_DeviceDesc[USB_LEN_DEV_DESC] __ALIGN_END =
{
    0x12,                       // bLength: always 18-> 0x12
    USB_DESC_TYPE_DEVICE,       // bDescriptorType:  0x01 -> device type
    0x00,                       // bcdUSB : 0x00 0x02 -> USB2; 0x01 0x01 -> USB1.1
    0x02,
    0xFF,                       // bDeviceClass : 0xFF -> vendor specific classs
    0x00,                       // bDeviceSubClass: 0x00 -> subclass is not defined
    0x00,                       // bDeviceProtocol: 0x00 -> protocol is not defined
    USB_MAX_EP0_SIZE,           // bMaxPacketSize: Control endpoint0 max packet size
    LOBYTE(USBD_VID),           // idVendor LSB
    HIBYTE(USBD_VID),           // idVendor MSB
    LOBYTE(USBD_PID),           // idProduct LSB
    HIBYTE(USBD_PID),           // idProduct MSB
    LOBYTE(USBD_DEV_VER),       // bcdDevice -> device version
    HIBYTE(USBD_DEV_VER),
    USBD_IDX_MFC_STR,           // Index of manufacturer  string -> usually 0; here 0x01
    USBD_IDX_PRODUCT_STR,       // Index of product string -> here 0x02
    0x00, //USBD_IDX_SERIAL_STR,// Index of serial number string: not used to prevent windows regestry records
    USBD_MAX_NUM_CONFIGURATION  // bNumConfigurations: number of device configurations -> here 0x01
};

#if defined ( __ICCARM__ ) // IAR Compiler
    #pragma data_alignment=4
#endif // defined ( __ICCARM__ )

// USB lang identifier descriptor
// uint8_t USBD_LangIDDesc[USB_LEN_LANGID_STR_DESC] __attribute__((aligned(4))) =
__ALIGN_BEGIN uint8_t USBD_LangIDDesc[USB_LEN_LANGID_STR_DESC] __ALIGN_END =
{
    USB_LEN_LANGID_STR_DESC,
    USB_DESC_TYPE_STRING,
    LOBYTE(USBD_LANGID_STRING),
    HIBYTE(USBD_LANGID_STRING)
};

#if defined ( __ICCARM__ ) // IAR Compiler
    #pragma data_alignment=4
#endif // defined ( __ICCARM__ )
// Internal string descriptor.
// uint8_t USBD_StrDesc[USBD_MAX_STR_DESC_SIZ] __attribute__((aligned(4))) =
__ALIGN_BEGIN uint8_t USBD_StrDesc[USBD_MAX_STR_DESC_SIZ] __ALIGN_END;

#if defined ( __ICCARM__ ) // IAR Compiler
    #pragma data_alignment=4
#endif // defined ( __ICCARM__ )

// uint8_t USBD_StringSerial[USB_SIZ_STRING_SERIAL] __attribute__((aligned(4))) =
__ALIGN_BEGIN uint8_t USBD_StringSerial[USB_SIZ_STRING_SERIAL] __ALIGN_END =
{
    // USB_SIZ_STRING_SERIAL,
    USB_DESC_TYPE_STRING,
};



/**
 ***********************************************************************************************************************
 * @brief  Return the device descriptor
 * @param  speed : Current device speed
 * @param  length : Pointer to data length variable
 * @retval Pointer to descriptor buffer
 ***********************************************************************************************************************
**/
static uint8_t * USBD_DeviceDescriptor(USBD_SpeedTypeDef speed, uint16_t *length)
{
    UNUSED(speed);
    *length = sizeof(USBD_FS_DeviceDesc);
    return USBD_FS_DeviceDesc;
}

/**
 ***********************************************************************************************************************
 * @brief  Return the LangID string descriptor
 * @param  speed : Current device speed
 * @param  length : Pointer to data length variable
 * @retval Pointer to descriptor buffer
 ***********************************************************************************************************************
**/
static uint8_t * USBD_LangIDStrDescriptor(USBD_SpeedTypeDef speed, uint16_t *length)
{
    UNUSED(speed);
    *length = sizeof(USBD_LangIDDesc);
    return USBD_LangIDDesc;
}

/**
 ***********************************************************************************************************************
 * @brief  Return the manufacturer string descriptor
 * @param  speed : Current device speed
 * @param  length : Pointer to data length variable
 * @retval Pointer to descriptor buffer
 ***********************************************************************************************************************
**/
static uint8_t * USBD_ManufacturerStrDescriptor(USBD_SpeedTypeDef speed, uint16_t *length)
{
    UNUSED(speed);
    USBD_GetString((uint8_t *)USBD_MANUFACTURER_STRING, USBD_StrDesc, length);
    return USBD_StrDesc;
}

/**
 ***********************************************************************************************************************
 * @brief  Return the product string descriptor
 * @param  speed : current device speed
 * @param  length : pointer to data length variable
 * @retval pointer to descriptor buffer
 ***********************************************************************************************************************
 */
static uint8_t * USBD_ProductStrDescriptor(USBD_SpeedTypeDef speed, uint16_t *length)
{
    UNUSED(speed);
    USBD_GetString((uint8_t *)USBD_PRODUCT_STRING, USBD_StrDesc, length);
    return USBD_StrDesc;
}

/**
 ***********************************************************************************************************************
 * @brief  Return the configuration string descriptor
 * @param  speed : Current device speed
 * @param  length : Pointer to data length variable
 * @retval Pointer to descriptor buffer
 ***********************************************************************************************************************
**/
static uint8_t * USBD_ConfigStrDescriptor(USBD_SpeedTypeDef speed, uint16_t *length)
{
    UNUSED(speed);
    USBD_GetString((uint8_t *)USBD_CONFIGURATION_STRING, USBD_StrDesc, length);
    return USBD_StrDesc;
}

/**
 ***********************************************************************************************************************
 * @brief  Return the interface string descriptor
 * @param  speed : Current device speed
 * @param  length : Pointer to data length variable
 * @retval Pointer to descriptor buffer
 ***********************************************************************************************************************
**/
static uint8_t * USBD_InterfaceStrDescriptor(USBD_SpeedTypeDef speed, uint16_t *length)
{
    UNUSED(speed);
    USBD_GetString((uint8_t *)USBD_INTERFACE_STRING, USBD_StrDesc, length);
    return USBD_StrDesc;
}

/**
 ***********************************************************************************************************************
 * @brief  Return the serial number string descriptor
 * @param  speed : Current device speed
 * @param  length : Pointer to data length variable
 * @retval Pointer to descriptor buffer
 ***********************************************************************************************************************
**/
static uint8_t * USBD_SerialStrDescriptor(USBD_SpeedTypeDef speed, uint16_t *length)
{
    UNUSED(speed);
    *length = USB_SIZ_STRING_SERIAL;
    Get_SerialNum();
    return (uint8_t *) USBD_StringSerial;
}

/**
 ***********************************************************************************************************************
 * @brief  Create the serial number string descriptor
 * @param  None
 * @retval None
 ***********************************************************************************************************************
**/
static void Get_SerialNum(void)
{
    uint32_t deviceserial0, deviceserial1, deviceserial2;
    deviceserial0 = *(uint32_t *) DEVICE_ID1;
    deviceserial1 = *(uint32_t *) DEVICE_ID2;
    deviceserial2 = *(uint32_t *) DEVICE_ID3;

    deviceserial0 += deviceserial2;

    if (deviceserial0 != 0) {
        IntToUnicode(deviceserial0, &USBD_StringSerial[2], 8);
        IntToUnicode(deviceserial1, &USBD_StringSerial[18], 4);
    }
}


/**
 ***********************************************************************************************************************
 * @brief  Convert Hex 32Bits value into char
 * @param  value: value to convert
 * @param  pbuf: pointer to the buffer
 * @param  len: buffer length
 * @retval None
 ***********************************************************************************************************************
**/
static void IntToUnicode(uint32_t value, uint8_t *pbuf, uint8_t len)
{
    uint8_t idx = 0;
    for (idx = 0; idx < len; idx++) {
        if (((value >> 28)) < 0xA)     {
            pbuf[2 * idx] = (value >> 28) + '0';
        }
        else {
            pbuf[2 * idx] = (value >> 28) + 'A' - 10;
        }
        value = value << 4;
        pbuf[2 * idx + 1] = 0;
    }
}

