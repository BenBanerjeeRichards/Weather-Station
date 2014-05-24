#include <stdio.h>
#include <string.h>
#include <libusb-1.0/libusb.h>

#include "ws.h"

int ws_init(ws_device *dev)
{
	int status;
	
	libusb_set_debug(NULL, LIBUSB_LOG_LEVEL_WARNING);
	// TODO set logging, and everything else
	status = libusb_init(NULL);
	if (status < 0)
	{
		printf("[LIBUSB ERROR] %s\n", libusb_error_name(status));
		return WS_ERR_USB_INIT_FAILED;
	}
	
	dev->hnd = libusb_open_device_with_vid_pid(NULL, 0x1941, 0x8021);
	if (dev->hnd == NULL)
	{
		printf("[LIBUSB ERROR] %s\n", libusb_error_name(status));
		return WS_ERR_NO_STATION_FOUND;
	}
	
	
	return 0;
}