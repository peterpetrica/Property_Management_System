/**
 * transaction.c
 * 交易记录和费用标准管理模块
 *
 * 该模块实现物业管理系统中与财务交易相关的功能，包括：
 * 1. 费用标准的添加和查询
 * 2. 交易记录的添加和查询
 * 3. 支持按业主查询交易历史
 *
 * 预期实现：
 * - 添加费用标准：管理员可设置不同类型的费用标准
 * - 获取当前费用标准：根据交易类型获取当前有效的费用标准
 * - 添加交易记录：记录业主的缴费信息
 * - 查询业主交易记录：获取特定业主的所有交易历史
 */
#include "models/transaction.h"
#include "auth/auth.h"
#include "utils/utils.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

/**
 * 生成交易ID
 *
 * 创建一个唯一的UUID作为交易记录的标识符
 *
 * @return 返回生成的交易ID字符串
 */
char *generate_transaction_id()
{
    static char transaction_id[40];
    generate_uuid(transaction_id);
    return transaction_id;
}

/**
 * 添加费用标准
 *
 * 向系统中添加新的费用标准，仅管理员可执行此操作
 *
 * @param db 数据库连接指针
 * @param user_id 执行操作的用户ID
 * @param user_type 用户类型，必须为管理员
 * @param standard 费用标准结构体指针，包含要添加的标准详情
 * @return 成功返回true，失败返回false
 */
bool add_fee_standard(Database *db, const char *user_id, UserType user_type, FeeStandard *standard)
{
    if (user_type != USER_ADMIN)
    {
        printf("权限不足：只有管理员可以添加费用标准");
        return false;
    }

    if (strlen(standard->standard_id) == 0)
    {
        generate_uuid(standard->standard_id);
    }

    char query[1024];
    snprintf(query, sizeof(query),
             "INSERT INTO fee_standards (standard_id, fee_type, price_per_unit, unit, effective_date, end_date) "
             "VALUES ('%s', %d, %.2f, '%s', %ld, %ld)",
             standard->standard_id, standard->fee_type, standard->price_per_unit,
             standard->unit, (long)standard->effective_date, (long)standard->end_date);

    if (!execute_query(db, query, NULL))
    {
        printf("添加费用标准失败: %s", query);
        return false;
    }

    printf("成功添加费用标准 ID: %s, 类型: %d", standard->standard_id, standard->fee_type);
    return true;
}

/**
 * 获取当前费用标准
 *
 * 根据费用类型获取当前生效的费用标准
 *
 * @param db 数据库连接指针
 * @param fee_type 要查询的费用类型
 * @param standard 输出参数，用于存储查询到的费用标准
 * @return 成功返回true，失败返回false
 */
bool get_current_fee_standard(Database *db, int fee_type, FeeStandard *standard)
{
    time_t current_time = time(NULL);
    char query[512];
    QueryResult result;

    snprintf(query, sizeof(query),
             "SELECT standard_id, fee_type, price_per_unit, unit, effective_date, end_date "
             "FROM fee_standards "
             "WHERE fee_type = %d AND effective_date <= %ld "
             "AND (end_date = 0 OR end_date >= %ld) "
             "ORDER BY effective_date DESC LIMIT 1",
             fee_type, (long)current_time, (long)current_time);

    if (!execute_query(db, query, &result))
    {
        printf("查询费用标准失败");
        return false;
    }

    if (result.row_count == 0)
    {
        printf("未找到费用类型 %d 的当前费用标准", fee_type);
        free_query_result(&result);
        return false;
    }

    strncpy(standard->standard_id, result.rows[0].values[0], sizeof(standard->standard_id) - 1);
    standard->fee_type = atoi(result.rows[0].values[1]);
    standard->price_per_unit = atof(result.rows[0].values[2]);
    strncpy(standard->unit, result.rows[0].values[3], sizeof(standard->unit) - 1);
    standard->effective_date = (time_t)atol(result.rows[0].values[4]);
    standard->end_date = (time_t)atol(result.rows[0].values[5]);

    free_query_result(&result);
    return true;
}

/**
 * 添加交易记录
 *
 * 向系统中添加新的交易记录
 *
 * @param db 数据库连接指针
 * @param user_id 执行操作的用户ID
 * @param user_type 用户类型
 * @param transaction 交易记录结构体指针，包含要添加的交易详情
 * @return 成功返回true，失败返回false
 */
bool add_transaction(Database *db, const char *user_id, UserType user_type, Transaction *transaction)
{
    if (strlen(transaction->transaction_id) == 0)
    {
        generate_uuid(transaction->transaction_id);
    }

    if (transaction->amount <= 0)
    {
        printf("交易金额必须大于0");
        return false;
    }

    if (transaction->fee_type == TRANS_PROPERTY_FEE && strlen(transaction->room_id) == 0)
    {
        printf("物业费交易必须指定房屋ID");
        return false;
    }

    if (transaction->fee_type == TRANS_PARKING_FEE && strlen(transaction->parking_id) == 0)
    {
        printf("停车费交易必须指定停车位ID");
        return false;
    }

    char query[1024];
    snprintf(query, sizeof(query),
             "INSERT INTO transactions (transaction_id, user_id, room_id, parking_id, fee_type, "
             "amount, payment_date, due_date, payment_method, status, period_start, period_end) "
             "VALUES ('%s', '%s', '%s', '%s', %d, %.2f, %ld, %ld, %d, %d, %ld, %ld)",
             transaction->transaction_id, transaction->user_id,
             transaction->room_id, transaction->parking_id,
             transaction->fee_type, transaction->amount,
             (long)transaction->payment_date, (long)transaction->due_date,
             transaction->payment_method, transaction->status,
             (long)transaction->period_start, (long)transaction->period_end);

    if (!execute_query(db, query, NULL))
    {
        printf("添加交易记录失败: %s", query);
        return false;
    }

    printf("成功添加交易记录 ID: %s, 用户: %s, 金额: %.2f",
           transaction->transaction_id, transaction->user_id, transaction->amount);
    return true;
}

/**
 * 获取业主交易记录
 *
 * 查询特定业主的所有交易历史记录
 *
 * @param db 数据库连接指针
 * @param user_id 执行查询的用户ID
 * @param user_type 用户类型
 * @param owner_id 要查询的业主ID
 * @param result 输出参数，存储查询结果
 * @return 成功返回true，失败返回false
 */
bool get_owner_transactions(Database *db, const char *user_id, UserType user_type, const char *owner_id, QueryResult *result)
{
    if (user_type == USER_OWNER && strcmp(user_id, owner_id) != 0)
    {
        return false;
    }

    char query[512];
    snprintf(query, sizeof(query),
             "SELECT t.transaction_id, t.user_id, t.room_id, t.parking_id, t.fee_type, "
             "t.amount, t.payment_date, t.due_date, t.payment_method, t.status, "
             "t.period_start, t.period_end "
             "FROM transactions t "
             "WHERE t.user_id = '%s' "
             "ORDER BY t.payment_date DESC",
             owner_id);

    if (!execute_query(db, query, result))
    {
        printf("查询业主交易记录失败: %s", query);
        return false;
    }

    return true;
}

/**
 * 获取房屋交易记录
 *
 * 查询特定房屋的所有交易历史记录，包含权限验证
 *
 * @param db 数据库连接指针
 * @param user_id 执行查询的用户ID
 * @param user_type 用户类型
 * @param room_id 要查询的房屋ID
 * @param result 输出参数，存储查询结果
 * @return 成功返回true，失败返回false
 */
bool get_room_transactions(Database *db, const char *user_id, UserType user_type, const char *room_id, QueryResult *result)
{
    if (user_type == USER_OWNER)
    {
        char query[512];
        QueryResult owner_check;

        snprintf(query, sizeof(query),
                 "SELECT owner_id FROM rooms WHERE room_id = '%s'",
                 room_id);

        if (!execute_query(db, query, &owner_check))
        {
            printf("查询房屋所有者失败");
            return false;
        }

        bool is_owner = false;
        if (owner_check.row_count > 0 && owner_check.rows[0].values[0] != NULL)
        {
            if (strcmp(owner_check.rows[0].values[0], user_id) == 0)
            {
                is_owner = true;
            }
        }

        free_query_result(&owner_check);

        if (!is_owner)
        {
            printf("该用户不是房屋 %s 的所有者", room_id);
            return false;
        }
    }
    else if (!validate_permission(db, user_id, user_type, 1))
    {
        return false;
    }

    char query[512];
    snprintf(query, sizeof(query),
             "SELECT t.transaction_id, t.user_id, t.room_id, t.parking_id, t.fee_type, "
             "t.amount, t.payment_date, t.due_date, t.payment_method, t.status, "
             "t.period_start, t.period_end "
             "FROM transactions t "
             "WHERE t.room_id = '%s' "
             "ORDER BY t.payment_date DESC",
             room_id);

    if (!execute_query(db, query, result))
    {
        printf("查询房屋交易记录失败: %s", query);
        return false;
    }

    return true;
}

/**
 * 获取停车位交易记录
 *
 * 查询特定停车位的所有交易历史记录，包含权限验证
 *
 * @param db 数据库连接指针
 * @param user_id 执行查询的用户ID
 * @param user_type 用户类型
 * @param parking_id 要查询的停车位ID
 * @param result 输出参数，存储查询结果
 * @return 成功返回true，失败返回false
 */
bool get_parking_transactions(Database *db, const char *user_id, UserType user_type, const char *parking_id, QueryResult *result)
{
    if (user_type == USER_OWNER)
    {
        char query[512];
        QueryResult owner_check;

        snprintf(query, sizeof(query),
                 "SELECT owner_id FROM parking_spaces WHERE parking_id = '%s'",
                 parking_id);

        if (!execute_query(db, query, &owner_check))
        {
            printf("查询停车位所有者失败");
            return false;
        }

        bool is_owner = false;
        if (owner_check.row_count > 0 && owner_check.rows[0].values[0] != NULL)
        {
            if (strcmp(owner_check.rows[0].values[0], user_id) == 0)
            {
                is_owner = true;
            }
        }

        free_query_result(&owner_check);

        if (!is_owner)
        {
            printf("该用户不是停车位 %s 的所有者", parking_id);
            return false;
        }
    }
    else if (!validate_permission(db, user_id, user_type, 1))
    {
        return false;
    }

    char query[512];
    snprintf(query, sizeof(query),
             "SELECT t.transaction_id, t.user_id, t.room_id, t.parking_id, t.fee_type, "
             "t.amount, t.payment_date, t.due_date, t.payment_method, t.status, "
             "t.period_start, t.period_end "
             "FROM transactions t "
             "WHERE t.parking_id = '%s' "
             "ORDER BY t.payment_date DESC",
             parking_id);

    if (!execute_query(db, query, result))
    {
        printf("查询停车位交易记录失败: %s", query);
        return false;
    }

    return true;
}
