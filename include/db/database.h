#ifndef DATABASE_H
#define DATABASE_H

#include <sqlite3.h>
#include <stdint.h>
#include <stdbool.h>

// 数据库连接句柄
typedef struct
{
    sqlite3 *db;
    char *db_path;
} Database;

// 初始化数据库
int db_init(Database *db, const char *db_path);

// 关闭数据库
void db_close(Database *db);

// 执行SQL语句
int db_execute(Database *db, const char *sql);

// 数据库备份
int db_backup(Database *db, const char *backup_path);

// 数据库恢复
int db_restore(Database *db, const char *backup_path);

#endif /* DATABASE_H */
