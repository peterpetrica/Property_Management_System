/**
 * database.c
 * 数据库操作模块的实现
 *
 * 本模块提供物业管理系统所需的数据库访问接口，实现了数据库的基本操作功能：
 * - 数据库的初始化和连接
 * - 关闭数据库连接
 * - SQL语句的执行
 * - 数据库的备份功能
 * - 数据库的恢复功能
 *
 * 该模块是系统与数据存储层之间的桥梁，为上层业务逻辑提供数据持久化支持。
 *
 * 所有函数需要进行适当的错误处理和资源管理
 */

#include "db/database.h"
#include "db/db_init.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/**
 * @brief 初始化数据库
 *
 * @param db 数据库结构体指针
 * @param db_path 数据库文件路径
 * @return int SQLITE_OK表示成功，其他值表示错误码
 */
int db_init(Database *db, const char *db_path)
{
    int rc;

    if (!db || !db_path)
    {
        fprintf(stderr, "数据库初始化失败：参数无效\n");
        return SQLITE_ERROR;
    }

    rc = sqlite3_open(db_path, &db->db);
    if (rc != SQLITE_OK)
    {
        fprintf(stderr, "无法打开数据库 %s: %s\n", db_path, sqlite3_errmsg(db->db));
        return rc;
    }

    rc = sqlite3_exec(db->db, "PRAGMA foreign_keys = ON;", NULL, NULL, NULL);
    if (rc != SQLITE_OK)
    {
        fprintf(stderr, "启用外键约束失败: %s\n", sqlite3_errmsg(db->db));
        sqlite3_close(db->db);
        db->db = NULL;
        return rc;
    }

    rc = db_init_tables(db);
    if (rc != SQLITE_OK)
    {
        fprintf(stderr, "初始化表结构失败\n");
        sqlite3_close(db->db);
        db->db = NULL;
        return rc;
    }

    rc = db_init_admin(db);
    if (rc != SQLITE_OK)
    {
        fprintf(stderr, "初始化默认管理员账户失败\n");
    }

    printf("数据库 %s 初始化成功\n", db_path);
    return SQLITE_OK;
}

/**
 * @brief 关闭数据库连接
 *
 * @param db 数据库结构体指针
 */
void db_close(Database *db)
{
    if (db && db->db)
    {
        sqlite3_close(db->db);
        db->db = NULL;
        printf("数据库连接已关闭\n");
    }
}

/**
 * @brief 执行SQL语句
 *
 * @param db 数据库结构体指针
 * @param sql 要执行的SQL语句
 * @return int SQLITE_OK表示成功，其他值表示错误码
 */
int db_execute(Database *db, const char *sql)
{
    int rc;
    char *err_msg = NULL;

    if (!db || !db->db || !sql)
    {
        fprintf(stderr, "执行SQL失败：参数无效\n");
        return SQLITE_ERROR;
    }

    rc = sqlite3_exec(db->db, sql, NULL, NULL, &err_msg);
    if (rc != SQLITE_OK)
    {
        fprintf(stderr, "SQL执行错误: %s\n", err_msg);
        sqlite3_free(err_msg);
        return rc;
    }

    return SQLITE_OK;
}

/**
 * @brief 准备SQL语句
 *
 * @param db 数据库结构体指针
 * @param sql 要准备的SQL语句
 * @param stmt 准备好的语句句柄
 * @return int SQLITE_OK表示成功，其他值表示错误码
 */
int db_prepare(Database *db, const char *sql, sqlite3_stmt **stmt)
{
    int rc;

    if (!db || !db->db || !sql || !stmt)
    {
        fprintf(stderr, "准备SQL语句失败：参数无效\n");
        return SQLITE_ERROR;
    }

    rc = sqlite3_prepare_v2(db->db, sql, -1, stmt, NULL);
    if (rc != SQLITE_OK)
    {
        fprintf(stderr, "准备SQL语句失败: %s\n", sqlite3_errmsg(db->db));
        return rc;
    }

    return SQLITE_OK;
}

/**
 * @brief 数据库备份
 *
 * @param db 数据库结构体指针
 * @param backup_path 备份文件路径
 * @return int SQLITE_OK表示成功，其他值表示错误码
 */
int db_backup(Database *db, const char *backup_path)
{
    sqlite3 *backup_db = NULL;
    sqlite3_backup *backup = NULL;
    int rc;

    if (!db || !db->db || !backup_path)
    {
        fprintf(stderr, "备份失败：参数无效\n");
        return SQLITE_ERROR;
    }

    rc = sqlite3_open(backup_path, &backup_db);
    if (rc != SQLITE_OK)
    {
        fprintf(stderr, "无法创建备份数据库 %s: %s\n", backup_path, sqlite3_errmsg(backup_db));
        sqlite3_close(backup_db);
        return rc;
    }

    backup = sqlite3_backup_init(backup_db, "main", db->db, "main");
    if (!backup)
    {
        fprintf(stderr, "初始化备份失败: %s\n", sqlite3_errmsg(backup_db));
        sqlite3_close(backup_db);
        return SQLITE_ERROR;
    }

    rc = sqlite3_backup_step(backup, -1);
    if (rc != SQLITE_DONE)
    {
        fprintf(stderr, "备份操作失败: %s\n", sqlite3_errstr(rc));
    }

    sqlite3_backup_finish(backup);
    sqlite3_close(backup_db);

    if (rc == SQLITE_DONE)
    {
        printf("数据库成功备份到 %s\n", backup_path);
        return SQLITE_OK;
    }
    else
    {
        return rc;
    }
}

/**
 * @brief 从备份文件恢复数据库
 *
 * @param db 数据库结构体指针
 * @param backup_path 备份文件路径
 * @return int SQLITE_OK表示成功，其他值表示错误码
 */
int db_restore(Database *db, const char *backup_path)
{
    sqlite3 *backup_db = NULL;
    sqlite3_backup *backup = NULL;
    int rc;

    if (!db || !db->db || !backup_path)
    {
        fprintf(stderr, "恢复失败：参数无效\n");
        return SQLITE_ERROR;
    }

    rc = sqlite3_open(backup_path, &backup_db);
    if (rc != SQLITE_OK)
    {
        fprintf(stderr, "无法打开备份数据库 %s: %s\n", backup_path, sqlite3_errmsg(backup_db));
        sqlite3_close(backup_db);
        return rc;
    }

    backup = sqlite3_backup_init(db->db, "main", backup_db, "main");
    if (!backup)
    {
        fprintf(stderr, "初始化恢复失败: %s\n", sqlite3_errmsg(db->db));
        sqlite3_close(backup_db);
        return SQLITE_ERROR;
    }

    rc = sqlite3_backup_step(backup, -1);
    if (rc != SQLITE_DONE)
    {
        fprintf(stderr, "恢复操作失败: %s\n", sqlite3_errstr(rc));
    }

    sqlite3_backup_finish(backup);
    sqlite3_close(backup_db);

    if (rc == SQLITE_DONE)
    {
        printf("数据库从 %s 成功恢复\n", backup_path);
        return SQLITE_OK;
    }
    else
    {
        return rc;
    }
}
