#ifndef WS_H
#define WS_H

#include <libusb-1.0/libusb.h>

// --------- Enum and Struct Definitions --------- //

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
	Initialises things:
		- Finds the weather station
		- Retrieves a handle for the station
		- Fills out the given ws_device struct 
	
	Parameters:
		- dev:ws_device 	A device struct, empty at this point which 
							will be filled out by the function
	
	Return:
		- WS_ERR_NO_STATION		No weather station could be found 
*/

int ws_init(ws_device *dev);


/**
	Closes the handle of the USB device, so that it can no
	longer be used.
	
	Passing ws_device to another ws_* function other that ws_init will
	result in errors.
	
	Parameters:
		- dev:ws_device 	A device struct for the device to be closed
	
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
		- dev:ws_device 	A device struct for the device 
		
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
	
	This data can be read in single 32 byte chunks by calling ws_read_block(): do not use this function
	for retrieving data from a single chunk, for example the next record location, as this would waste 
	time. 
	
	Parameters:
		- dev:ws_device 		A device struct for the device to be read from
		- data: unsigneed char	The data array
	
	Return:
		- WS_ERR_CONTROL_TRANSFER_FAILED	Request for data write failed
		- WS_ERR_BULK_TRANSFER_FAILED		Data read failed 
*/

int ws_read_fixed_block(ws_device *dev, unsigned char* data)

#endif