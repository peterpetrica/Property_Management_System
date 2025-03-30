#ifndef TRANSACTION_H
#define TRANSACTION_H

#include "db/database.h"
#include "db/db_query.h"
#include "auth/auth.h"
#include <stdbool.h>
#include <time.h>

// 交易类型
typedef enum
{
    TRANS_PROPERTY_FEE = 1,    // 物业费
    TRANS_PARKING_FEE = 2,     // 停车费
    TRANS_WATER_FEE = 3,       // 水费
    TRANS_ELECTRICITY_FEE = 4, // 电费
    TRANS_GAS_FEE = 5,         // 燃气费
    TRANS_OTHER = 99           // 其他费用
} FeeType;

// 支付方式
typedef enum
{
    PAYMENT_NONE = 0,   // 未支付
    PAYMENT_CASH = 1,   // 现金
    PAYMENT_CARD = 2,   // 银行卡
    PAYMENT_WECHAT = 3, // 微信
    PAYMENT_ALIPAY = 4, // 支付宝
    PAYMENT_OTHER = 99  // 其他方式
} PaymentMethod;

// 交易状态
typedef enum
{
    TRANS_UNPAID = 0, // 未付款
    TRANS_PAID = 1,   // 已付款
    TRANS_OVERDUE = 2 // 逾期未付
} TransactionStatus;

// 费用标准
typedef struct
{
    char standard_id[40];
    int fee_type;
    float price_per_unit;
    char unit[16];
    time_t effective_date;
    time_t end_date; // 0表示无限期
} FeeStandard;

// 交易记录
typedef struct
{
    char transaction_id[40];
    char user_id[40];
    char room_id[40];
    char parking_id[40];
    int fee_type;
    float amount;
    time_t payment_date;
    time_t due_date;
    int payment_method;
    int status;
    time_t period_start;
    time_t period_end;
} Transaction;

// 添加费用标准
bool add_fee_standard(Database *db, const char *user_id, UserType user_type, FeeStandard *standard);

// 获取当前费用标准
bool get_current_fee_standard(Database *db, int fee_type, FeeStandard *standard);

// 添加交易记录
bool add_transaction(Database *db, const char *user_id, UserType user_type, Transaction *transaction);

// 获取业主交易记录
bool get_owner_transactions(Database *db, const char *user_id, UserType user_type, const char *owner_id, QueryResult *result);

// 获取房屋交易记录
bool get_room_transactions(Database *db, const char *user_id, UserType user_type, const char *room_id, QueryResult *result);

// 获取停车位交易记录
bool get_parking_transactions(Database *db, const char *user_id, UserType user_type, const char *parking_id, QueryResult *result);

// 生成交易ID
char *generate_transaction_id();

// 检查并更新所有逾期未付的交易记录
bool update_overdue_transactions(Database *db);

// 查询用户未付费用
bool get_unpaid_transactions(Database *db, const char *user_id, QueryResult *result);

// 处理用户支付交易
bool process_payment(Database *db, const char *transaction_id, const char *user_id, int payment_method);

// 生成各类费用的函数声明
void generate_property_fees(Database *db, const char *user_id, UserType user_type);
bool generate_parking_fees(Database *db, time_t period_start, time_t period_end, int due_days);
bool generate_utility_fees(Database *db, time_t period_start, time_t period_end, int due_days);

#endif /* TRANSACTION_H */
