#ifndef DB_INIT_H
#define DB_INIT_H

#include "db/database.h"

// 初始化数据库并创建必要的表和默认数据
int db_init_tables(Database *db);

// 初始化默认管理员账户
int db_init_admin(Database *db);

// 关闭数据库连接
void db_close(Database *db);

#endif /* DB_INIT_H */
