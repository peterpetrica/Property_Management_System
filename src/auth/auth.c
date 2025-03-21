/*
 * 认证模块 (auth.c)
 *
 * 功能描述：
 * 此模块负责物业管理系统的用户认证和密码处理。
 *
 * 主要功能：
 * 1. 用户认证 - 验证用户名和密码的有效性，支持多种用户类型
 * 2. 权限验证 - 检查用户是否具有执行特定操作的权限
 * 3. 密码管理 - 提供密码修改和重置功能
 */

#include "auth/auth.h"
#include "utils/utils.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <openssl/sha.h>

// 验证帐号密码
static bool check_user_credentials(Database *db, const char *username, const char *password,
                                   UserType *user_type, char *user_id, int *permission_level)
{
    if (!db || !username || !password || !user_type || !user_id || !permission_level)
    {
        return false;
    }

    // 准备SQL查询语句，从users表中查询用户信息
    const char *query = "SELECT user_id, password_hash, role_id FROM users WHERE username = ? AND status = 1;";
    sqlite3_stmt *stmt = NULL;

    if (db_prepare(db, query, &stmt) != SQLITE_OK)
    {
        fprintf(stderr, "SQL准备失败: %s\n", sqlite3_errmsg(db->db));
        return false;
    }

    // 绑定用户名参数
    if (sqlite3_bind_text(stmt, 1, username, -1, SQLITE_STATIC) != SQLITE_OK)
    {
        fprintf(stderr, "参数绑定失败: %s\n", sqlite3_errmsg(db->db));
        sqlite3_finalize(stmt);
        return false;
    }

    bool found = false;
    // 执行查询
    if (sqlite3_step(stmt) == SQLITE_ROW)
    {
        // 获取用户ID
        const char *db_user_id = (const char *)sqlite3_column_text(stmt, 0);
        // 获取密码
        const char *db_password = (const char *)sqlite3_column_text(stmt, 1);
        // 获取角色ID
        const char *role_id = (const char *)sqlite3_column_text(stmt, 2);

        // 验证密码
        if (db_password && strcmp(password, db_password) == 0)
        {
            // 复制用户ID
            strcpy(user_id, db_user_id);

            // 根据角色ID设置用户类型和权限级别
            if (strcmp(role_id, "role_admin") == 0)
            {
                *user_type = USER_ADMIN;
                *permission_level = 1;
            }
            else if (strcmp(role_id, "role_staff") == 0)
            {
                *user_type = USER_STAFF;
                *permission_level = 2;
            }
            else if (strcmp(role_id, "role_owner") == 0)
            {
                *user_type = USER_OWNER;
                *permission_level = 3;
            }
            else
            {
                // 未知
                *user_type = USER_OWNER;
                *permission_level = 3;
            }
            found = true;
        }
    }

    // 清理
    sqlite3_finalize(stmt);

    return found;
}

// 用户认证主调用
LoginResult authenticate_user(Database *db, const char *username, const char *password)
{
    LoginResult result = {false, 0, 0, {0}};
    char user_id[80] = {0};
    int permission_level = 0;
    UserType user_type = 0;

    if (check_user_credentials(db, username, password, &user_type, user_id, &permission_level))
    {
        result.success = true;
        result.user_type = user_type;
        result.permission_level = permission_level;
        safe_strcpy(result.user_id, user_id, sizeof(result.user_id));
    }

    return result;
}

// 验证用户权限
bool validate_permission(Database *db, const char *user_id, UserType user_type, int min_permission_level)
{
    if (!db || !user_id)
    {
        return false;
    }

    int permission_level;
    switch (user_type)
    {
    case USER_ADMIN:
        permission_level = 1;
        break;
    case USER_STAFF:
        permission_level = 2;
        break;
    case USER_OWNER:
        permission_level = 3;
        break;
    default:
        return false;
    }

    return permission_level <= min_permission_level;
}

// 更改密码
bool change_password(Database *db, const char *user_id, UserType user_type, const char *old_password, const char *new_password)
{
    // TODO: 实现更改密码功能
    // 1. 验证旧密码是否正确
    // 2. 生成新密码的哈希
    // 3. 更新数据库中的密码
    if (db == NULL || db->db == NULL || user_id == NULL || old_password == NULL || new_password == NULL) {
        fprintf(stderr, "无效的参数\n");
        return false;
        sqlite3_stmt* stmt;
        char stored_password[256] = { 0 };  // 假设数据库中的密码最大长度为 255

        // **2. 查询数据库中存储的旧密码**
        const char* sql_query = "SELECT password FROM users WHERE id = ? AND user_type = ?;";
        if (sqlite3_prepare_v2(db->db, sql_query, -1, &stmt, 0) != SQLITE_OK) {
            fprintf(stderr, "SQL 查询失败: %s\n", sqlite3_errmsg(db->db));
            return false;
        }

        // 绑定参数
        sqlite3_bind_text(stmt, 1, user_id, -1, SQLITE_STATIC);
        sqlite3_bind_int(stmt, 2, user_type);

        // 执行查询
        if (sqlite3_step(stmt) == SQLITE_ROW) {
            strcpy(stored_password, (const char*)sqlite3_column_text(stmt, 0));
        }
        else {
            fprintf(stderr, "用户不存在或角色不匹配\n");
            sqlite3_finalize(stmt);
            return false;
        }
        sqlite3_finalize(stmt);  // 释放查询语句

        // **3. 验证旧密码是否正确**
        if (strcmp(stored_password, old_password) != 0) {
            fprintf(stderr, "旧密码错误\n");
            return false;
        }

        // **4. 更新密码**
        const char* sql_update = "UPDATE users SET password = ? WHERE id = ?;";
        if (sqlite3_prepare_v2(db->db, sql_update, -1, &stmt, 0) != SQLITE_OK) {
            fprintf(stderr, "SQL 预处理失败: %s\n", sqlite3_errmsg(db->db));
            return false;
        }

        // 绑定新密码和用户ID
        sqlite3_bind_text(stmt, 1, new_password, -1, SQLITE_STATIC);
        sqlite3_bind_text(stmt, 2, user_id, -1, SQLITE_STATIC);

        // 执行更新
        if (sqlite3_step(stmt) != SQLITE_DONE) {
            fprintf(stderr, "密码更新失败: %s\n", sqlite3_errmsg(db->db));
            sqlite3_finalize(stmt);
            return false;
        }

        sqlite3_finalize(stmt);  // 释放语句
        printf("密码修改成功\n");
        return true;
    }
    }

    
// 重置密码 (仅管理员)
bool reset_password(Database *db, const char *admin_id, UserType admin_type, const char *user_id, UserType user_type)
{
    // TODO: 实现重置密码功能
    // 1. 验证操作者是否为管理员
    // 2. 生成默认密码的哈希
    // 3. 更新数据库中的密码
    return false;
}
