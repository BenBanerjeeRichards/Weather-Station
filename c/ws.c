#include <stdio.h>
#include <string.h>
#include <libusb-1.0/libusb.h>
#include <string.h>
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
		ws_usb_error(status, "ws_init::libusb_init");
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
		ws_usb_error(status, "ws_init::libusb_open");
		return WS_ERR_OPEN_FAILED;  
	}
	

	return WS_SUCCESS;
}

void ws_close(ws_device *dev)
{
	libusb_close(dev->hnd);
}

int ws_initialise_read(ws_device *dev)
{
	// Does not matter if this fails, as the kernel may have never 
	// have attached a driver in the first instance or it could have been
	// already been removed earlier
	libusb_detach_kernel_driver(dev->hnd, 0);
	
	int status = libusb_claim_interface(dev->hnd, 0);
	if (status < 0)
	{
		ws_usb_error(status, "ws_initialise_read::libusb_claim_interface");
		return WS_ERR_INTERFACE_CLAIM_FAILED;
	}
	
	int req_type = LIBUSB_ENDPOINT_OUT | LIBUSB_REQUEST_TYPE_CLASS | LIBUSB_RECIPIENT_INTERFACE;			
	unsigned char buf[0];
	
	status =  libusb_control_transfer(dev->hnd, req_type, 0xA, 0x0, 0x0, buf, sizeof(buf), 0);
	if (status < 0)
	{
		ws_usb_error(status, "ws_initialise_read::libusb_control_transfer");
		return WS_ERR_CONTROL_TRANSFER_FAILED;
	}
	
	return WS_SUCCESS;
}

int ws_read_block(ws_device *dev, int address, unsigned char* data, int* read)
{
	int status;
	*read = 0;
	
	unsigned char address_high = address / 256;
	unsigned char address_low = address % 256;
	
	unsigned char write_data[8];
	write_data[0] = 0xA1;
	write_data[1] = address_high;
	write_data[2] = address_low;
	write_data[3] = 0x20;
	write_data[4] = 0xA1;
	write_data[5] = 0x00;
	write_data[6] = 0x00;
	write_data[7] = 0x20;
	
	int req_type = LIBUSB_ENDPOINT_OUT | LIBUSB_REQUEST_TYPE_CLASS | LIBUSB_RECIPIENT_INTERFACE;
	
	status =  libusb_control_transfer(dev->hnd, req_type, 0x9, 0x200, 0x0, write_data, sizeof(write_data), 0);
	if (status < 0)
	{
		ws_usb_error(status, "ws_read_block::libusb_control_transfer");
		return WS_ERR_CONTROL_TRANSFER_FAILED;
	}
	
	for (int i = 0; i < 4; i++)
	{
		int transferred;
		unsigned char buf[8];
		
		int status = libusb_bulk_transfer(dev->hnd, 0x81, buf, 8, &transferred, 0);
		if (status < 0)
		{
			ws_usb_error(status, "ws_read_block::libusb_bulk_transfer");
			return WS_ERR_BULK_TRANSFER_FAILED;
		}
				
		memcpy(&data[(i * 8)], buf, 8);
		*read += transferred;
	}
	
	return WS_SUCCESS;
}
