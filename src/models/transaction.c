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

// 添加费用标准
bool add_fee_standard(Database *db, const char *user_id, UserType user_type, FeeStandard *standard)
{
    if (user_type != USER_ADMIN)
    {
        printf("权限不足：只有管理员可以添加费用标准");
        return false;
    }

    // 生成标准ID
    if (strlen(standard->standard_id) == 0)
    {
        generate_uuid(standard->standard_id);
    }

    // 构建插入SQL语句
    char query[1024];
    snprintf(query, sizeof(query),
             "INSERT INTO fee_standards (standard_id, fee_type, price_per_unit, unit, effective_date, end_date) "
             "VALUES ('%s', %d, %.2f, '%s', %ld, %ld)",
             standard->standard_id, standard->fee_type, standard->price_per_unit,
             standard->unit, (long)standard->effective_date, (long)standard->end_date);

    // 执行SQL语句
    if (!execute_query(db, query, NULL))
    {
        printf("添加费用标准失败: %s", query);
        return false;
    }

    printf("成功添加费用标准 ID: %s, 类型: %d", standard->standard_id, standard->fee_type);
    return true;
}

// 获取当前费用标准
bool get_current_fee_standard(Database *db, int fee_type, FeeStandard *standard)
{
    time_t current_time = time(NULL);
    char query[512];
    QueryResult result;

    // 查询当前生效的费用标准（生效日期在当前时间之前，且结束日期为0或在当前时间之后）
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

    // 将查询结果填充到standard结构体
    strncpy(standard->standard_id, result.rows[0].values[0], sizeof(standard->standard_id) - 1);
    standard->fee_type = atoi(result.rows[0].values[1]);
    standard->price_per_unit = atof(result.rows[0].values[2]);
    strncpy(standard->unit, result.rows[0].values[3], sizeof(standard->unit) - 1);
    standard->effective_date = (time_t)atol(result.rows[0].values[4]);
    standard->end_date = (time_t)atol(result.rows[0].values[5]);

    free_query_result(&result);
    return true;
}

// 添加交易记录
bool add_transaction(Database *db, const char *user_id, UserType user_type, Transaction *transaction)
{
    // 生成交易ID
    if (strlen(transaction->transaction_id) == 0)
    {
        generate_uuid(transaction->transaction_id);
    }

    // 验证交易数据
    if (transaction->amount <= 0)
    {
        printf("交易金额必须大于0");
        return false;
    }

    // 根据费用类型验证相关ID
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

    // 构建插入SQL语句
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

    // 执行SQL语句
    if (!execute_query(db, query, NULL))
    {
        printf("添加交易记录失败: %s", query);
        return false;
    }

    printf("成功添加交易记录 ID: %s, 用户: %s, 金额: %.2f",
           transaction->transaction_id, transaction->user_id, transaction->amount);
    return true;
}

// 获取业主交易记录
bool get_owner_transactions(Database *db, const char *user_id, UserType user_type, const char *owner_id, QueryResult *result)
{
    // 验证权限 - 普通用户只能查看自己的交易记录
    if (user_type == USER_OWNER && strcmp(user_id, owner_id) != 0)
    {
        return false;
    }

    // 构建查询SQL语句
    char query[512];
    snprintf(query, sizeof(query),
             "SELECT t.transaction_id, t.user_id, t.room_id, t.parking_id, t.fee_type, "
             "t.amount, t.payment_date, t.due_date, t.payment_method, t.status, "
             "t.period_start, t.period_end "
             "FROM transactions t "
             "WHERE t.user_id = '%s' "
             "ORDER BY t.payment_date DESC",
             owner_id);

    // 执行查询
    if (!execute_query(db, query, result))
    {
        printf("查询业主交易记录失败: %s", query);
        return false;
    }

    return true;
}

// 获取房屋交易记录
bool get_room_transactions(Database *db, const char *user_id, UserType user_type, const char *room_id, QueryResult *result)
{
    // 验证权限
    if (user_type == USER_OWNER)
    {
        // 检查房屋是否属于该业主
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

        // 检查房屋是否存在且属于该业主
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
        // 管理员和物业服务人员需要验证权限
        return false;
    }

    // 构建查询SQL语句
    char query[512];
    snprintf(query, sizeof(query),
             "SELECT t.transaction_id, t.user_id, t.room_id, t.parking_id, t.fee_type, "
             "t.amount, t.payment_date, t.due_date, t.payment_method, t.status, "
             "t.period_start, t.period_end "
             "FROM transactions t "
             "WHERE t.room_id = '%s' "
             "ORDER BY t.payment_date DESC",
             room_id);

    // 执行查询
    if (!execute_query(db, query, result))
    {
        printf("查询房屋交易记录失败: %s", query);
        return false;
    }

    return true;
}

// 获取停车位交易记录
bool get_parking_transactions(Database *db, const char *user_id, UserType user_type, const char *parking_id, QueryResult *result)
{
    // 验证权限
    if (user_type == USER_OWNER)
    {
        // 检查停车位是否属于该业主
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

        // 检查停车位是否存在且属于该业主
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
        // 管理员和物业服务人员需要验证权限
        return false;
    }

    // 构建查询SQL语句
    char query[512];
    snprintf(query, sizeof(query),
             "SELECT t.transaction_id, t.user_id, t.room_id, t.parking_id, t.fee_type, "
             "t.amount, t.payment_date, t.due_date, t.payment_method, t.status, "
             "t.period_start, t.period_end "
             "FROM transactions t "
             "WHERE t.parking_id = '%s' "
             "ORDER BY t.payment_date DESC",
             parking_id);

    // 执行查询
    if (!execute_query(db, query, result))
    {
        printf("查询停车位交易记录失败: %s", query);
        return false;
    }

    return true;
}
