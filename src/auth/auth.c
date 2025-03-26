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

/**
 * @brief 验证用户的账号和密码
 *
 * @param db 数据库连接
 * @param username 用户名
 * @param password 密码
 * @param user_type 指向用户类型的指针，存储验证成功后的用户类型
 * @param user_id 存储验证成功后的用户ID的缓冲区
 * @param permission_level 指向权限级别的指针，存储验证成功后的权限级别
 * @return bool 验证成功返回true，失败返回false
 */
static bool check_user_credentials(Database *db, const char *username, const char *password,
                                   UserType *user_type, char *user_id, int *permission_level)
{
    if (!db || !username || !password || !user_type || !user_id || !permission_level)
    {
        return false;
    }

    const char *query = "SELECT user_id, password_hash, role_id FROM users WHERE username = ? AND status = 1;";
    sqlite3_stmt *stmt = NULL;

    if (db_prepare(db, query, &stmt) != SQLITE_OK)
    {
        fprintf(stderr, "SQL准备失败: %s\n", sqlite3_errmsg(db->db));
        return false;
    }

    if (sqlite3_bind_text(stmt, 1, username, -1, SQLITE_STATIC) != SQLITE_OK)
    {
        fprintf(stderr, "参数绑定失败: %s\n", sqlite3_errmsg(db->db));
        sqlite3_finalize(stmt);
        return false;
    }

    bool found = false;
    if (sqlite3_step(stmt) == SQLITE_ROW)
    {
        const char *db_user_id = (const char *)sqlite3_column_text(stmt, 0);
        const char *db_password = (const char *)sqlite3_column_text(stmt, 1);
        const char *role_id = (const char *)sqlite3_column_text(stmt, 2);

        if (db_password && strcmp(password, db_password) == 0)
        {
            strcpy(user_id, db_user_id);

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
                *user_type = USER_OWNER;
                *permission_level = 3;
            }
            found = true;
        }
    }

    sqlite3_finalize(stmt);
    return found;
}

/**
 * @brief 验证用户登录信息并返回登录结果
 *
 * @param db 数据库连接
 * @param username 用户名
 * @param password 密码
 * @return LoginResult 包含登录结果的结构体，成功时包含用户信息
 */
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

/**
 * @brief 验证用户是否具有执行特定操作的权限
 *
 * @param db 数据库连接
 * @param user_id 用户ID
 * @param user_type 用户类型
 * @param min_permission_level 需要的最低权限级别
 * @return bool 用户具有足够权限返回true，否则返回false
 */
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

/**
 * @brief 修改用户密码
 *
 * @param db 数据库连接
 * @param user_id 用户ID
 * @param user_type 用户类型
 * @param old_password 旧密码
 * @param new_password 新密码
 * @return bool 密码修改成功返回true，失败返回false
 */
bool change_password(Database *db, const char *user_id, UserType user_type, const char *old_password, const char *new_password)
{
    if (db == NULL || db->db == NULL || user_id == NULL || old_password == NULL || new_password == NULL)
    {
        fprintf(stderr, "无效的参数\n");
        return false;
        sqlite3_stmt *stmt;
        char stored_password[256] = {0};

        const char *sql_query = "SELECT password FROM users WHERE id = ? AND user_type = ?;";
        if (sqlite3_prepare_v2(db->db, sql_query, -1, &stmt, 0) != SQLITE_OK)
        {
            fprintf(stderr, "SQL 查询失败: %s\n", sqlite3_errmsg(db->db));
            return false;
        }

        sqlite3_bind_text(stmt, 1, user_id, -1, SQLITE_STATIC);
        sqlite3_bind_int(stmt, 2, user_type);

        if (sqlite3_step(stmt) == SQLITE_ROW)
        {
            strcpy(stored_password, (const char *)sqlite3_column_text(stmt, 0));
        }
        else
        {
            fprintf(stderr, "用户不存在或角色不匹配\n");
            sqlite3_finalize(stmt);
            return false;
        }
        sqlite3_finalize(stmt);

        if (strcmp(stored_password, old_password) != 0)
        {
            fprintf(stderr, "旧密码错误\n");
            return false;
        }

        const char *sql_update = "UPDATE users SET password = ? WHERE id = ?;";
        if (sqlite3_prepare_v2(db->db, sql_update, -1, &stmt, 0) != SQLITE_OK)
        {
            fprintf(stderr, "SQL 预处理失败: %s\n", sqlite3_errmsg(db->db));
            return false;
        }

        sqlite3_bind_text(stmt, 1, new_password, -1, SQLITE_STATIC);
        sqlite3_bind_text(stmt, 2, user_id, -1, SQLITE_STATIC);

        if (sqlite3_step(stmt) != SQLITE_DONE)
        {
            fprintf(stderr, "密码更新失败: %s\n", sqlite3_errmsg(db->db));
            sqlite3_finalize(stmt);
            return false;
        }

        sqlite3_finalize(stmt);
        printf("密码修改成功\n");
        return true;
    }
}
/**
 * @brief 重置用户密码为默认密码（仅管理员可操作）
 *
 * @param db 数据库连接
 * @param admin_id 管理员ID
 * @param admin_type 管理员类型
 * @param user_id 要重置密码的用户ID
 * @param user_type 要重置密码的用户类型
 * @return bool 重置成功返回true，失败返回false
 */
bool reset_password(Database *db, const char *admin_id, UserType admin_type, const char *user_id, UserType user_type)
{
    if (!db || !admin_id || !user_id)
    {
        fprintf(stderr, "无效的参数\n");
        return false;
    }

    const char *default_password = "default_password";
    char default_password_hash[256];
    if (!hash_password(default_password, default_password_hash, sizeof(default_password_hash)))
    {
        fprintf(stderr, "密码加密失败\n");
        return false;
    }

    if (admin_type != USER_ADMIN)
    {
        fprintf(stderr, "只有管理员可以重置密码\n");
        return false;
    }

    const char *query = "UPDATE users SET password_hash = ? WHERE user_id = ? AND role_id = ?;";
    sqlite3_stmt *stmt;
    if (sqlite3_prepare_v2(db->db, query, -1, &stmt, NULL) != SQLITE_OK)
    {
        fprintf(stderr, "SQL更新失败:%s\n", sqlite3_errmsg(db->db));
        return false;
    }
    sqlite3_bind_text(stmt, 1, default_password_hash, -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 2, user_id, -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 3, user_type == USER_ADMIN ? "role_admin" : "role_staff", -1, SQLITE_STATIC);
    if (sqlite3_step(stmt) != SQLITE_DONE)
    {
        fprintf(stderr, "密码重置失败:%s\n", sqlite3_errmsg(db->db));
        sqlite3_finalize(stmt);
        return false;
    }
    sqlite3_finalize(stmt);
    printf("密码重置成功\n");
    return true;
}
