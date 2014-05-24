#include <stdio.h>
#include <string.h>
#include <libusb-1.0/libusb.h>

/* W-8681: 	vendor_id: 1941 
			product_id: 8021			
*/

int read_data(libusb_device_handle* hnd, int address, unsigned char* data);

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
			 
			
			
			// Control Transfer 1 																			//
			// ------------------------------------------------------------------------------------------- //
			
			int req_type = LIBUSB_ENDPOINT_OUT | LIBUSB_REQUEST_TYPE_CLASS | LIBUSB_RECIPIENT_INTERFACE;			
			unsigned char buf[0];
			
			printf("[STAT] Begin control transfer ... ");
			status =  libusb_control_transfer(hnd, req_type, 0xA, 0x0, 0x0, buf, sizeof(buf), 0);
			
			if (status < 0)
			{
				printf("failed\n");
				printf("[ERROR] Control Transfer Failed: %s\n", libusb_error_name(status));
				return 1;
			}
			printf("done\n");

			
			// Control Transfer 2 																			//
			// ------------------------------------------------------------------------------------------- //
			unsigned char data[32];
			
			
			int address = 0x00000000;
			
			for (int i = 0x0; i < 0x10000; i += 0x20)
			{
				read_data(hnd, address, data);
				address += 0x20;

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

int read_data(libusb_device_handle* hnd, int address, unsigned char* data)
{
	//printf("Reading from 0x%0x\n", address );
	printf("\n");
	int status;
			
	unsigned char address_high = address / 256;
	unsigned char address_low = address % 256;
	
	unsigned char write_data[8];
	write_data[0] = (char)0xA1;
	write_data[1] =  address_high;
	write_data[2] =  address_low;
	write_data[3] = (char)0x20;
	write_data[4] = (char)0xA1;
	write_data[5] = (char) 0x00;
	write_data[6] = (char) 0x00;
	write_data[7] = (char) 0x20;
	
	int req_type = LIBUSB_ENDPOINT_OUT | LIBUSB_REQUEST_TYPE_CLASS | LIBUSB_RECIPIENT_INTERFACE;		

	//printf("[STAT] Begin control transfer ... ");
	status =  libusb_control_transfer(hnd, req_type, 0x9, 0x200, 0x0, write_data, sizeof(write_data), 0);
	
	if (status < 0)
	{
		printf("failed\n");
		printf("[ERROR] Control Transfer Failed: %s\n", libusb_error_name(status));
		return 1;
	}
	
	//printf("done\n");
				 
				 
	// Bulk Transfer (read)																		//
	// ------------------------------------------------------------------------------------------- //
	
	
	for (int i = 0; i < 4; i++)
	{
		int transferred;
		unsigned char buf[8];
		
		//printf("[STAT] Begin bulk read ... ");
		status = libusb_bulk_transfer(hnd, 0x81, buf, 8, &transferred, 0);

		if (status < 0)
		{
			//printf("failed\n");
			printf("[ERROR] Bulk Read Failed: %s\n", libusb_error_name(status));
			return 1;
		}
		
		//printf("done\n");
		
		memcpy(&data[(i * 8)], buf, 8);
	}
	
	int address2 = address;
	
	for (int i = 0; i < 32; i++)
	{	
		if (i % 8 == 0)
		{
			printf("0x%0x\t", address2);
		}	
		
		printf("%0x\t", data[i]);
		
		if (i == 7 || i == 15 || i == 23 || i == 31)
		{
			address2 += 0x8;
			printf("\n");
		}
		
	}
		
		
	
	return 0;
}