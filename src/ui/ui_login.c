#include "ui/ui_login.h"
#include "ui/ui_main.h"
#include "utils/utils.h"
#include "models/user.h"
#include "auth/tokens.h"
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
    LoginResult result = {false, "", 0, 0, ""};
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
bool show_change_password_screen(Database *db, const char *token, UserType user_type)
{
    // TODO: 实现修改密码界面功能
    return false;
}

// 重置密码界面 (仅管理员)
bool show_reset_password_screen(Database *db, const char *token)
{
    // TODO: 实现重置密码界面功能
    return false;
}
