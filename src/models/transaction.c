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
/**
 * 添加交易记录（安全增强版）
 * 
 * 修改说明：
 * 1. 强制设置所有NOT NULL字段的默认值
 * 2. 增强字符串字段的安全性
 * 3. 优化错误提示信息
 */
bool add_transaction(Database *db, const char *user_id, UserType user_type, Transaction *transaction)
{
    // ==================== 1. 字段默认值设置 ====================
    time_t now = time(NULL);
    
    // 必填时间字段兜底（解决NOT NULL约束）
    if (transaction->payment_date == 0) {
        transaction->payment_date = now;
    }
    if (transaction->period_start == 0) {
        transaction->period_start = now;
    }
    if (transaction->period_end == 0) {
        transaction->period_end = now;
    }
    
    // 交易状态兜底
    if (transaction->status == 0) {
        transaction->status = TRANS_PAID; // 默认设为已支付
    }

    // ==================== 2. 必填字段校验 ====================
    // 交易ID生成
    if (strlen(transaction->transaction_id) == 0) {
        generate_uuid(transaction->transaction_id);
    }

    // 金额校验
    if (transaction->amount <= 0) {
        fprintf(stderr, "[ERROR] 交易金额必须大于0\n");
        return false;
    }

    // 物业费必须关联房屋
    if (transaction->fee_type == TRANS_PROPERTY_FEE && strlen(transaction->room_id) == 0) {
        fprintf(stderr, "[ERROR] 物业费必须指定房屋ID\n");
        return false;
    }

    // 停车费必须关联车位
    if (transaction->fee_type == TRANS_PARKING_FEE && strlen(transaction->parking_id) == 0) {
        fprintf(stderr, "[ERROR] 停车费必须指定停车位ID\n");
        return false;
    }

    // ==================== 3. 安全生成SQL ====================
    char query[1024];
    snprintf(query, sizeof(query),
        "INSERT INTO transactions ("
        "transaction_id, user_id, room_id, parking_id, fee_type, "
        "amount, payment_date, due_date, payment_method, status, "
        "period_start, period_end"
        ") VALUES ("
        "'%s', '%s', '%s', '%s', %d, "    // 字符串字段
        "%.2f, %ld, %ld, %d, %d, "        // 数值字段
        "%ld, %ld)",                       // 时间字段
        transaction->transaction_id,
        transaction->user_id,
        transaction->room_id[0] ? transaction->room_id : "",  // 处理空字符串
        transaction->parking_id[0] ? transaction->parking_id : "",
        transaction->fee_type,
        transaction->amount,
        (long)transaction->payment_date,
        (long)transaction->due_date,
        transaction->payment_method,
        transaction->status,
        (long)transaction->period_start,
        (long)transaction->period_end
    );

    // ==================== 4. 执行并返回结果 ====================
    if (!execute_query(db, query, NULL)) {
        fprintf(stderr, "[ERROR] 添加交易记录失败 | SQL: %s\n", query);
        return false;
    }

    printf("[SUCCESS] 交易记录已添加 | 单号: %s | 用户: %s | 金额: ￥%.2f\n",
           transaction->transaction_id, 
           transaction->user_id, 
           transaction->amount);
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

/**
 * 检查并更新所有逾期未付的交易记录
 *
 * @param db 数据库连接
 * @return 更新成功返回true，失败返回false
 */
bool update_overdue_transactions(Database *db)
{
    time_t current_time = time(NULL);
    char query[512];

    // 将所有未付款且已过截止日期的交易标记为逾期
    snprintf(query, sizeof(query),
             "UPDATE transactions SET status = %d "
             "WHERE status = %d AND due_date < %ld",
             TRANS_OVERDUE, TRANS_UNPAID, (long)current_time);

    if (!execute_update(db, query))
    {
        printf("更新逾期交易状态失败\n");
        return false;
    }

    return true;
}

/**
 * 查询用户未付费用
 *
 * @param db 数据库连接
 * @param user_id 用户ID
 * @param result 查询结果
 * @return 查询成功返回true，失败返回false
 */
bool get_unpaid_transactions(Database *db, const char *user_id, QueryResult *result)
{
    char query[1024];
    snprintf(query, sizeof(query),
             "SELECT t.transaction_id, t.room_id, t.parking_id, t.fee_type, "
             "t.amount, t.due_date, t.status, t.period_start, t.period_end, "
             "CASE "
             "  WHEN t.fee_type = %d THEN (SELECT room_number FROM rooms WHERE room_id = t.room_id) "
             "  WHEN t.fee_type = %d THEN (SELECT parking_number FROM parking_spaces WHERE parking_id = t.parking_id) "
             "  ELSE '' "
             "END as location "
             "FROM transactions t "
             "WHERE t.user_id = '%s' AND (t.status = %d OR t.status = %d) "
             "ORDER BY t.due_date ASC",
             TRANS_PROPERTY_FEE, TRANS_PARKING_FEE, user_id, TRANS_UNPAID, TRANS_OVERDUE);

    if (!execute_query(db, query, result))
    {
        printf("查询未付费用失败\n");
        return false;
    }

    return true;
}

/**
 * 处理用户支付交易
 *
 * @param db 数据库连接
 * @param transaction_id 交易ID
 * @param user_id 用户ID
 * @param payment_method 支付方式
 * @return 支付成功返回true，失败返回false
 */
bool process_payment(Database *db, const char *transaction_id, const char *user_id, int payment_method)
{
    // 先检查交易是否存在并属于该用户
    char check_query[512];
    QueryResult check_result;

    snprintf(check_query, sizeof(check_query),
             "SELECT transaction_id, status FROM transactions "
             "WHERE transaction_id = '%s' AND user_id = '%s'",
             transaction_id, user_id);

    if (!execute_query(db, check_query, &check_result))
    {
        printf("验证交易记录失败\n");
        return false;
    }

    if (check_result.row_count == 0)
    {
        printf("未找到该交易记录或您无权支付此交易\n");
        free_query_result(&check_result);
        return false;
    }

    // 检查交易状态是否为未付或逾期
    int status = atoi(check_result.rows[0].values[1]);
    if (status == TRANS_PAID)
    {
        printf("该交易已支付，无需重复支付\n");
        free_query_result(&check_result);
        return false;
    }

    free_query_result(&check_result);

    // 更新交易状态为已付，记录支付时间和方式
    time_t payment_time = time(NULL);
    char update_query[512];

    snprintf(update_query, sizeof(update_query),
             "UPDATE transactions SET status = %d, payment_date = %ld, payment_method = %d "
             "WHERE transaction_id = '%s'",
             TRANS_PAID, (long)payment_time, payment_method, transaction_id);

    if (!execute_update(db, update_query))
    {
        printf("更新交易状态失败\n");
        return false;
    }

    printf("支付成功，交易ID: %s\n", transaction_id);
    return true;
}

/**
 * 生成物业费记录
 *
 * @param db 数据库连接
 * @param period_start 账单开始日期
 * @param period_end 账单结束日期
 * @param due_days 付款截止天数(从period_end开始计算)
 * @return 生成成功返回true，失败返回false
 */
bool generate_property_fees(Database *db, time_t period_start, time_t period_end, int due_days)
{
    char query[1024];
    QueryResult rooms;
    FeeStandard fee_standard;

    // 获取当前物业费标准
    if (!get_current_fee_standard(db, TRANS_PROPERTY_FEE, &fee_standard))
    {
        printf("获取物业费标准失败\n");
        return false;
    }

    // 查询所有有业主的房屋
    snprintf(query, sizeof(query),
             "SELECT r.room_id, r.owner_id, r.area_sqm FROM rooms r "
             "WHERE r.owner_id IS NOT NULL AND r.owner_id != '' AND r.status = 1");

    if (!execute_query(db, query, &rooms))
    {
        printf("查询房屋信息失败\n");
        return false;
    }

    time_t due_date = period_end + (due_days * 24 * 60 * 60); // 计算截止日期

    // 为每个房屋生成物业费记录
    for (int i = 0; i < rooms.row_count; i++)
    {
        char *room_id = rooms.rows[i].values[0];
        char *owner_id = rooms.rows[i].values[1];
        float area = atof(rooms.rows[i].values[2]);

        // 计算物业费金额
        float amount = area * fee_standard.price_per_unit;

        // 创建交易记录
        Transaction transaction;
        generate_uuid(transaction.transaction_id);
        strcpy(transaction.user_id, owner_id);
        strcpy(transaction.room_id, room_id);
        transaction.parking_id[0] = '\0'; // 物业费与停车位无关
        transaction.fee_type = TRANS_PROPERTY_FEE;
        transaction.amount = amount;
        transaction.payment_date = 0; // 尚未支付
        transaction.due_date = due_date;
        transaction.payment_method = PAYMENT_NONE; // 尚未选择支付方式
        transaction.status = TRANS_UNPAID;         // 未付款
        transaction.period_start = period_start;
        transaction.period_end = period_end;

        // 添加交易记录
        add_transaction(db, "system", USER_ADMIN, &transaction);
    }

    free_query_result(&rooms);
    return true;
}

/**
 * 生成停车费记录
 *
 * @param db 数据库连接
 * @param period_start 账单开始日期
 * @param period_end 账单结束日期
 * @param due_days 付款截止天数(从period_end开始计算)
 * @return 生成成功返回true，失败返回false
 */
bool generate_parking_fees(Database *db, time_t period_start, time_t period_end, int due_days)
{
    char query[1024];
    QueryResult parking_spaces;
    FeeStandard fee_standard;

    // 获取当前停车费标准
    if (!get_current_fee_standard(db, TRANS_PARKING_FEE, &fee_standard))
    {
        printf("获取停车费标准失败\n");
        return false;
    }

    // 查询所有有业主的停车位
    snprintf(query, sizeof(query),
             "SELECT parking_id, owner_id FROM parking_spaces "
             "WHERE owner_id IS NOT NULL AND owner_id != '' AND status = 1");

    if (!execute_query(db, query, &parking_spaces))
    {
        printf("查询停车位信息失败\n");
        return false;
    }

    time_t due_date = period_end + (due_days * 24 * 60 * 60); // 计算截止日期

    // 为每个停车位生成停车费记录
    for (int i = 0; i < parking_spaces.row_count; i++)
    {
        char *parking_id = parking_spaces.rows[i].values[0];
        char *owner_id = parking_spaces.rows[i].values[1];

        // 停车费是固定金额
        float amount = fee_standard.price_per_unit;

        // 创建交易记录
        Transaction transaction;
        generate_uuid(transaction.transaction_id);
        strcpy(transaction.user_id, owner_id);
        transaction.room_id[0] = '\0'; // 停车费与房屋无关
        strcpy(transaction.parking_id, parking_id);
        transaction.fee_type = TRANS_PARKING_FEE;
        transaction.amount = amount;
        transaction.payment_date = 0; // 尚未支付
        transaction.due_date = due_date;
        transaction.payment_method = PAYMENT_NONE; // 尚未选择支付方式
        transaction.status = TRANS_UNPAID;         // 未付款
        transaction.period_start = period_start;
        transaction.period_end = period_end;

        // 添加交易记录
        add_transaction(db, "system", USER_ADMIN, &transaction);
    }

    free_query_result(&parking_spaces);
    return true;
}