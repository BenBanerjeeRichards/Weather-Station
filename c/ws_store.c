#include "ws_store.h"
#include <stdio.h>
#include <sqlite3.h>

void db_error(sqlite3* info, const char* extra)
{
	printf("[SQLITE3 ERR] %s (%s)\n",sqlite3_errmsg(info), extra);
}

int ws_store_open_db(sqlite3** info)
{
	int status = sqlite3_open_v2("WeatherDB.sqlite", info, SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE, NULL);
	if (status != SQLITE_OK)
	{
		db_error(*info, "ws_store_open_db");
		return WS_ERR_DB_OPEN;
	}

	return WS_SUCCESS;
}

int ws_store_close_db(sqlite3** info)
{
	int status = sqlite3_close(*info);
	if (status != SQLITE_OK)
	{
		db_error(*info, "ws_store_close_db");
		return WS_ERR_DB_CLOSE;
	}

	return WS_SUCCESS;
}

int ws_store_create_statement(sqlite3** info, char* sql, int sql_size, sqlite3_stmt** statement)
{
	const char* tail;
	int status = sqlite3_prepare_v2(*info, sql, sql_size, statement, &tail);
	if (status != SQLITE_OK)
	{
		db_error(*info, "ws_store_create_statement");
		return WS_ERR_DB_PREPARE;
	}

	return WS_SUCCESS;
}

int ws_store_execute_query(sqlite3** info, sqlite3_stmt** statement)
{
	int status = sqlite3_step(*statement);
	if (status == SQLITE_ROW)
	{
		return WS_DB_ROW;
	}

	if (status != SQLITE_OK && status != SQLITE_DONE && status != SQLITE_ROW)
	{
		db_error(*info, "ws_store_execute_query");
		return WS_ERR_DB_QUERY;
	}

	return WS_SUCCESS;
}

int ws_store_delete_stmt(sqlite3** info, sqlite3_stmt** statement)
{
	int status = sqlite3_finalize(*statement);
	if (status != SQLITE_OK)
	{
		db_error(*info, "ws_store_delete_stmt");
		return WS_ERR_DEL_STMT;
	}

	return WS_SUCCESS;
}

int ws_store_prepare_db(sqlite3** info)
{
	/* Create the table for storing weather records */
	char sql[] = "CREATE TABLE IF NOT EXISTS WeatherData( RecordDateTime TEXT PRIMARY KEY, IndoorHumidity INTEGER, OutdoorHumidity INTEGER,"
				"IndoorTemperature REAL, OutdoorTemperature REAL, DewPoint REAL, AbsolutePressure REAL, WindSpeed REAL,"
				"GuestSpeed REAL, WindDirection REAL, TotalRain REAL, SensorContactError INTEGER, RainCounterOverflow INTEGER )";
	
	sqlite3_stmt* statement = NULL;
	
	int status = ws_store_create_statement(info, sql, sizeof(sql) / sizeof(sql[0]), &statement);
	if (status != WS_SUCCESS)
	{
		return status;
	}

	status = ws_store_execute_query(info, &statement);
	if (status != WS_DB_ROW && status != WS_SUCCESS)
	{
		return status;
	}
	
	status = ws_store_delete_stmt(info, &statement);
	if (status != WS_SUCCESS)
	{
		return status;
	}

	/* Create the table for storing weather extremes */
	char sql2[] = "CREATE TABLE IF NOT EXISTS WeatherExtremes(Name TEXT PRIMARY KEY, MinValue REAL, MinDateTime TEXT, MaxValue REAL,  MaxDateTime TEXT)";
	sqlite3_stmt* statement2;

	 status = ws_store_create_statement(info, sql2, sizeof(sql2) / sizeof(sql2[0]), &statement2);
	if (status != WS_SUCCESS)
	{
		return status;
	}

	status = ws_store_execute_query(info, &statement2);
	if (status != WS_DB_ROW && status != WS_SUCCESS)
	{
		return status;
	}
	
	status = ws_store_delete_stmt(info, &statement2);
	if (status != WS_SUCCESS)
	{
		return status;
	}

	return WS_SUCCESS;
}

int ws_store_add_weather_record(ws_weather_record record)
{
	char sql[512];
	char date[20];

	// Format the date
	snprintf(date, 20, "%.4i-%.2i-%.2i %.2i:%.2i:%.2i.%.3i", record.date_time->tm_year + 1900, record.date_time->tm_mon + 1, 
															 record.date_time->tm_mday, record.date_time->tm_hour, record.date_time->tm_min, 0, 0);

	snprintf(sql, 512, "INSERT INTO WeatherData VALUES(NULL, %s, %i, %i, %f, %f, %f, %f, %f, %f, %f, %f, %i, %i)", 
		     date, record.indoor_humidity, record.outdoor_humidity, record.indoor_temperature, record.outdoor_temperature, 
		     record.dew_point, record.absolute_pressure, record.wind_speed, record.gust_speed, record.wind_direction, 
		     record.total_rain, record.status.sensor_contact_error, record.status.rain_counter_overflow);

	printf("%s\n", sql);

	return WS_SUCCESS;


}
