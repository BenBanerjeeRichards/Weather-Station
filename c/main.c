#include <stdio.h>
#include <libusb-1.0/libusb.h>

/* W-8681: 	vendor_id: 1941 
			product_id: 8021			
*/

int main(int argc, char** args)
{	
	libusb_device **devs;
	if (libusb_init(NULL) < 0)
	{
		printf("Failed to intialise libusb\n");
		return 1;
	}
	
	
	size_t count = libusb_get_device_list(NULL, &devs);
	
	int i = 0;
	
	uint16_t vendor_id = 0x1941;
	uint16_t product_id = 0x8021;
	
	while (devs[i] != NULL)
	{
		struct libusb_device_descriptor desc;
		if (libusb_get_device_descriptor(devs[i], &desc) < 0)
		{
			printf("Failed to get device descriptor\n");
		}
		
		if (desc.idVendor == vendor_id && desc.idProduct == product_id)
		{
			int status;
			printf("[INFO] Found Weather Station vid:1941 pid:8021 \n");
			// Weather station 
			struct libusb_device_handle* hnd;
			
			// Open USB device
			status = libusb_open(devs[i], &hnd);
			if (status < 0)
			{
				printf("[ERROR] Failed to open device %s\n", libusb_error_name(status));
				return 1;
			}
			
			status = libusb_detach_kernel_driver(hnd, 0);
			
			if ( status < 0)
			{
				// Not a fatel error, just means the kernal never had a driver for this device attached
			}
			
			status = libusb_claim_interface(hnd, 0);
			if (status < 0)
			{
				printf("Failed to claim interface %s\n", libusb_error_name(status));
			}
			
			status = libusb_set_interface_alt_setting(hnd, 0, 0);
			
			if (status < 0 )
			{
				printf("Alt Setting Failed: %s\n", libusb_error_name(status));
				return 1;
			}
			 
			int address = 0x01FFFF; 
			
			unsigned char address_high = address / 256;
			unsigned char address_low = address % 256;
			
			unsigned char write_data[8];
			write_data[0] = (char)0xA1;
			write_data[1] = (char)0x00; //address_high;
			write_data[2] = (char) 0x00; //address_low;
			write_data[3] = (char)0x20;
			write_data[4] = (char)0xA1;
			write_data[5] = (char) 0x00;
			write_data[6] = (char) 0x00;
			write_data[7] = (char) 0x20;
			
			// Control Transfer 1 																			//
			// ------------------------------------------------------------------------------------------- //
			
			int req_type = LIBUSB_ENDPOINT_OUT | LIBUSB_REQUEST_TYPE_CLASS | LIBUSB_RECIPIENT_INTERFACE;			
			unsigned char buf[0];
			
			printf("[STAT] Begin control transfer ... ");
			status =  libusb_control_transfer(hnd, req_type, 0xA, 0x0, 0x0, buf, sizeof(buf), 500);
			
			if (status < 0)
			{
				printf("failed\n");
				printf("[ERROR] Control Transfer Failed: %s\n", libusb_error_name(status));
				return 1;
			}
			printf("done\n");

			
			// Control Transfer 2 																			//
			// ------------------------------------------------------------------------------------------- //
			
			req_type = LIBUSB_ENDPOINT_OUT | LIBUSB_REQUEST_TYPE_CLASS | LIBUSB_RECIPIENT_INTERFACE;		

			printf("[STAT] Begin control transfer ... ");
			status =  libusb_control_transfer(hnd, req_type, 0x9, 0x200, 0x0, write_data, sizeof(write_data), 500);
			
			if (status < 0)
			{
				printf("failed\n");
				printf("[ERROR] Control Transfer Failed: %s\n", libusb_error_name(status));
				return 1;
			}
			
			printf("done\n");
						 
						 
			// Bulk Transfer (read)																		//
			// ------------------------------------------------------------------------------------------- //
			
			unsigned char data[32];
			int transferred;
			
			printf("[STAT] Begin bulk read ... ");
			status = libusb_bulk_transfer(hnd, 0x81, data, 32, &transferred, 500);
		
			if (status < 0)
			{
				printf("failed\n");
				printf("[ERROR] Control Transfer Failed: %s\n", libusb_error_name(status));
				return 1;
			}
			
			printf("done\n");
			
			for (int i = 0; i < 8; i++)
			{
				printf(" %0x ", data[i]);
			}
			
			printf("\n");
		
			// Close USB device
			libusb_close(hnd);
			
		}
		  
				
		i++;
	}
	
	libusb_free_device_list(devs, 1);
	return 0;
}