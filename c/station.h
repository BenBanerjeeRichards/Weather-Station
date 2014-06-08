#ifndef STATION_H
#define STATION_H

#include "ws.h"

typedef struct {
	int year;
	int month;
	int day;
} date_t;


int station_download_data(ws_device *dev);

void station_check_record(ws_weather_record *record);
#endif 