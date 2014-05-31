Weather Station (Work in Progress)  
====================================

#### Custom software for the W-8681 weather station.

This software, in its completed form, will be able to:
- Read weather data (live, history and weather extremes) from a W-8681 station, plugged into a Linux computer.
- Process this data to determine more information, for example longest dry period and as well as performing validation and sanity checks.
- Store this data in a database (of which is to be decided)
- Provide a restful API which allows external applications to access the data. 

#### C API for Accessing Weather Station Data

_Note: The information regarding the memory layout and encoding is based of [Jim Easterbrook's memory map](http://www.jim-easterbrook.me.uk/weather/mm/)_

The weather station's memory is composed of two parts: the first 256 bytes, from address 0x0 to 0x100, is a fixed block of memory - the values are updated,
but each address will always hold the same _type_ of information. For example, address 0xDD always contains the maximum wind speed, but the value at that 
address can be updated by the weather station if a new record is reached. This block of data contains the weather extremes reached (the value and when
they happened), the unit alarm settings, the display unit settings (that is the touchscreen device) along with some other misc. settings and memory
information.

_To be completed_