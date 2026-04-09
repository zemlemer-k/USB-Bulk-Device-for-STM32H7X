#include <stdio.h>
#include <stdint.h>
#include <libusb.h>

#define USBD_VID    0x1833
#define USBD_PID    0x4712

#define INP_ENDP    0x81
#define OUT_ENDP    0x01        

#define TIMEOUT     1000


static libusb_device_handle *openDevice(uint16_t dev_vid, uint16_t dev_pid);
static int getDeviceEndpoints(libusb_device_handle* hDev, uint8_t *endpPtr, int epNum);


int main(void)
{
    // Libusb initialization
    int result = libusb_init(NULL);
    if(result) {
        fprintf(stderr, "Error initializing libusb: %s\n", libusb_error_name(result));
        return -1;
    }

    // Opening device
    libusb_device_handle *device_handle = openDevice(USBD_VID, USBD_PID);
    if(NULL == device_handle) {
        fprintf(stderr, "Failed to open device\n");
        libusb_exit(NULL);
        return -2;
    }

    // Endpoints verification
    uint8_t endpArr[2] = {0,0};
    if(2 != getDeviceEndpoints(device_handle, endpArr, 2)) {
        fprintf(stderr, "Error reading device endpoints\n");
        libusb_close(device_handle);
        libusb_exit(NULL);
    }

    if(!(((INP_ENDP == endpArr[0]) && (OUT_ENDP == endpArr[1])) ||
         ((INP_ENDP == endpArr[1]) && (OUT_ENDP == endpArr[0])))) {
        fprintf(stderr, "Device endpoints are other than expected: 0x%X and 0x%X instead of 0x%X and 0x%X\n", 
                endpArr[0], endpArr[1], INP_ENDP, OUT_ENDP);
        libusb_close(device_handle);
        libusb_exit(NULL);
    }
    uint8_t transferBuf[3] = {0, 1, 2};
    int32_t actual_length;

    result = libusb_bulk_transfer(device_handle, OUT_ENDP, transferBuf, 3, &actual_length, TIMEOUT);
    if(result) {
        fprintf(stderr, "out bulk transfer error: %s\n", libusb_error_name(result));
        libusb_close(device_handle);
        libusb_exit(NULL);
    }
  
    fprintf(stdout, "out bulk transfer: %d, %d, %d \n", transferBuf[0], transferBuf[1], transferBuf[2]);

    result = libusb_bulk_transfer(device_handle, INP_ENDP, transferBuf, 3, &actual_length, TIMEOUT);
    if(result) {
        fprintf(stderr, "input bulk transfer error: %s\n", libusb_error_name(result));
        libusb_close(device_handle);
        libusb_exit(NULL);
    }

    fprintf(stdout, "input bulk transfer: %d, %d, %d \n", transferBuf[0], transferBuf[1], transferBuf[2]);

    libusb_close(device_handle);
    libusb_exit(NULL);

    fprintf(stdout, "Bulk device test successful \n");
    return 0;
}

/**
 ***********************************************************************************************************************
 * \brief   openDevice
 * \details Searching our device among available usb devices and opening it
 * \param   uint16_t dev_vid -- device vendor id
 * \param   uint16_t dev_pid -- device product id
 * \retval  Pointer to the device descriptor handle of NULL in a case of fail
 ***********************************************************************************************************************
**/
static libusb_device_handle *openDevice(uint16_t dev_vid, uint16_t dev_pid)
{
    libusb_device **list = NULL;
	ssize_t cnt = libusb_get_device_list(NULL, &list);
	if (cnt < 0) {
		fprintf(stderr, "Cannot retrieve USB devices: %s\n", libusb_error_name(cnt));
		return NULL;
	}
    libusb_device_handle *dev_handle = NULL;

    for(size_t idx = 0; idx < cnt; idx++) {
        libusb_device *device = list[idx];
        struct libusb_device_descriptor desc;

        int result = libusb_get_device_descriptor(device, &desc);
        if(result < 0) {
            fprintf(stderr, "Error reading device descriptor: %s\n", libusb_error_name(result));
            libusb_free_device_list(list, 1);
            return NULL;
        }

        if((dev_vid == desc.idVendor) && (dev_pid == desc.idProduct)) {
            fprintf(stdout, "Device found\n");
            result = libusb_open(device, &dev_handle);
            if(result < 0) {
                fprintf(stderr, "Error opening found device: %s\n", libusb_error_name(result));
                libusb_free_device_list(list, 1);
                return NULL;
            }
            break;
        }
    }
    libusb_free_device_list(list, 1);
    return dev_handle;
}

/**
 ***********************************************************************************************************************
 * \brief   getDeviceEndpoints
 * \details Searching for device input and output endpoints
 * \param   libusb_device* hDev -- our device handle
 * \param   uint8_t *endpPtr -- pointer to the endpoints array
 * \param   uint8_t epNum -- expected number of endpoints 
 * \retval  Return value: Positive of zero value -- number of available endpoints
 *                       -1 -- Invalid input parameters
 *                       -2 -- Cannot get device from handle
 *                       -3 -- cannot get active config descriptor
 ***********************************************************************************************************************
**/
static int getDeviceEndpoints(libusb_device_handle* hDev, uint8_t *endpPtr, int epNum)
{
    if (hDev == NULL || endpPtr == NULL || epNum <= 0) {
        fprintf(stderr, "getDeviceEndpoints: invalid parameters\n");
        return -1;
    }

    libusb_device *dev = libusb_get_device(hDev);
    if(dev == NULL) {
        fprintf(stderr, "getDeviceEndpoints: cannot get device from handle\n");
        return -2;
    }

    struct libusb_config_descriptor *configDescr = NULL;
    int retval = libusb_get_active_config_descriptor(dev, &configDescr);
    if(retval != 0) {
		fprintf(stderr, "getDeviceEndpoints: cannot get active config descriptor: %s\n", libusb_error_name(retval));
		return -3;    
    }

    int epCnt = 0;
    if (configDescr->bNumInterfaces == 0) {
        libusb_free_config_descriptor(configDescr);
        return 0;
    }

    for (int i = 0; i < configDescr->bNumInterfaces; i++) {
        const struct libusb_interface *iface = &configDescr->interface[i];
        for (int j = 0; j < iface->num_altsetting; j++) {
            const struct libusb_interface_descriptor *altsetting = &iface->altsetting[j];
            for (int k = 0; k < altsetting->bNumEndpoints; k++) {
                if (epCnt < epNum) {
                    endpPtr[epCnt++] = altsetting->endpoint[k].bEndpointAddress;
                }
            }
        }
    }
 
	libusb_free_config_descriptor(configDescr);
    return epCnt;
}

