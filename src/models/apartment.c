/**
 * apartment.c
 * 房屋管理模块实现
 *
 * 该文件实现了系统中与房屋相关的所有功能，包括：
 * - 添加新房屋
 * - 更新房屋信息
 * - 删除房屋
 * - 获取房屋列表
 * - 获取特定楼宇的所有房屋
 * - 获取业主的所有房屋
 */

#include "models/apartment.h"
#include "auth/auth.h"
#include "utils/utils.h"
#include "db/db_query.h" // 添加缺失的头文件

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define SQL_MAX_LENGTH 4096

/**
 * 记录错误日志
 *
 * @param format 格式化字符串
 * @param ... 可变参数列表
 */
void log_error(const char *format, ...)
{
    va_list args;
    va_start(args, format);
    fprintf(stderr, "[ERROR] ");
    vfprintf(stderr, format, args);
    fprintf(stderr, "\n");
    va_end(args);
}

/**
 * 记录信息日志
 *
 * @param format 格式化字符串
 * @param ... 可变参数列表
 */
void log_info(const char *format, ...)
{
    va_list args;
    va_start(args, format);
    fprintf(stdout, "[INFO] ");
    vfprintf(stdout, format, args);
    fprintf(stdout, "\n");
    va_end(args);
}

/**
 * 添加新房屋
 *
 * 在系统中创建新的房屋记录，需要管理员或物业服务人员权限
 *
 * @param db 数据库连接
 * @param user_id 执行操作的用户ID
 * @param user_type 执行操作的用户类型
 * @param room 房屋信息结构体
 * @return 操作成功返回true，失败返回false
 */
bool add_room(Database *db, const char *user_id, UserType user_type, Room *room)
{
    if (user_type != USER_ADMIN && user_type != USER_STAFF)
    {
        log_error("用户 %s 无权添加房屋", user_id);
        return false;
    }

    char query[SQL_MAX_LENGTH];
    snprintf(query, sizeof(query),
             "SELECT building_id FROM buildings WHERE building_id='%s'",
             room->building_id);

    QueryResult result;
    if (!execute_query(db, query, &result) || result.row_count == 0)
    {
        log_error("楼宇 ID %s 不存在", room->building_id);
        free_query_result(&result);
        return false;
    }
    free_query_result(&result);

    snprintf(query, sizeof(query),
             "SELECT room_id FROM rooms WHERE building_id='%s' AND room_number='%s'",
             room->building_id, room->room_number);

    if (!execute_query(db, query, &result))
    {
        log_error("查询房间时出错");
        return false;
    }

    if (result.row_count > 0)
    {
        log_error("该房间号已存在于此楼宇");
        free_query_result(&result);
        return false;
    }
    free_query_result(&result);

    generate_uuid(room->room_id);

    snprintf(query, sizeof(query),
             "INSERT INTO rooms (room_id, building_id, room_number, floor, area_sqm, owner_id, status) "
             "VALUES ('%s', '%s', '%s', %d, %.2f, '%s', '%s')",
             room->room_id, room->building_id, room->room_number,
             room->floor, room->area_sqm,
             room->owner_id[0] ? room->owner_id : "NULL",
             room->status);

    if (!execute_update(db, query))
    {
        log_error("添加房屋失败");
        return false;
    }

    log_info("成功添加房屋：楼宇 %s, 房间号 %s", room->building_id, room->room_number);
    return true;
}

/**
 * 修改房屋信息
 *
 * 更新已有房屋的信息，需要管理员或物业服务人员权限
 *
 * @param db 数据库连接
 * @param user_id 执行操作的用户ID
 * @param user_type 执行操作的用户类型
 * @param room 更新后的房屋信息结构体
 * @return 操作成功返回true，失败返回false
 */
bool update_room(Database *db, const char *user_id, UserType user_type, Room *room)
{
    if (user_type != USER_ADMIN && user_type != USER_STAFF)
    {
        log_error("用户 %s 无权修改房屋信息", user_id);
        return false;
    }

    char query[SQL_MAX_LENGTH];
    snprintf(query, sizeof(query),
             "SELECT * FROM rooms WHERE room_id='%s'",
             room->room_id);

    QueryResult result;
    if (!execute_query(db, query, &result) || result.row_count == 0)
    {
        log_error("房间 ID %s 不存在", room->room_id);
        free_query_result(&result);
        return false;
    }
    free_query_result(&result);

    snprintf(query, sizeof(query),
             "UPDATE rooms SET "
             "building_id='%s', room_number='%s', floor=%d, "
             "area_sqm=%.2f, owner_id='%s', status='%s' "
             "WHERE room_id='%s'",
             room->building_id, room->room_number, room->floor,
             room->area_sqm, room->owner_id, room->status,
             room->room_id);

    if (!execute_update(db, query))
    {
        log_error("更新房屋信息失败");
        return false;
    }

    log_info("成功更新房屋信息：ID %s", room->room_id);
    return true;
}

/**
 * 删除房屋
 *
 * 从系统中删除房屋记录，仅限管理员使用，且房屋不能有关联的交易记录
 *
 * @param db 数据库连接
 * @param user_id 执行操作的用户ID
 * @param user_type 执行操作的用户类型
 * @param room_id 要删除的房屋ID
 * @return 操作成功返回true，失败返回false
 */
bool delete_room(Database *db, const char *user_id, UserType user_type, const char *room_id)
{
    if (user_type != USER_ADMIN)
    {
        log_error("用户 %s 无权删除房屋", user_id);
        return false;
    }

    char query[SQL_MAX_LENGTH];

    snprintf(query, sizeof(query),
             "SELECT * FROM rooms WHERE room_id='%s'", room_id);

    QueryResult result;
    if (!execute_query(db, query, &result) || result.row_count == 0)
    {
        log_error("房间 ID %s 不存在", room_id);
        free_query_result(&result);
        return false;
    }
    free_query_result(&result);

    snprintf(query, sizeof(query),
             "SELECT COUNT(*) FROM transactions WHERE room_id='%s'", room_id);

    if (!execute_query(db, query, &result))
    {
        log_error("查询关联交易记录失败");
        return false;
    }

    int transaction_count = 0;
    if (result.row_count > 0 && result.rows[0].values[0])
    {
        transaction_count = atoi(result.rows[0].values[0]);
    }
    free_query_result(&result);

    if (transaction_count > 0)
    {
        log_error("无法删除房屋，存在 %d 条关联的交易记录", transaction_count);
        return false;
    }

    snprintf(query, sizeof(query), "DELETE FROM rooms WHERE room_id='%s'", room_id);

    if (!execute_update(db, query))
    {
        log_error("删除房屋失败");
        return false;
    }

    log_info("成功删除房屋：ID %s", room_id);
    return true;
}

/**
 * 获取房屋信息
 *
 * 根据房屋ID获取详细信息
 *
 * @param db 数据库连接
 * @param room_id 房屋ID
 * @param room 用于存储房屋信息的结构体指针
 * @return 获取成功返回true，失败返回false
 */
bool get_room(Database *db, const char *room_id, Room *room)
{
    char query[SQL_MAX_LENGTH];
    snprintf(query, sizeof(query),
             "SELECT room_id, building_id, room_number, floor, area_sqm, owner_id, status "
             "FROM rooms WHERE room_id='%s'",
             room_id);

    QueryResult result;
    if (!execute_query(db, query, &result))
    {
        log_error("查询房屋信息失败");
        return false;
    }

    if (result.row_count == 0)
    {
        log_error("房间 ID %s 不存在", room_id);
        free_query_result(&result);
        return false;
    }

    QueryRow *row = &result.rows[0];
    strncpy(room->room_id, row->values[0], sizeof(room->room_id) - 1);
    strncpy(room->building_id, row->values[1], sizeof(room->building_id) - 1);
    strncpy(room->room_number, row->values[2], sizeof(room->room_number) - 1);
    room->floor = atoi(row->values[3]);
    room->area_sqm = atof(row->values[4]);
    strncpy(room->owner_id, row->values[5] ? row->values[5] : "", sizeof(room->owner_id) - 1);
    strncpy(room->status, row->values[6], sizeof(room->status) - 1);

    free_query_result(&result);
    return true;
}

/**
 * 获取某楼宇内所有房屋
 *
 * 查询并返回指定楼宇的所有房屋信息列表
 *
 * @param db 数据库连接
 * @param user_id 执行操作的用户ID
 * @param user_type 执行操作的用户类型
 * @param building_id 楼宇ID
 * @param result 查询结果存储结构体
 * @return 操作成功返回true，失败返回false
 */
bool list_rooms_by_building(Database *db, const char *user_id, UserType user_type, const char *building_id, QueryResult *result)
{
    char query[SQL_MAX_LENGTH];

    snprintf(query, sizeof(query),
             "SELECT r.room_id, r.building_id, r.room_number, r.floor, r.area_sqm, "
             "r.owner_id, u.name as owner_name, r.status "
             "FROM rooms r "
             "LEFT JOIN users u ON r.owner_id = u.user_id "
             "WHERE r.building_id='%s' "
             "ORDER BY r.floor, r.room_number",
             building_id);

    if (!execute_query(db, query, result))
    {
        log_error("查询楼宇 %s 的房屋列表失败", building_id);
        return false;
    }

    log_info("成功查询楼宇 %s 的房屋列表，共 %d 条记录", building_id, result->row_count);
    return true;
}

/**
 * 查询业主的房屋
 *
 * 获取指定业主名下的所有房屋信息，需要管理员、物业服务人员或业主本人权限
 *
 * @param db 数据库连接
 * @param user_id 执行操作的用户ID
 * @param user_type 执行操作的用户类型
 * @param owner_id 业主ID
 * @param result 查询结果存储结构体
 * @return 操作成功返回true，失败返回false
 */
bool get_owner_rooms(Database *db, const char *user_id, UserType user_type, const char *owner_id, QueryResult *result)
{
    if (user_type != USER_ADMIN && user_type != USER_STAFF &&
        strcmp(user_id, owner_id) != 0)
    {
        log_error("用户 %s 无权查询业主 %s 的房屋信息", user_id, owner_id);
        return false;
    }

    char query[SQL_MAX_LENGTH];

    snprintf(query, sizeof(query),
             "SELECT r.room_id, r.building_id, b.building_name, r.room_number, "
             "r.floor, r.area_sqm, r.status "
             "FROM rooms r "
             "JOIN buildings b ON r.building_id = b.building_id "
             "WHERE r.owner_id='%s' "
             "ORDER BY b.building_name, r.floor, r.room_number",
             owner_id);

    if (!execute_query(db, query, result))
    {
        log_error("查询业主 %s 的房屋列表失败", owner_id);
        return false;
    }

    log_info("成功查询业主 %s 的房屋列表，共 %d 条记录", owner_id, result->row_count);
    return true;
}
