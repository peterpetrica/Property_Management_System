#ifndef DB_QUERY_H
#define DB_QUERY_H

#include "db/database.h"

// 查询行结构
typedef struct
{
    int columns;   // 列数
    char **values; // 行数据
} QueryRow;

// 查询结果结构
typedef struct
{
    int column_count;    // 列数
    int row_count;       // 行数
    QueryRow *rows;      // 行数据
    char **column_names; // 列名
} QueryResult;

// 执行SQL查询并返回结果
bool execute_query(Database *db, const char *sql, QueryResult *result);

// 执行SQL更新操作
bool execute_update(Database *db, const char *sql);

// 释放查询结果资源
void free_query_result(QueryResult *result);

// 执行参数化查询
int db_parameterized_query(Database *db, const char *query, void *params,
                           void (*bind_params)(sqlite3_stmt *, void *),
                           QueryResult *result);

#endif /* DB_QUERY_H */
