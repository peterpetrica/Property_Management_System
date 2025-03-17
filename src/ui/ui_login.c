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
#include "ui/ui_main.h"
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
    // TODO: 实现注册界面功能
    return false;
}

// 修改密码界面
bool show_change_password_screen(Database *db, const char *user_id, UserType user_type)
{
    char old_password[64] = {0};
    char new_password[64] = {0};
    char confirm_password[64] = {0};

    printf("\n===== 修改密码 =====\n");

    printf("请输入当前密码: ");
    read_password(old_password, sizeof(old_password));

    printf("请输入新密码: ");
    read_password(new_password, sizeof(new_password));

    printf("请确认新密码: ");
    read_password(confirm_password, sizeof(confirm_password));

    if (strcmp(new_password, confirm_password) != 0)
    {
        printf("两次输入的新密码不一致！\n");
        return false;
    }

    if (change_password(db, user_id, user_type, old_password, new_password))
    {
        printf("密码修改成功！\n");
        return true;
    }
    else
    {
        printf("密码修改失败，当前密码可能不正确。\n");
        return false;
    }
}

// 重置密码界面 (仅管理员)
bool show_reset_password_screen(Database *db, const char *admin_id, UserType admin_type)
{
    if (admin_type != USER_ADMIN)
    {
        printf("只有管理员可以重置密码！\n");
        return false;
    }

    char user_id[32] = {0};
    int user_type_choice = 0;
    UserType user_type;

    printf("\n===== 重置用户密码 =====\n");

    printf("请输入要重置密码的用户ID: ");
    scanf("%31s", user_id);

    printf("请选择用户类型 (1:管理员, 2:服务人员, 3:业主): ");
    scanf("%d", &user_type_choice);

    switch (user_type_choice)
    {
    case 1:
        user_type = USER_ADMIN;
        break;
    case 2:
        user_type = USER_STAFF;
        break;
    case 3:
        user_type = USER_OWNER;
        break;
    default:
        printf("无效的用户类型！\n");
        return false;
    }

    if (reset_password(db, admin_id, admin_type, user_id, user_type))
    {
        printf("密码重置成功！\n");
        return true;
    }
    else
    {
        printf("密码重置失败！\n");
        return false;
    }
}
