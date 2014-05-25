#include <stdio.h>
#include <string.h>
#include <libusb-1.0/libusb.h>

#include "ws.h"

void ws_usb_error(int status, const char* additonal_info)
{
	printf("[LIBUSB ERR][ERR NO %i] %s (%s)\n", status, libusb_error_name(status), additonal_info);
}

int ws_init(ws_device *dev)
{
	int status;
	
	status = libusb_init(NULL);
	if (status < 0)
	{
		ws_usb_error(status, "libusb_init");
		return WS_ERR_USB_INIT_FAILED;  
	}
	
	// libusb_open_device_with_vid_pid() doesn't seem to work as intended, so 
	// instead the program loops through all of the devices until the weather station
	// is found
	
	libusb_device **devs;
	int count = libusb_get_device_list(NULL, &devs);
	
	if (count == 0)
	{
		return WS_ERR_NO_STATION_FOUND;
	}
	
	for (int i = 0; i < count; i++)
	{
		struct libusb_device_descriptor desc;
		
		libusb_get_device_descriptor(devs[i], &desc);
		
		if (desc.idVendor == 0x1941 && desc.idProduct == 0x8021)
		{
			dev->dev = devs[i];
			dev->desc = &desc;
			break;
		}
		
		if (i == (count - 1))
		{
			return WS_ERR_NO_STATION_FOUND;
		}

	}
	
	
	status = libusb_open(dev->dev, &dev->hnd);
	if (status < 0)
	{
		ws_usb_error(status, "libusb_open");
		return WS_ERR_OPEN_FAILED;  
	}
	
	return WS_SUCCESS;
}

void ws_close(ws_device *dev)
{
	libusb_close(dev->hnd);
}