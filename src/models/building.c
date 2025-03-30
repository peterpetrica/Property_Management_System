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

/**
 * 添加楼宇
 *
 * 向系统中添加新的楼宇信息，需要管理员权限
 *
 * @param db 数据库连接
 * @param user_id 用户ID
 * @param user_type 用户类型
 * @param building 楼宇信息结构体，包含楼宇的基本信息
 * @return 添加成功返回true，失败返回false
 */
bool add_building(Database *db, const char *user_id, UserType user_type, Building *building)
{
    if (!validate_permission(db, user_id, user_type, 1))
    {
        printf("权限不足，无法添加楼宇。\n");
        return false;
    }

    char uuid[37];
    generate_uuid(uuid);
    strncpy(building->building_id, uuid, sizeof(building->building_id) - 1);
    building->building_id[sizeof(building->building_id) - 1] = '\0';

    char sql[1024];
    snprintf(sql, sizeof(sql),
             "INSERT INTO buildings (building_id, building_name, address, floors_count) "
             "VALUES ('%s', '%s', '%s', %d)",
             building->building_id, building->building_name, building->address,
             building->floors_count);

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

/**
 * 修改楼宇信息
 *
 * 更新系统中已存在楼宇的信息，需要管理员权限
 *
 * @param db 数据库连接
 * @param user_id 用户ID
 * @param user_type 用户类型
 * @param building 包含更新信息的楼宇结构体
 * @return 修改成功返回true，失败返回false
 */
bool update_building(Database *db, const char *user_id, UserType user_type, Building *building)
{
    if (!validate_permission(db, user_id, user_type, 1))
    {
        printf("权限不足，无法修改楼宇信息。\n");
        return false;
    }

    char check_sql[256];
    snprintf(check_sql, sizeof(check_sql),
             "SELECT building_id FROM buildings WHERE building_id = '%s'",
             building->building_id);

    QueryResult check_result;
    if (!execute_query(db, check_sql, &check_result) || check_result.row_count == 0)
    {
        if (check_result.row_count > 0)
            free_query_result(&check_result);
        printf("楼宇不存在，无法修改。\n");
        return false;
    }
    free_query_result(&check_result);

    char sql[1024];
    snprintf(sql, sizeof(sql),
             "UPDATE buildings SET "
             "building_name = '%s', address = '%s', floors_count = %d "
             "WHERE building_id = '%s'",
             building->building_name, building->address,
             building->floors_count, building->building_id);

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

/**
 * 删除楼宇
 *
 * 从系统中删除指定楼宇，需要管理员权限
 * 如果楼宇存在关联的房屋，则无法删除
 * 如果存在关联的服务区域，会先删除关联关系
 *
 * @param db 数据库连接
 * @param user_id 用户ID
 * @param user_type 用户类型
 * @param building_id 要删除的楼宇ID
 * @return 删除成功返回true，失败返回false
 */
bool delete_building(Database *db, const char *user_id, UserType user_type, const char *building_id)
{
    if (!validate_permission(db, user_id, user_type, 1))
    {
        printf("权限不足，无法删除楼宇。\n");
        return false;
    }

    char check_rooms_sql[256];
    snprintf(check_rooms_sql, sizeof(check_rooms_sql),
             "SELECT room_id FROM rooms WHERE building_id = '%s' LIMIT 1",
             building_id);

    QueryResult check_rooms_result;
    if (execute_query(db, check_rooms_sql, &check_rooms_result) && check_rooms_result.row_count > 0)
    {
        free_query_result(&check_rooms_result);
        printf("楼宇存在关联的房屋，无法删除。\n");
        return false;
    }
    if (check_rooms_result.row_count > 0)
        free_query_result(&check_rooms_result);

    char check_areas_sql[256];
    snprintf(check_areas_sql, sizeof(check_areas_sql),
             "SELECT area_id FROM service_areas WHERE building_id = '%s' LIMIT 1",
             building_id);

    QueryResult check_areas_result;
    if (execute_query(db, check_areas_sql, &check_areas_result) && check_areas_result.row_count > 0)
    {
        free_query_result(&check_areas_result);
        char delete_areas_sql[256];
        snprintf(delete_areas_sql, sizeof(delete_areas_sql),
                 "DELETE FROM service_areas WHERE building_id = '%s'", building_id);

        if (!execute_update(db, delete_areas_sql))
        {
            printf("删除楼宇关联的服务区域失败。\n");
            return false;
        }
    }
    if (check_areas_result.row_count > 0)
        free_query_result(&check_areas_result);

    char sql[256];
    snprintf(sql, sizeof(sql),
             "DELETE FROM buildings WHERE building_id = '%s'",
             building_id);

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

/**
 * 获取楼宇信息
 *
 * 根据楼宇ID获取楼宇的详细信息
 *
 * @param db 数据库连接
 * @param building_id 要查询的楼宇ID
 * @param building 用于存储查询结果的楼宇结构体
 * @return 查询成功返回true，失败返回false
 */
bool get_building(Database *db, const char *building_id, Building *building)
{
    char sql[256];
    snprintf(sql, sizeof(sql),
             "SELECT building_id, building_name, address, floors_count "
             "FROM buildings WHERE building_id = '%s'",
             building_id);

    QueryResult result;
    if (!execute_query(db, sql, &result) || result.row_count == 0)
    {
        if (result.row_count > 0)
            free_query_result(&result);
        printf("查询楼宇信息失败或楼宇不存在。\n");
        return false;
    }

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

/**
 * 获取所有楼宇列表
 *
 * 查询系统中所有楼宇的基本信息
 *
 * @param db 数据库连接
 * @param user_id 用户ID
 * @param user_type 用户类型
 * @param result 查询结果结构体
 * @return 查询成功返回true，失败返回false
 */
bool list_buildings(Database *db, const char *user_id, UserType user_type, QueryResult *result)
{
    char sql[256];
    snprintf(sql, sizeof(sql),
             "SELECT building_id, building_name, address, floors_count "
             "FROM buildings ORDER BY building_name");

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

/**
 * 分配服务人员到楼宇
 *
 * 创建服务人员与楼宇之间的关联关系，需要管理员权限
 *
 * @param db 数据库连接
 * @param user_id 用户ID
 * @param user_type 用户类型
 * @param staff_id 服务人员ID
 * @param building_id 楼宇ID
 * @return 分配成功返回true，失败返回false
 */
bool assign_staff_to_building(Database *db, const char *admin_user_id, UserType admin_type, const char *staff_user_id, const char *building_id)
{
    // 验证权限
    if (admin_type != USER_ADMIN)
    {
        printf("权限不足，无法分配服务人员。\n");
        return false;
    }

    char sql[512];
    QueryResult result;
    char staff_id[41] = {0};
    char staff_type_id[41] = {0}; // 添加这行，声明staff_type_id变量

    // 检查该用户是否已经在staff表中有记录
    snprintf(sql, sizeof(sql),
             "SELECT staff_id FROM staff WHERE user_id = '%s'",
             staff_user_id);

    if (!execute_query(db, sql, &result))
    {
        printf("查询staff表失败。\n");
        return false;
    }

    // 如果staff表中没有该用户的记录，则创建一个
    if (result.row_count == 0)
    {
        free_query_result(&result);

        // 生成一个新的staff_id
        char new_staff_id[41];
        generate_uuid(new_staff_id);

        // 获取一个默认的staff_type_id，假设有一个通用的服务人员类型
        snprintf(sql, sizeof(sql),
                 "SELECT staff_type_id FROM staff_types LIMIT 1");

        if (!execute_query(db, sql, &result))
        {
            printf("查询staff_types表失败。\n");

            // 如果没有查到任何staff_type，尝试创建一个默认的
            snprintf(sql, sizeof(sql),
                     "INSERT INTO staff_types (staff_type_id, type_name, description) "
                     "VALUES ('default_type', '普通服务人员', '默认服务人员类型')");

            if (!execute_update(db, sql))
            {
                printf("创建默认staff_type失败。\n");
                return false;
            }

            strcpy(staff_type_id, "default_type");
        }
        else
        {
            if (result.row_count > 0)
            {
                strcpy(staff_type_id, result.rows[0].values[0]);
                free_query_result(&result);
            }
            else
            {
                free_query_result(&result);
                // 如果没有任何staff_type，创建一个默认的
                char default_type_id[41];
                generate_uuid(default_type_id);

                snprintf(sql, sizeof(sql),
                         "INSERT INTO staff_types (staff_type_id, type_name, description) "
                         "VALUES ('%s', '普通服务人员', '默认服务人员类型')",
                         default_type_id);

                if (!execute_update(db, sql))
                {
                    printf("创建默认staff_type失败。\n");
                    return false;
                }

                strcpy(staff_type_id, default_type_id);
            }
        }

        // 将用户添加到staff表
        snprintf(sql, sizeof(sql),
                 "INSERT INTO staff (staff_id, user_id, staff_type_id, hire_date, status) "
                 "VALUES ('%s', '%s', '%s', datetime('now'), 'active')",
                 new_staff_id, staff_user_id, staff_type_id);

        if (!execute_update(db, sql))
        {
            printf("创建staff记录失败。\n");
            return false;
        }

        // 使用新创建的staff_id
        strcpy(staff_id, new_staff_id);
    }
    else
    {
        // 使用已存在的staff_id
        strcpy(staff_id, result.rows[0].values[0]);
        free_query_result(&result);
    }

    // 检查该服务人员是否已经被分配到这个楼宇
    snprintf(sql, sizeof(sql),
             "SELECT area_id FROM service_areas "
             "WHERE staff_id = '%s' AND building_id = '%s'",
             staff_id, building_id);

    if (!execute_query(db, sql, &result))
    {
        printf("查询service_areas表失败。\n");
        return false;
    }

    if (result.row_count > 0)
    {
        printf("该服务人员已经分配到这个楼宇。\n");
        free_query_result(&result);
        return false;
    }
    free_query_result(&result);

    // 生成一个服务区域ID
    char area_id[41];
    generate_uuid(area_id);

    // 添加服务区域分配记录
    snprintf(sql, sizeof(sql),
             "INSERT INTO service_areas (area_id, staff_id, building_id, assignment_date) "
             "VALUES ('%s', '%s', '%s', datetime('now'))",
             area_id, staff_id, building_id);

    if (!execute_update(db, sql))
    {
        printf("分配服务区域失败。\n");
        return false;
    }

    return true;
}

/**
 * 取消服务人员的楼宇分配
 *
 * 删除服务人员与楼宇之间的关联关系，需要管理员权限
 *
 * @param db 数据库连接
 * @param user_id 用户ID
 * @param user_type 用户类型
 * @param staff_id 服务人员ID
 * @param building_id 楼宇ID
 * @return 取消分配成功返回true，失败返回false
 */
bool unassign_staff_from_building(Database *db, const char *user_id, UserType user_type, const char *staff_id, const char *building_id)
{
    if (!validate_permission(db, user_id, user_type, 1))
    {
        printf("权限不足，无法取消服务人员分配。\n");
        return false;
    }

    char check_sql[256];
    snprintf(check_sql, sizeof(check_sql),
             "SELECT area_id FROM service_areas WHERE staff_id = '%s' AND building_id = '%s'",
             staff_id, building_id);

    QueryResult check_result;
    if (!execute_query(db, check_sql, &check_result) || check_result.row_count == 0)
    {
        if (check_result.row_count > 0)
            free_query_result(&check_result);
        printf("该服务人员未分配到此楼宇。\n");
        return false;
    }
    free_query_result(&check_result);

    char sql[256];
    snprintf(sql, sizeof(sql),
             "DELETE FROM service_areas WHERE staff_id = '%s' AND building_id = '%s'",
             staff_id, building_id);

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
// ...existing code...

bool get_unpaid_owners_count(Database *db, int *count)
{
    char sql[256];
    snprintf(sql, sizeof(sql),
             "SELECT COUNT(DISTINCT o.user_id) "
             "FROM users o "
             "LEFT JOIN payments p ON o.user_id = p.user_id AND p.status = 'paid' "
             "WHERE o.role_id = 3 AND p.payment_id IS NULL");

    QueryResult result;
    if (!execute_query(db, sql, &result))
    {
        return false;
    }

    *count = atoi(result.rows[0].values[0]);
    free_query_result(&result);
    return true;
}

bool get_owners_count_by_payment_status(Database *db, int *paid_count, int *unpaid_count)
{
    char sql[512];
    snprintf(sql, sizeof(sql),
             "SELECT "
             "(SELECT COUNT(DISTINCT user_id) FROM payments WHERE status = 'paid') as paid_count, "
             "(SELECT COUNT(user_id) FROM users WHERE role_id = 3) - "
             "(SELECT COUNT(DISTINCT user_id) FROM payments WHERE status = 'paid') as unpaid_count");

    QueryResult result;
    if (!execute_query(db, sql, &result))
    {
        return false;
    }

    *paid_count = atoi(result.rows[0].values[0]);
    *unpaid_count = atoi(result.rows[0].values[1]);
    free_query_result(&result);
    return true;
}

bool get_unpaid_owners_by_year(Database *db, int year, QueryResult *result)
{
    char sql[512];
    snprintf(sql, sizeof(sql),
             "SELECT DISTINCT u.user_id, u.name, u.phone_number "
             "FROM users u "
             "LEFT JOIN payments p ON u.user_id = p.user_id "
             "AND strftime('%%Y', p.payment_date) = '%d' "
             "WHERE u.role_id = 3 AND p.payment_id IS NULL "
             "ORDER BY u.name",
             year);

    return execute_query(db, sql, result);
}

bool get_paid_owners_count_by_date(Database *db, const char *end_date, int *count)
{
    char sql[256];
    snprintf(sql, sizeof(sql),
             "SELECT COUNT(DISTINCT user_id) "
             "FROM payments "
             "WHERE status = 'paid' AND payment_date <= '%s'",
             end_date);

    QueryResult result;
    if (!execute_query(db, sql, &result))
    {
        return false;
    }

    *count = atoi(result.rows[0].values[0]);
    free_query_result(&result);
    return true;
}
