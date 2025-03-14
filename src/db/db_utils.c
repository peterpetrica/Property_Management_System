#include "db/database.h"
#include <stdio.h>
#include <string.h>

// 执行简单的SQL查询并返回结果数量
int db_count_query(Database *db, const char *query, int *count)
{
    if (!db || !db->db || !query || !count)
        return SQLITE_ERROR;

    sqlite3_stmt *stmt;
    int rc = sqlite3_prepare_v2(db->db, query, -1, &stmt, NULL);
    if (rc != SQLITE_OK)
    {
        fprintf(stderr, "SQL准备错误: %s\n", sqlite3_errmsg(db->db));
        return rc;
    }

    rc = sqlite3_step(stmt);
    if (rc == SQLITE_ROW)
    {
        *count = sqlite3_column_int(stmt, 0);
        rc = SQLITE_OK;
    }
    else
    {
        fprintf(stderr, "SQL执行错误: %s\n", sqlite3_errmsg(db->db));
    }

    sqlite3_finalize(stmt);
    return rc;
}

// 检查数据库表是否存在
bool db_table_exists(Database *db, const char *table_name)
{
    if (!db || !db->db || !table_name)
        return false;

    char query[256];
    snprintf(query, sizeof(query),
             "SELECT count(*) FROM sqlite_master WHERE type='table' AND name='%s';",
             table_name);

    int count = 0;
    if (db_count_query(db, query, &count) != SQLITE_OK)
        return false;

    return count > 0;
}

// 检查数据库记录是否存在
bool db_record_exists(Database *db, const char *query)
{
    if (!db || !db->db || !query)
        return false;

    int count = 0;
    if (db_count_query(db, query, &count) != SQLITE_OK)
        return false;

    return count > 0;
}
