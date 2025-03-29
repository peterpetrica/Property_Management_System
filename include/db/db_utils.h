/**
 * db_utils.h
 * 数据库工具模块头文件
 *
 * 本模块提供各种数据库辅助工具函数，简化数据库常见操作。
 *
 * 主要功能：
 * - 计数查询：快速获取查询结果的记录数量
 * - 表存在检查：验证特定表是否存在于数据库中
 * - 记录存在检查：验证特定记录是否存在
 * - 数据库备份：创建数据库的备份副本
 * - 数据库恢复：从备份副本恢复数据库
 */

#ifndef DB_UTILS_H
#define DB_UTILS_H

#include "db/database.h"
#include <stdbool.h>

// 数据库操作函数
bool backup_database(Database *db);
bool restore_database(Database *db); 
bool clean_database(Database *db);
bool db_init_tables(Database *db);

// 查询辅助函数
bool execute_update(Database *db, const char *sql);
int db_count_query(Database *db, const char *query, int *count);
bool db_table_exists(Database *db, const char *table_name);
bool db_record_exists(Database *db, const char *query);

#endif /* DB_UTILS_H */
