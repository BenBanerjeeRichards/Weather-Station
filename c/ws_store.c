#include "ws_store.h"
#include <stdio.h>
#include <sqlite3.h>

void db_error(struct dbinfo* info, const char* extra)
{
	printf("[SQLITE3 ERR] %s (%s)\n",sqlite3_errmsg(info->db_handle), extra);
}

int ws_store_open_db(struct dbinfo* info)
{
	int status = sqlite3_open("WeatherDB.sqlite", &info->db_handle);
	if (status != SQLITE_OK)
	{
		db_error(info, "ws_store_open_db");
		return WS_ERR_DB_OPEN;
	}

	return WS_SUCCESS;
}

int ws_store_close_db(struct dbinfo* info)
{
	int status = sqlite3_close(info->db_handle);
	if (status != SQLITE_OK)
	{
		db_error(info, "ws_store_close_db");
		return WS_ERR_DB_CLOSE;
	}

	return WS_SUCCESS;
}

int ws_store_create_statement(struct dbinfo* info, sqlite3_stmt* statement)
{
	
}