/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __USBD_BD_H
#define __USBD_BD_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include  "usbd_ioreq.h"


/* BD Class Config */
#ifndef BD_MEDIA_PACKET
#define BD_MEDIA_PACKET             512U
#endif /* BD_MEDIA_PACKET */

#define BD_MAX_FS_PACKET            0x40U
#define BD_MAX_HS_PACKET            0x200U

#define USB_BD_CONFIG_DESC_SIZ      32
#define USBD_CBW_LENGTH             64

#define BD_EPIN_ADDR                0x81U
#define BD_EPOUT_ADDR               0x01U

typedef struct
{
    uint8_t     cbw[USBD_CBW_LENGTH];
    uint32_t    interface;
}
USBD_BD_HandleTypeDef;

/* Structure for MSC process */
extern USBD_ClassTypeDef  USBD_BD;
#define USBD_BD_CLASS     &USBD_BD

#ifdef __cplusplus
}
#endif

#endif  /* __USBD_BD_H */
