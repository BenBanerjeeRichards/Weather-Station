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

	if (status == SQLITE_MISUSE)
	{
		printf("MISUSE!\n");
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