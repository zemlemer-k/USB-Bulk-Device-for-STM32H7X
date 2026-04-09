/**
 ***********************************************************************************************************************
 * \file        bd_usbexch.c
 * \author      Kirill Gribovskiy
 * \version     V 1.0
 * \date        08-Dec-2025
 * \details     USB packets exchange API
 *
 ***********************************************************************************************************************
**/
/**
 ***********************************************************************************************************************
 * Include section
 ***********************************************************************************************************************
**/
// System includes
#include <stdbool.h>
#include <string.h>
// Device library includes
#include "stm32h7xx_hal.h"
#include "stm32h7xx_hal_pcd.h"
#include "usbd_def.h"
// Local includes
#include "usbd_bd.h"
#include "usbd_init.h"
#include "lib_terminal.h"
#include "bd_usbexch.h"

/**
 ***********************************************************************************************************************
 * Definitions
 ***********************************************************************************************************************
**/
#define USBD_TRN_BUFF_SZ	1024


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
static uint8_t *rcvData;
static uint16_t rcvDataSz = 0;
static volatile bool rcvFlag = false;
__attribute__((section(".noncache")))  uint8_t usbdTrnBuffer[USBD_TRN_BUFF_SZ];

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
 * \brief   clearInEP
 * \details Clearing endpoint data to stop data exchange
 * \param   uint8_t ep_num -- transmit endpoint number
 * \retval  Nothing
 ***********************************************************************************************************************
**/
void clearInEP(uint8_t ep_num)
{
	UNUSED(ep_num);
	//uint8_t inEpnt = ep_num | 80; // BD_EPIN_ADDR
	uint8_t inEpnt = BD_EPIN_ADDR;
	PCD_HandleTypeDef *hpcd = hUsbDevice.pData;
	uint32_t USBx_BASE = (uint32_t)(hpcd->Instance);
	//USB_OTG_INEndpointTypeDef* inEp = USBx_INEP(hpcd->IN_ep[inEpnt & 0x7F].num);
	USB_OTG_INEndpointTypeDef* inEp = USBx_INEP(hpcd->IN_ep[inEpnt & 0xF].num);
	//Disable IN endpoint
	if ((inEp->DIEPCTL & USB_OTG_DIEPCTL_EPENA) == USB_OTG_DIEPCTL_EPENA)
	{
		USBx_DEVICE->DIEPMSK &= ~USB_OTG_DIEPMSK_INEPNEM;
		//mask NAK effective interrupt
		USBx_DEVICE->DIEPMSK &= ~USB_OTG_DIEPMSK_EPDM;
		//Mask EP disabled interrupt for polling
		inEp->DIEPCTL |= USB_OTG_DIEPCTL_SNAK;
		while((inEp->DIEPINT & USB_OTG_DIEPINT_INEPNE) == 0);
		//wait 'till stopped
		inEp->DIEPCTL |= (USB_OTG_DIEPCTL_EPDIS | USB_OTG_DIEPCTL_SNAK);
		while((inEp->DIEPINT & USB_OTG_DIEPINT_EPDISD) == 0);
		//wait for disable done
		inEp->DIEPINT = USB_OTG_DIEPINT_EPDISD;
		//Clear endpoint disabled interrupt
		USBx_DEVICE->DIEPMSK |= USB_OTG_DIEPMSK_EPDM; //unmask;
		//USBD_LL_FlushEP(&hUsbDevice, inEpnt);
		USBD_LL_FlushEP(&hUsbDevice, BD_EPIN_ADDR);
		inEp->DIEPCTL |= USB_OTG_DIEPCTL_CNAK;
		while((inEp->DIEPCTL & USB_OTG_DIEPCTL_NAKSTS) != 0);
		inEp->DIEPINT = USB_OTG_DIEPINT_INEPNE;
		//Clear NAK effective interrupt
		//unmask NAK EFFECTIVE interrupt
		USBx_DEVICE->DIEPMSK |= USB_OTG_DIEPMSK_INEPNEM;
	}
	inEp->DIEPTSIZ = 0;
}

/**
 ***********************************************************************************************************************
 * \brief   trnUsbData
 * \details Transmit data to the host computer
 * \param   uint16_t dataSz -- data size in bytes
 *			uint8_t *dataPtr -- data pointer
 * \retval  Error code:		0 -- Ok
 *						   -1 -- Unable to transmit
 ***********************************************************************************************************************
**/
int trnUsbData(uint16_t dataSz, uint8_t *dataPtr, bool copyData, uint8_t epNum)
{
	UNUSED(epNum);
	uint8_t inpEp = BD_EPIN_ADDR; // inpEp - epNum | 0x80;

	if(true == copyData) {
		if(dataSz > USBD_TRN_BUFF_SZ)
			dataSz = USBD_TRN_BUFF_SZ;
		memcpy(usbdTrnBuffer, dataPtr, dataSz);
		if(USBD_OK == USBD_LL_Transmit(&hUsbDevice, inpEp, usbdTrnBuffer, dataSz))
			return 0;
		else
			return -1;
		
	}
	
	if(USBD_OK == USBD_LL_Transmit(&hUsbDevice, BD_EPIN_ADDR, dataPtr, dataSz))
		return 0;
	return -1;
}

/**
 ***********************************************************************************************************************
 * \brief   rcvUsbData
 * \details Reading data from host computer
 * \param   USBD_HandleTypeDef *pdev -- pointer to the USB handle
 * \retval  Nothing
 ***********************************************************************************************************************
**/
void rcvUsbData(USBD_HandleTypeDef *pdev, uint8_t epnum)
{
	USBD_BD_HandleTypeDef *hmsc = (USBD_BD_HandleTypeDef *)pdev->pClassData;
	rcvData = (uint8_t*)hmsc->cbw;
	rcvFlag = true;
	rcvDataSz = USBD_LL_GetRxDataSize(pdev, epnum);

	setTerminalMessage(info, "received data size: %d", rcvDataSz);
}

/**
 ***********************************************************************************************************************
 * \brief   newHostCommand
 * \details Checking if there is a valid new host command
 * \param   Nothing
 * \retval  true - new command from host, otherwize - false
 ***********************************************************************************************************************
**/
bool newHostCommand(void)
{
	return rcvFlag;
}

/**
 ***********************************************************************************************************************
 * \brief   getRcvHostData
 * \details Getting last data pointer
 * \param   Nothing
 * \retval  Pointer to the received data
 ***********************************************************************************************************************
**/
uint8_t *getRcvHostData(void)
{
	rcvFlag = false;
	return rcvData;
}

/**
 ***********************************************************************************************************************
 * \brief   getRcvHostDataSz
 * \details Getting last data size
 * \param   Nothing
 * \retval  Size of bytes received from host
 ***********************************************************************************************************************
**/
uint16_t getRcvHostDataSz(void)
{
	return rcvDataSz;
}

