#include "models/building.h"
#include "auth/auth.h"
#include "utils/utils.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// 添加小区
bool add_community(Database *db, const char *token, Community *community)
{
    if (!db || !token || !community)
        return false;

    // 验证令牌和管理员权限
    int weight;
    UserType user_type;
    if (!validate_token(db, token, &weight, &user_type) || user_type != USER_ADMIN)
    {
        return false;
    }

    // 构造SQL语句
    const char *sql = "INSERT INTO communities (id, name) VALUES (?, ?);";

    // 准备语句
    sqlite3_stmt *stmt;
    int rc = sqlite3_prepare_v2(db->db, sql, -1, &stmt, NULL);
    if (rc != SQLITE_OK)
        return false;

    // 绑定参数
    sqlite3_bind_text(stmt, 1, community->id, -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 2, community->name, -1, SQLITE_STATIC);

    // 执行SQL
    rc = sqlite3_step(stmt);
    bool result = (rc == SQLITE_DONE);

    sqlite3_finalize(stmt);
    return result;
}

// 添加楼宇
bool add_building(Database *db, const char *token, Building *building)
{
    if (!db || !token || !building)
        return false;

    // 验证令牌和管理员权限
    int weight;
    UserType user_type;
    if (!validate_token(db, token, &weight, &user_type) || user_type != USER_ADMIN)
    {
        return false;
    }

    // 构造SQL语句
    const char *sql = "INSERT INTO buildings (id, building_number, community_id, floors, units_per_floor) "
                      "VALUES (?, ?, ?, ?, ?);";

    // 准备语句
    sqlite3_stmt *stmt;
    int rc = sqlite3_prepare_v2(db->db, sql, -1, &stmt, NULL);
    if (rc != SQLITE_OK)
        return false;

    // 绑定参数
    sqlite3_bind_text(stmt, 1, building->id, -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 2, building->building_number, -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 3, building->community_id, -1, SQLITE_STATIC);
    sqlite3_bind_int(stmt, 4, building->floors);
    sqlite3_bind_int(stmt, 5, building->units_per_floor);

    // 执行SQL
    rc = sqlite3_step(stmt);
    bool result = (rc == SQLITE_DONE);

    sqlite3_finalize(stmt);
    return result;
}

// 修改楼宇信息
bool update_building(Database *db, const char *token, Building *building)
{
    if (!db || !token || !building)
        return false;

    // 验证令牌和管理员权限
    int weight;
    UserType user_type;
    if (!validate_token(db, token, &weight, &user_type) || user_type != USER_ADMIN)
    {
        return false;
    }

    // 构造SQL语句
    const char *sql = "UPDATE buildings SET building_number = ?, community_id = ?, floors = ?, units_per_floor = ? "
                      "WHERE id = ?;";

    // 准备语句
    sqlite3_stmt *stmt;
    int rc = sqlite3_prepare_v2(db->db, sql, -1, &stmt, NULL);
    if (rc != SQLITE_OK)
        return false;

    // 绑定参数
    sqlite3_bind_text(stmt, 1, building->building_number, -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 2, building->community_id, -1, SQLITE_STATIC);
    sqlite3_bind_int(stmt, 3, building->floors);
    sqlite3_bind_int(stmt, 4, building->units_per_floor);
    sqlite3_bind_text(stmt, 5, building->id, -1, SQLITE_STATIC);

    // 执行SQL
    rc = sqlite3_step(stmt);
    bool result = (rc == SQLITE_DONE);

    sqlite3_finalize(stmt);
    return result;
}

// 删除楼宇
bool delete_building(Database *db, const char *token, const char *building_id)
{
    if (!db || !token || !building_id)
        return false;

    // 验证令牌和管理员权限
    int weight;
    UserType user_type;
    if (!validate_token(db, token, &weight, &user_type) || user_type != USER_ADMIN)
    {
        return false;
    }

    // 检查楼宇中是否有房屋
    const char *check_sql = "SELECT COUNT(*) FROM apartments WHERE building_id = ?;";
    sqlite3_stmt *check_stmt;
    int rc = sqlite3_prepare_v2(db->db, check_sql, -1, &check_stmt, NULL);
    if (rc != SQLITE_OK)
        return false;

    sqlite3_bind_text(check_stmt, 1, building_id, -1, SQLITE_STATIC);

    if (sqlite3_step(check_stmt) == SQLITE_ROW)
    {
        int count = sqlite3_column_int(check_stmt, 0);
        sqlite3_finalize(check_stmt);

        if (count > 0)
        {
            // 楼宇中有房屋，不能删除
            return false;
        }
    }
    else
    {
        sqlite3_finalize(check_stmt);
        return false;
    }

    // 构造删除SQL
    const char *sql = "DELETE FROM buildings WHERE id = ?;";

    // 准备语句
    sqlite3_stmt *stmt;
    rc = sqlite3_prepare_v2(db->db, sql, -1, &stmt, NULL);
    if (rc != SQLITE_OK)
        return false;

    // 绑定参数
    sqlite3_bind_text(stmt, 1, building_id, -1, SQLITE_STATIC);

    // 执行删除
    rc = sqlite3_step(stmt);
    bool result = (rc == SQLITE_DONE);

    sqlite3_finalize(stmt);
    return result;
}

// 获取楼宇信息
bool get_building(Database *db, const char *building_id, Building *building)
{
    if (!db || !building_id || !building)
        return false;

    // 构造SQL语句
    const char *sql = "SELECT id, building_number, community_id, floors, units_per_floor "
                      "FROM buildings WHERE id = ?;";

    // 准备语句
    sqlite3_stmt *stmt;
    int rc = sqlite3_prepare_v2(db->db, sql, -1, &stmt, NULL);
    if (rc != SQLITE_OK)
        return false;

    // 绑定参数
    sqlite3_bind_text(stmt, 1, building_id, -1, SQLITE_STATIC);

    // 执行SQL
    bool result = false;
    if (sqlite3_step(stmt) == SQLITE_ROW)
    {
        const char *id = (const char *)sqlite3_column_text(stmt, 0);
        const char *building_number = (const char *)sqlite3_column_text(stmt, 1);
        const char *community_id = (const char *)sqlite3_column_text(stmt, 2);
        int floors = sqlite3_column_int(stmt, 3);
        int units_per_floor = sqlite3_column_int(stmt, 4);

        safe_strcpy(building->id, id ? id : "", sizeof(building->id));
        safe_strcpy(building->building_number, building_number ? building_number : "", sizeof(building->building_number));
        safe_strcpy(building->community_id, community_id ? community_id : "", sizeof(building->community_id));
        building->floors = floors;
        building->units_per_floor = units_per_floor;

        result = true;
    }

    sqlite3_finalize(stmt);
    return result;
}

// 获取所有楼宇列表
bool list_buildings(Database *db, const char *token, QueryResult *result)
{
    if (!db || !token || !result)
        return false;

    // 验证令牌
    int weight;
    UserType user_type;
    if (!validate_token(db, token, &weight, &user_type))
    {
        return false;
    }

    // 构造SQL语句
    const char *sql = "SELECT b.id, b.building_number, c.name as community_name, b.floors, b.units_per_floor "
                      "FROM buildings b JOIN communities c ON b.community_id = c.id;";

    // 执行查询
    return db_simple_query(db, sql, result) == SQLITE_OK;
}

// 分配服务人员到楼宇
bool assign_staff_to_building(Database *db, const char *token, const char *staff_id, const char *building_id)
{
    if (!db || !token || !staff_id || !building_id)
        return false;

    // 验证令牌和管理员权限
    int weight;
    UserType user_type;
    if (!validate_token(db, token, &weight, &user_type) || user_type != USER_ADMIN)
    {
        return false;
    }

    // 检查分配是否已存在
    const char *check_sql = "SELECT COUNT(*) FROM staff_assignments WHERE staff_id = ? AND building_id = ?;";
    sqlite3_stmt *check_stmt;
    int rc = sqlite3_prepare_v2(db->db, check_sql, -1, &check_stmt, NULL);
    if (rc != SQLITE_OK)
        return false;

    sqlite3_bind_text(check_stmt, 1, staff_id, -1, SQLITE_STATIC);
    sqlite3_bind_text(check_stmt, 2, building_id, -1, SQLITE_STATIC);

    bool exists = false;
    if (sqlite3_step(check_stmt) == SQLITE_ROW)
    {
        exists = (sqlite3_column_int(check_stmt, 0) > 0);
    }
    sqlite3_finalize(check_stmt);

    if (exists)
    {
        // 分配已存在，无需重复添加
        return true;
    }

    // 生成唯一ID
    char *uuid = generate_uuid();
    if (!uuid)
        return false;

    // 构造SQL语句
    const char *sql = "INSERT INTO staff_assignments (id, staff_id, building_id) VALUES (?, ?, ?);";

    // 准备语句
    sqlite3_stmt *stmt;
    rc = sqlite3_prepare_v2(db->db, sql, -1, &stmt, NULL);
    if (rc != SQLITE_OK)
    {
        free(uuid);
        return false;
    }

    // 绑定参数
    sqlite3_bind_text(stmt, 1, uuid, -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 2, staff_id, -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 3, building_id, -1, SQLITE_STATIC);

    // 执行SQL
    rc = sqlite3_step(stmt);
    bool result = (rc == SQLITE_DONE);

    sqlite3_finalize(stmt);
    free(uuid);

    return result;
}

// 取消服务人员的楼宇分配
bool unassign_staff_from_building(Database *db, const char *token, const char *staff_id, const char *building_id)
{
    if (!db || !token || !staff_id || !building_id)
        return false;

    // 验证令牌和管理员权限
    int weight;
    UserType user_type;
    if (!validate_token(db, token, &weight, &user_type) || user_type != USER_ADMIN)
    {
        return false;
    }

    // 构造SQL语句
    const char *sql = "DELETE FROM staff_assignments WHERE staff_id = ? AND building_id = ?;";

    // 准备语句
    sqlite3_stmt *stmt;
    int rc = sqlite3_prepare_v2(db->db, sql, -1, &stmt, NULL);
    if (rc != SQLITE_OK)
        return false;

    // 绑定参数
    sqlite3_bind_text(stmt, 1, staff_id, -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 2, building_id, -1, SQLITE_STATIC);

    // 执行SQL
    rc = sqlite3_step(stmt);
    bool result = (rc == SQLITE_DONE);

    sqlite3_finalize(stmt);

    return result;
}
