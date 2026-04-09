# Custom USB Bulk Device for STM32H7X

## Purpose

This application provides a template for creating a **custom bulk USB device** on the **STM32H7X** microcontroller series.  
The standard HAL Cube library generates CDC, HID, or Mass Storage devices — but not a fully custom device.  
This example helps you develop your own USB device with your own Vendor ID (VID), Product ID (PID), Custom vendor name etc.
Also, you'll get bulk packet data transfers.  I've removed standar

## USB Mode Selection: FS or HS

The example has been tested in **USB_FS** (Full Speed) mode.  
If you wish to use **USB_HS** (High Speed) and have a compatible chip, replace the following line in `usbd_config.h`:
```c
#define DEVICE_FS 1
```

with

```c
#define DEVICE_HS 1
```


>  Ensure your chip supports HS mode and the pins are correctly configured.

## Pin Configuration and USB Initialisation

- Edit `usbd_init.c` - function `initUsbDevicePins` -  to change the USB pins according to your hardware.
    
- Double check that the correct USB mode (`FS` or `HS`) is selected in `usbd_config.h`.
    

## DMA and Buffer Placement

This example uses **DMA** by default. Transmit buffers must be placed in **uncached memory**.  
The following variables must reside in an uncached region:

- `usb_ctl_tx_buffer` (array)
    
- `usbd_bd_Handle` (structure)
    
- `hpcd_USB_OTG` (structure)
    

In this code, they are placed in the **`.noncache`** section, which is defined in the linker script and initialized in `MPU_Config()` (main initialization section).  
Adjust this placement if your memory layout differs.

## Device Descriptors (VID, PID, Strings)

File: `usbd_desc.c`. Here you can change: **VID** (Vendor ID), **PID** (Product ID), Manufacturer string, Product string and other device descriptor fields.


## Working with Endpoints

### Default configuration

The device has been tested with two endpoints: **EP1_OUT** (data reception from host) and **EP1_IN** (data transmission to host)

### Adding more endpoints

If you need additional endpoints you need to modify the descriptors in `usbd_bd.c` - add new endpoint descriptors. Also, you need to Allocate FIFO memory for each new transmit endpoint in `usbd_conf.c`: inside `USBD_LL_Init`, add calls to `HAL_PCDEx_SetTxFiFo`. Note that the receive FIFO (Rx) is single and shared among all endpoints.
## Linux Test Application

### What it does

The test program (in `TestLin/`):

1. Scans the USB bus for the described bulk device.
    
2. Opens the device.
    
3. Sends a test data array.
    
4. Receives a test data array.
    
5. Closes the device.
    

### Dependencies

The program uses **libusb**. Make sure it is installed:

```sh
sudo apt install libusb-1.0-0-dev   # Debian/Ubuntu
```

### Access without root privileges

To use the device as a normal user, create an **udev rule**:

1. Create `/etc/udev/rules.d/99-bulk-device.rules` with the following content:
```
SUBSYSTEM=="usb", ATTRS{idVendor}=="1234", ATTRS{idProduct}=="5678", MODE="0666"
```
    

> Replace `1234` and `5678` with your actual VID and PID.

2. Reload `udev` rules:    
	```sh
		sudo udevadm control --reload-rules
		sudo udevadm trigger
	```

3. Ensure your user belongs to the `plugdev` group:    
```sh
sudo usermod -aG plugdev $USER
```

