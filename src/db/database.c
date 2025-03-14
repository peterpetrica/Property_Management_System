#include "db/database.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// 初始化数据库
int db_init(Database *db, const char *db_path)
{
    if (!db || !db_path)
        return SQLITE_ERROR;

    int rc = sqlite3_open(db_path, &db->db);
    if (rc != SQLITE_OK)
    {
        fprintf(stderr, "无法打开数据库: %s\n", sqlite3_errmsg(db->db));
        return rc;
    }

    // 保存数据库路径
    db->db_path = strdup(db_path);
    if (!db->db_path)
    {
        sqlite3_close(db->db);
        db->db = NULL;
        return SQLITE_NOMEM;
    }

    return SQLITE_OK;
}

// 关闭数据库
void db_close(Database *db)
{
    if (!db)
        return;

    if (db->db)
    {
        sqlite3_close(db->db);
        db->db = NULL;
    }

    if (db->db_path)
    {
        free(db->db_path);
        db->db_path = NULL;
    }
}

// 执行SQL语句
int db_execute(Database *db, const char *sql)
{
    if (!db || !db->db || !sql)
        return SQLITE_ERROR;

    char *err_msg = NULL;
    int rc = sqlite3_exec(db->db, sql, NULL, 0, &err_msg);

    if (rc != SQLITE_OK)
    {
        fprintf(stderr, "SQL错误: %s\n", err_msg);
        sqlite3_free(err_msg);
    }

    return rc;
}

// 数据库备份
int db_backup(Database *db, const char *backup_path)
{
    if (!db || !db->db || !backup_path)
        return SQLITE_ERROR;

    sqlite3 *backup_db = NULL;
    int rc = sqlite3_open(backup_path, &backup_db);
    if (rc != SQLITE_OK)
    {
        fprintf(stderr, "无法创建备份数据库: %s\n", sqlite3_errmsg(backup_db));
        sqlite3_close(backup_db);
        return rc;
    }

    sqlite3_backup *backup = sqlite3_backup_init(backup_db, "main", db->db, "main");
    if (!backup)
    {
        fprintf(stderr, "备份初始化失败: %s\n", sqlite3_errmsg(backup_db));
        sqlite3_close(backup_db);
        return SQLITE_ERROR;
    }

    rc = sqlite3_backup_step(backup, -1);
    sqlite3_backup_finish(backup);

    if (rc != SQLITE_DONE)
    {
        fprintf(stderr, "备份失败: %s\n", sqlite3_errmsg(backup_db));
    }

    sqlite3_close(backup_db);
    return (rc == SQLITE_DONE) ? SQLITE_OK : rc;
}

// 数据库恢复
int db_restore(Database *db, const char *backup_path)
{
    if (!db || !db->db || !backup_path)
        return SQLITE_ERROR;

    sqlite3 *backup_db = NULL;
    int rc = sqlite3_open(backup_path, &backup_db);
    if (rc != SQLITE_OK)
    {
        fprintf(stderr, "无法打开备份数据库: %s\n", sqlite3_errmsg(backup_db));
        sqlite3_close(backup_db);
        return rc;
    }

    sqlite3_backup *backup = sqlite3_backup_init(db->db, "main", backup_db, "main");
    if (!backup)
    {
        fprintf(stderr, "恢复初始化失败: %s\n", sqlite3_errmsg(db->db));
        sqlite3_close(backup_db);
        return SQLITE_ERROR;
    }

    rc = sqlite3_backup_step(backup, -1);
    sqlite3_backup_finish(backup);

    if (rc != SQLITE_DONE)
    {
        fprintf(stderr, "恢复失败: %s\n", sqlite3_errmsg(db->db));
    }

    sqlite3_close(backup_db);
    return (rc == SQLITE_DONE) ? SQLITE_OK : rc;
}
