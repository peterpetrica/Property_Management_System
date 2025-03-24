#ifndef DB_INIT_H
#define DB_INIT_H

#include "db/database.h"

/**
 * db_init_tables
 *
 * 创建物业管理系统所需的所有数据库表
 *
 * @param db 数据库连接指针
 * @return SQLITE_OK 表示成功，其他值表示失败
 */
int db_init_tables(Database *db);

/**
 * db_init_admin
 *
 * 初始化系统默认管理员账户
 *
 * @param db 数据库连接指针
 * @return SQLITE_OK 表示成功，其他值表示失败
 */
int db_init_admin(Database *db);

/**
 * db_init_staff
 *
 * 初始化系统默认物业服务人员账户
 *
 * @param db 数据库连接指针
 * @return SQLITE_OK 表示成功，其他值表示失败
 */
int db_init_staff(Database *db);

#endif /* DB_INIT_H */
