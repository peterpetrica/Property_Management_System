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

/**
 * query_callback - 查询回调函数
 *
 * 将SQLite查询结果添加到QueryResult结构中
 *
 * @param data 指向QueryResult结构的指针
 * @param argc 结果列数
 * @param argv 结果值数组
 * @param azColName 列名数组
 * @return 0表示成功，1表示失败
 */
static int query_callback(void *data, int argc, char **argv, char **azColName)
{
    QueryResult *result = (QueryResult *)data;

    if (result->rows == NULL)
    {
        result->column_count = argc;
        result->row_count = 0;

        result->column_names = (char **)malloc(argc * sizeof(char *));
        if (!result->column_names)
        {
            fprintf(stderr, "内存分配失败：列名数组\n");
            return 1;
        }

        for (int i = 0; i < argc; i++)
        {
            result->column_names[i] = strdup(azColName[i]);
            if (!result->column_names[i])
            {
                fprintf(stderr, "内存分配失败：列名\n");
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

    QueryRow *new_rows = (QueryRow *)realloc(result->rows, (result->row_count + 1) * sizeof(QueryRow));
    if (!new_rows)
    {
        fprintf(stderr, "内存分配失败：行数组扩展\n");
        return 1;
    }
    result->rows = new_rows;

    result->rows[result->row_count].columns = argc;
    result->rows[result->row_count].values = (char **)malloc(argc * sizeof(char *));
    if (!result->rows[result->row_count].values)
    {
        fprintf(stderr, "内存分配失败：行值数组\n");
        return 1;
    }

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

/**
 * execute_query - 执行SQL查询
 *
 * 执行指定的SQL查询语句并将结果存入QueryResult结构
 *
 * @param db 数据库连接指针
 * @param sql 要执行的SQL查询语句
 * @param result 用于存储查询结果的结构指针
 * @return true表示成功，false表示失败
 */
bool execute_query(Database *db, const char *sql, QueryResult *result)
{
    memset(result, 0, sizeof(QueryResult));

    char *err_msg = NULL;
    int rc = sqlite3_exec(db->db, sql, query_callback, (void *)result, &err_msg);

    if (rc != SQLITE_OK)
    {
        fprintf(stderr, "SQL错误: %s\n", err_msg);
        sqlite3_free(err_msg);
        free_query_result(result);
        return false;
    }

    return true;
}

/**
 * 执行更新操作（无需返回结果的查询）
 *
 * @param db 数据库连接
 * @param query SQL查询语句
 * @return 执行成功返回true，失败返回false
 */
bool execute_update(Database *db, const char *query)
{
    if (!db || !db->db)
    {
        fprintf(stderr, "数据库连接无效\n");
        return false;
    }

    char *err_msg = NULL;
    int rc = sqlite3_exec(db->db, query, NULL, NULL, &err_msg);

    if (rc != SQLITE_OK)
    {
        fprintf(stderr, "SQL执行错误: %s\n", err_msg);
        sqlite3_free(err_msg);
        return false;
    }

    return true;
}

/**
 * free_query_result - 释放查询结果资源
 *
 * 释放QueryResult结构中分配的所有内存
 *
 * @param result 要释放的查询结果结构指针
 */
void free_query_result(QueryResult *result)
{
    if (!result)
        return;

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

/**
 * get_building_id_by_name - 通过建筑名称获取建筑ID
 *
 * 根据给定的建筑名称查询对应的建筑ID
 *
 * @param db 数据库连接指针
 * @param building_name 建筑名称
 * @param building_id 用于存储查询到的建筑ID的缓冲区
 * @return true表示成功找到，false表示未找到或查询失败
 */
bool get_building_id_by_name(Database *db, const char *building_name, char *building_id)
{
    char sql[256];
    snprintf(sql, sizeof(sql),
             "SELECT building_id FROM buildings WHERE building_name = '%s'",
             building_name);

    QueryResult result;
    if (!execute_query(db, sql, &result) || result.row_count == 0)
    {
        free_query_result(&result);
        return false;
    }

    strncpy(building_id, result.rows[0].values[0], 40);
    free_query_result(&result);
    return true;
}

/**
 * db_compound_query - 执行复合查询
 *
 * 执行涉及多表联合的复杂SQL查询
 *
 * @param db 数据库连接指针
 * @param query 要执行的复合SQL查询
 * @param result 用于存储查询结果的结构指针
 * @return SQLITE_OK表示成功，其他值表示失败
 */
int db_compound_query(Database *db, const char *query, QueryResult *result)
{
    // TODO: 实现复合查询
    return SQLITE_ERROR;
}

/**
 * db_fuzzy_query - 执行模糊查询
 *
 * 使用模糊匹配条件执行SQL查询，支持使用通配符进行字符串模式匹配
 *
 * @param db 数据库连接指针
 * @param table 要查询的表名
 * @param column 要进行模糊匹配的列名
 * @param pattern 模糊匹配的模式
 * @param result 用于存储查询结果的结构指针
 * @return SQLITE_OK表示成功，其他值表示失败
 *
 * 示例:
 *   QueryResult result;
 *   int rc = db_fuzzy_query(db, "users", "name", "张", &result);
 *   if (rc == SQLITE_OK) {
 *       // 处理查询结果
 *       free_query_result(&result);
 *   }
 */
int db_fuzzy_query(Database *db, const char *table, const char *column, const char *pattern, QueryResult *result)
{
    if (!db || !table || !column || !pattern || !result)
    {
        fprintf(stderr, "模糊查询参数无效\n");
        return SQLITE_ERROR;
    }

    // 分配足够大的缓冲区构建SQL语句
    char sql[512];

    // 使用SQLite的安全转义机制
    char *safe_pattern = sqlite3_mprintf("%%%q%%", pattern);
    if (!safe_pattern)
    {
        fprintf(stderr, "内存分配失败：安全转义模式\n");
        return SQLITE_NOMEM;
    }

    // 构建安全的SQL查询语句
    snprintf(sql, sizeof(sql), "SELECT * FROM %s WHERE %s LIKE '%s'",
             table, column, safe_pattern);

    // 释放转义后的模式字符串
    sqlite3_free(safe_pattern);

    // 执行查询并存储结果
    bool success = execute_query(db, sql, result);

    if (!success)
    {
        fprintf(stderr, "执行模糊查询失败\n");
        return SQLITE_ERROR;
    }

    return SQLITE_OK;
}

/**
 * db_privileged_query - 执行特权查询
 *
 * 基于用户权重执行访问控制的查询
 *
 * @param db 数据库连接指针
 * @param query 要执行的SQL查询
 * @param weight 用户权重值
 * @param result 用于存储查询结果的结构指针
 * @return SQLITE_OK表示成功，其他值表示失败
 */
int db_privileged_query(Database *db, const char *query, int weight, QueryResult *result)
{
    // TODO: 实现特权查询
    return SQLITE_ERROR;
}

/**
 * db_parameterized_query - 执行参数化查询
 *
 * 使用预编译语句和参数绑定执行SQL查询，防止SQL注入
 *
 * @param db 数据库连接指针
 * @param query 要执行的参数化SQL查询
 * @param params 参数数据结构指针
 * @param bind_params 用于绑定参数的回调函数
 * @param result 用于存储查询结果的结构指针
 * @return SQLITE_OK表示成功，其他值表示失败
 */
int db_parameterized_query(Database *db, const char *query, void *params,
                           void (*bind_params)(sqlite3_stmt *, void *),
                           QueryResult *result)
{
    sqlite3_stmt *stmt;
    int rc;

    rc = sqlite3_prepare_v2(db->db, query, -1, &stmt, NULL);
    if (rc != SQLITE_OK)
    {
        fprintf(stderr, "准备SQL语句失败: %s\n", sqlite3_errmsg(db->db));
        return rc;
    }

    if (bind_params)
    {
        bind_params(stmt, params);
    }

    memset(result, 0, sizeof(QueryResult));

    int column_count = sqlite3_column_count(stmt);
    result->column_count = column_count;

    result->column_names = (char **)malloc(column_count * sizeof(char *));
    if (!result->column_names)
    {
        sqlite3_finalize(stmt);
        return SQLITE_NOMEM;
    }

    for (int i = 0; i < column_count; i++)
    {
        const char *column_name = sqlite3_column_name(stmt, i);
        result->column_names[i] = strdup(column_name);
    }

    while ((rc = sqlite3_step(stmt)) == SQLITE_ROW)
    {
        QueryRow *new_rows = (QueryRow *)realloc(result->rows, (result->row_count + 1) * sizeof(QueryRow));
        if (!new_rows)
        {
            free_query_result(result);
            sqlite3_finalize(stmt);
            return SQLITE_NOMEM;
        }
        result->rows = new_rows;

        result->rows[result->row_count].columns = column_count;
        result->rows[result->row_count].values = (char **)malloc(column_count * sizeof(char *));

        if (!result->rows[result->row_count].values)
        {
            free_query_result(result);
            sqlite3_finalize(stmt);
            return SQLITE_NOMEM;
        }

        for (int i = 0; i < column_count; i++)
        {
            const unsigned char *value = sqlite3_column_text(stmt, i);
            result->rows[result->row_count].values[i] = value ? strdup((const char *)value) : NULL;
        }

        result->row_count++;
    }

    sqlite3_finalize(stmt);

    if (rc != SQLITE_DONE)
    {
        fprintf(stderr, "执行查询失败: %s\n", sqlite3_errmsg(db->db));
        free_query_result(result);
        return rc;
    }

    return SQLITE_OK;
}
