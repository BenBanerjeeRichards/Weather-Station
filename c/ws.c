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

int ws_latest_record_address(ws_device *dev, int *address)
{
	unsigned char data[32];
	int read;
	*address = 0;
	int status = ws_read_block(dev, 0x00, data, &read);
	if (status != WS_SUCCESS)
	{
		return status;
	}
	
	if (read != 32)
	{
		return WS_ERR_TOO_LITTLE_DATA_READ;
	}
	
	uint8_t byte2 = data[30];
	uint8_t byte1 = data[31];
	uint16_t byte;
	
	byte = ((byte1 << 8) | byte2);
	*address = byte;
	
	return WS_SUCCESS;
}

int ws_process_record_data(unsigned char *data, ws_weather_record *record)
{
	record->indoor_humidity = data[1];
	record->outdoor_humidity = data[4];
	record->indoor_temperature = 0.1 * ws_value_of_bytes(data[3], data[2]);
	record->outdoor_temperature = 0.1 * ws_value_of_bytes(data[6], data[5]);
	record->pressure = 0.1 * ws_value_of_bytes(data[8], data[7]);

	uint8_t wind_speed_low = data[9];
	uint8_t wind_speed_high = data[11] & 0xF;
	record->wind_speed = 0.1 * ((wind_speed_high >> 8) | wind_speed_low);
	
	uint8_t gusting_low = data[10];
	uint8_t gusting_high = (data[11] >> 4);
	record->gust_speed = 0.1 * ((gusting_high >> 8) | gusting_low);
	
	record->wind_direction = 22.5 * data[12];
	record->total_rain = 0.3 * ws_value_of_bytes(data[14], data[13]);
	
	int rain_overflow_mask = 0x80;
	int contact_lost_mask = 0x40;
	
	record->status.sensor_contact_error = ((data[15] & contact_lost_mask) == contact_lost_mask);
	record->status.rain_counter_overflow = ((data[15] & rain_overflow_mask) == rain_overflow_mask);

	return WS_SUCCESS;
}

int ws_read_weather_record(ws_device *dev, int address, ws_weather_record *record)
{
	if (address < 0x100 || address > 0x10000)
	{
		return WS_ERR_INVALID_ADDR;
	}
	
	//Round down to nearest 16
	address = address - (address % 16);
	
	unsigned char data[32];
	int read;
	ws_read_block(dev, address, data, &read);  
	ws_process_record_data(data, record);
	
	return WS_SUCCESS;

}

int ws_read_fixed_block_data(ws_device *dev, unsigned char* fixed_block_data, int* read)
{
	unsigned char data[32];
	int rd;

	for (int i = 0; i < 0x100; i += 0x20)
	{
		int status = ws_read_block(dev, i, data, &rd);
		if (status != WS_SUCCESS)
		{
			return status;
		}

		memcpy(&fixed_block_data[(i / 0x20) * 32], data, 32);
		*read += rd;
	}

	return WS_SUCCESS;
}



void ws_print_block(unsigned char* data)
{
	for (int i = 0; i < 32; i++)
	{	
		printf("%0x\t", data[i]);
		
		if ((i + 1) % 8 == 0)
		{  
			printf("\n");
		}
	}
	
	printf("\n\n");
}

void ws_print_mem_dump(ws_device *dev, int blocks)
{	
	blocks = (blocks == -1) ? 0x10000 : blocks;
	int address = 0x0;
	unsigned char data[32];
	int read;

	for (int j = 0; j < blocks; j++)
	{
	
		int status = ws_read_block(dev, address, data, &read);
		
		if (status != WS_SUCCESS)
		{
			printf("an error occured\n");	// TODO 
		}	
		
		for (int i = 0; i < 32; i++)
		{	
			if (i % 8 == 0)
			{
				printf("0x%0x\t", address);
			}	
			
			printf("%0x\t", data[i]);
			
			if ((i + 1) % 8 == 0)
			{
				address += 0x8;
				printf("\n");
			}
		}
		
		printf("\n");
		
	}
		
}

void ws_print_weather_record(ws_weather_record record)
{
	printf("Indoor Humidity:\t\t %i%% \n", record.indoor_humidity);
	printf("Outdoor Humidity:\t\t %i%% \n", record.outdoor_humidity);
	printf("Indoor Temperature:\t\t %f°C\n", record.indoor_temperature);
	printf("Outdoor Temperature:\t\t %f°C\n", record.outdoor_temperature);
	printf("Pressure:\t\t\t %fhPa\n", record.pressure);
	printf("Wind Speed:\t\t\t %fm/s\n", record.wind_speed);
	printf("Gust Speed:\t\t\t %fm/s\n", record.gust_speed);
	printf("Wind Direction:\t\t\t %f° from north\n", record.wind_direction);
	printf("Total Rain:\t\t\t %fmm\n", record.total_rain);
	printf("Counter Overflow:\t\t %s\n", (record.status.rain_counter_overflow) ? "true" : "false");
	printf("Contact Error:\t\t\t %s\n", (record.status.sensor_contact_error) ? "true" : "false");
	printf("\n");
}


uint16_t ws_value_of_bytes(uint8_t byte1, uint8_t byte2)
{
	return ((byte1 << 8) | byte2);
}

