#include "station.h"
#include "ws.h"
#include <stdio.h>
#include <math.h>

int station_download_data(ws_device *dev)
{
	ws_init(dev);
	ws_initialise_read(dev);

	int address;
	int status = ws_latest_record_address(dev, &address);
	if (status != WS_SUCCESS)
	{
		printf("no\n");
		return status;
	}

	address -= 0x20;
	int n = 0;
	for (int i = 0x100; i < 0x1000; i += 0x20)
	{
		// Calculate when the data was recorded
		n++;
		double days = n * (1.0 / 48.0);
		double tm = (24 * days) - (24 * floor(days));
		int days_ago = floor(days);

		// Download data from address
		ws_weather_record record;
		int read;
		status = ws_read_weather_record(dev, i, &record);
		if (status != WS_SUCCESS)
		{
			return status;
		}

		ws_print_weather_record(record);


	}


	return WS_SUCCESS;
}