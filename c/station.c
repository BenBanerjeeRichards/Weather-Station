#include "station.h"
#include "ws.h"
#include <stdio.h>
#include <math.h>
#include <time.h>

void station_check_record(ws_weather_record *record)
{
	if (record->outdoor_humidity > 99 || record->outdoor_humidity < 0)
	{
		record->data_invalid = 1;
		return;
	}

	if (record->indoor_humidity > 99 || record->indoor_humidity < 0)
	{
		record->data_invalid = 1;
		return;
	}

	if (record->indoor_temperature >= 255 || record->indoor_temperature <= -255)
	{
		record->data_invalid = 1;
		return;
	}

	if (record->outdoor_temperature >= 255 || record->outdoor_temperature <= -255)
	{
		record->data_invalid = 1;
		return;
	}

	if (record->dew_point >= 255 || record->dew_point <= -255)
	{
		record->data_invalid = 1;
		return;
	}

	if (record->absolute_pressure > 10000000 || record->absolute_pressure < -10000000)
	{
		record->data_invalid = 1;
		return;
	}

	if (record->wind_speed >= 255 || record->wind_speed < 0)
	{
		record->data_invalid = 1;
		return;
	}

	if (record->gust_speed >= 255 || record->gust_speed < 0)
	{
		record->data_invalid = 1;
		return;
	}

	if (record->wind_direction > 360 || record->wind_direction < 0)
	{
		record->data_invalid = 1;
		return;
	}

}

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

	for (int i = 0x100; i < address; i += 0x10)
	{
		printf("reading from address 0x%04x\n", i);
		// Calculate when the data was recorded
		n++;
		double days = n * (1.0 / 48.0);
		double record_time = (24 * days) - (24 * floor(days));
		int day_offset_from_start = floor(days);

		// Calculate the actual date of the data
	    t_today = time(0);
		now = localtime(&t_today);

		double hours_from_present = hours_eclapsed - 0.5 * (n - 1);
		now->tm_hour -= floor(hours_from_present);
		now->tm_min -= (floor(hours_from_present) == hours_from_present) ? 0 : 30;
		now->tm_isdst = 0;
		mktime(now);
		
		// Download data from address
		ws_weather_record record;
		int read;

		status = ws_read_weather_record(dev, i, &record);
		if (status == WS_ERR_TIMEOUT)
		{
			printf("Timeout\n");
		}
		if (status != WS_SUCCESS)
		{
			return status;
		}

		record.date_time = now;
		station_check_record(&record);		
	}

	return WS_SUCCESS;
}