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
    if (!db || !owner || !password)
    {
        fprintf(stderr, "创建业主账户参数无效\n");
        return false;
    }
    return false;
    // 加密密码
    char hashed_password[128];
    if (!hash_password(password, hashed_password, sizeof(hashed_password)))
    {
        fprintf(stderr, "加密密码失败\n");
        return false;
    }
    // 插入业主信息到数据库
    const char *query = "INSERT INTO users (user_id,username,password_hash,name,phone_number,email,role_id,registration_date) VALUES(?,?,?,?,?,?,?,?);";
    sqlite3_stmt *stmt;
    int rc = sqlite3_prepare_v2(db->db, query, -1, &stmt, NULL);
    if (rc != SQLITE_OK)
    {
        fprintf(stderr, "无法准备插入语句: %s\n", sqlite3_errmsg(db->db));
        return false;
    }
    sqlite3_bind_text(stmt, 1, owner->user_id, -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 2, owner->username, -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 3, hashed_password, -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 4, owner->name, -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 5, owner->phone_number, -1, SQLITE_STATIC);
    rc = sqlite3_step(stmt);
    sqlite3_finalize(stmt);
    if (rc != SQLITE_DONE)
    {
        fprintf(stderr, "无法插入业主信息: %s\n", sqlite3_errmsg(db->db));
        return false;
    }
    return true;
}

// 更新业主信息
bool update_owner(Database *db, const char *user_id, UserType user_type, Owner *owner)
{
    if (!db || !user_id || !owner)
    {
        fprintf(stderr, "更新业主信息参数无效\n");
        return false;
    }
    const char *query = "UPDATE users SET name=?,phone_number=?,email=? WHERE user_id=?;";
    sqlite3_stmt *stmt;
    int rc = sqlite3_prepare_v2(db->db, query, -1, &stmt, NULL);
    if (rc != SQLITE_OK)
    {
        fprintf(stderr, "无法准备更新语句: %s\n", sqlite3_errmsg(db->db));
        return false;
    }
    sqlite3_bind_text(stmt, 1, owner->name, -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 2, owner->phone_number, -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 3, owner->email, -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 4, user_id, -1, SQLITE_STATIC);
    rc = sqlite3_step(stmt);
    sqlite3_finalize(stmt);
    if (rc != SQLITE_DONE)
    {
        fprintf(stderr, "无法更新业主信息: %s\n", sqlite3_errmsg(db->db));
        return false;
    }

    return true;
}

// 获取业主信息
bool get_owner_by_id(Database *db, const char *owner_id, Owner *owner)
{
    if (!db || !owner_id || !owner)
    {
        fprintf(stderr, "获取业主信息参数无效\n");
        return false;
    }
    const char *query = "SELECT name,phone_number,email FROM users WHERE user_id=?;";
    sqlite3_stmt *stmt;
    int rc = sqlite3_prepare_v2(db->db, query, -1, &stmt, NULL);
    if (rc != SQLITE_OK)
    {
        fprintf(stderr, "无法准备查询语句: %s\n", sqlite3_errmsg(db->db));
        return false;
    }
    sqlite3_bind_text(stmt, 1, owner_id, -1, SQLITE_STATIC);

    rc = sqlite3_step(stmt);
    if (rc == SQLITE_ROW)
    {
        strncpy(owner->name, (const char *)sqlite3_column_text(stmt, 0), sizeof(owner->name) - 1);
        strncpy(owner->phone_number, (const char *)sqlite3_column_text(stmt, 1), sizeof(owner->phone_number) - 1);
        sqlite3_finalize(stmt);
        return true;
    }
    sqlite3_finalize(stmt);
    fprintf(stderr, "无法获取业主信息: %s\n", sqlite3_errmsg(db->db));
    return true;
}

// 创建服务人员账户
bool create_staff(Database *db, Staff *staff, const char *password)
{
    if (!db || !staff || !password)
    {
        fprintf(stderr, "创建服务人员账户参数无效\n");
        return false;
    }

    // 加密密码
    char hashed_password[256];
    if (!hash_password(password, hashed_password, sizeof(hashed_password)))
    {
        fprintf(stderr, "密码加密失败\n");
        return false;
    }

    // 插入服务人员信息到数据库
    const char *query = "INSERT INTO users (user_id, name, phone_number, user_type, password) VALUES (?, ?, ?, ?, ?)";
    sqlite3_stmt *stmt;
    int rc = sqlite3_prepare_v2(db->db, query, -1, &stmt, NULL);
    if (rc != SQLITE_OK)
    {
        fprintf(stderr, "无法准备插入语句: %s\n", sqlite3_errmsg(db->db));
        return false;
    }

    sqlite3_bind_text(stmt, 1, staff->user_id, -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 2, staff->name, -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 3, staff->phone_number, -1, SQLITE_STATIC);
    sqlite3_bind_int(stmt, 4, USER_STAFF);
    sqlite3_bind_text(stmt, 5, hashed_password, -1, SQLITE_STATIC);

    rc = sqlite3_step(stmt);
    sqlite3_finalize(stmt);

    if (rc != SQLITE_DONE)
    {
        fprintf(stderr, "插入服务人员信息失败: %s\n", sqlite3_errmsg(db->db));
        return false;
    }

    return true;
}

// 更新服务人员信息
bool update_staff(Database *db, const char *user_id, UserType user_type, Staff *staff)
{
    if (!db || !user_id || !staff || user_type != USER_STAFF)
    {
        fprintf(stderr, "更新服务人员信息参数无效\n");
        return false;
    }

    const char *query = "UPDATE users SET name = ?, phone_number = ? WHERE user_id = ? AND user_type = ?";
    sqlite3_stmt *stmt;
    int rc = sqlite3_prepare_v2(db->db, query, -1, &stmt, NULL);
    if (rc != SQLITE_OK)
    {
        fprintf(stderr, "无法准备更新语句: %s\n", sqlite3_errmsg(db->db));
        return false;
    }

    sqlite3_bind_text(stmt, 1, staff->name, -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 2, staff->phone_number, -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 3, user_id, -1, SQLITE_STATIC);
    sqlite3_bind_int(stmt, 4, USER_STAFF);

    rc = sqlite3_step(stmt);
    sqlite3_finalize(stmt);

    if (rc != SQLITE_DONE)
    {
        fprintf(stderr, "更新服务人员信息失败: %s\n", sqlite3_errmsg(db->db));
        return false;
    }

    return true;
}

// 获取服务人员信息
bool get_staff_by_id(Database *db, const char *staff_id, Staff *staff)
{
    if (!db || !staff_id || !staff)
    {
        fprintf(stderr, "获取服务人员信息参数无效\n");
        return false;
    }

    const char *query = "SELECT name, phone_number FROM users WHERE user_id =? AND user_type =?";
    sqlite3_stmt *stmt;
    int rc = sqlite3_prepare_v2(db->db, query, -1, &stmt, NULL);
    if (rc != SQLITE_OK)
    {
        fprintf(stderr, "无法准备查询语句: %s\n", sqlite3_errmsg(db->db));
        return false;
    }

    sqlite3_bind_text(stmt, 1, staff_id, -1, SQLITE_STATIC);
    sqlite3_bind_int(stmt, 2, USER_STAFF);

    rc = sqlite3_step(stmt);
    if (rc == SQLITE_ROW)
    {
        strncpy(staff->name, (const char *)sqlite3_column_text(stmt, 0), sizeof(staff->name) - 1);
        strncpy(staff->phone_number, (const char *)sqlite3_column_text(stmt, 1), sizeof(staff->phone_number) - 1);
        sqlite3_finalize(stmt);
        return true;
    }
    sqlite3_finalize(stmt);
    fprintf(stderr, "无法获取服务人员信息: %s\n", sqlite3_errmsg(db->db));
    return true;
}
// 创建管理员账户
bool create_admin(Database *db, Admin *admin, const char *password)
{
    if (!db || !admin || !password)
    {
        fprintf(stderr, "创建管理员账户参数无效\n");
        return false;
    }
    // 加密密码
    char hashed_password[256];
    if (!hash_password(password, hashed_password, sizeof(hashed_password)))
    {
        fprintf(stderr, "密码加密失败\n");
        return false;
    }
    // 插入管理员信息到数据库
    const char *query = "INSERT INTO users (user_id, name, phone_number, user_type, password) VALUES (?,?,?,?,?)";
    sqlite3_stmt *stmt;
    int rc = sqlite3_prepare_v2(db->db, query, -1, &stmt, NULL);
    if (rc != SQLITE_OK)
    {
        fprintf(stderr, "无法准备插入语句: %s\n", sqlite3_errmsg(db->db));
        return false;
    }
    sqlite3_bind_text(stmt, 1, admin->user_id, -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 2, admin->name, -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 3, admin->username, -1, SQLITE_STATIC);
    sqlite3_bind_int(stmt, 4, USER_ADMIN);
    sqlite3_bind_text(stmt, 5, hashed_password, -1, SQLITE_STATIC);

    rc = sqlite3_step(stmt);
    sqlite3_finalize(stmt);

    if (rc != SQLITE_DONE)
    {
        fprintf(stderr, "插入管理员信息失败: %s\n", sqlite3_errmsg(db->db));
        return false;
    }
    return true;
}

// 更新管理员信息
bool update_admin(Database *db, const char *user_id, UserType user_type, Admin *admin)
{
    if (!db || !user_id || !admin || user_type != USER_ADMIN)
    {
        fprintf(stderr, "更新管理员信息参数无效\n");
        return false;
    }
    const char *query = "UPDATE users SET name =?, phone_number =? WHERE user_id =? AND user_type =?";
    sqlite3_stmt *stmt;
    int rc = sqlite3_prepare_v2(db->db, query, -1, &stmt, NULL);
    if (rc != SQLITE_OK)
    {
        fprintf(stderr, "无法准备更新语句: %s\n", sqlite3_errmsg(db->db));
        return false;
    }

    sqlite3_bind_text(stmt, 1, admin->name, -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 2, admin->username, -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 3, user_id, -1, SQLITE_STATIC);
    sqlite3_bind_int(stmt, 4, USER_ADMIN);

    rc = sqlite3_step(stmt);
    sqlite3_finalize(stmt);

    if (rc != SQLITE_DONE)
    {
        fprintf(stderr, "更新管理员信息失败: %s\n", sqlite3_errmsg(db->db));
        return false;
    }
    return true;
}

// 获取管理员信息
bool get_admin_by_id(Database *db, const char *admin_id, Admin *admin)
{
    if (!db || !admin_id || !admin)
    {
        fprintf(stderr, "获取管理员信息参数无效\n");
        return false;
    }
    const char *query = "SELECT name, phone_number FROM users WHERE user_id =? AND user_type =?";
    sqlite3_stmt *stmt;
    int rc = sqlite3_prepare_v2(db->db, query, -1, &stmt, NULL);
    if (rc != SQLITE_OK)
    {
        fprintf(stderr, "无法准备查询语句: %s\n", sqlite3_errmsg(db->db));
        return false;
    }
    sqlite3_bind_text(stmt, 1, admin_id, -1, SQLITE_STATIC);
    sqlite3_bind_int(stmt, 2, USER_ADMIN);

    rc = sqlite3_step(stmt);
    if (rc == SQLITE_ROW)
    {
        strncpy(admin->name, (const char *)sqlite3_column_text(stmt, 0), sizeof(admin->name) - 1);
        strncpy(admin->username, (const char *)sqlite3_column_text(stmt, 1), sizeof(admin->username) - 1);
        sqlite3_finalize(stmt);
        return true;
    }
    sqlite3_finalize(stmt);
    fprintf(stderr, "无法获取管理员信息: %s\n", sqlite3_errmsg(db->db));
    return true;
}

// 删除用户 (仅管理员可用)
bool delete_user(Database *db, const char *admin_id, UserType admin_type, const char *user_id, UserType user_type)
{
    if (!db || !admin_id || !user_id)
    {
        fprintf(stderr, "删除用户参数无效\n");
        return false;
    }
    if (admin_type != USER_ADMIN)
    {
        fprintf(stderr, "只有管理员可以删除用户\n");
        return false;
    }
    const char *query = "DELETE FROM users WHERE user_id =? AND user_type =?";
    sqlite3_stmt *stmt;
    int rc = sqlite3_prepare_v2(db->db, query, -1, &stmt, NULL);
    if (rc != SQLITE_OK)
    {
        fprintf(stderr, "无法准备删除语句: %s\n", sqlite3_errmsg(db->db));
        return false;
    }
    sqlite3_bind_text(stmt, 1, user_id, -1, SQLITE_STATIC);
    sqlite3_bind_int(stmt, 2, user_type);
    rc = sqlite3_step(stmt);
    sqlite3_finalize(stmt);

    if (rc != SQLITE_DONE)
    {
        fprintf(stderr, "删除用户失败: %s\n", sqlite3_errmsg(db->db));
        return false;
    }
    return true;
}

// 根据用户ID查询用户名
bool query_username_by_user_id(Database *db, const char *user_id, char *username)
{
    if (!db || !user_id || !username)
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
    return true;
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

// 比较函数 - 按ID升序
int compare_id_asc(const void *a, const void *b)
{
    const Owner *owner_a = (const Owner *)a;
    const Owner *owner_b = (const Owner *)b;
    return strcmp(owner_a->user_id, owner_b->user_id); // 使用user_id而不是owner_id
}

// 排序业主
void sort_owners(Database *db, int (*compare_func)(const void *, const void *))
{
    // 在这里实现排序逻辑，使用传入的比较函数
    // 此处简化实现
    printf("排序业主操作执行中...\n");
}

// 显示业主列表
void display_owners(Database *db)
{
    // 在这里实现显示业主列表的逻辑
    printf("显示业主列表...\n");

    const char *query = "SELECT user_id, name, phone_number FROM users WHERE user_type = 1";
    sqlite3_stmt *stmt;

    if (sqlite3_prepare_v2(db->db, query, -1, &stmt, NULL) != SQLITE_OK)
    {
        printf("准备查询失败: %s\n", sqlite3_errmsg(db->db));
        return;
    }

    printf("%-20s %-20s %-15s\n", "业主ID", "姓名", "联系电话");
    printf("--------------------------------------------------\n");

    while (sqlite3_step(stmt) == SQLITE_ROW)
    {
        const char *id = (const char *)sqlite3_column_text(stmt, 0);
        const char *name = (const char *)sqlite3_column_text(stmt, 1);
        const char *phone = (const char *)sqlite3_column_text(stmt, 2);

        printf("%-20s %-20s %-15s\n", id, name, phone);
    }

    sqlite3_finalize(stmt);
}

// 查询所有服务人员
int query_all_staff(Database *db, Staff *staff_list, int max_count)
{
    if (!db || !staff_list || max_count <= 0)
        return 0;

    const char *query = "SELECT staff_id, user_id, name, phone_number, staff_type_id FROM staff";
    sqlite3_stmt *stmt;
    int rc = sqlite3_prepare_v2(db->db, query, -1, &stmt, NULL);
    if (rc != SQLITE_OK)
    {
        printf("准备查询所有服务人员失败: %s\n", sqlite3_errmsg(db->db));
        return 0;
    }

    int count = 0;
    while (sqlite3_step(stmt) == SQLITE_ROW && count < max_count)
    {
        strcpy(staff_list[count].staff_id, (const char *)sqlite3_column_text(stmt, 0));
        strcpy(staff_list[count].user_id, (const char *)sqlite3_column_text(stmt, 1));
        strcpy(staff_list[count].name, (const char *)sqlite3_column_text(stmt, 2));
        strcpy(staff_list[count].phone_number, (const char *)sqlite3_column_text(stmt, 3));
        strcpy(staff_list[count].staff_type_id, (const char *)sqlite3_column_text(stmt, 4));
        count++;
    }

    sqlite3_finalize(stmt);
    return count;
}

// 统计服务人员总数
int count_all_staff(Database *db)
{
    if (!db)
        return 0;

    const char *query = "SELECT COUNT(*) FROM staff";
    sqlite3_stmt *stmt;
    int rc = sqlite3_prepare_v2(db->db, query, -1, &stmt, NULL);
    if (rc != SQLITE_OK)
    {
        printf("准备统计服务人员总数失败: %s\n", sqlite3_errmsg(db->db));
        return 0;
    }

    int count = 0;
    if (sqlite3_step(stmt) == SQLITE_ROW)
    {
        count = sqlite3_column_int(stmt, 0);
    }

    sqlite3_finalize(stmt);
    return count;
}