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

/*
	Holds all of the imformation for accessing the device
*/

typedef struct
{
	libusb_device* dev;
	struct libusb_device_descriptor* desc;
	struct libusb_device_handle* hnd;
}  ws_device;

/*
	Holds the details for finding the USB device 
*/

typedef struct 
{
	uint16_t vendor;
	uint16_t product;
} ws_product;

/*
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


/*
Status of the weather station
*/
typedef struct
{
	int sensor_contack_error;
	int rain_counter_overflow;
} ws_station_status;


/*
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

// --------- Function Definitions --------- //
int ws_init();



#endif