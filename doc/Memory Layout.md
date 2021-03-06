## Memory Layout of the Weather Station

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

An important thing to note is that the station will overwrite the latest weather record constantly __until__ the next interval period is reached. This means that 
this record block always contains the latest __live__ data that is visible on the station's touchscreen display.  
