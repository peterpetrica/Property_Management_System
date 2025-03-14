#include "auth/tokens.h"
#include "utils/utils.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

// 令牌有效期（秒）
#define TOKEN_EXPIRE_TIME (60 * 60 * 24) // 24小时

// 生成令牌
char *generate_token(Database *db, const char *user_id, int user_type)
{
    if (!db || !user_id)
        return NULL;

    // 生成随机令牌
    char *token = generate_uuid();
    if (!token)
        return NULL;

    // 计算过期时间
    time_t expire_time = time(NULL) + TOKEN_EXPIRE_TIME;

    // 构造SQL语句
    const char *sql = "INSERT INTO tokens (token, user_id, user_type, expire_time) VALUES (?, ?, ?, ?);";

    // 准备语句
    sqlite3_stmt *stmt;
    int rc = sqlite3_prepare_v2(db->db, sql, -1, &stmt, NULL);
    if (rc != SQLITE_OK)
    {
        free(token);
        return NULL;
    }

    // 绑定参数
    sqlite3_bind_text(stmt, 1, token, -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 2, user_id, -1, SQLITE_STATIC);
    sqlite3_bind_int(stmt, 3, user_type);
    sqlite3_bind_int64(stmt, 4, expire_time);

    // 执行SQL
    rc = sqlite3_step(stmt);
    if (rc != SQLITE_DONE)
    {
        sqlite3_finalize(stmt);
        free(token);
        return NULL;
    }

    sqlite3_finalize(stmt);
    return token;
}

// 验证令牌并返回用户ID
bool get_user_id_from_token(Database *db, const char *token, char *user_id, size_t user_id_size)
{
    if (!db || !token || !user_id || user_id_size == 0)
        return false;

    time_t now = time(NULL);

    // 构造SQL语句
    const char *sql = "SELECT user_id FROM tokens WHERE token = ? AND expire_time > ?;";

    // 准备语句
    sqlite3_stmt *stmt;
    int rc = sqlite3_prepare_v2(db->db, sql, -1, &stmt, NULL);
    if (rc != SQLITE_OK)
        return false;

    // 绑定参数
    sqlite3_bind_text(stmt, 1, token, -1, SQLITE_STATIC);
    sqlite3_bind_int64(stmt, 2, now);

    // 执行SQL
    rc = sqlite3_step(stmt);
    bool result = false;
    if (rc == SQLITE_ROW)
    {
        const char *id = (const char *)sqlite3_column_text(stmt, 0);
        if (id)
        {
            safe_strcpy(user_id, id, user_id_size);
            result = true;
        }
    }

    sqlite3_finalize(stmt);
    return result;
}

// 使令牌失效
bool invalidate_token(Database *db, const char *token)
{
    if (!db || !token)
        return false;

    // 构造SQL语句
    const char *sql = "DELETE FROM tokens WHERE token = ?;";

    // 准备语句
    sqlite3_stmt *stmt;
    int rc = sqlite3_prepare_v2(db->db, sql, -1, &stmt, NULL);
    if (rc != SQLITE_OK)
        return false;

    // 绑定参数
    sqlite3_bind_text(stmt, 1, token, -1, SQLITE_STATIC);

    // 执行SQL
    rc = sqlite3_step(stmt);
    bool result = (rc == SQLITE_DONE);

    sqlite3_finalize(stmt);
    return result;
}

// 清理过期令牌
int cleanup_expired_tokens(Database *db)
{
    if (!db)
        return -1;

    time_t now = time(NULL);

    // 构造SQL语句
    const char *sql = "DELETE FROM tokens WHERE expire_time <= ?;";

    // 准备语句
    sqlite3_stmt *stmt;
    int rc = sqlite3_prepare_v2(db->db, sql, -1, &stmt, NULL);
    if (rc != SQLITE_OK)
        return -1;

    // 绑定参数
    sqlite3_bind_int64(stmt, 1, now);

    // 执行SQL
    rc = sqlite3_step(stmt);
    if (rc != SQLITE_DONE)
    {
        sqlite3_finalize(stmt);
        return -1;
    }

    // 获取删除的行数
    int deleted_count = sqlite3_changes(db->db);

    sqlite3_finalize(stmt);
    return deleted_count;
}
