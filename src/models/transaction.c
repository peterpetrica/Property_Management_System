#include "models/transaction.h"
#include "auth/auth.h"
#include "utils/utils.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

// 添加费用标准
bool add_fee_standard(Database *db, const char *token, FeeStandard *standard)
{
    if (!db || !token || !standard)
        return false;

    // 验证令牌和管理员权限
    int weight;
    UserType user_type;
    if (!validate_token(db, token, &weight, &user_type) || user_type != USER_ADMIN)
    {
        return false;
    }

    // 构造SQL语句
    const char *sql = "INSERT INTO fee_standards (id, type, rate, valid_from, valid_to) "
                      "VALUES (?, ?, ?, ?, ?);";

    // 准备语句
    sqlite3_stmt *stmt;
    int rc = sqlite3_prepare_v2(db->db, sql, -1, &stmt, NULL);
    if (rc != SQLITE_OK)
        return false;

    // 绑定参数
    sqlite3_bind_text(stmt, 1, standard->id, -1, SQLITE_STATIC);
    sqlite3_bind_int(stmt, 2, standard->type);
    sqlite3_bind_double(stmt, 3, standard->rate);
    sqlite3_bind_int64(stmt, 4, standard->valid_from);
    sqlite3_bind_int64(stmt, 5, standard->valid_to);

    // 执行SQL
    rc = sqlite3_step(stmt);
    bool result = (rc == SQLITE_DONE);

    sqlite3_finalize(stmt);
    return result;
}

// 获取当前费用标准
bool get_current_fee_standard(Database *db, TransactionType type, FeeStandard *standard)
{
    if (!db || !standard)
        return false;

    time_t now = time(NULL);

    // 构造SQL语句
    const char *sql = "SELECT id, type, rate, valid_from, valid_to FROM fee_standards "
                      "WHERE type = ? AND valid_from <= ? AND (valid_to >= ? OR valid_to = 0) "
                      "ORDER BY valid_from DESC LIMIT 1;";

    // 准备语句
    sqlite3_stmt *stmt;
    int rc = sqlite3_prepare_v2(db->db, sql, -1, &stmt, NULL);
    if (rc != SQLITE_OK)
        return false;

    // 绑定参数
    sqlite3_bind_int(stmt, 1, type);
    sqlite3_bind_int64(stmt, 2, now);
    sqlite3_bind_int64(stmt, 3, now);

    // 执行SQL
    bool result = false;
    if (sqlite3_step(stmt) == SQLITE_ROW)
    {
        const char *id = (const char *)sqlite3_column_text(stmt, 0);
        int fee_type = sqlite3_column_int(stmt, 1);
        double rate = sqlite3_column_double(stmt, 2);
        sqlite3_int64 valid_from = sqlite3_column_int64(stmt, 3);
        sqlite3_int64 valid_to = sqlite3_column_int64(stmt, 4);

        safe_strcpy(standard->id, id ? id : "", sizeof(standard->id));
        standard->type = (TransactionType)fee_type;
        standard->rate = (float)rate;
        standard->valid_from = (time_t)valid_from;
        standard->valid_to = (time_t)valid_to;

        result = true;
    }

    sqlite3_finalize(stmt);
    return result;
}

// 添加交易记录
bool add_transaction(Database *db, const char *token, Transaction *transaction)
{
    if (!db || !token || !transaction)
        return false;

    // 验证令牌
    int weight;
    UserType user_type;
    if (!validate_token(db, token, &weight, &user_type))
    {
        return false;
    }

    // 构造SQL语句
    const char *sql = "INSERT INTO transactions "
                      "(id, owner_id, apartment_id, type, amount, payment_date, description) "
                      "VALUES (?, ?, ?, ?, ?, ?, ?);";

    // 准备语句
    sqlite3_stmt *stmt;
    int rc = sqlite3_prepare_v2(db->db, sql, -1, &stmt, NULL);
    if (rc != SQLITE_OK)
        return false;

    // 绑定参数
    sqlite3_bind_text(stmt, 1, transaction->id, -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 2, transaction->owner_id, -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 3, transaction->apartment_id, -1, SQLITE_STATIC);
    sqlite3_bind_int(stmt, 4, transaction->type);
    sqlite3_bind_double(stmt, 5, transaction->amount);
    sqlite3_bind_int64(stmt, 6, transaction->payment_date);
    sqlite3_bind_text(stmt, 7, transaction->description, -1, SQLITE_STATIC);

    // 执行SQL
    rc = sqlite3_step(stmt);
    bool result = (rc == SQLITE_DONE);

    sqlite3_finalize(stmt);
    return result;
}

// 获取业主交易记录
bool get_owner_transactions(Database *db, const char *token, const char *owner_id, QueryResult *result)
{
    if (!db || !token || !owner_id || !result)
        return false;

    // 验证令牌
    int weight;
    UserType user_type;
    if (!validate_token(db, token, &weight, &user_type))
    {
        return false;
    }

    // 构造SQL语句
    char sql[512];
    sprintf(sql, "SELECT * FROM transactions WHERE owner_id = '%s' ORDER BY payment_date DESC;", owner_id);

    // 执行查询
    return db_simple_query(db, sql, result) == SQLITE_OK;
}
