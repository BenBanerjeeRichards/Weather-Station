#ifndef WS_H
#define WS_H

// --------- Enum and Struct Definitions --------- //

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


// --------- Function Definitions --------- //
int ws_init();



#endif