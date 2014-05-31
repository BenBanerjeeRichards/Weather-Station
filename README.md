Weather Station (Work in Progress)  
====================================

#### Custom software for the W-8681 weather station.

This software, in its completed form, will be able to:
- Read weather data (live, history and weather extremes) from a W-8681 station, plugged into a Linux computer.
- Process this data to determine more information, for example longest dry period and as well as performing validation and sanity checks.
- Store this data in a database (of which is to be decided)
- Provide a restful API which allows external applications to access the data. 

#### Memory Layout of the Weather Station

_Note: The information regarding the memory layout and encoding is based of [Jim Easterbrook's memory map](http://www.jim-easterbrook.me.uk/weather/mm/)_

The weather station's memory is composed of two parts: the first 256 bytes, from address 0x0 to 0x100, is a fixed block of memory - the values are updated,
but each address will always hold the same _type_ of information. For example, address 0xDD always contains the maximum wind speed, but the value at that 
address can be updated by the weather station if a new record is reached. This block of data contains weather extremes (the value and when
they happened), the unit alarm settings, the display unit settings (that is the touchscreen device) along with some other misc. settings and memory
information.

Memory in the range 0x100 to 0x10000 is a [circular buffer](http://en.wikipedia.org/wiki/Circular_buffer) which consists of 16 byte blocks of weather data. 
Every set period of time (by default, 30 minutes), the weather station writes a new block to memory. This block has information such as pressure, wind speed, 
gust speed, indoor and outdoor temperature and humidity and rain information. If the memory has not been reached yet, it is set to 0xFF. It the memory becomes
full, the weather station starts to overwrite the oldest data (starting at 0x100). 

An important thing to note is that the station will overwrite the latest weather record constantly _until_ the next interval period is reached. This means that 
this record block always contains the latest _live_ data that is visible on the station's touchscreen display.  

#### C API 

To interact directly with the device, use the `ws.h` header file. The following sample code initialises everything, preparing everything for further interaction
with the device

``` C
#include <stdio.h>
#include "w.h"

int main(int argc, char** args)
{
	// Device stuct contains information about the weather station, allowing the program to communicate via libusb
	ws_device dev;

	// Find the device, filling out the device struct.
	int status = ws_init(&ws_device);
	if (status != WS_SUCCESS)
	{
		printf("ws_init failed: %s", ws_get_str_error(status));
		return 1;
	}

	// Prepare for IO communications with the station
	status = ws_initialise_read(&dev);
	if (status != WS_SUCCESS)
	{
		printf("ws_initialise_read failed: %s", ws_get_str_error(status));
		return 1;
	}

	return 0;

}
```

_To be completed_