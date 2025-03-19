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

// 检查用户是否存在并验证密码
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
        // 获取密码哈希（实际上是明文密码）
        const char *db_password = (const char *)sqlite3_column_text(stmt, 1);
        // 获取角色ID
        const char *role_id = (const char *)sqlite3_column_text(stmt, 2);

        // 直接比较明文密码
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
                // 未知角色，设置为最低权限
                *user_type = USER_OWNER;
                *permission_level = 3;
            }
            found = true;
        }
    }

    // 清理语句
    sqlite3_finalize(stmt);

    return found;
}

// 用户认证
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

    // 此处简单实现，直接根据用户类型判断权限
    // 在实际应用中，应该从数据库中获取用户的实际权重
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
    return false;
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
