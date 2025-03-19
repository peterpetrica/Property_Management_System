/**
 * db_query.c
 * 数据库查询模块
 *
 * 本模块提供各种数据库查询功能，支持物业管理系统中的数据检索需求。
 *
 * 主要功能：
 * - 通用查询执行与结果处理
 * - 简单查询：执行基础SQL查询
 * - 复合查询：执行复杂多表联合查询
 * - 模糊查询：支持模糊匹配查询
 * - 特权查询：基于用户权重的访问控制查询
 * - 参数化查询：支持绑定参数的安全查询
 * - 查询结果资源管理
 */

#include "db/db_query.h"
#include "auth/auth.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>

// 查询回调函数，将结果添加到QueryResult结构中
static int query_callback(void *data, int argc, char **argv, char **azColName)
{
    QueryResult *result = (QueryResult *)data;

    // 首次调用时，初始化结果结构
    if (result->rows == NULL)
    {
        result->column_count = argc;
        result->row_count = 0;

        // 分配列名数组
        result->column_names = (char **)malloc(argc * sizeof(char *));
        if (!result->column_names)
        {
            fprintf(stderr, "内存分配失败：列名数组\n");
            return 1;
        }

        // 复制列名
        for (int i = 0; i < argc; i++)
        {
            result->column_names[i] = strdup(azColName[i]);
            if (!result->column_names[i])
            {
                fprintf(stderr, "内存分配失败：列名\n");
                // 释放之前分配的内存
                for (int j = 0; j < i; j++)
                {
                    free(result->column_names[j]);
                }
                free(result->column_names);
                result->column_names = NULL;
                return 1;
            }
        }
    }

    // 扩展行数组
    QueryRow *new_rows = (QueryRow *)realloc(result->rows, (result->row_count + 1) * sizeof(QueryRow));
    if (!new_rows)
    {
        fprintf(stderr, "内存分配失败：行数组扩展\n");
        return 1;
    }
    result->rows = new_rows;

    // 为当前行分配内存
    result->rows[result->row_count].columns = argc;
    result->rows[result->row_count].values = (char **)malloc(argc * sizeof(char *));
    if (!result->rows[result->row_count].values)
    {
        fprintf(stderr, "内存分配失败：行值数组\n");
        return 1;
    }

    // 复制行数据
    for (int i = 0; i < argc; i++)
    {
        if (argv[i])
        {
            result->rows[result->row_count].values[i] = strdup(argv[i]);
        }
        else
        {
            result->rows[result->row_count].values[i] = NULL;
        }
    }

    result->row_count++;
    return 0;
}

// 执行查询并返回结果
bool execute_query(Database *db, const char *sql, QueryResult *result)
{
    // 初始化查询结果
    memset(result, 0, sizeof(QueryResult));

    char *err_msg = NULL;
    int rc = sqlite3_exec(db->db, sql, query_callback, (void *)result, &err_msg);

    if (rc != SQLITE_OK)
    {
        fprintf(stderr, "SQL错误: %s\n", err_msg);
        sqlite3_free(err_msg);

        // 清理已分配的内存
        free_query_result(result);
        return false;
    }

    return true;
}

// 执行更新操作（插入、更新、删除）
bool execute_update(Database *db, const char *sql)
{
    char *err_msg = NULL;
    int rc = sqlite3_exec(db->db, sql, NULL, 0, &err_msg);

    if (rc != SQLITE_OK)
    {
        fprintf(stderr, "SQL错误: %s\n", err_msg);
        sqlite3_free(err_msg);
        return false;
    }

    return true;
}

// 释放查询结果资源
void free_query_result(QueryResult *result)
{
    if (!result)
        return;

    // 释放列名
    if (result->column_names)
    {
        for (int i = 0; i < result->column_count; i++)
        {
            if (result->column_names[i])
            {
                free(result->column_names[i]);
            }
        }
        free(result->column_names);
        result->column_names = NULL;
    }

    // 释放行数据
    if (result->rows)
    {
        for (int i = 0; i < result->row_count; i++)
        {
            if (result->rows[i].values)
            {
                for (int j = 0; j < result->rows[i].columns; j++)
                {
                    if (result->rows[i].values[j])
                    {
                        free(result->rows[i].values[j]);
                    }
                }
                free(result->rows[i].values);
            }
        }
        free(result->rows);
        result->rows = NULL;
    }

    result->row_count = 0;
    result->column_count = 0;
}

// 执行复合查询
int db_compound_query(Database *db, const char *query, QueryResult *result)
{
    // TODO: 实现复合查询
    return SQLITE_ERROR;
}

// 执行模糊查询
int db_fuzzy_query(Database *db, const char *table, const char *column, const char *pattern, QueryResult *result)
{
    // TODO: 实现模糊查询
    return SQLITE_ERROR;
}

// 执行特权查询
int db_privileged_query(Database *db, const char *query, int weight, QueryResult *result)
{
    // TODO: 实现特权查询
    return SQLITE_ERROR;
}

// 执行参数化查询 (保留已有函数接口)
int db_parameterized_query(Database *db, const char *query, void *params,
                           void (*bind_params)(sqlite3_stmt *, void *),
                           QueryResult *result)
{
    // 参数化查询的实现
    sqlite3_stmt *stmt;
    int rc;

    // 准备SQL语句
    rc = sqlite3_prepare_v2(db->db, query, -1, &stmt, NULL);
    if (rc != SQLITE_OK)
    {
        fprintf(stderr, "准备SQL语句失败: %s\n", sqlite3_errmsg(db->db));
        return rc;
    }

    // 绑定参数
    if (bind_params)
    {
        bind_params(stmt, params);
    }

    // 初始化结果结构
    memset(result, 0, sizeof(QueryResult));

    // 获取列数
    int column_count = sqlite3_column_count(stmt);
    result->column_count = column_count;

    // 分配列名数组
    result->column_names = (char **)malloc(column_count * sizeof(char *));
    if (!result->column_names)
    {
        sqlite3_finalize(stmt);
        return SQLITE_NOMEM;
    }

    // 获取列名
    for (int i = 0; i < column_count; i++)
    {
        const char *column_name = sqlite3_column_name(stmt, i);
        result->column_names[i] = strdup(column_name);
    }

    // 执行查询并获取结果
    while ((rc = sqlite3_step(stmt)) == SQLITE_ROW)
    {
        // 扩展行数组
        QueryRow *new_rows = (QueryRow *)realloc(result->rows, (result->row_count + 1) * sizeof(QueryRow));
        if (!new_rows)
        {
            free_query_result(result);
            sqlite3_finalize(stmt);
            return SQLITE_NOMEM;
        }
        result->rows = new_rows;

        // 设置当前行
        result->rows[result->row_count].columns = column_count;
        result->rows[result->row_count].values = (char **)malloc(column_count * sizeof(char *));

        if (!result->rows[result->row_count].values)
        {
            free_query_result(result);
            sqlite3_finalize(stmt);
            return SQLITE_NOMEM;
        }

        // 获取行数据
        for (int i = 0; i < column_count; i++)
        {
            const unsigned char *value = sqlite3_column_text(stmt, i);
            result->rows[result->row_count].values[i] = value ? strdup((const char *)value) : NULL;
        }

        result->row_count++;
    }

    // 完成查询
    sqlite3_finalize(stmt);

    if (rc != SQLITE_DONE)
    {
        fprintf(stderr, "执行查询失败: %s\n", sqlite3_errmsg(db->db));
        free_query_result(result);
        return rc;
    }

    return SQLITE_OK;
}
