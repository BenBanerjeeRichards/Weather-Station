#include "station.h"
#include "ws.h"
#include <stdio.h>
#include <math.h>
#include <time.h>

int station_download_data(ws_device *dev)
{
	ws_init(dev);
	ws_initialise_read(dev);

	int address;
	int status = ws_latest_record_address(dev, &address);
	if (status != WS_SUCCESS)
	{
		return status;
	}

	address -= 0x20;
	int n = 0;
	
	int total_record_count = (address - 0x100) / 16;
	int days_recorded = ceil(total_record_count / 48);

	time_t t_today;
	struct tm* now = NULL; 

	for (int i = 0x100; i < 0x500; i += 0x10)
	{
		// Calculate when the data was recorded
		n++;
		double days = n * (1.0 / 48.0);
		double record_time = (24 * days) - (24 * floor(days));
		int day_offset_from_start = floor(days);

		// Calculate the actual date of the data
	    t_today = time(0);
		now = localtime(&t_today);

		now->tm_mday -= (22 - day_offset_from_start);
		now->tm_hour = (int)floor(record_time) - 1;

		mktime(now);



		printf("[\t%i\t] %s", n, asctime(now));
		// Download data from address
		ws_weather_record record;
		int read;

		status = ws_read_weather_record(dev, i, &record);
		if (status != WS_SUCCESS)
		{
			return status;
		}

		//ws_print_weather_record(record);


	}


	return WS_SUCCESS;
}

