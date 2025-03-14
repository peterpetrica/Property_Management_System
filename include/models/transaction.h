#ifndef TRANSACTION_H
#define TRANSACTION_H

#include "db/database.h"
#include "db/db_query.h"
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
} TransactionType;

// 费用标准
typedef struct
{
    char id[32];
    TransactionType type;
    float rate;
    time_t valid_from;
    time_t valid_to; // 0表示无限期
} FeeStandard;

// 交易记录
typedef struct
{
    char id[32];
    char owner_id[32];
    char apartment_id[32];
    TransactionType type;
    float amount;
    time_t payment_date;
    char description[256];
} Transaction;

// 添加费用标准
bool add_fee_standard(Database *db, const char *token, FeeStandard *standard);

// 获取当前费用标准
bool get_current_fee_standard(Database *db, TransactionType type, FeeStandard *standard);

// 添加交易记录
bool add_transaction(Database *db, const char *token, Transaction *transaction);

// 获取业主交易记录
bool get_owner_transactions(Database *db, const char *token, const char *owner_id, QueryResult *result);

#endif /* TRANSACTION_H */
