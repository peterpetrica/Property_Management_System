#include "models/service.h"
#include "auth/auth.h"
#include "utils/utils.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

// 添加服务类型
bool add_service_type(Database *db, const char *token, ServiceType *type)
{
    if (!db || !token || !type)
        return false;

    // 验证令牌和管理员权限
    int weight;
    UserType user_type;
    if (!validate_token(db, token, &weight, &user_type) || user_type != USER_ADMIN)
    {
        return false;
    }

    // 构造SQL语句
    const char *sql = "INSERT INTO service_types (id, name, description) VALUES (?, ?, ?);";

    // 准备语句
    sqlite3_stmt *stmt;
    int rc = sqlite3_prepare_v2(db->db, sql, -1, &stmt, NULL);
    if (rc != SQLITE_OK)
        return false;

    // 绑定参数
    sqlite3_bind_text(stmt, 1, type->id, -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 2, type->name, -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 3, type->description, -1, SQLITE_STATIC);

    // 执行SQL
    rc = sqlite3_step(stmt);
    bool result = (rc == SQLITE_DONE);

    sqlite3_finalize(stmt);
    return result;
}

// 修改服务类型
bool update_service_type(Database *db, const char *token, ServiceType *type)
{
    if (!db || !token || !type)
        return false;

    // 验证令牌和管理员权限
    int weight;
    UserType user_type;
    if (!validate_token(db, token, &weight, &user_type) || user_type != USER_ADMIN)
    {
        return false;
    }

    // 构造SQL语句
    const char *sql = "UPDATE service_types SET name = ?, description = ? WHERE id = ?;";

    // 准备语句
    sqlite3_stmt *stmt;
    int rc = sqlite3_prepare_v2(db->db, sql, -1, &stmt, NULL);
    if (rc != SQLITE_OK)
        return false;

    // 绑定参数
    sqlite3_bind_text(stmt, 1, type->name, -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 2, type->description, -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 3, type->id, -1, SQLITE_STATIC);

    // 执行SQL
    rc = sqlite3_step(stmt);
    bool result = (rc == SQLITE_DONE);

    sqlite3_finalize(stmt);
    return result;
}

// 删除服务类型
bool delete_service_type(Database *db, const char *token, const char *type_id)
{
    if (!db || !token || !type_id)
        return false;

    // 验证令牌和管理员权限
    int weight;
    UserType user_type;
    if (!validate_token(db, token, &weight, &user_type) || user_type != USER_ADMIN)
    {
        return false;
    }

    // 构造SQL语句
    const char *sql = "DELETE FROM service_types WHERE id = ?;";

    // 准备语句
    sqlite3_stmt *stmt;
    int rc = sqlite3_prepare_v2(db->db, sql, -1, &stmt, NULL);
    if (rc != SQLITE_OK)
        return false;

    // 绑定参数
    sqlite3_bind_text(stmt, 1, type_id, -1, SQLITE_STATIC);

    // 执行SQL
    rc = sqlite3_step(stmt);
    bool result = (rc == SQLITE_DONE);

    sqlite3_finalize(stmt);
    return result;
}

// 获取所有服务类型
bool list_service_types(Database *db, QueryResult *result)
{
    if (!db || !result)
        return false;

    // 构造SQL语句
    const char *sql = "SELECT * FROM service_types;";

    // 执行查询
    return db_simple_query(db, sql, result) == SQLITE_OK;
}

// 记录服务
bool record_service(Database *db, const char *token, ServiceRecord *record)
{
    if (!db || !token || !record)
        return false;

    // 验证令牌
    int weight;
    UserType user_type;
    if (!validate_token(db, token, &weight, &user_type) || user_type == USER_OWNER)
    {
        // 业主不能记录服务
        return false;
    }

    // 构造SQL语句
    const char *sql = "INSERT INTO service_records "
                      "(id, staff_id, building_id, apartment_id, service_type_id, service_time, description) "
                      "VALUES (?, ?, ?, ?, ?, ?, ?);";

    // 准备语句
    sqlite3_stmt *stmt;
    int rc = sqlite3_prepare_v2(db->db, sql, -1, &stmt, NULL);
    if (rc != SQLITE_OK)
        return false;

    // 绑定参数
    sqlite3_bind_text(stmt, 1, record->id, -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 2, record->staff_id, -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 3, record->building_id, -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 4, record->apartment_id, -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 5, record->service_type_id, -1, SQLITE_STATIC);
    sqlite3_bind_int64(stmt, 6, record->service_time);
    sqlite3_bind_text(stmt, 7, record->description, -1, SQLITE_STATIC);

    // 执行SQL
    rc = sqlite3_step(stmt);
    bool result = (rc == SQLITE_DONE);

    sqlite3_finalize(stmt);
    return result;
}

// 获取服务记录
bool get_service_records_by_staff(Database *db, const char *token, const char *staff_id, QueryResult *result)
{
    if (!db || !token || !staff_id || !result)
        return false;

    // 验证令牌
    int weight;
    UserType user_type;
    if (!validate_token(db, token, &weight, &user_type))
    {
        return false;
    }

    // 构造SQL语句
    char sql[512];
    sprintf(sql, "SELECT * FROM service_records WHERE staff_id = '%s' ORDER BY service_time DESC;", staff_id);

    // 执行查询
    return db_simple_query(db, sql, result) == SQLITE_OK;
}

// 获取房屋的服务记录
bool get_service_records_by_apartment(Database *db, const char *token, const char *apartment_id, QueryResult *result)
{
    if (!db || !token || !apartment_id || !result)
        return false;

    // 验证令牌
    int weight;
    UserType user_type;
    if (!validate_token(db, token, &weight, &user_type))
    {
        return false;
    }

    // 构造SQL语句
    char sql[512];
    sprintf(sql, "SELECT * FROM service_records WHERE apartment_id = '%s' ORDER BY service_time DESC;", apartment_id);

    // 执行查询
    return db_simple_query(db, sql, result) == SQLITE_OK;
}
