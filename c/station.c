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
	double hours_eclapsed = 0.5 * total_record_count;

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

		double hours_from_present = hours_eclapsed - 0.5 * (n - 1);
		now->tm_mday -= (days_recorded - day_offset_from_start);
		now->tm_hour -= floor(hours_from_present);
		now->tm_min -= (floor(hours_from_present) == hours_from_present) ? 0 : 30;
		mktime(now);

		// Download data from address
		ws_weather_record record;
		int read;

		status = ws_read_weather_record(dev, i, &record);
		if (status != WS_SUCCESS)
		{
			return status;
		}

		record.date_time = now;
	}


	return WS_SUCCESS;
}

