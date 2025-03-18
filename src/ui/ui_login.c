/**
 * ui_login.c
 * 用户登录界面实现
 *
 * 本文件实现了物业管理系统的登录界面相关功能，包括：
 * - 用户登录验证
 * - 新用户注册
 * - 修改用户密码
 * - 管理员重置密码
 */

#include "ui/ui_login.h"
#include "utils/utils.h"
#include "models/user.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#ifdef _WIN32
#include <conio.h>
#else
#include "utils/console.h"
#include <termios.h>
#include <unistd.h>
#endif

// 登录界面
LoginResult show_login_screen(Database *db)
{
    LoginResult result = {false, 0, 0, ""};
    char username[64] = {0};
    char password[64] = {0};
    int choice;

    while (true)
    {
        printf("\n===== 物业管理系统登录 =====\n");
        printf("1. 登录\n");
        printf("2. 注册\n");
        printf("0. 退出\n");
        printf("请选择操作: ");

        if (scanf("%d", &choice) != 1)
        {
            // 处理非数字输入
            scanf("%*[^\n]");
            scanf("%*c");
            printf("无效的输入，请重新选择\n");
            continue;
        }

        switch (choice)
        {
        case 0:
            printf("谢谢使用，再见！\n");
            return result;

        case 1:
            printf("\n===== 用户登录 =====\n");
            printf("用户名: ");
            scanf("%63s", username);

            printf("密码: ");
            // 清空输入缓冲区
            while (getchar() != '\n')
                ;
            read_password(password, sizeof(password));

            // 调用身份验证函数
            result = authenticate_user(db, username, password);

            if (result.success)
            {
                printf("\n登录成功！欢迎 %s\n", username);
                return result;
            }
            else
            {
                printf("\n登录失败：用户名或密码错误\n");
            }
            break;

        case 2:
            if (show_registration_screen(db))
            {
                printf("\n注册成功！请使用新账户登录\n");
            }
            else
            {
                printf("\n注册失败\n");
            }
            break;

        default:
            printf("无效的选择，请重新输入\n");
        }
    }
}

// 显示注册界面
bool show_registration_screen(Database *db)
{
    char username[64] = {0};
    char password[64] = {0};
    char confirm_password[64] = {0};
    char name[64] = {0};
    char phone[20] = {0};
    char email[64] = {0};
    char user_id[37] = {0}; // UUID字符串长度为36+结束符
    char query[512] = {0};
    char error_message[128] = {0};
    sqlite3_stmt *stmt = NULL;
    bool success = false;
    time_t current_time;

    printf("\n===== 用户注册 =====\n");

    // 输入用户名
    printf("用户名 (必填): ");
    scanf("%63s", username);

    // 检查用户名是否已存在
    sprintf(query, "SELECT COUNT(*) FROM users WHERE username = ?;");

    if (db_prepare(db, query, &stmt) != SQLITE_OK)
    {
        printf("数据库错误: %s\n", sqlite3_errmsg(db->db));
        return false;
    }

    sqlite3_bind_text(stmt, 1, username, -1, SQLITE_STATIC);

    if (sqlite3_step(stmt) == SQLITE_ROW)
    {
        int count = sqlite3_column_int(stmt, 0);
        if (count > 0)
        {
            printf("用户名已存在，请选择其他用户名\n");
            sqlite3_finalize(stmt);
            return false;
        }
    }

    sqlite3_finalize(stmt);
    stmt = NULL;

    // 输入密码
    printf("密码 (必填): ");
    while (getchar() != '\n')
        ; // 清空输入缓冲区
    read_password(password, sizeof(password));

    printf("确认密码: ");
    read_password(confirm_password, sizeof(confirm_password));

    // 检查密码一致性
    if (strcmp(password, confirm_password) != 0)
    {
        printf("两次输入的密码不匹配\n");
        return false;
    }

    // 输入个人信息
    printf("真实姓名 (必填): ");
    scanf("%63s", name);

    printf("电话号码 (选填): ");
    scanf("%19s", phone);

    printf("电子邮箱 (选填): ");
    scanf("%63s", email);

    // 生成唯一用户ID
    generate_uuid(user_id);

    // 获取当前时间戳
    time(&current_time);

    // 插入新用户记录 (默认为业主角色)
    sprintf(query,
            "INSERT INTO users (user_id, username, password_hash, name, phone_number, email, role_id, registration_date) "
            "VALUES (?, ?, ?, ?, ?, ?, 'role_owner', ?);");

    if (db_prepare(db, query, &stmt) != SQLITE_OK)
    {
        printf("数据库错误: %s\n", sqlite3_errmsg(db->db));
        return false;
    }

    sqlite3_bind_text(stmt, 1, user_id, -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 2, username, -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 3, password, -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 4, name, -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 5, phone, -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 6, email, -1, SQLITE_STATIC);
    sqlite3_bind_int64(stmt, 7, current_time);

    if (sqlite3_step(stmt) == SQLITE_DONE)
    {
        success = true;
    }
    else
    {
        strcpy(error_message, sqlite3_errmsg(db->db));
        printf("注册失败: %s\n", error_message);
    }

    sqlite3_finalize(stmt);
    return success;
}
