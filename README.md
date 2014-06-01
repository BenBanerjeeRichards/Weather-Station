Weather Station (Work in Progress)  
====================================

#### Custom software for the W-8681 weather station.

This software, in its completed form, will be able to:
- Read weather data (live, history and weather extremes) from a W-8681 station, plugged into a Linux computer.
- Process this data to determine more information, for example longest dry period and as well as performing validation and sanity checks.
- Store this data in a database (of which is to be decided)
- Provide a restful API which allows external applications to access the data. 

#### Dependencies
- [libusb](http://www.libusb.org/)

#### Contents
- [Memory Layout of the Weather Station](https://github.com/BenBanerjeeRichards/Weather-Station/blob/master/doc/Memory%20Layout.md)
- [C API](https://github.com/BenBanerjeeRichards/Weather-Station/blob/master/doc/C%20API.md)