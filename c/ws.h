#ifndef WS_H
#define WS_H

#include <libusb-1.0/libusb.h>

// --------- Enum and Struct Definitions --------- //

/**
	Error enums with const* char array
    http://stackoverflow.com/questions/9907160/how-to-convert-enum-names-to-string-in-c
*/

#define FOREACH_WS_ERR(ERROR) 		\
	ERROR(WS_ERR_NO_STATION_FOUND) 		\
	ERROR(WS_ERR_USB_INIT_FAILED) 	\
	ERROR(WS_ERR_INTERFACE_CLAIM_FAILED) 	\
	ERROR(WS_ERR_INVALID_PERMISSIONS) 	\
	ERROR(WS_ERR_CONTROL_TRANSFER_FAILED) 	\
	ERROR(WS_ERR_BULK_TRANSFER_FAILED) 	\
	ERROR(WS_ERR_INVALID_ADDR) 	\
	ERROR(WS_ERR_NO_DEVICE)		\
	ERROR(WS_ERR_OPEN_FAILED) \

	
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
	Note that rain extremes have no minimums
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
	thatn only 32.
	
	Parameters:
		- dev: 		A device struct for the device to be read from
		- data:  char	The data array
	
	Return:
		- WS_ERR_CONTROL_TRANSFER_FAILED	Request for data write failed
		- WS_ERR_BULK_TRANSFER_FAILED		Data read failed 
*/

int ws_read_fixed_block(ws_device *dev, unsigned char* data);


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
								
	Return:
		- WS_ERR_CONTROL_TRANSFER_FAILED	Request for data write failed
		- WS_ERR_BULK_TRANSFER_FAILED		Data read failed 
*/
int ws_read_block(ws_device *dev, int address, unsigned char* data);

/**
	Retrieves the address in the device's memory of the latest weather record saved. Useful, as
	the data is stored in a circular buffer where old data is overwritten when the memory has all
	been used up.   
	
	This address is stored in the fixed block memory, at position 0x1E.
	
	Parameters:
		- dev: 			A device struct for the device 
		- address: 		The address of the latest record.	
		
	Return:
		- WS_ERR_CONTROL_TRANSFER_FAILED	Request for data write failed
		- WS_ERR_BULK_TRANSFER_FAILED		Data read failed 
*/
int ws_latest_record_address(ws_device *dev, int *address);


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
	Prints an error from libusb
	
	Parameters:
		status:			The error status return from a libusb function
		additonal_info	Optional, adds more detail to the error string
*/

void ws_usb_error(int status, const char* additonal_info);

#endif