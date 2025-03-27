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

/**
 * 执行简单的SQL查询并返回结果数量
 *
 * @param db 数据库连接指针
 * @param query 要执行的SQL查询语句
 * @param count 输出参数，用于存储查询结果的记录数量
 * @return 成功返回0，失败返回错误码
 */
int db_count_query(Database *db, const char *query, int *count);

/**
 * 检查数据库表是否存在
 *
 * @param db 数据库连接指针
 * @param table_name 要检查的表名
 * @return 表存在返回true，不存在返回false
 */
bool db_table_exists(Database *db, const char *table_name);

/**
 * 检查数据库记录是否存在
 *
 * @param db 数据库连接指针
 * @param query 用于检查记录是否存在的SQL查询
 * @return 记录存在返回true，不存在返回false
 */
bool db_record_exists(Database *db, const char *query);

/**
 * 备份数据库
 *
 * @param db 数据库连接指针
 * @return 备份成功返回true，失败返回false
 */
bool backup_database(Database *db);

/**
 * 恢复数据库
 *
 * @param db 数据库连接指针
 * @return 恢复成功返回true，失败返回false
 */
bool restore_database(Database *db);

#endif /* DB_UTILS_H */
