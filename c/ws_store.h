#ifndef WS_STORE_H
#define WS_STORE_H

#include "ws.h"
#include <sqlite3.h>

void db_error(sqlite3*, const char* extra);

int ws_store_open_db(sqlite3** info);
int ws_store_close_db(sqlite3** info);
int ws_store_create_statement(sqlite3** info, char* sql, int sql_size, sqlite3_stmt** statement);
int ws_store_execute_query(sqlite3** info, sqlite3_stmt** statement);
int ws_store_delete_stmt(sqlite3** info, sqlite3_stmt** statement);
#endif  