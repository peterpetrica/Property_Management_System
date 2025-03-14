#include "models/apartment.h"
#include "auth/auth.h"
#include "auth/tokens.h" // 添加此行，引入 get_user_id_from_token 函数声明
#include "utils/utils.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// 添加房屋
bool add_apartment(Database *db, const char *token, Apartment *apartment)
{
    if (!db || !token || !apartment)
        return false;

    // 验证令牌和管理员权限
    int weight;
    UserType user_type;
    if (!validate_token(db, token, &weight, &user_type) || user_type != USER_ADMIN)
    {
        return false;
    }

    // 构造SQL语句
    const char *sql = "INSERT INTO apartments (id, building_id, floor, unit, area, occupied, property_fee_balance, fee_paid) "
                      "VALUES (?, ?, ?, ?, ?, ?, ?, ?);";

    // 准备语句
    sqlite3_stmt *stmt;
    int rc = sqlite3_prepare_v2(db->db, sql, -1, &stmt, NULL);
    if (rc != SQLITE_OK)
        return false;

    // 绑定参数
    sqlite3_bind_text(stmt, 1, apartment->id, -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 2, apartment->building_id, -1, SQLITE_STATIC);
    sqlite3_bind_int(stmt, 3, apartment->floor);
    sqlite3_bind_int(stmt, 4, apartment->unit);
    sqlite3_bind_double(stmt, 5, apartment->area);
    sqlite3_bind_int(stmt, 6, apartment->occupied ? 1 : 0);
    sqlite3_bind_double(stmt, 7, apartment->property_fee_balance);
    sqlite3_bind_int(stmt, 8, apartment->fee_paid ? 1 : 0);

    // 执行SQL
    rc = sqlite3_step(stmt);
    bool result = (rc == SQLITE_DONE);

    sqlite3_finalize(stmt);
    return result;
}

// 修改房屋信息
bool update_apartment(Database *db, const char *token, Apartment *apartment)
{
    if (!db || !token || !apartment)
        return false;

    // 验证令牌
    int weight;
    UserType user_type;
    if (!validate_token(db, token, &weight, &user_type))
    {
        return false;
    }

    // 管理员可以修改所有信息，服务人员只能修改部分信息
    const char *sql;
    if (user_type == USER_ADMIN)
    {
        sql = "UPDATE apartments SET building_id = ?, floor = ?, unit = ?, area = ?, "
              "occupied = ?, property_fee_balance = ?, fee_paid = ? WHERE id = ?;";
    }
    else if (user_type == USER_STAFF)
    {
        // 服务人员只能更新物业费余额和缴费状态
        sql = "UPDATE apartments SET property_fee_balance = ?, fee_paid = ? WHERE id = ?;";
    }
    else
    {
        return false; // 业主不能修改房屋信息
    }

    // 准备语句
    sqlite3_stmt *stmt;
    int rc = sqlite3_prepare_v2(db->db, sql, -1, &stmt, NULL);
    if (rc != SQLITE_OK)
        return false;

    // 绑定参数
    if (user_type == USER_ADMIN)
    {
        sqlite3_bind_text(stmt, 1, apartment->building_id, -1, SQLITE_STATIC);
        sqlite3_bind_int(stmt, 2, apartment->floor);
        sqlite3_bind_int(stmt, 3, apartment->unit);
        sqlite3_bind_double(stmt, 4, apartment->area);
        sqlite3_bind_int(stmt, 5, apartment->occupied ? 1 : 0);
        sqlite3_bind_double(stmt, 6, apartment->property_fee_balance);
        sqlite3_bind_int(stmt, 7, apartment->fee_paid ? 1 : 0);
        sqlite3_bind_text(stmt, 8, apartment->id, -1, SQLITE_STATIC);
    }
    else
    {
        sqlite3_bind_double(stmt, 1, apartment->property_fee_balance);
        sqlite3_bind_int(stmt, 2, apartment->fee_paid ? 1 : 0);
        sqlite3_bind_text(stmt, 3, apartment->id, -1, SQLITE_STATIC);
    }

    // 执行SQL
    rc = sqlite3_step(stmt);
    bool result = (rc == SQLITE_DONE);

    sqlite3_finalize(stmt);
    return result;
}

// 删除房屋
bool delete_apartment(Database *db, const char *token, const char *apartment_id)
{
    if (!db || !token || !apartment_id)
        return false;

    // 验证令牌和管理员权限
    int weight;
    UserType user_type;
    if (!validate_token(db, token, &weight, &user_type) || user_type != USER_ADMIN)
    {
        return false;
    }

    // 构造删除SQL
    const char *sql = "DELETE FROM apartments WHERE id = ?;";

    // 准备语句
    sqlite3_stmt *stmt;
    int rc = sqlite3_prepare_v2(db->db, sql, -1, &stmt, NULL);
    if (rc != SQLITE_OK)
        return false;

    // 绑定参数
    sqlite3_bind_text(stmt, 1, apartment_id, -1, SQLITE_STATIC);

    // 执行删除
    rc = sqlite3_step(stmt);
    bool result = (rc == SQLITE_DONE);

    sqlite3_finalize(stmt);
    return result;
}

// 获取房屋信息
bool get_apartment(Database *db, const char *apartment_id, Apartment *apartment)
{
    if (!db || !apartment_id || !apartment)
        return false;

    // 构造SQL语句
    const char *sql = "SELECT id, building_id, floor, unit, area, occupied, property_fee_balance, fee_paid "
                      "FROM apartments WHERE id = ?;";

    // 准备语句
    sqlite3_stmt *stmt;
    int rc = sqlite3_prepare_v2(db->db, sql, -1, &stmt, NULL);
    if (rc != SQLITE_OK)
        return false;

    // 绑定参数
    sqlite3_bind_text(stmt, 1, apartment_id, -1, SQLITE_STATIC);

    // 执行SQL
    bool result = false;
    if (sqlite3_step(stmt) == SQLITE_ROW)
    {
        const char *id = (const char *)sqlite3_column_text(stmt, 0);
        const char *building_id = (const char *)sqlite3_column_text(stmt, 1);
        int floor = sqlite3_column_int(stmt, 2);
        int unit = sqlite3_column_int(stmt, 3);
        double area = sqlite3_column_double(stmt, 4);
        int occupied = sqlite3_column_int(stmt, 5);
        double property_fee_balance = sqlite3_column_double(stmt, 6);
        int fee_paid = sqlite3_column_int(stmt, 7);

        safe_strcpy(apartment->id, id ? id : "", sizeof(apartment->id));
        safe_strcpy(apartment->building_id, building_id ? building_id : "", sizeof(apartment->building_id));
        apartment->floor = floor;
        apartment->unit = unit;
        apartment->area = (float)area;
        apartment->occupied = occupied ? true : false;
        apartment->property_fee_balance = (float)property_fee_balance;
        apartment->fee_paid = fee_paid ? true : false;

        result = true;
    }

    sqlite3_finalize(stmt);
    return result;
}

// 获取某楼宇内所有房屋
bool list_apartments_by_building(Database *db, const char *token, const char *building_id, QueryResult *result)
{
    if (!db || !token || !building_id || !result)
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
    sprintf(sql, "SELECT * FROM apartments WHERE building_id = '%s';", building_id);

    // 执行查询
    return db_simple_query(db, sql, result) == SQLITE_OK;
}

// 查询业主的房屋
bool get_owner_apartments(Database *db, const char *token, const char *owner_id, QueryResult *result)
{
    if (!db || !token || !owner_id || !result)
        return false;

    // 验证令牌
    int weight;
    UserType user_type;
    char user_id[32] = {0};

    if (!validate_token(db, token, &weight, &user_type))
    {
        return false;
    }

    if (!get_user_id_from_token(db, token, user_id, sizeof(user_id)))
    {
        return false;
    }

    // 如果不是管理员且查询的不是自己的房屋，则拒绝
    if (user_type != USER_ADMIN && strcmp(user_id, owner_id) != 0)
    {
        // 检查是否是相关服务人员
        if (user_type == USER_STAFF)
        {
            // 此处应检查服务人员是否负责该业主房屋所在楼宇
            // 简化处理，允许所有服务人员查询
        }
        else
        {
            return false; // 业主只能查询自己的房屋
        }
    }

    // 构造SQL语句
    char sql[512];
    sprintf(sql,
            "SELECT a.* FROM apartments a "
            "JOIN owners o ON o.id = '%s' "
            "WHERE a.id = o.apartment_id;",
            owner_id);

    // 执行查询
    return db_simple_query(db, sql, result) == SQLITE_OK;
}
