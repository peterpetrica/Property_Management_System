/**
 * models/service.c
 * 物业管理系统 - 服务管理模块实现
 *
 * 本文件实现了物业管理系统中与服务相关的功能，包括：
 * - 服务类型管理：添加、修改、删除和查询服务类型
 * - 服务记录管理：创建服务记录、查询服务记录
 *
 * 服务记录可以按照员工ID或房屋ID进行查询，支持物业管理人员
 * 对各类服务进行全面管理和统计分析。
 */
#include "models/service.h"
#include "auth/auth.h"
#include "utils/utils.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

/**
 * @brief 添加服务类型
 *
 * @param db 数据库连接
 * @param user_id 用户ID
 * @param user_type 用户类型
 * @param type 服务类型结构体指针
 * @return bool 操作成功返回true，失败返回false
 */
bool add_service_type(Database *db, const char *user_id, UserType user_type, ServiceType *type)
{
    if (user_type != USER_ADMIN && user_type != USER_STAFF)
    {
        printf("权限不足，无法添加服务类型");
        return false;
    }

    if (strlen(type->id) == 0)
    {
        generate_uuid(type->id);
    }

    char query[512];
    snprintf(query, sizeof(query),
             "INSERT INTO service_types (id, name, description) VALUES ('%s', '%s', '%s')",
             type->id, type->name, type->description);

    if (!execute_update(db, query))
    {
        printf("添加服务类型失败");
        return false;
    }

    return true;
}

/**
 * @brief 修改服务类型
 *
 * @param db 数据库连接
 * @param user_id 用户ID
 * @param user_type 用户类型
 * @param type 更新后的服务类型结构体指针
 * @return bool 操作成功返回true，失败返回false
 */
bool update_service_type(Database *db, const char *user_id, UserType user_type, ServiceType *type)
{
    if (user_type != USER_ADMIN && user_type != USER_STAFF)
    {
        printf("权限不足，无法修改服务类型");
        return false;
    }

    char check_query[256];
    snprintf(check_query, sizeof(check_query),
             "SELECT id FROM service_types WHERE id = '%s'", type->id);

    QueryResult check_result;
    if (!execute_query(db, check_query, &check_result) || check_result.row_count == 0)
    {
        printf("服务类型不存在");
        if (check_result.row_count > 0)
        {
            free_query_result(&check_result);
        }
        return false;
    }
    free_query_result(&check_result);

    char query[512];
    snprintf(query, sizeof(query),
             "UPDATE service_types SET name = '%s', description = '%s' WHERE id = '%s'",
             type->name, type->description, type->id);

    if (!execute_update(db, query))
    {
        printf("更新服务类型失败");
        return false;
    }

    return true;
}

/**
 * @brief 删除服务类型
 *
 * @param db 数据库连接
 * @param user_id 用户ID
 * @param user_type 用户类型
 * @param type_id 要删除的服务类型ID
 * @return bool 操作成功返回true，失败返回false
 */
bool delete_service_type(Database *db, const char *user_id, UserType user_type, const char *type_id)
{
    if (user_type != USER_ADMIN)
    {
        printf("权限不足，只有管理员可以删除服务类型");
        return false;
    }

    char check_query[256];
    snprintf(check_query, sizeof(check_query),
             "SELECT id FROM service_types WHERE id = '%s'", type_id);

    QueryResult check_result;
    if (!execute_query(db, check_query, &check_result) || check_result.row_count == 0)
    {
        printf("服务类型不存在");
        if (check_result.row_count > 0)
        {
            free_query_result(&check_result);
        }
        return false;
    }
    free_query_result(&check_result);

    char ref_query[256];
    snprintf(ref_query, sizeof(ref_query),
             "SELECT COUNT(*) FROM service_records WHERE service_type = '%s'", type_id);

    QueryResult ref_result;
    if (execute_query(db, ref_query, &ref_result) && ref_result.row_count > 0)
    {
        int count = atoi(ref_result.rows[0].values[0]);
        free_query_result(&ref_result);

        if (count > 0)
        {
            printf("无法删除服务类型，存在关联的服务记录");
            return false;
        }
    }
    else
    {
        if (ref_result.row_count > 0)
        {
            free_query_result(&ref_result);
        }
        printf("检查服务类型引用失败");
        return false;
    }

    char query[256];
    snprintf(query, sizeof(query), "DELETE FROM service_types WHERE id = '%s'", type_id);

    if (!execute_update(db, query))
    {
        printf("删除服务类型失败");
        return false;
    }

    return true;
}

/**
 * @brief 获取所有服务类型列表
 *
 * @param db 数据库连接
 * @param result 查询结果
 * @return bool 操作成功返回true，失败返回false
 */
bool list_service_types(Database *db, QueryResult *result)
{
    const char *query = "SELECT id, name, description FROM service_types ORDER BY name";

    if (!execute_query(db, query, result))
    {
        printf("获取服务类型列表失败");
        return false;
    }

    return true;
}

/**
 * @brief 添加服务人员类型
 *
 * @param db 数据库连接
 * @param user_id 用户ID
 * @param user_type 用户类型
 * @param type 服务人员类型结构体指针
 * @return bool 操作成功返回true，失败返回false
 */
bool add_staff_type(Database *db, const char *user_id, UserType user_type, StaffType *type)
{
    if (user_type != USER_ADMIN)
    {
        printf("权限不足，无法添加服务人员类型");
        return false;
    }

    if (strlen(type->staff_type_id) == 0)
    {
        generate_uuid(type->staff_type_id);
    }

    char query[512];
    snprintf(query, sizeof(query),
             "INSERT INTO staff_types (staff_type_id, type_name, description) VALUES ('%s', '%s', '%s')",
             type->staff_type_id, type->type_name, type->description);

    if (!execute_update(db, query))
    {
        printf("添加服务人员类型失败");
        return false;
    }

    return true;
}

/**
 * @brief 修改服务人员类型
 *
 * @param db 数据库连接
 * @param user_id 用户ID
 * @param user_type 用户类型
 * @param type 更新后的服务人员类型结构体指针
 * @return bool 操作成功返回true，失败返回false
 */
bool update_staff_type(Database *db, const char *user_id, UserType user_type, StaffType *type)
{
    if (user_type != USER_ADMIN)
    {
        printf("权限不足，无法修改服务人员类型");
        return false;
    }

    char check_query[256];
    snprintf(check_query, sizeof(check_query),
             "SELECT staff_type_id FROM staff_types WHERE staff_type_id = '%s'", type->staff_type_id);

    QueryResult check_result;
    if (!execute_query(db, check_query, &check_result) || check_result.row_count == 0)
    {
        printf("服务人员类型不存在");
        if (check_result.row_count > 0)
        {
            free_query_result(&check_result);
        }
        return false;
    }
    free_query_result(&check_result);

    char query[512];
    snprintf(query, sizeof(query),
             "UPDATE staff_types SET type_name = '%s', description = '%s' WHERE staff_type_id = '%s'",
             type->type_name, type->description, type->staff_type_id);

    if (!execute_update(db, query))
    {
        printf("更新服务人员类型失败");
        return false;
    }

    return true;
}

/**
 * @brief 删除服务人员类型
 *
 * @param db 数据库连接
 * @param user_id 用户ID
 * @param user_type 用户类型
 * @param staff_type_id 要删除的服务人员类型ID
 * @return bool 操作成功返回true，失败返回false
 */
bool delete_staff_type(Database *db, const char *user_id, UserType user_type, const char *staff_type_id)
{
    if (user_type != USER_ADMIN)
    {
        printf("权限不足，无法删除服务人员类型");
        return false;
    }

    char check_query[256];
    snprintf(check_query, sizeof(check_query),
             "SELECT staff_type_id FROM staff_types WHERE staff_type_id = '%s'", staff_type_id);

    QueryResult check_result;
    if (!execute_query(db, check_query, &check_result) || check_result.row_count == 0)
    {
        printf("服务人员类型不存在");
        if (check_result.row_count > 0)
        {
            free_query_result(&check_result);
        }
        return false;
    }
    free_query_result(&check_result);

    char ref_query[256];
    snprintf(ref_query, sizeof(ref_query),
             "SELECT COUNT(*) FROM staff WHERE staff_type_id = '%s'", staff_type_id);

    QueryResult ref_result;
    if (execute_query(db, ref_query, &ref_result) && ref_result.row_count > 0)
    {
        int count = atoi(ref_result.rows[0].values[0]);
        free_query_result(&ref_result);

        if (count > 0)
        {
            printf("无法删除服务人员类型，存在关联的服务人员");
            return false;
        }
    }
    else
    {
        if (ref_result.row_count > 0)
        {
            free_query_result(&ref_result);
        }
        printf("检查服务人员类型引用失败");
        return false;
    }

    char query[256];
    snprintf(query, sizeof(query), "DELETE FROM staff_types WHERE staff_type_id = '%s'", staff_type_id);

    if (!execute_update(db, query))
    {
        printf("删除服务人员类型失败");
        return false;
    }

    return true;
}

/**
 * @brief 获取所有服务人员类型列表
 *
 * @param db 数据库连接
 * @param result 查询结果
 * @return bool 操作成功返回true，失败返回false
 */
bool list_staff_types(Database *db, QueryResult *result)
{
    const char *query = "SELECT staff_type_id, type_name, description FROM staff_types ORDER BY type_name";

    if (!execute_query(db, query, result))
    {
        printf("获取服务人员类型列表失败");
        return false;
    }

    return true;
}

/**
 * @brief 分配服务区域
 *
 * @param db 数据库连接
 * @param user_id 用户ID
 * @param user_type 用户类型
 * @param area 服务区域结构体指针
 * @return bool 操作成功返回true，失败返回false
 */
bool assign_service_area(Database *db, const char *user_id, UserType user_type, ServiceArea *area)
{
    if (user_type != USER_ADMIN)
    {
        printf("权限不足，无法分配服务区域");
        return false;
    }

    if (strlen(area->area_id) == 0)
    {
        generate_uuid(area->area_id);
    }

    char check_staff_query[256];
    snprintf(check_staff_query, sizeof(check_staff_query),
             "SELECT staff_id FROM staff WHERE staff_id = '%s'", area->staff_id);

    QueryResult check_staff_result;
    if (!execute_query(db, check_staff_query, &check_staff_result) || check_staff_result.row_count == 0)
    {
        printf("服务人员不存在");
        if (check_staff_result.row_count > 0)
        {
            free_query_result(&check_staff_result);
        }
        return false;
    }
    free_query_result(&check_staff_result);

    char check_building_query[256];
    snprintf(check_building_query, sizeof(check_building_query),
             "SELECT building_id FROM buildings WHERE building_id = '%s'", area->building_id);

    QueryResult check_building_result;
    if (!execute_query(db, check_building_query, &check_building_result) || check_building_result.row_count == 0)
    {
        printf("楼宇不存在");
        if (check_building_result.row_count > 0)
        {
            free_query_result(&check_building_result);
        }
        return false;
    }
    free_query_result(&check_building_result);

    char check_assign_query[256];
    snprintf(check_assign_query, sizeof(check_assign_query),
             "SELECT area_id FROM service_areas WHERE staff_id = '%s' AND building_id = '%s'",
             area->staff_id, area->building_id);

    QueryResult check_assign_result;
    if (execute_query(db, check_assign_query, &check_assign_result))
    {
        if (check_assign_result.row_count > 0)
        {
            free_query_result(&check_assign_result);
            printf("该服务人员已分配到此楼宇");
            return false;
        }
    }
    free_query_result(&check_assign_result);

    char query[512];
    char date_str[20];
    snprintf(date_str, sizeof(date_str), "%ld", (long)area->assignment_date);

    snprintf(query, sizeof(query),
             "INSERT INTO service_areas (area_id, staff_id, building_id, assignment_date) VALUES ('%s', '%s', '%s', '%s')",
             area->area_id, area->staff_id, area->building_id, date_str);

    if (!execute_update(db, query))
    {
        printf("分配服务区域失败");
        return false;
    }

    return true;
}

/**
 * @brief 取消服务区域分配
 *
 * @param db 数据库连接
 * @param user_id 用户ID
 * @param user_type 用户类型
 * @param area_id 要取消的服务区域ID
 * @return bool 操作成功返回true，失败返回false
 */
bool unassign_service_area(Database *db, const char *user_id, UserType user_type, const char *area_id)
{
    if (user_type != USER_ADMIN)
    {
        printf("权限不足，无法取消服务区域分配");
        return false;
    }

    char check_query[256];
    snprintf(check_query, sizeof(check_query),
             "SELECT area_id FROM service_areas WHERE area_id = '%s'", area_id);

    QueryResult check_result;
    if (!execute_query(db, check_query, &check_result) || check_result.row_count == 0)
    {
        printf("服务区域不存在");
        if (check_result.row_count > 0)
        {
            free_query_result(&check_result);
        }
        return false;
    }
    free_query_result(&check_result);

    char query[256];
    snprintf(query, sizeof(query), "DELETE FROM service_areas WHERE area_id = '%s'", area_id);

    if (!execute_update(db, query))
    {
        printf("取消服务区域分配失败");
        return false;
    }

    return true;
}

/**
 * @brief 获取服务人员的服务区域列表
 *
 * @param db 数据库连接
 * @param user_id 用户ID
 * @param user_type 用户类型
 * @param staff_id 服务人员ID
 * @param result 查询结果
 * @return bool 操作成功返回true，失败返回false
 */
bool get_staff_service_areas(Database *db, const char *user_id, UserType user_type, const char *staff_id, QueryResult *result)
{
    char query[512];
    snprintf(query, sizeof(query),
             "SELECT sa.area_id, sa.staff_id, sa.building_id, b.building_name, sa.assignment_date "
             "FROM service_areas sa "
             "JOIN buildings b ON sa.building_id = b.building_id "
             "WHERE sa.staff_id = '%s' "
             "ORDER BY b.building_name",
             staff_id);

    if (!execute_query(db, query, result))
    {
        printf("获取服务人员服务区域失败");
        return false;
    }

    return true;
}

/**
 * @brief 记录服务
 *
 * @param db 数据库连接
 * @param user_id 用户ID
 * @param user_type 用户类型
 * @param record 服务记录结构体指针
 * @return bool 操作成功返回true，失败返回false
 */
bool record_service(Database *db, const char *user_id, UserType user_type, ServiceRecord *record)
{
    if (user_type != USER_ADMIN && user_type != USER_STAFF)
    {
        printf("权限不足，无法记录服务");
        return false;
    }

    if (strlen(record->record_id) == 0)
    {
        generate_uuid(record->record_id);
    }

    char check_staff_query[256];
    snprintf(check_staff_query, sizeof(check_staff_query),
             "SELECT staff_id FROM staff WHERE staff_id = '%s'", record->staff_id);

    QueryResult check_staff_result;
    if (!execute_query(db, check_staff_query, &check_staff_result) || check_staff_result.row_count == 0)
    {
        printf("服务人员不存在");
        if (check_staff_result.row_count > 0)
        {
            free_query_result(&check_staff_result);
        }
        return false;
    }
    free_query_result(&check_staff_result);

    char query[1024];
    char date_str[20];
    snprintf(date_str, sizeof(date_str), "%ld", (long)record->service_date);

    snprintf(query, sizeof(query),
             "INSERT INTO service_records (record_id, staff_id, service_type, service_date, description, status, target_id) "
             "VALUES ('%s', '%s', '%s', '%s', '%s', %d, '%s')",
             record->record_id, record->staff_id, record->service_type, date_str,
             record->description, record->status, record->target_id);

    if (!execute_update(db, query))
    {
        printf("记录服务失败");
        return false;
    }

    return true;
}

/**
 * @brief 获取服务人员的服务记录
 *
 * @param db 数据库连接
 * @param user_id 用户ID
 * @param user_type 用户类型
 * @param staff_id 服务人员ID
 * @param result 查询结果
 * @return bool 操作成功返回true，失败返回false
 */
bool get_service_records_by_staff(Database *db, const char *user_id, UserType user_type, const char *staff_id, QueryResult *result)
{
    char check_query[256];
    snprintf(check_query, sizeof(check_query),
             "SELECT staff_id FROM staff WHERE staff_id = '%s'", staff_id);

    QueryResult check_result;
    if (!execute_query(db, check_query, &check_result) || check_result.row_count == 0)
    {
        printf("服务人员不存在");
        if (check_result.row_count > 0)
        {
            free_query_result(&check_result);
        }
        return false;
    }
    free_query_result(&check_result);

    char query[512];
    snprintf(query, sizeof(query),
             "SELECT sr.record_id, sr.staff_id, u.name as staff_name, sr.service_type, "
             "sr.service_date, sr.description, sr.status, sr.target_id "
             "FROM service_records sr "
             "JOIN staff s ON sr.staff_id = s.staff_id "
             "JOIN users u ON s.user_id = u.user_id "
             "WHERE sr.staff_id = '%s' "
             "ORDER BY sr.service_date DESC",
             staff_id);

    if (!execute_query(db, query, result))
    {
        printf("获取服务人员服务记录失败");
        return false;
    }

    return true;
}

/**
 * @brief 获取房屋的服务记录
 *
 * @param db 数据库连接
 * @param user_id 用户ID
 * @param user_type 用户类型
 * @param room_id 房屋ID
 * @param result 查询结果
 * @return bool 操作成功返回true，失败返回false
 */
bool get_service_records_by_room(Database *db, const char *user_id, UserType user_type, const char *room_id, QueryResult *result)
{
    char check_query[256];
    snprintf(check_query, sizeof(check_query),
             "SELECT room_id FROM rooms WHERE room_id = '%s'", room_id);

    QueryResult check_result;
    if (!execute_query(db, check_query, &check_result) || check_result.row_count == 0)
    {
        printf("房屋不存在");
        if (check_result.row_count > 0)
        {
            free_query_result(&check_result);
        }
        return false;
    }
    free_query_result(&check_result);

    if (user_type == USER_OWNER)
    {
        char owner_check[256];
        snprintf(owner_check, sizeof(owner_check),
                 "SELECT owner_id FROM rooms WHERE room_id = '%s' AND owner_id = '%s'",
                 room_id, user_id);

        QueryResult owner_result;
        if (!execute_query(db, owner_check, &owner_result) || owner_result.row_count == 0)
        {
            printf("权限不足，无法查看非自有房屋的服务记录");
            if (owner_result.row_count > 0)
            {
                free_query_result(&owner_result);
            }
            return false;
        }
        free_query_result(&owner_result);
    }

    char query[512];
    snprintf(query, sizeof(query),
             "SELECT sr.record_id, sr.staff_id, u.name as staff_name, sr.service_type, "
             "sr.service_date, sr.description, sr.status "
             "FROM service_records sr "
             "JOIN staff s ON sr.staff_id = s.staff_id "
             "JOIN users u ON s.user_id = u.user_id "
             "WHERE sr.target_id = '%s' "
             "ORDER BY sr.service_date DESC",
             room_id);

    if (!execute_query(db, query, result))
    {
        printf("获取房屋服务记录失败");
        return false;
    }

    return true;
}

/**
 * @brief 获取楼宇的服务记录
 *
 * @param db 数据库连接
 * @param user_id 用户ID
 * @param user_type 用户类型
 * @param building_id 楼宇ID
 * @param result 查询结果
 * @return bool 操作成功返回true，失败返回false
 */
bool get_service_records_by_building(Database *db, const char *user_id, UserType user_type, const char *building_id, QueryResult *result)
{
    char check_query[256];
    snprintf(check_query, sizeof(check_query),
             "SELECT building_id FROM buildings WHERE building_id = '%s'", building_id);

    QueryResult check_result;
    if (!execute_query(db, check_query, &check_result) || check_result.row_count == 0)
    {
        printf("楼宇不存在");
        if (check_result.row_count > 0)
        {
            free_query_result(&check_result);
        }
        return false;
    }
    free_query_result(&check_result);

    char query[512];
    snprintf(query, sizeof(query),
             "SELECT sr.record_id, sr.staff_id, u.name as staff_name, sr.service_type, "
             "sr.service_date, sr.description, sr.status, sr.target_id "
             "FROM service_records sr "
             "JOIN staff s ON sr.staff_id = s.staff_id "
             "JOIN users u ON s.user_id = u.user_id "
             "WHERE sr.target_id = '%s' "
             "ORDER BY sr.service_date DESC",
             building_id);

    if (!execute_query(db, query, result))
    {
        printf("获取楼宇服务记录失败");
        return false;
    }

    return true;
}
