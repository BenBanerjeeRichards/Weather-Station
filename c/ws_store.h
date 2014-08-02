#ifndef WS_STORE_H
#define WS_STORE_H

#include "ws.h"
#include <sqlite3.h>

struct dbinfo {
	sqlite3* db_handle;
};

void db_error(struct dbinfo* info, const char* extra);

int ws_store_open_db(struct dbinfo* info);
int ws_store_close_db(struct dbinfo* info);
int ws_store_create_statement(struct dbinfo* info, const char* sql, int sql_size sqlite3_stmt* statement);
#endif  