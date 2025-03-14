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
#include <termios.h>
#include <unistd.h>

// Linux平台下的getch实现
static int getch()
{
    struct termios oldt, newt;
    int ch;
    tcgetattr(STDIN_FILENO, &oldt);
    newt = oldt;
    newt.c_lflag &= ~(ICANON | ECHO);
    tcsetattr(STDIN_FILENO, TCSANOW, &newt);
    ch = getchar();
    tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
    return ch;
}
#endif

// 读取密码（不显示输入的字符）
static void read_password(char *password, size_t size)
{
    int i = 0;
    char ch;

    while (i < size - 1 && (ch = getch()) != '\n' && ch != '\r')
    {
        if (ch == 127 || ch == 8)
        { // Backspace
            if (i > 0)
            {
                i--;
                printf("\b \b");
            }
        }
        else
        {
            password[i++] = ch;
            printf("*");
        }
    }

    password[i] = '\0';
    printf("\n");
}

// 显示登录界面
LoginResult show_login_screen(Database *db)
{
    LoginResult result = {false};
    char username[64] = {0};
    char password[64] = {0};
    int choice;

    while (true)
    {
        // 清屏
        system("clear || cls");

        printf("===== 物业管理系统 =====\n");
        printf("1. 登录\n");
        printf("2. 注册 (仅限业主)\n");
        printf("0. 退出\n");
        printf("请选择操作 [0-2]: ");

        scanf("%d", &choice);
        getchar(); // 消耗换行符

        switch (choice)
        {
        case 1:
        {
            printf("\n=== 用户登录 ===\n");
            printf("请输入用户名: ");
            fgets(username, sizeof(username), stdin);
            username[strcspn(username, "\n")] = '\0'; // 移除换行符

            printf("请输入密码: ");
            read_password(password, sizeof(password));

            // 尝试登录
            result = authenticate_user(db, username, password);

            if (result.success)
            {
                printf("\n登录成功！欢迎 %s\n", username);
                printf("按任意键继续...");
                getch();
                return result;
            }
            else
            {
                printf("\n登录失败，用户名或密码错误\n");
                printf("按任意键继续...");
                getch();
            }
            break;
        }
        case 2:
        {
            if (show_registration_screen(db))
            {
                printf("\n注册成功，请登录\n");
            }
            else
            {
                printf("\n注册失败\n");
            }
            printf("按任意键继续...");
            getch();
            break;
        }
        case 0:
            return result; // 返回失败的登录结果
        default:
            printf("\n无效的选择\n");
            printf("按任意键继续...");
            getch();
        }
    }
}

// 显示注册界面
bool show_registration_screen(Database *db)
{
    Owner owner = {0};
    char password[64] = {0};
    char password_confirm[64] = {0};

    system("clear || cls");
    printf("===== 业主注册 =====\n");

    printf("请输入姓名: ");
    fgets(owner.name, sizeof(owner.name), stdin);
    owner.name[strcspn(owner.name, "\n")] = '\0'; // 移除换行符

    printf("请输入联系电话: ");
    fgets(owner.phone, sizeof(owner.phone), stdin);
    owner.phone[strcspn(owner.phone, "\n")] = '\0'; // 移除换行符

    printf("请输入密码: ");
    read_password(password, sizeof(password));

    printf("请确认密码: ");
    read_password(password_confirm, sizeof(password_confirm));

    if (strcmp(password, password_confirm) != 0)
    {
        show_error("两次输入的密码不一致");
        return false;
    }

    // 需要缴费提醒？
    char need_notification[8] = {0};
    printf("是否需要缴费提醒？(y/n): ");
    fgets(need_notification, sizeof(need_notification), stdin);
    need_notification[strcspn(need_notification, "\n")] = '\0'; // 移除换行符

    owner.notification_required = (need_notification[0] == 'y' || need_notification[0] == 'Y');

    // 生成业主ID
    char *uuid = generate_uuid();
    if (!uuid)
    {
        show_error("生成用户ID失败");
        return false;
    }

    strncpy(owner.id, uuid, sizeof(owner.id) - 1);
    free(uuid);

    // 设置权重
    owner.weight = 3; // 业主权重

    // 创建业主账户
    bool success = create_owner(db, &owner, password);

    return success;
}

// 修改密码界面
bool show_change_password_screen(Database *db, const char *token, UserType user_type)
{
    char user_id[32] = {0};
    char old_password[64] = {0};
    char new_password[64] = {0};
    char confirm_password[64] = {0};

    system("clear || cls");
    printf("===== 修改密码 =====\n");

    // 从令牌获取用户ID
    if (!get_user_id_from_token(db, token, user_id, sizeof(user_id)))
    {
        show_error("无效的用户令牌");
        return false;
    }

    printf("请输入当前密码: ");
    read_password(old_password, sizeof(old_password));

    printf("请输入新密码: ");
    read_password(new_password, sizeof(new_password));

    printf("请确认新密码: ");
    read_password(confirm_password, sizeof(confirm_password));

    if (strcmp(new_password, confirm_password) != 0)
    {
        show_error("两次输入的新密码不一致");
        return false;
    }

    // 更改密码
    bool success = change_password(db, user_id, user_type, old_password, new_password);

    if (success)
    {
        show_success("密码修改成功");
    }
    else
    {
        show_error("密码修改失败，可能是当前密码不正确");
    }

    return success;
}

// 重置密码界面 (仅管理员)
bool show_reset_password_screen(Database *db, const char *token)
{
    char user_id[32] = {0};
    UserType user_type;
    int type_choice;

    system("clear || cls");
    printf("===== 重置用户密码 =====\n");

    printf("1. 重置物业服务人员密码\n");
    printf("2. 重置业主密码\n");
    printf("请选择用户类型 [1-2]: ");

    scanf("%d", &type_choice);
    getchar(); // 消耗换行符

    if (type_choice == 1)
    {
        user_type = USER_STAFF;
        printf("\n=== 重置物业服务人员密码 ===\n");
    }
    else if (type_choice == 2)
    {
        user_type = USER_OWNER;
        printf("\n=== 重置业主密码 ===\n");
    }
    else
    {
        show_error("无效的选择");
        return false;
    }

    printf("请输入用户ID: ");
    fgets(user_id, sizeof(user_id), stdin);
    user_id[strcspn(user_id, "\n")] = '\0'; // 移除换行符

    // 重置密码
    if (reset_password(db, token, user_id, user_type))
    {
        show_success("密码已重置为默认密码: 123456");
        return true;
    }
    else
    {
        show_error("密码重置失败，可能是用户ID不存在或您没有权限");
        return false;
    }
}
