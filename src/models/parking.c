#include "models/parking.h"
#include "utils/utils.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>

/**
 * @brief 添加新的停车位
 *
 * @param db 数据库连接
 * @param user_id 操作用户ID
 * @param user_type 操作用户类型
 * @param space 停车位信息结构体
 * @return bool 添加成功返回true，失败返回false
 */
bool add_parking_space(Database *db, const char *user_id, UserType user_type, ParkingSpace *space)
{
    if (!validate_permission(db, user_id, user_type, 1))
    {
        printf("权限不足，无法添加停车位\n");
        return false;
    }

    generate_uuid(space->parking_id);

    char query[512];
    QueryResult check_result;
    snprintf(query, sizeof(query),
             "SELECT parking_id FROM parking_spaces WHERE parking_number = '%s'",
             space->parking_number);

    if (!execute_query(db, query, &check_result))
    {
        printf("查询停车位失败\n");
        return false;
    }

    if (check_result.row_count > 0)
    {
        printf("停车位号已存在\n");
        free_query_result(&check_result);
        return false;
    }

    free_query_result(&check_result);

    snprintf(query, sizeof(query),
             "INSERT INTO parking_spaces (parking_id, parking_number, owner_id, status) "
             "VALUES ('%s', '%s', '%s', %d)",
             space->parking_id, space->parking_number, space->owner_id, space->status);

    if (!execute_update(db, query))
    {
        printf("添加停车位失败: %s\n", query);
        return false;
    }

    return true;
}

/**
 * @brief 修改停车位信息
 *
 * @param db 数据库连接
 * @param user_id 操作用户ID
 * @param user_type 操作用户类型
 * @param space 更新后的停车位信息
 * @return bool 修改成功返回true，失败返回false
 */
bool update_parking_space(Database *db, const char *user_id, UserType user_type, ParkingSpace *space)
{
    if (!validate_permission(db, user_id, user_type, 1))
    {
        printf("权限不足，无法修改停车位信息\n");
        return false;
    }

    ParkingSpace existing_space;
    if (!get_parking_space(db, space->parking_id, &existing_space))
    {
        printf("停车位不存在\n");
        return false;
    }

    char query[512];
    snprintf(query, sizeof(query),
             "UPDATE parking_spaces SET parking_number = '%s', owner_id = '%s', status = %d "
             "WHERE parking_id = '%s'",
             space->parking_number, space->owner_id, space->status, space->parking_id);

    if (!execute_update(db, query))
    {
        printf("更新停车位信息失败: %s\n", query);
        return false;
    }

    return true;
}

/**
 * @brief 删除停车位
 *
 * @param db 数据库连接
 * @param user_id 操作用户ID
 * @param user_type 操作用户类型
 * @param parking_id 要删除的停车位ID
 * @return bool 删除成功返回true，失败返回false
 */
bool delete_parking_space(Database *db, const char *user_id, UserType user_type, const char *parking_id)
{
    if (!validate_permission(db, user_id, user_type, 1))
    {
        printf("权限不足，无法删除停车位\n");
        return false;
    }

    ParkingSpace space;
    if (!get_parking_space(db, parking_id, &space))
    {
        printf("停车位不存在\n");
        return false;
    }

    char query[512];
    QueryResult check_result;
    snprintf(query, sizeof(query),
             "SELECT transaction_id FROM transactions WHERE parking_id = '%s' LIMIT 1",
             parking_id);

    if (!execute_query(db, query, &check_result))
    {
        printf("查询停车位交易记录失败\n");
        return false;
    }

    if (check_result.row_count > 0)
    {
        printf("该停车位存在交易记录，无法删除\n");
        free_query_result(&check_result);
        return false;
    }

    free_query_result(&check_result);

    snprintf(query, sizeof(query),
             "DELETE FROM parking_spaces WHERE parking_id = '%s'",
             parking_id);

    if (!execute_update(db, query))
    {
        printf("删除停车位失败: %s\n", query);
        return false;
    }

    return true;
}

/**
 * @brief 获取停车位信息
 *
 * @param db 数据库连接
 * @param parking_id 停车位ID
 * @param space 用于存储停车位信息的结构体指针
 * @return bool 获取成功返回true，失败返回false
 */
bool get_parking_space(Database *db, const char *parking_id, ParkingSpace *space)
{
    char query[512];
    QueryResult result;

    snprintf(query, sizeof(query),
             "SELECT parking_id, parking_number, owner_id, status "
             "FROM parking_spaces WHERE parking_id = '%s'",
             parking_id);

    if (!execute_query(db, query, &result))
    {
        printf("查询停车位信息失败: %s\n", query);
        return false;
    }

    if (result.row_count == 0)
    {
        printf("停车位不存在\n");
        free_query_result(&result);
        return false;
    }

    strcpy(space->parking_id, result.rows[0].values[0]);
    strcpy(space->parking_number, result.rows[0].values[1]);
    strcpy(space->owner_id, result.rows[0].values[2] ? result.rows[0].values[2] : "");
    space->status = result.rows[0].values[3] ? atoi(result.rows[0].values[3]) : 0;

    free_query_result(&result);
    return true;
}

/**
 * @brief 获取所有停车位列表
 *
 * @param db 数据库连接
 * @param user_id 操作用户ID
 * @param user_type 操作用户类型
 * @param result 用于存储查询结果的结构体指针
 * @return bool 查询成功返回true，失败返回false
 */
bool list_parking_spaces(Database *db, const char *user_id, UserType user_type, QueryResult *result)
{
    if (!validate_permission(db, user_id, user_type, 1))
    {
        printf("权限不足，无法查看所有停车位\n");
        return false;
    }

    char query[512];
    snprintf(query, sizeof(query),
             "SELECT p.parking_id, p.parking_number, p.owner_id, p.status, "
             "u.name as owner_name "
             "FROM parking_spaces p "
             "LEFT JOIN users u ON p.owner_id = u.user_id "
             "ORDER BY p.parking_number");

    if (!execute_query(db, query, result))
    {
        printf("查询所有停车位失败: %s\n", query);
        return false;
    }

    return true;
}

/**
 * @brief 获取特定业主的停车位列表
 *
 * @param db 数据库连接
 * @param user_id 操作用户ID
 * @param user_type 操作用户类型
 * @param owner_id 业主ID
 * @param result 用于存储查询结果的结构体指针
 * @return bool 查询成功返回true，失败返回false
 */
bool get_owner_parking_spaces(Database *db, const char *user_id, UserType user_type, const char *owner_id, QueryResult *result)
{
    if (user_type == USER_OWNER && strcmp(user_id, owner_id) != 0)
    {
        printf("业主只能查看自己的停车位\n");
        return false;
    }
    else if (user_type != USER_OWNER && !validate_permission(db, user_id, user_type, 1))
    {
        printf("权限不足，无法查看业主停车位\n");
        return false;
    }

    char query[512];
    snprintf(query, sizeof(query),
             "SELECT p.parking_id, p.parking_number, p.owner_id, p.status "
             "FROM parking_spaces p "
             "WHERE p.owner_id = '%s' "
             "ORDER BY p.parking_number",
             owner_id);

    if (!execute_query(db, query, result))
    {
        printf("查询业主停车位失败: %s\n", query);
        return false;
    }

    return true;
}
