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

/**
 * @brief 创建业主账户
 *
 * @param db 数据库连接
 * @param owner 业主信息结构体
 * @param password 业主密码（明文）
 * @return bool 创建成功返回true，失败返回false
 */
bool create_owner(Database *db, Owner *owner, const char *password)
{
    if (!db || !owner || !password)
    {
        fprintf(stderr, "创建业主账户参数无效\n");
        return false;
    }
    return false;

    char hashed_password[128];
    if (!hash_password(password, hashed_password, sizeof(hashed_password)))
    {
        fprintf(stderr, "加密密码失败\n");
        return false;
    }

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

/**
 * @brief 更新业主信息
 *
 * @param db 数据库连接
 * @param user_id 业主ID
 * @param user_type 用户类型
 * @param owner 更新后的业主信息
 * @return bool 更新成功返回true，失败返回false
 */
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

/**
 * @brief 通过ID获取业主信息
 *
 * @param db 数据库连接
 * @param owner_id 业主ID
 * @param owner 用于存储查询结果的业主结构体
 * @return bool 查询成功返回true，失败返回false
 */
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

/**
 * @brief 创建服务人员账户
 *
 * @param db 数据库连接
 * @param staff 服务人员信息结构体
 * @param password 服务人员密码（明文）
 * @return bool 创建成功返回true，失败返回false
 */
bool create_staff(Database *db, Staff *staff, const char *password)
{
    if (!db || !staff || !password)
    {
        fprintf(stderr, "创建服务人员账户参数无效\n");
        return false;
    }

    char hashed_password[256];
    if (!hash_password(password, hashed_password, sizeof(hashed_password)))
    {
        fprintf(stderr, "密码加密失败\n");
        return false;
    }

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

/**
 * @brief 更新服务人员信息
 *
 * @param db 数据库连接
 * @param user_id 服务人员ID
 * @param user_type 用户类型，必须为USER_STAFF
 * @param staff 更新后的服务人员信息
 * @return bool 更新成功返回true，失败返回false
 */
bool update_staff(Database *db, const char *user_id, UserType user_type, Staff *staff)
{
    if (!db || !user_id || !staff || user_type != USER_STAFF)
    {
        fprintf(stderr, "更新服务人员信息参数无效\n");
        return false;
    }

    const char *query = "UPDATE staff"
                        "SET staff_type_id= ?"
                        "WHERE user_id=?";
    sqlite3_stmt *stmt;
    int rc = sqlite3_prepare_v2(db->db, query, -1, &stmt, NULL);
    if (rc != SQLITE_OK)
    {
        fprintf(stderr, "无法准备更新语句: %s\n", sqlite3_errmsg(db->db));
        return false;
    }

    sqlite3_bind_text(stmt, 1, staff->staff_type_id, -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 2, user_id, -1, SQLITE_STATIC);

    rc = sqlite3_step(stmt);
    sqlite3_finalize(stmt);

    if (rc != SQLITE_DONE)
    {
        fprintf(stderr, "更新服务人员信息失败: %s\n", sqlite3_errmsg(db->db));
        return false;
    }

    return true;
}

/**
 * @brief 通过ID获取服务人员信息
 *
 * @param db 数据库连接
 * @param staff_id 服务人员ID
 * @param staff 用于存储查询结果的服务人员结构体
 * @return bool 查询成功返回true，失败返回false
 */
bool get_staff_by_id(Database *db, const char *staff_id, Staff *staff)
{
    if (!db || !staff_id || !staff)
    {
        fprintf(stderr, "获取服务人员信息参数无效\n");
        return false;
    }

    // 初始化staff结构体
    memset(staff, 0, sizeof(Staff));

    const char *query =
        "SELECT u.name, u.phone_number, s.staff_type_id, s.staff_id "
        "FROM users u JOIN staff s ON u.user_id = s.user_id "
        "WHERE u.user_id = ? AND u.role_id = 'role_staff'";

    sqlite3_stmt *stmt;
    int rc = sqlite3_prepare_v2(db->db, query, -1, &stmt, NULL);
    if (rc != SQLITE_OK)
    {
        fprintf(stderr, "无法准备查询语句: %s\n", sqlite3_errmsg(db->db));
        return false;
    }

    sqlite3_bind_text(stmt, 1, staff_id, -1, SQLITE_STATIC);

    rc = sqlite3_step(stmt);
    if (rc == SQLITE_ROW)
    {
        // ID
        strncpy(staff->user_id, staff_id, sizeof(staff->user_id) - 1);
        staff->user_id[sizeof(staff->user_id) - 1] = '\0';

        // 姓名
        const char *name = (const char *)sqlite3_column_text(stmt, 0);
        if (name)
        {
            strncpy(staff->name, name, sizeof(staff->name) - 1);
            staff->name[sizeof(staff->name) - 1] = '\0';
        }

        // 电话
        const char *phone = (const char *)sqlite3_column_text(stmt, 1);
        if (phone)
        {
            strncpy(staff->phone_number, phone, sizeof(staff->phone_number) - 1);
            staff->phone_number[sizeof(staff->phone_number) - 1] = '\0';
        }

        // 服务类型ID
        const char *staff_type = (const char *)sqlite3_column_text(stmt, 2);
        if (staff_type)
        {
            strncpy(staff->staff_type_id, staff_type, sizeof(staff->staff_type_id) - 1);
            staff->staff_type_id[sizeof(staff->staff_type_id) - 1] = '\0';
        }

        // 员工ID
        const char *staff_id_val = (const char *)sqlite3_column_text(stmt, 3);
        if (staff_id_val)
        {
            strncpy(staff->staff_id, staff_id_val, sizeof(staff->staff_id) - 1);
            staff->staff_id[sizeof(staff->staff_id) - 1] = '\0';
        }

        sqlite3_finalize(stmt);
        return true;
    }

    sqlite3_finalize(stmt);
    fprintf(stderr, "未找到服务人员信息\n");
    return false;
}

/**
 * @brief 创建管理员账户
 *
 * @param db 数据库连接
 * @param admin 管理员信息结构体
 * @param password 管理员密码（明文）
 * @return bool 创建成功返回true，失败返回false
 */
bool create_admin(Database *db, Admin *admin, const char *password)
{
    if (!db || !admin || !password)
    {
        fprintf(stderr, "创建管理员账户参数无效\n");
        return false;
    }

    char hashed_password[256];
    if (!hash_password(password, hashed_password, sizeof(hashed_password)))
    {
        fprintf(stderr, "密码加密失败\n");
        return false;
    }

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

/**
 * @brief 更新管理员信息
 *
 * @param db 数据库连接
 * @param user_id 管理员ID
 * @param user_type 用户类型，必须为USER_ADMIN
 * @param admin 更新后的管理员信息
 * @return bool 更新成功返回true，失败返回false
 */
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

/**
 * @brief 通过ID获取管理员信息
 *
 * @param db 数据库连接
 * @param admin_id 管理员ID
 * @param admin 用于存储查询结果的管理员结构体
 * @return bool 查询成功返回true，失败返回false
 */
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

/**
 * @brief 删除用户（仅限管理员使用）
 *
 * @param db 数据库连接
 * @param admin_id 执行删除操作的管理员ID
 * @param admin_type 执行删除操作的用户类型，必须为USER_ADMIN
 * @param user_id 被删除的用户ID
 * @param user_type 被删除的用户类型
 * @return bool 删除成功返回true，失败返回false
 */
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

/**
 * @brief 根据用户ID查询用户名
 *
 * @param db 数据库连接
 * @param user_id 用户ID
 * @param username 用于存储查询结果的用户名字符串
 * @return bool 查询成功返回true，失败返回false
 */
bool query_username_by_user_id(Database *db, const char *user_id, char *username)
{
    if (!db || !user_id || !username)
    {
        fprintf(stderr, "查询用户名参数无效\n");
        return false;
    }

    const char *query = "SELECT username FROM users WHERE user_id = ?";
    sqlite3_stmt *stmt;

    int rc = sqlite3_prepare_v2(db->db, query, -1, &stmt, NULL);
    if (rc != SQLITE_OK)
    {
        fprintf(stderr, "无法准备查询语句: %s\n", sqlite3_errmsg(db->db));
        return false;
    }

    rc = sqlite3_bind_text(stmt, 1, user_id, -1, SQLITE_STATIC);
    if (rc != SQLITE_OK)
    {
        fprintf(stderr, "无法绑定用户ID: %s\n", sqlite3_errmsg(db->db));
        sqlite3_finalize(stmt);
        return false;
    }

    rc = sqlite3_step(stmt);
    if (rc == SQLITE_ROW)
    {
        const unsigned char *result = sqlite3_column_text(stmt, 0);
        if (result)
        {
            strncpy(username, (const char *)result, 99);
            username[99] = '\0';
            sqlite3_finalize(stmt);
            return true;
        }
    }
    else if (rc != SQLITE_DONE)
    {
        fprintf(stderr, "查询执行失败: %s\n", sqlite3_errmsg(db->db));
    }

    strncpy(username, "未知用户", 99);
    sqlite3_finalize(stmt);
    return true;
}

/**
 * @brief 通过用户姓名查询用户ID
 *
 * @param db 数据库连接
 * @param name 用户姓名
 * @param user_id 用于存储查询结果的用户ID字符串
 * @return bool 查询成功返回true，失败返回false
 */
bool query_user_id_by_name(Database *db, const char *name, char *user_id)
{
    if (db == NULL || db->db == NULL || name == NULL || user_id == NULL)
    {
        fprintf(stderr, "查询用户ID参数无效\n");
        return false;
    }

    const char *count_query = "SELECT COUNT(*) FROM users WHERE name = ?";
    sqlite3_stmt *count_stmt;
    int total_users = 0;

    int rc = sqlite3_prepare_v2(db->db, count_query, -1, &count_stmt, NULL);
    if (rc != SQLITE_OK)
    {
        fprintf(stderr, "无法准备计数查询语句: %s\n", sqlite3_errmsg(db->db));
        return false;
    }

    rc = sqlite3_bind_text(count_stmt, 1, name, -1, SQLITE_STATIC);
    if (rc != SQLITE_OK)
    {
        fprintf(stderr, "无法绑定用户姓名: %s\n", sqlite3_errmsg(db->db));
        sqlite3_finalize(count_stmt);
        return false;
    }

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

        for (int i = 0; i < total_users; i++)
        {
            user_ids[i] = (char *)malloc(37 * sizeof(char));
            phone_numbers[i] = (char *)malloc(20 * sizeof(char));

            if (user_ids[i] == NULL || phone_numbers[i] == NULL)
            {
                fprintf(stderr, "内存分配失败\n");
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

        int count = 0;
        while ((rc = sqlite3_step(stmt)) == SQLITE_ROW && count < total_users)
        {
            const unsigned char *id_result = sqlite3_column_text(stmt, 0);
            const unsigned char *phone_result = sqlite3_column_text(stmt, 1);

            if (id_result)
            {
                strncpy(user_ids[count], (const char *)id_result, 36);
                user_ids[count][36] = '\0';

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

        printf("找到多个姓名为 \"%s\" 的用户，请选择：\n", name);
        for (int i = 0; i < count; i++)
        {
            char phone_suffix[15] = "未知";
            int phone_len = strlen(phone_numbers[i]);

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

        bool success = false;
        if (choice >= 1 && choice <= count)
        {
            strcpy(user_id, user_ids[choice - 1]);
            success = true;
        }
        else
        {
            fprintf(stderr, "无效的选择\n");
        }

    cleanup:
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

/**
 * @brief 比较函数 - 按ID升序排序
 *
 * @param a 第一个比较对象
 * @param b 第二个比较对象
 * @return int 比较结果：< 0表示a小于b，0表示相等，> 0表示a大于b
 */
int compare_id_asc(const void *a, const void *b)
{
    const Owner *owner_a = (const Owner *)a;
    const Owner *owner_b = (const Owner *)b;
    return strcmp(owner_a->user_id, owner_b->user_id);
}

/**
 * @brief 排序业主列表
 *
 * @param db 数据库连接
 * @param compare_func 用于排序的比较函数
 */
void sort_owners(Database *db, int (*compare_func)(const void *, const void *))
{
    printf("排序业主操作执行中...\n");
}

/**
 * @brief 显示业主列表
 *
 * @param db 数据库连接
 */
void display_owners(Database *db)
{
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

/**
 * @brief 查询所有服务人员
 *
 * @param db 数据库连接
 * @param staff_list 用于存储查询结果的服务人员数组
 * @param max_count 数组最大容量
 * @return int 实际查询到的服务人员数量
 */
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

/**
 * @brief 统计服务人员总数
 *
 * @param db 数据库连接
 * @return int 服务人员总数
 */
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