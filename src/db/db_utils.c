#include "db/database.h"
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <sqlite3.h>

char* get_current_date() {
    time_t now = time(NULL);
    if (now == -1) return NULL;

    struct tm *t = localtime(&now);
    if (!t) return NULL;

    char* date = malloc(11);
    if (!date) return NULL;

    strftime(date, 11, "%Y-%m-%d", t);
    return date;
}
/**
 * 执行简单的SQL查询并返回结果数量
 *
 * @param db 数据库连接指针
 * @param query 要执行的SQL查询语句
 * @param count 输出参数，用于存储查询结果的记录数量
 * @return 成功返回0，失败返回错误码
 */
int db_count_query(Database *db, const char *query, int *count)
{
    // TODO
}


/**
 * 检查数据库表是否存在
 *
 * @param db 数据库连接指针
 * @param table_name 要检查的表名
 * @return 表存在返回true，不存在返回false
 */
bool db_table_exists(Database *db, const char *table_name)
{
    // TODO
}

/**
 * 检查数据库记录是否存在
 *
 * @param db 数据库连接指针
 * @param query 用于检查记录是否存在的SQL查询
 * @return 记录存在返回true，不存在返回false
 */
bool db_record_exists(Database *db, const char *query)
{
    // TODO
}

/**
 * 备份数据库
 *
 * @param db 数据库连接指针
 * @return 备份成功返回true，失败返回false
 */
bool backup_database(Database *db)
{
    if (!db)
        return false;

    char backup_path[256];
    time_t now = time(NULL);
    struct tm *t = localtime(&now);
    sprintf(backup_path, "backup_%04d%02d%02d_%02d%02d%02d.db",
            t->tm_year + 1900, t->tm_mon + 1, t->tm_mday,
            t->tm_hour, t->tm_min, t->tm_sec);

    sqlite3 *backup_db;
    int rc = sqlite3_open(backup_path, &backup_db);
    if (rc != SQLITE_OK)
    {
        printf("无法创建备份数据库: %s\n", sqlite3_errmsg(backup_db));
        sqlite3_close(backup_db);
        return false;
    }

    sqlite3_backup *backup = sqlite3_backup_init(backup_db, "main", db->db, "main");
    if (!backup)
    {
        printf("无法初始化备份: %s\n", sqlite3_errmsg(backup_db));
        sqlite3_close(backup_db);
        return false;
    }

    rc = sqlite3_backup_step(backup, -1);
    if (rc != SQLITE_DONE)
    {
        printf("备份过程中出错: %s\n", sqlite3_errmsg(backup_db));
    }

    sqlite3_backup_finish(backup);
    sqlite3_close(backup_db);

    return (rc == SQLITE_DONE);
}

/**
 * 恢复数据库
 *
 * @param db 数据库连接指针
 * @return 恢复成功返回true，失败返回false
 */
bool restore_database(Database *db)
{
    if (!db)
        return false;

    char backup_path[256];
    printf("请输入备份文件名: ");
    scanf("%255s", backup_path);

    sqlite3 *backup_db;
    int rc = sqlite3_open(backup_path, &backup_db);
    if (rc != SQLITE_OK)
    {
        printf("无法打开备份数据库: %s\n", sqlite3_errmsg(backup_db));
        sqlite3_close(backup_db);
        return false;
    }

    sqlite3_backup *backup = sqlite3_backup_init(db->db, "main", backup_db, "main");
    if (!backup)
    {
        printf("无法初始化恢复: %s\n", sqlite3_errmsg(db->db));
        sqlite3_close(backup_db);
        return false;
    }

    rc = sqlite3_backup_step(backup, -1);
    if (rc != SQLITE_DONE)
    {
        printf("恢复过程中出错: %s\n", sqlite3_errmsg(db->db));
    }

    sqlite3_backup_finish(backup);
    sqlite3_close(backup_db);

    return (rc == SQLITE_DONE);
}

/**
 * 重置数据库
 *
 * @param db 数据库连接指针
 * @return 重置成功返回true，失败返回false
 */
bool clean_database(Database *db) {
    const char *cleanup_queries[] = {
        "DELETE FROM transactions;",
        "DELETE FROM rooms;",
        "DELETE FROM users WHERE role_id = 'role_owner';",
        "VACUUM;",
        NULL
    };

    for (int i = 0; cleanup_queries[i]; i++) {
        if (!execute_update(db, cleanup_queries[i])) {
            fprintf(stderr, "清理数据失败: %s\n", cleanup_queries[i]);
            return false;
        }
    }

    return db_init_tables(db);
}
