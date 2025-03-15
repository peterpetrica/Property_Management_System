#include "db/database.h"
#include <stdio.h>
#include <string.h>

// 执行简单的SQL查询并返回结果数量
int db_count_query(Database *db, const char *query, int *count)
{
    // TODO
}

// 检查数据库表是否存在
bool db_table_exists(Database *db, const char *table_name)
{
    // TODO
}

// 检查数据库记录是否存在
bool db_record_exists(Database *db, const char *query)
{
    // TODO
}
