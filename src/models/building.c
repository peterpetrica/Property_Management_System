/*
 * 楼宇管理模块
 *
 * 该文件实现了物业管理系统中楼宇相关的功能，包括：
 * - 楼宇管理：添加、修改、删除、查询楼宇信息
 * - 楼宇列表：获取系统中所有楼宇的列表
 * - 人员分配：管理服务人员与楼宇之间的分配关系
 *
 * 所有函数都需要数据库连接和合法的用户令牌来验证操作权限
 */
#include "models/building.h"
#include "auth/auth.h"
#include "utils/utils.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

// 添加楼宇
bool add_building(Database *db, const char *user_id, UserType user_type, Building *building)
{
    // 验证权限
    if (!validate_permission(db, user_id, user_type, 1))
    {
        printf("权限不足，无法添加楼宇。\n");
        return false;
    }

    // 生成唯一的楼宇ID
    char uuid[37];
    generate_uuid(uuid);
    strncpy(building->building_id, uuid, sizeof(building->building_id) - 1);
    building->building_id[sizeof(building->building_id) - 1] = '\0';

    // 构建SQL语句
    char sql[1024];
    snprintf(sql, sizeof(sql),
             "INSERT INTO buildings (building_id, building_name, address, floors_count) "
             "VALUES ('%s', '%s', '%s', %d)",
             building->building_id, building->building_name, building->address,
             building->floors_count);

    // 执行SQL语句 - 从db_execute改为execute_update
    if (execute_update(db, sql))
    {
        printf("楼宇添加成功。\n");
        return true;
    }
    else
    {
        printf("楼宇添加失败。\n");
        return false;
    }
}

// 修改楼宇信息
bool update_building(Database *db, const char *user_id, UserType user_type, Building *building)
{
    // 验证权限
    if (!validate_permission(db, user_id, user_type, 1))
    {
        printf("权限不足，无法修改楼宇信息。\n");
        return false;
    }

    // 检查楼宇是否存在
    char check_sql[256];
    snprintf(check_sql, sizeof(check_sql),
             "SELECT building_id FROM buildings WHERE building_id = '%s'",
             building->building_id);

    QueryResult check_result;
    // 从db_query改为execute_query
    if (!execute_query(db, check_sql, &check_result) || check_result.row_count == 0)
    {
        if (check_result.row_count > 0)
            free_query_result(&check_result);
        printf("楼宇不存在，无法修改。\n");
        return false;
    }
    free_query_result(&check_result);

    // 构建SQL语句
    char sql[1024];
    snprintf(sql, sizeof(sql),
             "UPDATE buildings SET "
             "building_name = '%s', address = '%s', floors_count = %d "
             "WHERE building_id = '%s'",
             building->building_name, building->address,
             building->floors_count, building->building_id);

    // 执行SQL语句 - 从db_execute改为execute_update
    if (execute_update(db, sql))
    {
        printf("楼宇信息更新成功。\n");
        return true;
    }
    else
    {
        printf("楼宇信息更新失败。\n");
        return false;
    }
}

// 删除楼宇
bool delete_building(Database *db, const char *user_id, UserType user_type, const char *building_id)
{
    // 验证权限
    if (!validate_permission(db, user_id, user_type, 1))
    {
        printf("权限不足，无法删除楼宇。\n");
        return false;
    }

    // 检查是否有关联的房屋
    char check_rooms_sql[256];
    snprintf(check_rooms_sql, sizeof(check_rooms_sql),
             "SELECT room_id FROM rooms WHERE building_id = '%s' LIMIT 1",
             building_id);

    QueryResult check_rooms_result;
    // 从db_query改为execute_query
    if (execute_query(db, check_rooms_sql, &check_rooms_result) && check_rooms_result.row_count > 0)
    {
        free_query_result(&check_rooms_result);
        printf("楼宇存在关联的房屋，无法删除。\n");
        return false;
    }
    if (check_rooms_result.row_count > 0)
        free_query_result(&check_rooms_result);

    // 检查是否有关联的服务区域
    char check_areas_sql[256];
    snprintf(check_areas_sql, sizeof(check_areas_sql),
             "SELECT area_id FROM service_areas WHERE building_id = '%s' LIMIT 1",
             building_id);

    QueryResult check_areas_result;
    // 从db_query改为execute_query
    if (execute_query(db, check_areas_sql, &check_areas_result) && check_areas_result.row_count > 0)
    {
        free_query_result(&check_areas_result);
        // 删除服务区域关联
        char delete_areas_sql[256];
        snprintf(delete_areas_sql, sizeof(delete_areas_sql),
                 "DELETE FROM service_areas WHERE building_id = '%s'", building_id);

        // 从db_execute改为execute_update
        if (!execute_update(db, delete_areas_sql))
        {
            printf("删除楼宇关联的服务区域失败。\n");
            return false;
        }
    }
    if (check_areas_result.row_count > 0)
        free_query_result(&check_areas_result);

    // 构建SQL语句
    char sql[256];
    snprintf(sql, sizeof(sql),
             "DELETE FROM buildings WHERE building_id = '%s'",
             building_id);

    // 执行SQL语句 - 从db_execute改为execute_update
    if (execute_update(db, sql))
    {
        printf("楼宇删除成功。\n");
        return true;
    }
    else
    {
        printf("楼宇删除失败。\n");
        return false;
    }
}

// 获取楼宇信息
bool get_building(Database *db, const char *building_id, Building *building)
{
    // 构建SQL语句
    char sql[256];
    snprintf(sql, sizeof(sql),
             "SELECT building_id, building_name, address, floors_count "
             "FROM buildings WHERE building_id = '%s'",
             building_id);

    // 执行查询 - 从db_query改为execute_query
    QueryResult result;
    if (!execute_query(db, sql, &result) || result.row_count == 0)
    {
        if (result.row_count > 0)
            free_query_result(&result);
        printf("查询楼宇信息失败或楼宇不存在。\n");
        return false;
    }

    // 解析结果
    strncpy(building->building_id, result.rows[0].values[0], sizeof(building->building_id) - 1);
    building->building_id[sizeof(building->building_id) - 1] = '\0';

    strncpy(building->building_name, result.rows[0].values[1], sizeof(building->building_name) - 1);
    building->building_name[sizeof(building->building_name) - 1] = '\0';

    strncpy(building->address, result.rows[0].values[2], sizeof(building->address) - 1);
    building->address[sizeof(building->address) - 1] = '\0';

    building->floors_count = atoi(result.rows[0].values[3]);

    free_query_result(&result);
    return true;
}

// 获取所有楼宇列表
bool list_buildings(Database *db, const char *user_id, UserType user_type, QueryResult *result)
{
    // 构建SQL语句
    char sql[256];
    snprintf(sql, sizeof(sql),
             "SELECT building_id, building_name, address, floors_count "
             "FROM buildings ORDER BY building_name");

    // 执行查询 - 从db_query改为execute_query
    if (execute_query(db, sql, result))
    {
        return true;
    }
    else
    {
        printf("获取楼宇列表失败。\n");
        return false;
    }
}

// 分配服务人员到楼宇
bool assign_staff_to_building(Database *db, const char *user_id, UserType user_type, const char *staff_id, const char *building_id)
{
    // 验证权限
    if (!validate_permission(db, user_id, user_type, 1))
    {
        printf("权限不足，无法分配服务人员。\n");
        return false;
    }

    // 检查服务人员是否存在
    char check_staff_sql[256];
    snprintf(check_staff_sql, sizeof(check_staff_sql),
             "SELECT staff_id FROM staff WHERE staff_id = '%s'",
             staff_id);

    QueryResult check_staff_result;
    // 从db_query改为execute_query
    if (!execute_query(db, check_staff_sql, &check_staff_result) || check_staff_result.row_count == 0)
    {
        if (check_staff_result.row_count > 0)
            free_query_result(&check_staff_result);
        printf("服务人员不存在。\n");
        return false;
    }
    free_query_result(&check_staff_result);

    // 检查楼宇是否存在
    char check_building_sql[256];
    snprintf(check_building_sql, sizeof(check_building_sql),
             "SELECT building_id FROM buildings WHERE building_id = '%s'",
             building_id);

    QueryResult check_building_result;
    // 从db_query改为execute_query
    if (!execute_query(db, check_building_sql, &check_building_result) || check_building_result.row_count == 0)
    {
        if (check_building_result.row_count > 0)
            free_query_result(&check_building_result);
        printf("楼宇不存在。\n");
        return false;
    }
    free_query_result(&check_building_result);

    // 检查是否已存在分配关系
    char check_assign_sql[256];
    snprintf(check_assign_sql, sizeof(check_assign_sql),
             "SELECT area_id FROM service_areas WHERE staff_id = '%s' AND building_id = '%s'",
             staff_id, building_id);

    QueryResult check_assign_result;
    // 从db_query改为execute_query
    if (execute_query(db, check_assign_sql, &check_assign_result) && check_assign_result.row_count > 0)
    {
        free_query_result(&check_assign_result);
        printf("该服务人员已分配到此楼宇。\n");
        return false;
    }
    if (check_assign_result.row_count > 0)
        free_query_result(&check_assign_result);

    // 生成唯一的区域ID
    char area_id[37];
    generate_uuid(area_id);

    // 获取当前日期
    time_t now = time(NULL);
    struct tm *timeinfo = localtime(&now);
    char date_str[11];
    strftime(date_str, sizeof(date_str), "%Y-%m-%d", timeinfo);

    // 构建SQL语句
    char sql[512];
    snprintf(sql, sizeof(sql),
             "INSERT INTO service_areas (area_id, staff_id, building_id, assignment_date) "
             "VALUES ('%s', '%s', '%s', '%s')",
             area_id, staff_id, building_id, date_str);

    // 执行SQL语句 - 从db_execute改为execute_update
    if (execute_update(db, sql))
    {
        printf("服务人员成功分配到楼宇。\n");
        return true;
    }
    else
    {
        printf("服务人员分配失败。\n");
        return false;
    }
}

// 取消服务人员的楼宇分配
bool unassign_staff_from_building(Database *db, const char *user_id, UserType user_type, const char *staff_id, const char *building_id)
{
    // 验证权限
    if (!validate_permission(db, user_id, user_type, 1))
    {
        printf("权限不足，无法取消服务人员分配。\n");
        return false;
    }

    // 检查是否存在分配关系
    char check_sql[256];
    snprintf(check_sql, sizeof(check_sql),
             "SELECT area_id FROM service_areas WHERE staff_id = '%s' AND building_id = '%s'",
             staff_id, building_id);

    QueryResult check_result;
    // 从db_query改为execute_query
    if (!execute_query(db, check_sql, &check_result) || check_result.row_count == 0)
    {
        if (check_result.row_count > 0)
            free_query_result(&check_result);
        printf("该服务人员未分配到此楼宇。\n");
        return false;
    }
    free_query_result(&check_result);

    // 构建SQL语句
    char sql[256];
    snprintf(sql, sizeof(sql),
             "DELETE FROM service_areas WHERE staff_id = '%s' AND building_id = '%s'",
             staff_id, building_id);

    // 执行SQL语句 - 从db_execute改为execute_update
    if (execute_update(db, sql))
    {
        printf("服务人员与楼宇的分配关系已取消。\n");
        return true;
    }
    else
    {
        printf("取消分配关系失败。\n");
        return false;
    }
}
