#ifndef WS_H
#define WS_H

#include <libusb-1.0/libusb.h>

// --------- Enum and Struct Definitions --------- //

/**
	Error enums with const* char array
    http://stackoverflow.com/questions/9907160/how-to-convert-enum-names-to-string-in-c
*/

#define FOREACH_WS_ERR(ERROR) 				\
	ERROR(WS_ERR_NO_STATION_FOUND) 			\
	ERROR(WS_ERR_USB_INIT_FAILED) 			\
	ERROR(WS_ERR_INTERFACE_CLAIM_FAILED) 	\
	ERROR(WS_ERR_INVALID_PERMISSIONS) 		\
	ERROR(WS_ERR_CONTROL_TRANSFER_FAILED) 	\
	ERROR(WS_ERR_BULK_TRANSFER_FAILED)	 	\
	ERROR(WS_ERR_INVALID_ADDR) 				\
	ERROR(WS_ERR_NO_DEVICE)					\
	ERROR(WS_ERR_OPEN_FAILED) 				\
	ERROR(WS_ERR_TOO_LITTLE_DATA_READ) 		\
	
#define GENERATE_ENUM(ENUM) ENUM,
#define GENERATE_STRING(STRING) #STRING,

enum ws_errors {
	FOREACH_WS_ERR(GENERATE_ENUM)
};

enum ws_success {
	WS_SUCCESS = -10
};

static const char *ws_errors_string[] = {
	FOREACH_WS_ERR(GENERATE_STRING)
};

/*
	Following enums define types for units 
*/

enum ws_unit_speed
{
	METRES_SECOND, KILOMETERS_HOUR, KNOT, MILES_HOUR, BEAUFORT
};

enum ws_unit_volume 
{
	MILLIMETER, INCH
};

enum ws_unit_temp
{
	CELSIUS, FAHRENHEIT
};

enum ws_unit_pressure
{
	HECTOPASCALS, INCH_MERCURY, MILLIMETER_MERCURY
};

/**
	Holds all of the information for accessing the device
*/

typedef struct
{
	libusb_device* dev;
	struct libusb_device_descriptor* desc;
	struct libusb_device_handle* hnd;
}  ws_device;


/**
	Holds information regarding the units begin used
*/

typedef struct 
{
	enum ws_unit_temp indoor_temp_unit;
	enum ws_unit_temp outdoor_temp_unit;
	enum ws_unit_volume rain_unit;
	enum ws_unit_pressure pressure_unit;
	enum ws_unit_speed wind_speed_unit;
} ws_data_info;


/**
Status of the weather station
*/
typedef struct
{
	int sensor_contact_error;
	int rain_counter_overflow;
} ws_station_status;


/**
	Holds a single weather record - this is where the actual weather data is stored
*/
typedef struct 
{
	int indoor_humidity;
	int outdoor_humidity;
	
	double indoor_temperature;
	double outdoor_temperature;
	
	double pressure;
	
	double wind_speed;
	double gust_speed;
	double wind_direction;
	
	double total_rain;
	ws_station_status status;

} ws_weather_record;  

/** 
	Holds a time
*/
typedef struct 
{
	int year;
	int month;
	int day;
	int hour;
	int minute;
} ws_time;

/**
	Holds the min, max values for an item as well as when that event occurred
*/

typedef struct 
{
	double min;
	double max;
	
	ws_time min_time;
	ws_time max_time;

} ws_min_max;

/**
	Holds all of the extreme values (min-maxes)
	Note that rain extremes have no minimums, so they will be set to 0.
*/

typedef struct 
{
	ws_min_max indoor_humidity;
	ws_min_max outdoor_humidity;
	
	ws_min_max indoor_temperature;
	ws_min_max outdoor_temperature;
	
	ws_min_max wind_chill;
	ws_min_max dew_point;
	
	ws_min_max absolute_pressure;
	ws_min_max relative_pressure;
	
	ws_min_max wind_speed;
	ws_min_max gust_speed;
	
	ws_min_max rain_daily;		// no min
	ws_min_max rain_weekly;		// no min
	ws_min_max rain_monthly;	// no min
	ws_min_max rain_total;		// no min
} ws_weather_extremes;


// --------- Function Definitions --------- //

/**
	Notes: 	
		- Every int-returning function returns a number LESS THAN 0 on success. This is as a 
		result of using enums for errors. The actual value being returned is WS_SUCCESS. This means
		that errors can be checked by:
		
		if (int_returning_func() >= 0) 		// Has no meaning to someone who hasn't read the documentation.
		{
			// Handle error
		}
		
		or
		
		if (int_returning_func() != WS_SUCCESS)		// Much more obvious, hence this is the recommended method.
		{
			// Handle error
		}
		
		- Most functions require root priviliges to work.
*/



/**
	Initialises things:
		- Finds the weather station
		- Retrieves a handle for the station
		- Fills out the given ws_device struct 
	
	Parameters:
		- dev:		A device struct, empty at this point which 
					will be filled out by the function
	
	Return:
		- WS_ERR_NO_STATION_FOUND	No weather station could be found 
		- WS_ERR_USB_INIT_FAILED	libusb  failed to initialise
		- WS_ERR_NO_DEVICE			No device could be found from the handle
		- WS_ERR_OPEN_FAILED		Libusb failed to open the device
*/

int ws_init(ws_device *dev);


/**
	Closes the handle of the USB device, so that it can no
	longer be used.
	
	Passing ws_device to another ws_* function other that ws_init will
	result in errors.
	
	Parameters:
		- dev: 		A device struct for the device to be closed
	
*/

void ws_close(ws_device *dev);


/**
	Sends a control transfer to the device just before any reading takes place.
	This function must be called before any reading (ws_read*) takes place.
	   
	Function will:
		- If the kernel has an attached driver, detach it
		- Claim the interface 
		- Send a control message to the weather station 
		
	This function only needs to be called once after ws_init

	
	Parameters:
		- dev	 	A device struct for the device 
		
	Return:
		- WS_ERR_INTERFACE_CLAIM_FAILED 	The function was unable to claim the 
											interface
											 
		- WS_ERR_INVALID_PERMISSIONS		The program was not given sufficient 
											priviliges to complete the task
		
		- WS_ERR_CONTROL_TRANSFER_FAILED	Control transfer failed. See log for 
											libusb error.
*/
int ws_initialise_read(ws_device *dev);


/**
	Reads the first 256 bytes of the weather station's memory. This data contains 
	alarm information and min and max values, along with some other useful data such 
	as the latest weather record position.
	
	This function, on a high level, is considered a read function. But two transfers must
	happen to retrieve the data:
		
		1) Control transfer to endpoint 0x0. This sends a few commands to the weather station,
		giving it information, including the address of the date in the EPROM we would like
		to access.
		
		2) Bulk transfer from endpoint 0x81. This reads the data from the endpoint that the 
		weather station has written to, from the address we requested. 
		
	32 Bytes of data at a time is requested, and the data is retrieved in 4 8 byte bulk transfers.
	This means that to retrive all 256 bytes of data, 8 control transfers must take place.
	
	This data can be read in single 32 byte chunks by calling ws_read_block(), which should be used 
	if only specific chunks are required, due to the fact that reading 256 bytes of data takes longer 
	that only 32.
	
	Parameters:
		- dev: 		A device struct for the device to be read from
		- data:  	The data array
	
	Return:
		- WS_ERR_CONTROL_TRANSFER_FAILED	Request for data write failed
		- WS_ERR_BULK_TRANSFER_FAILED		Data read failed 
*/

int ws_read_fixed_block_data(ws_device *dev, unsigned char* fixed_block_data, int* read);


/**
	Reads a single block from a specified address from the weather station. A single 
	block is 32 bytes.
	
	Parameters:
		- dev: 					A device struct for the device to be read from
		- data:  				The data array
		
		- address:				The address to be read from. Note that this number
								will be rounded down to the nearest multiple of 32, 
								as all data is read in 32 byte chunks
								
								The memory is in the range of 0x0000 -> 0xFFFF. Weather
								records start at 0x100
		
		- read					The number of bytes read					
								
	Return:
		- WS_ERR_CONTROL_TRANSFER_FAILED	Request for data write failed
		- WS_ERR_BULK_TRANSFER_FAILED		Data read failed 
*/
int ws_read_block(ws_device *dev, int address, unsigned char* data, int* read);

/**
	Retrieves the address in the device's memory of the latest weather record saved. Useful, as
	the data is stored in a circular buffer where old data is overwritten when the memory has all
	been used up.   
	
	This address is stored in the fixed block memory, at position 0x1E and 0x1F.
	
	Parameters:
		- dev: 			A device struct for the device 
		- address: 		The address of the latest record.	
		
	Return:
		- WS_ERR_CONTROL_TRANSFER_FAILED	Request for data write failed
		- WS_ERR_BULK_TRANSFER_FAILED		Data read failed 
*/

int ws_latest_record_address(ws_device *dev, int *address);


/**
	Takes a weather record's raw data (32 byte unsigned char array) and processes
	it, placing the data in a ws_weather_record struct.
	
	Parameters:
		data:		32 bytes of record data. This data must be from with the range 
					0x100 -> 0xFFFF, and contain 32 bytes. Any more bytes will be 
					ignored, any less will cause the program to crash.
					
		record		A record stuct to put the processed data in.
		
	Return:
		
*/

int ws_process_record_data(unsigned char *data, ws_weather_record *record);


/**
	Reads a weather record, formats the data and puts it in a ws_weather_record stuct.
	
	Parameters:
		- dev:		 	A device struct for the device 

		- address: 		The address of the 	block in memory. If this value is -1, then 
						the latest record written will be read. The data must be in the 
						range of 0x100 -> 0xFFFF, otherwise WS_ERR_INVALID_ADDR will be 
						returned.
					
						This number will be rounded down to the nearest 16. As blocks are
						read in 32 byte blocks, if the next/previous record is in the same 
						block, and you would like to access it, then use ws_read_multiple_records()
						as it will prevent multiple reads of the data.
		
		- record		The struct for the data to be stored.
		
	Return:
		- WS_ERR_CONTROL_TRANSFER_FAILED	Request for data write failed
		- WS_ERR_BULK_TRANSFER_FAILED		Data read failed 
		- WS_ERR_INVALID_ADDR				Invalid address provided, most likly out of range.
*/
int ws_read_weather_record(ws_device *dev, int address, ws_weather_record *record);


/**
	Reads all of the weather records between two addresses. It should be noted that retrieving large amounts of 
	data can take a long time. address_to and address_from must be in the range 0x100 -> 0xFFFF.
	
	Parameters:
		- dev: 				A device struct for the device 
		
		- address_from:		The address to start reading from. This number will be down rounded to the 
							nearest 16 - the size of a weather record.
							
		- address_to:		The address to read to. This number will be up rounded to the 
							nearest 16 - the size of a weather record.
							
		- record			An array of records retrieved from memory
		
		- record_count		The number of records read from the device.
							
	Return:
		- WS_ERR_CONTROL_TRANSFER_FAILED	Request for data write failed
		- WS_ERR_BULK_TRANSFER_FAILED		Data read failed 
		- WS_ERR_INVALID_ADDR				Invalid address provided, most likly out of range.
*/
int ws_read_multiple_weather_records(ws_device *dev, int address_from, int address_to, ws_weather_record ***record, int *record_count);

/**
	Reads the fixed block memory and processes it, filling the weather exteames in the ws_weather_extremes struct

	Parameters:
		- dev: 				The weather station device
		- extremes 			The struct for storing all of the extremes

*/
int ws_read_weather_extremes(ws_device *dev, ws_weather_extremes *extremes);

/**
	Prints an error from libusb
	
	Parameters:
		status:			The error status return from a libusb function
		additonal_info	Optional, adds more detail to the error string
*/

void ws_usb_error(int status, const char* additonal_info);

/**
	Prints a 32 byte memory block to the screen
	
	Parameters:
		data		The memory block to be printed. Must be 32 bytes;
*/

void ws_print_block(unsigned char* data);


/**
	Prints the memory of the device to stdout.
	
	NOTE: **** VERY DANGEROUS FUNCTION ****
	=======================================
	Dangerous as it can run for a long time; interupting it has a high chance of 
	interupting a usb read or write. This can cause following attempts to retrieve
	data to fail: a common issue is that the data is shifted down by 8 bytes, meaning
	that other parts of the program which assume a constant address end up reading 
	the wrong data or even worse random garbage from the host's memory.
	
	Parameters:
		blocks		The number of blocks to print, giving -1 will print all of the data
*/
void ws_print_mem_dump(ws_device *dev, int blocks);

/**
	Decodes a signed short (taking into account sign bit)

	Parameter:
		byte_1		First byte
		byte_2		Second byte

	Return:
		Decoded value as a uint16_t
*/
int16_t ws_decode_signed_short(uint8_t byte_1, uint8_t byte_2);

/*
	Takes two bytes and gets their value by combining them into a uint16_t
	
	Parameters:
		byte1:		The most significant byte
		byte2		The least significant byte
*/

uint16_t ws_value_of_bytes(uint8_t byte1, uint8_t byte2);

/*
	Prints the value of a weather record to stdout
	
	Parameter:
		record:		The record to print out
*/
void ws_print_weather_record(ws_weather_record record);

/*
	Decodes a Binary Coded Decimal (BDC) and puts the decoded data in the struct.

	Parameters:
		time_data 	The raw time data. Must be 5 bytes long. (this is an assumption)

	Return: 
		The decodeed time 
*/

/**
	Prints the contents of a ws_min_max (debug function). 

	Parameters:
		min_max 	The min-max struct to be printed
		value_name	The name of the value

*/
void ws_print_min_max(ws_min_max max_min, const char* value_name);


/**
	Decodes a BCD date, given as 5 raw data bytes.

	Parameter:
		time_data 		The raw time data. Must be 5 bytes.

	Return:
		The struct ws_time containing the decoded data
*/
ws_time ws_decode_bcd(unsigned char* time_data);

/**
	Decodes a  single Binary Coded Decimal byte

	Parameter:
		byte 	The byte to decode

	Returns
		The decoded byte
*/

unsigned char ws_decode_bcd_byte(unsigned char byte);

/**
	Compares unsigned char array data_1 to data_2, assuming an equal length of length bytes.

	Parameters:
		data_1		Array 1   ----____ To be compared
		data_2		Array 2   ----	   to eachover
		length		The length of the arrays

	Return:
		0 if the arrays are not equal, 1 if they are equal.
*/
int ws_cmp_data(unsigned char* data_1, unsigned char* data_2, int length);

/**
	Function reads a stable block. Reads blocks from the device until they are
	the same. This prevents reading corrupt data if the device is writing.
	Therefore, this function is slower then ws_read_block, but much safer.
	
	Parameters:
		- dev: 					A device struct for the device to be read from
		- data:  				The data array
		
		- address:				The address to be read from. Note that this number
								will be rounded down to the nearest multiple of 32, 
								as all data is read in 32 byte chunks
								
								The memory is in the range of 0x0000 -> 0xFFFF. Weather
								records start at 0x100
		
		- read					The number of bytes read					
								
	Return:
		- WS_ERR_CONTROL_TRANSFER_FAILED	Request for data write failed
		- WS_ERR_BULK_TRANSFER_FAILED		Data read failed 

*/
int ws_read_stable_block(ws_device *dev, int address, unsigned char* data, int* read);

/**
	Reads a high low (weather extreme) values and times. Very specific function, only works with some
	of the weather types (values which are signed shorts/unsigned shorts and which require to be multiplied by 0.1).
	
	Parameters:
		data: 				The data. Must be the 256 byte fixed block.
		is_unsigned			Non zero value if the data is an unsigned short
		addr_value_begin	Address where the value data begins 
		addr_time_begin		Address where the time data begins 

	Return
		The min-max struct filled out with the data.
*/
ws_min_max ws_read_stddec_extreme(unsigned char *data, int is_unsigned, int addr_value_begin, int addr_time_begin);

#endif