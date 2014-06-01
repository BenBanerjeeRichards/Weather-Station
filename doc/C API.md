## C API 

This API is capable of the following:
- Reading data at an arbitrary address from the weather station.
- Processing the data of a weather record (history entry).
- Processing the data to retrieve weather extremes.

### Data Structures
#### ws_weather_record

Holds the data for a single weather record stored in the circular buffer.

| Name                | Type              | Unit               | Notes                              |
|---------------------|-------------------|--------------------|------------------------------------|
| indoor_humidity     | int               | Percent            |                                    |
| outdoor_humidity    | int               | Percent            |                                    |
| indoor_temperature  | double            | Degrees Celcius    |                                    |
| outdoor_temperature | double            | Degrees Celcius    |                                    |
| absolute_pressure   | double            | Hectopascals       |                                    |
| wind_speed          | double            | Meters per Second  |                                    |
| gust_speed          | double            | Meters per Second  |                                    |
| wind_direction      | double            | Degrees from North |                                    |
| total_rain          | double            | Millimetre         |                                    |
| status              | ws_station_status |                    | Weather Station Status information |

#### ws_station_status

The status of the weather station.

| Name                  | Type | Notes                  |
|-----------------------|------|------------------------|
| sensor_contact_error  | int  | 1 if there is an error |
| rain_counter_overflow | int  | 1 if there is an error |


### Initialisation
To interact directly with the device, use the `ws.h` header file. The following sample code initialises everything, preparing everything for further interaction
with the device

``` C
#include <stdio.h>
#include "ws.h"

int main(int argc, char** args)
{
    // Device stuct contains information about the weather station, allowing the program to communicate via libusb
    ws_device dev;

    // Find the device, filling out the device struct.
    int status = ws_init(&dev);
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
### Reading Live Data from the Device

To do this, there are two steps that need to be taken:

1. Get the latest address of which data is being written to. (`ws_latest_record_address`)
2. Retrieve the data from this location and process it (`ws_read_weather_record`)

The following program does the above. Note that error handling has been removed. For int returning functions, the error
handling shown in the above initialisation example should be utilised.

``` C
int main(int argc, char** args)
{
    // Device stuct contains information about the weather station, allowing the program to communicate via libusb
    ws_device dev;

    // *** SNIP Program initalisation (see above initialisation code) ***//
    // Get the latest address
    int address;
    ws_latest_record_address(&dev, &address);

    // Read and process the data
    ws_weather_record record;
    ws_read_weather_record(&dev, address, &record);

    // Debug function prints the contents of a ws_weather_record
    ws_print_weather_record(record);
}
```