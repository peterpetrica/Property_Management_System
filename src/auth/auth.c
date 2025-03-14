#include "auth/auth.h"
#include "auth/tokens.h" // 添加此行，引入 tokens.h
#include "utils/utils.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

// 检查用户是否存在并验证密码
static bool check_user_credentials(Database *db, const char *username, const char *password,
                                   UserType user_type, char *user_id, int *weight)
{
    const char *table_name;
    switch (user_type)
    {
    case USER_ADMIN:
        table_name = "admins";
        break;
    case USER_STAFF:
        table_name = "staff";
        break;
    case USER_OWNER:
        table_name = "owners";
        break;
    default:
        return false;
    }

    // 构造查询SQL
    char sql[256];
    snprintf(sql, sizeof(sql), "SELECT id, password_hash, weight FROM %s WHERE name = ?;", table_name);

    // 准备语句
    sqlite3_stmt *stmt;
    int rc = sqlite3_prepare_v2(db->db, sql, -1, &stmt, NULL);
    if (rc != SQLITE_OK)
    {
        fprintf(stderr, "SQL准备失败: %s\n", sqlite3_errmsg(db->db));
        return false;
    }

    // 绑定参数
    rc = sqlite3_bind_text(stmt, 1, username, -1, SQLITE_STATIC);
    if (rc != SQLITE_OK)
    {
        sqlite3_finalize(stmt);
        return false;
    }

    // 执行查询
    bool result = false;
    if (sqlite3_step(stmt) == SQLITE_ROW)
    {
        // 获取用户ID和密码哈希
        const char *db_user_id = (const char *)sqlite3_column_text(stmt, 0);
        const char *db_password_hash = (const char *)sqlite3_column_text(stmt, 1);
        int db_weight = sqlite3_column_int(stmt, 2);

        // 验证密码
        if (verify_password(password, db_password_hash))
        {
            // 密码验证成功
            strcpy(user_id, db_user_id);
            *weight = db_weight;
            result = true;
        }
    }

    // 清理
    sqlite3_finalize(stmt);
    return result;
}

// 生成令牌并存储到数据库
static bool generate_and_store_token(Database *db, const char *user_id, UserType user_type,
                                     int weight, char *token_buffer)
{
    // 生成令牌
    char *token = generate_uuid();
    if (!token)
        return false;

    // 存储令牌有效期
    time_t now = time(NULL);
    time_t expires = now + 3600; // 1小时后过期

    // 构造插入SQL
    char sql[512];
    snprintf(sql, sizeof(sql),
             "INSERT INTO tokens (token, user_id, user_type, weight, created_at, expires_at) "
             "VALUES (?, ?, ?, ?, ?, ?);");

    // 准备语句
    sqlite3_stmt *stmt;
    int rc = sqlite3_prepare_v2(db->db, sql, -1, &stmt, NULL);
    if (rc != SQLITE_OK)
    {
        free(token);
        return false;
    }

    // 绑定参数
    sqlite3_bind_text(stmt, 1, token, -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 2, user_id, -1, SQLITE_STATIC);
    sqlite3_bind_int(stmt, 3, (int)user_type);
    sqlite3_bind_int(stmt, 4, weight);
    sqlite3_bind_int64(stmt, 5, (sqlite3_int64)now);
    sqlite3_bind_int64(stmt, 6, (sqlite3_int64)expires);

    // 执行插入
    rc = sqlite3_step(stmt);
    bool result = (rc == SQLITE_DONE);

    // 清理
    sqlite3_finalize(stmt);

    if (result)
    {
        // 复制令牌到输出缓冲区
        strcpy(token_buffer, token);
    }

    free(token);
    return result;
}

LoginResult authenticate_user(Database *db, const char *username, const char *password)
{
    LoginResult result = {false};

    // 尝试作为管理员登录
    char user_id[32] = {0};
    int weight = 0;

    if (check_user_credentials(db, username, password, USER_ADMIN, user_id, &weight))
    {
        result.user_type = USER_ADMIN;
    }
    // 尝试作为物业服务人员登录
    else if (check_user_credentials(db, username, password, USER_STAFF, user_id, &weight))
    {
        result.user_type = USER_STAFF;
    }
    // 尝试作为业主登录
    else if (check_user_credentials(db, username, password, USER_OWNER, user_id, &weight))
    {
        result.user_type = USER_OWNER;
    }
    else
    {
        // 所有尝试都失败，返回失败结果
        return result;
    }

    // 生成并存储令牌
    if (generate_and_store_token(db, user_id, result.user_type, weight, result.token))
    {
        result.success = true;
        result.weight = weight;
        strcpy(result.user_id, user_id);
    }

    return result;
}

bool validate_token(Database *db, const char *token, int *weight, UserType *user_type)
{
    if (!db || !token || !weight || !user_type)
        return false;

    // 构造查询SQL
    const char *sql = "SELECT user_type, weight, expires_at FROM tokens WHERE token = ?;";

    // 准备语句
    sqlite3_stmt *stmt;
    int rc = sqlite3_prepare_v2(db->db, sql, -1, &stmt, NULL);
    if (rc != SQLITE_OK)
        return false;

    // 绑定参数
    sqlite3_bind_text(stmt, 1, token, -1, SQLITE_STATIC);

    // 执行查询
    bool result = false;
    if (sqlite3_step(stmt) == SQLITE_ROW)
    {
        // 获取令牌信息
        int db_user_type = sqlite3_column_int(stmt, 0);
        int db_weight = sqlite3_column_int(stmt, 1);
        sqlite3_int64 db_expires = sqlite3_column_int64(stmt, 2);

        // 检查令牌是否已过期
        time_t now = time(NULL);
        if (now < db_expires)
        {
            // 令牌有效
            *user_type = (UserType)db_user_type;
            *weight = db_weight;
            result = true;
        }
    }

    // 清理
    sqlite3_finalize(stmt);
    return result;
}

bool change_password(Database *db, const char *user_id, UserType user_type, const char *old_password, const char *new_password)
{
    const char *table_name;
    switch (user_type)
    {
    case USER_ADMIN:
        table_name = "admins";
        break;
    case USER_STAFF:
        table_name = "staff";
        break;
    case USER_OWNER:
        table_name = "owners";
        break;
    default:
        return false;
    }

    // 1. 验证旧密码
    char sql_verify[256];
    snprintf(sql_verify, sizeof(sql_verify), "SELECT password_hash FROM %s WHERE id = ?;", table_name);

    sqlite3_stmt *stmt_verify;
    int rc = sqlite3_prepare_v2(db->db, sql_verify, -1, &stmt_verify, NULL);
    if (rc != SQLITE_OK)
        return false;

    sqlite3_bind_text(stmt_verify, 1, user_id, -1, SQLITE_STATIC);

    bool password_verified = false;
    if (sqlite3_step(stmt_verify) == SQLITE_ROW)
    {
        const char *db_password_hash = (const char *)sqlite3_column_text(stmt_verify, 0);
        password_verified = verify_password(old_password, db_password_hash);
    }

    sqlite3_finalize(stmt_verify);

    if (!password_verified)
        return false;

    // 2. 更新密码
    char *new_password_hash = hash_password(new_password);
    if (!new_password_hash)
        return false;

    char sql_update[256];
    snprintf(sql_update, sizeof(sql_update), "UPDATE %s SET password_hash = ? WHERE id = ?;", table_name);

    sqlite3_stmt *stmt_update;
    rc = sqlite3_prepare_v2(db->db, sql_update, -1, &stmt_update, NULL);
    if (rc != SQLITE_OK)
    {
        free(new_password_hash);
        return false;
    }

    sqlite3_bind_text(stmt_update, 1, new_password_hash, -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt_update, 2, user_id, -1, SQLITE_STATIC);

    rc = sqlite3_step(stmt_update);
    bool result = (rc == SQLITE_DONE);

    // 清理
    sqlite3_finalize(stmt_update);
    free(new_password_hash);

    return result;
}

bool reset_password(Database *db, const char *admin_token, const char *user_id, UserType user_type)
{
    // 验证管理员token
    int admin_weight;
    UserType admin_type;
    if (!validate_token(db, admin_token, &admin_weight, &admin_type) || admin_type != USER_ADMIN)
    {
        return false;
    }

    // 确定用户表
    const char *table_name;
    switch (user_type)
    {
    case USER_STAFF:
        table_name = "staff";
        break;
    case USER_OWNER:
        table_name = "owners";
        break;
    default:
        return false; // 不允许重置管理员密码
    }

    // 生成默认密码 "123456"
    char *default_password_hash = hash_password("123456");
    if (!default_password_hash)
        return false;

    // 更新密码
    char sql_update[256];
    snprintf(sql_update, sizeof(sql_update), "UPDATE %s SET password_hash = ? WHERE id = ?;", table_name);

    sqlite3_stmt *stmt_update;
    int rc = sqlite3_prepare_v2(db->db, sql_update, -1, &stmt_update, NULL);
    if (rc != SQLITE_OK)
    {
        free(default_password_hash);
        return false;
    }

    sqlite3_bind_text(stmt_update, 1, default_password_hash, -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt_update, 2, user_id, -1, SQLITE_STATIC);

    rc = sqlite3_step(stmt_update);
    bool result = (rc == SQLITE_DONE);

    // 清理
    sqlite3_finalize(stmt_update);
    free(default_password_hash);

    return result;
}
