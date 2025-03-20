/**
 * 用户模型实现文件 (user.c)
 *
 * 功能：
 * 本文件提供物业管理系统中用户相关操作的实现，包括业主、服务人员和管理员
 * 三种用户类型的账户创建、信息更新、信息获取以及用户删除功能。
 *
 * 主要功能：
 * 1. 业主(Owner)账户管理：创建、更新和获取业主信息
 * 2. 服务人员(Staff)账户管理：创建、更新和获取服务人员信息
 * 3. 管理员(Admin)账户管理：创建、更新和获取管理员信息
 * 4. 用户删除功能：仅限管理员使用
 *
 * 预期实现：
 * - 所有函数应与数据库交互，确保用户数据的持久化存储
 * - 用户操作应进行适当的权限验证
 * - 敏感信息如密码应进行加密处理
 * - 所有函数应有适当的错误处理和状态返回
 */

#include "models/user.h"
#include "utils/utils.h"
#include "auth/auth.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// 创建业主账户
bool create_owner(Database *db, Owner *owner, const char *password)
{
    // TODO: 实现创建业主账户功能
    return false;
}

// 更新业主信息
bool update_owner(Database *db, const char *user_id, UserType user_type, Owner *owner)
{
    // TODO: 实现更新业主信息功能
    return false;
}

// 获取业主信息
bool get_owner_by_id(Database *db, const char *owner_id, Owner *owner)
{
    // TODO: 实现获取业主信息功能
    return false;
}

// 创建服务人员账户
bool create_staff(Database *db, Staff *staff, const char *password)
{
    // TODO: 实现创建服务人员账户功能
    return false;
}

// 更新服务人员信息
bool update_staff(Database *db, const char *user_id, UserType user_type, Staff *staff)
{
    // TODO: 实现更新服务人员信息功能
    return false;
}

// 获取服务人员信息
bool get_staff_by_id(Database *db, const char *staff_id, Staff *staff)
{
    // TODO: 实现获取服务人员信息功能
    return false;
}

// 创建管理员账户
bool create_admin(Database *db, Admin *admin, const char *password)
{
    // TODO: 实现创建管理员账户功能
    return false;
}

// 更新管理员信息
bool update_admin(Database *db, const char *user_id, UserType user_type, Admin *admin)
{
    // TODO: 实现更新管理员信息功能
    return false;
}

// 获取管理员信息
bool get_admin_by_id(Database *db, const char *admin_id, Admin *admin)
{
    // TODO: 实现获取管理员信息功能
    return false;
}

// 删除用户 (仅管理员可用)
bool delete_user(Database *db, const char *admin_id, UserType admin_type, const char *user_id, UserType user_type)
{
    // TODO: 实现删除用户功能
    return false;
}

// 根据用户ID查询用户名
bool query_username_by_user_id(Database *db, const char *user_id, char *username)
{
    if (db == NULL || db->db == NULL || user_id == NULL || username == NULL)
    {
        fprintf(stderr, "查询用户名参数无效\n");
        return false;
    }

    // 准备SQL查询语句 - 从users表查询username字段
    const char *query = "SELECT username FROM users WHERE user_id = ?";
    sqlite3_stmt *stmt;

    // 准备语句
    int rc = sqlite3_prepare_v2(db->db, query, -1, &stmt, NULL);
    if (rc != SQLITE_OK)
    {
        fprintf(stderr, "无法准备查询语句: %s\n", sqlite3_errmsg(db->db));
        return false;
    }

    // 绑定参数
    rc = sqlite3_bind_text(stmt, 1, user_id, -1, SQLITE_STATIC);
    if (rc != SQLITE_OK)
    {
        fprintf(stderr, "无法绑定用户ID: %s\n", sqlite3_errmsg(db->db));
        sqlite3_finalize(stmt);
        return false;
    }

    // 执行查询
    rc = sqlite3_step(stmt);
    if (rc == SQLITE_ROW)
    {
        // 获取结果
        const unsigned char *result = sqlite3_column_text(stmt, 0);
        if (result)
        {
            strncpy(username, (const char *)result, 99);
            username[99] = '\0'; // 确保字符串结束
            sqlite3_finalize(stmt);
            return true;
        }
    }
    else if (rc != SQLITE_DONE)
    {
        fprintf(stderr, "查询执行失败: %s\n", sqlite3_errmsg(db->db));
    }

    // 没有找到
    strncpy(username, "未知用户", 99);
    sqlite3_finalize(stmt);
    return false;
}

// 通过用户姓名查询用户ID
bool query_user_id_by_name(Database *db, const char *name, char *user_id)
{
    if (db == NULL || db->db == NULL || name == NULL || user_id == NULL)
    {
        fprintf(stderr, "查询用户ID参数无效\n");
        return false;
    }

    // 准备SQL查询语句 - 首先计算匹配的用户数量
    const char *count_query = "SELECT COUNT(*) FROM users WHERE name = ?";
    sqlite3_stmt *count_stmt;
    int total_users = 0;

    // 准备计数语句
    int rc = sqlite3_prepare_v2(db->db, count_query, -1, &count_stmt, NULL);
    if (rc != SQLITE_OK)
    {
        fprintf(stderr, "无法准备计数查询语句: %s\n", sqlite3_errmsg(db->db));
        return false;
    }

    // 绑定参数
    rc = sqlite3_bind_text(count_stmt, 1, name, -1, SQLITE_STATIC);
    if (rc != SQLITE_OK)
    {
        fprintf(stderr, "无法绑定用户姓名: %s\n", sqlite3_errmsg(db->db));
        sqlite3_finalize(count_stmt);
        return false;
    }

    // 执行计数查询
    if (sqlite3_step(count_stmt) == SQLITE_ROW)
    {
        total_users = sqlite3_column_int(count_stmt, 0);
    }
    sqlite3_finalize(count_stmt);

    if (total_users == 0)
    {
        fprintf(stderr, "未找到姓名为 \"%s\" 的用户\n", name);
        return false;
    }
    else if (total_users == 1)
    {
        // 只有一个匹配，直接查询并返回
        const char *query = "SELECT user_id FROM users WHERE name = ?";
        sqlite3_stmt *stmt;

        rc = sqlite3_prepare_v2(db->db, query, -1, &stmt, NULL);
        if (rc != SQLITE_OK)
        {
            fprintf(stderr, "无法准备查询语句: %s\n", sqlite3_errmsg(db->db));
            return false;
        }

        rc = sqlite3_bind_text(stmt, 1, name, -1, SQLITE_STATIC);
        if (rc != SQLITE_OK)
        {
            fprintf(stderr, "无法绑定用户姓名: %s\n", sqlite3_errmsg(db->db));
            sqlite3_finalize(stmt);
            return false;
        }

        if (sqlite3_step(stmt) == SQLITE_ROW)
        {
            const unsigned char *result = sqlite3_column_text(stmt, 0);
            if (result)
            {
                strncpy(user_id, (const char *)result, 36);
                user_id[36] = '\0';
                sqlite3_finalize(stmt);
                return true;
            }
        }

        sqlite3_finalize(stmt);
        return false;
    }
    else
    {
        // 有多个匹配，需要动态分配内存存储所有匹配的结果
        char **user_ids = (char **)malloc(total_users * sizeof(char *));
        char **phone_numbers = (char **)malloc(total_users * sizeof(char *));

        if (user_ids == NULL || phone_numbers == NULL)
        {
            fprintf(stderr, "内存分配失败\n");
            if (user_ids)
                free(user_ids);
            if (phone_numbers)
                free(phone_numbers);
            return false;
        }

        // 为每个用户ID和手机号分配内存
        for (int i = 0; i < total_users; i++)
        {
            user_ids[i] = (char *)malloc(37 * sizeof(char));      // UUID长度最大36字符+结束符
            phone_numbers[i] = (char *)malloc(20 * sizeof(char)); // 手机号最大19字符+结束符

            if (user_ids[i] == NULL || phone_numbers[i] == NULL)
            {
                fprintf(stderr, "内存分配失败\n");
                // 释放已分配的内存
                for (int j = 0; j < i; j++)
                {
                    free(user_ids[j]);
                    free(phone_numbers[j]);
                }
                if (i < total_users && user_ids[i])
                    free(user_ids[i]);
                if (i < total_users && phone_numbers[i])
                    free(phone_numbers[i]);

                free(user_ids);
                free(phone_numbers);
                return false;
            }
        }

        // 准备SQL查询语句 - 获取匹配的用户及其手机号
        const char *query = "SELECT user_id, phone_number FROM users WHERE name = ?";
        sqlite3_stmt *stmt;

        rc = sqlite3_prepare_v2(db->db, query, -1, &stmt, NULL);
        if (rc != SQLITE_OK)
        {
            fprintf(stderr, "无法准备查询语句: %s\n", sqlite3_errmsg(db->db));
            goto cleanup;
        }

        rc = sqlite3_bind_text(stmt, 1, name, -1, SQLITE_STATIC);
        if (rc != SQLITE_OK)
        {
            fprintf(stderr, "无法绑定用户姓名: %s\n", sqlite3_errmsg(db->db));
            sqlite3_finalize(stmt);
            goto cleanup;
        }

        // 获取所有匹配的记录
        int count = 0;
        while ((rc = sqlite3_step(stmt)) == SQLITE_ROW && count < total_users)
        {
            const unsigned char *id_result = sqlite3_column_text(stmt, 0);
            const unsigned char *phone_result = sqlite3_column_text(stmt, 1);

            if (id_result)
            {
                strncpy(user_ids[count], (const char *)id_result, 36);
                user_ids[count][36] = '\0';

                // 复制手机号或设置为空字符串
                if (phone_result)
                {
                    strncpy(phone_numbers[count], (const char *)phone_result, 19);
                    phone_numbers[count][19] = '\0';
                }
                else
                {
                    strcpy(phone_numbers[count], "未知手机号");
                }

                count++;
            }
        }

        sqlite3_finalize(stmt);

        // 有多个匹配，需要用户选择
        printf("找到多个姓名为 \"%s\" 的用户，请选择：\n", name);
        for (int i = 0; i < count; i++)
        {
            char phone_suffix[15] = "未知";
            int phone_len = strlen(phone_numbers[i]);

            // 获取手机号后四位
            if (phone_len >= 4 && strcmp(phone_numbers[i], "未知手机号") != 0)
            {
                strncpy(phone_suffix, phone_numbers[i] + (phone_len - 4), 4);
                phone_suffix[4] = '\0';
            }

            printf("%d. 用户(手机尾号: %s)\n", i + 1, phone_suffix);
        }

        int choice;
        printf("请输入数字选择 (1-%d): ", count);
        scanf("%d", &choice);

        // 验证选择的有效性
        bool success = false;
        if (choice >= 1 && choice <= count)
        {
            // 返回选择的用户ID
            strcpy(user_id, user_ids[choice - 1]);
            success = true;
        }
        else
        {
            fprintf(stderr, "无效的选择\n");
        }

    cleanup:
        // 释放分配的内存
        for (int i = 0; i < total_users; i++)
        {
            free(user_ids[i]);
            free(phone_numbers[i]);
        }
        free(user_ids);
        free(phone_numbers);

        return success;
    }
}
