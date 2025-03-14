#include "db/db_query.h"
#include "auth/auth.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// 查询回调函数，将结果添加到QueryResult结构中
static int query_callback(void *data, int argc, char **argv, char **azColName)
{
    QueryResult *result = (QueryResult *)data;

    // 第一次调用时，初始化列名
    if (result->column_count == 0 && result->column_names == NULL)
    {
        result->column_count = argc;
        result->column_names = (char **)malloc(sizeof(char *) * argc);

        if (!result->column_names)
        {
            fprintf(stderr, "内存分配失败\n");
            return 1;
        }

        for (int i = 0; i < argc; i++)
        {
            result->column_names[i] = strdup(azColName[i]);
        }
    }

    // 分配新的行结构
    QueryRow *new_rows = (QueryRow *)realloc(result->rows, (result->row_count + 1) * sizeof(QueryRow));
    if (!new_rows)
    {
        fprintf(stderr, "内存分配失败\n");
        return 1;
    }
    result->rows = new_rows;

    // 初始化新行
    QueryRow *row = &result->rows[result->row_count];
    row->columns = argc;
    row->values = (char **)malloc(sizeof(char *) * argc);

    if (!row->values)
    {
        fprintf(stderr, "内存分配失败\n");
        return 1;
    }

    // 复制行数据
    for (int i = 0; i < argc; i++)
    {
        row->values[i] = argv[i] ? strdup(argv[i]) : NULL;
    }

    // 增加行计数
    result->row_count++;

    return 0; // 继续查询
}

// 执行通用查询
static int execute_query(Database *db, const char *query, QueryResult *result)
{
    // 初始化结果结构
    if (!result)
        return SQLITE_ERROR;

    memset(result, 0, sizeof(QueryResult));

    char *err_msg = NULL;
    int rc = sqlite3_exec(db->db, query, query_callback, result, &err_msg);

    if (rc != SQLITE_OK)
    {
        fprintf(stderr, "SQL错误: %s\n", err_msg);
        sqlite3_free(err_msg);

        // 清理已分配的资源
        free_query_result(result);
    }

    return rc;
}

int db_simple_query(Database *db, const char *query, QueryResult *result)
{
    return execute_query(db, query, result);
}

int db_compound_query(Database *db, const char *query, QueryResult *result)
{
    return execute_query(db, query, result);
}

int db_fuzzy_query(Database *db, const char *table, const char *column, const char *pattern, QueryResult *result)
{
    // 构造模糊查询SQL
    char *sql = (char *)malloc(strlen(table) + strlen(column) + strlen(pattern) + 64);
    if (!sql)
    {
        fprintf(stderr, "内存分配失败\n");
        return SQLITE_NOMEM;
    }

    sprintf(sql, "SELECT * FROM %s WHERE %s LIKE '%%%s%%';", table, column, pattern);

    int rc = execute_query(db, sql, result);
    free(sql);

    return rc;
}

int db_privileged_query(Database *db, const char *query, int weight, QueryResult *result)
{
    // 验证权重
    // 权重值越小，权限越高: 1=管理员, 2=物业服务人员, 3=业主

    // TODO: 实现基于权重的查询限制
    // 例如，防止业主访问其他业主的数据，等等

    return execute_query(db, query, result);
}

void free_query_result(QueryResult *result)
{
    if (!result)
        return;

    // 释放列名
    if (result->column_names)
    {
        for (int i = 0; i < result->column_count; i++)
        {
            free(result->column_names[i]);
        }
        free(result->column_names);
        result->column_names = NULL;
    }

    // 释放行数据
    if (result->rows)
    {
        for (int i = 0; i < result->row_count; i++)
        {
            QueryRow *row = &result->rows[i];
            if (row->values)
            {
                for (int j = 0; j < row->columns; j++)
                {
                    if (row->values[j])
                        free(row->values[j]);
                }
                free(row->values);
            }
        }
        free(result->rows);
        result->rows = NULL;
    }

    // 重置计数器
    result->column_count = 0;
    result->row_count = 0;
}

int db_parameterized_query(Database *db, const char *query, void *params,
                           void (*bind_params)(sqlite3_stmt *, void *),
                           QueryResult *result)
{
    if (!db || !db->db || !query || !result)
        return SQLITE_ERROR;

    // 初始化结果
    memset(result, 0, sizeof(QueryResult));

    sqlite3_stmt *stmt;
    int rc = sqlite3_prepare_v2(db->db, query, -1, &stmt, NULL);
    if (rc != SQLITE_OK)
    {
        fprintf(stderr, "SQL准备错误: %s\n", sqlite3_errmsg(db->db));
        return rc;
    }

    // 绑定参数
    if (bind_params)
    {
        bind_params(stmt, params);
    }

    // 获取列数
    int cols = sqlite3_column_count(stmt);
    result->column_count = cols;
    result->column_names = (char **)malloc(sizeof(char *) * cols);
    if (!result->column_names)
    {
        sqlite3_finalize(stmt);
        return SQLITE_NOMEM;
    }

    // 获取列名
    for (int i = 0; i < cols; i++)
    {
        const char *col_name = sqlite3_column_name(stmt, i);
        result->column_names[i] = strdup(col_name ? col_name : "");
    }

    // 执行查询并获取结果
    while ((rc = sqlite3_step(stmt)) == SQLITE_ROW)
    {
        // 分配新行
        QueryRow *new_rows = (QueryRow *)realloc(result->rows, (result->row_count + 1) * sizeof(QueryRow));
        if (!new_rows)
        {
            free_query_result(result);
            sqlite3_finalize(stmt);
            return SQLITE_NOMEM;
        }
        result->rows = new_rows;

        // 初始化新行
        QueryRow *row = &result->rows[result->row_count];
        row->columns = cols;
        row->values = (char **)malloc(sizeof(char *) * cols);
        if (!row->values)
        {
            free_query_result(result);
            sqlite3_finalize(stmt);
            return SQLITE_NOMEM;
        }

        // 获取行数据
        for (int i = 0; i < cols; i++)
        {
            const char *val = (const char *)sqlite3_column_text(stmt, i);
            row->values[i] = val ? strdup(val) : NULL;
        }

        result->row_count++;
    }

    sqlite3_finalize(stmt);
    return rc == SQLITE_DONE ? SQLITE_OK : rc;
}
