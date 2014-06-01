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
        printf("ws_init failed: %s\n", ws_get_str_error(status));
        return 1;
    }

    // Prepare for IO communications with the station
    status = ws_initialise_read(&dev);
    if (status != WS_SUCCESS)
    {
        printf("ws_initialise_read failed: %s\n", ws_get_str_error(status));
        return 1;
    }

    int address;
    ws_latest_record_address(&dev, &address);

    // Read and process the data
    ws_weather_record record;
    ws_read_weather_record(&dev, address, &record);

    ws_print_weather_record(record);
    return 0;

}