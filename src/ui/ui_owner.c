#include "ui/ui_owner.h"
#include "ui/ui_main.h"
#include "ui/ui_login.h"
#include "models/building.h"
#include "models/apartment.h"
#include "models/user.h"
#include "models/transaction.h"
#include "models/service.h"
#include "db/db_query.h"
#include "utils/utils.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

// 显示业主主界面
void show_owner_main_screen(Database *db, const char *token)
{
    int choice;
    bool exit_flag = false;

    while (!exit_flag)
    {
        system("clear || cls");
        printf("===== 物业管理系统 - 业主主界面 =====\n");
        printf("1. 个人信息管理\n");
        printf("2. 缴费管理\n");
        printf("3. 信息查询\n");
        printf("4. 信息排序\n");
        printf("5. 信息统计\n");
        printf("6. 系统维护\n");
        printf("0. 退出登录\n");
        printf("请选择操作 [0-6]: ");

        scanf("%d", &choice);
        getchar(); // 消耗换行符

        switch (choice)
        {
        case 1:
            show_personal_info_screen(db, token);
            break;
        case 2:
            show_payment_management_screen(db, token);
            break;
        case 3:
            show_owner_query_screen(db, token);
            break;
        case 4:
            show_owner_sort_screen(db, token);
            break;
        case 5:
            show_owner_statistics_screen(db, token);
            break;
        case 6:
            show_owner_maintenance_screen(db, token);
            break;
        case 0:
            exit_flag = true;
            break;
        default:
            show_error("无效的选择");
        }
    }
}

// 个人信息管理界面
void show_personal_info_screen(Database *db, const char *token)
{
    int choice;
    bool exit_flag = false;

    while (!exit_flag)
    {
        system("clear || cls");
        printf("===== 个人信息管理 =====\n");
        printf("1. 查看个人信息\n");
        printf("2. 修改联系电话\n");
        printf("3. 设置缴费提醒\n");
        printf("0. 返回上级菜单\n");
        printf("请选择操作 [0-3]: ");

        scanf("%d", &choice);
        getchar(); // 消耗换行符

        switch (choice)
        {
        case 0:
            exit_flag = true;
            break;
        case 1:
            // TODO: 查看个人信息功能
            printf("查看个人信息功能待实现\n");
            printf("按任意键继续...");
            getchar();
            break;
        case 2:
            // TODO: 修改联系电话功能
            printf("修改联系电话功能待实现\n");
            printf("按任意键继续...");
            getchar();
            break;
        case 3:
            // TODO: 设置缴费提醒功能
            printf("设置缴费提醒功能待实现\n");
            printf("按任意键继续...");
            getchar();
            break;
        default:
            show_error("无效的选择");
        }
    }
}

// 缴费管理界面
void show_payment_management_screen(Database *db, const char *token)
{
    int choice;
    bool exit_flag = false;

    while (!exit_flag)
    {
        system("clear || cls");
        printf("===== 缴费管理 =====\n");
        printf("1. 查看应缴费用\n");
        printf("2. 缴费\n");
        printf("3. 查看缴费记录\n");
        printf("0. 返回上级菜单\n");
        printf("请选择操作 [0-3]: ");

        scanf("%d", &choice);
        getchar(); // 消耗换行符

        switch (choice)
        {
        case 0:
            exit_flag = true;
            break;
        case 1:
            // TODO: 查看应缴费用功能
            printf("查看应缴费用功能待实现\n");
            printf("按任意键继续...");
            getchar();
            break;
        case 2:
            // TODO: 缴费功能
            printf("缴费功能待实现\n");
            printf("按任意键继续...");
            getchar();
            break;
        case 3:
            // TODO: 查看缴费记录功能
            printf("查看缴费记录功能待实现\n");
            printf("按任意键继续...");
            getchar();
            break;
        default:
            show_error("无效的选择");
        }
    }
}

// 业主信息查询界面
void show_owner_query_screen(Database *db, const char *token)
{
    int choice;
    bool exit_flag = false;

    while (!exit_flag)
    {
        system("clear || cls");
        printf("===== 信息查询 =====\n");
        printf("1. 查询我的房屋信息\n");
        printf("2. 查询物业费标准\n");
        printf("3. 查询服务我的物业人员\n");
        printf("4. 查询小区楼宇信息\n");
        printf("0. 返回上级菜单\n");
        printf("请选择操作 [0-4]: ");

        scanf("%d", &choice);
        getchar(); // 消耗换行符

        switch (choice)
        {
        case 0:
            exit_flag = true;
            break;
        case 1:
            // TODO: 查询房屋信息功能
            printf("查询房屋信息功能待实现\n");
            printf("按任意键继续...");
            getchar();
            break;
        case 2:
            // TODO: 查询物业费标准功能
            printf("查询物业费标准功能待实现\n");
            printf("按任意键继续...");
            getchar();
            break;
        case 3:
            // TODO: 查询服务的物业人员功能
            printf("查询服务人员功能待实现\n");
            printf("按任意键继续...");
            getchar();
            break;
        case 4:
            // TODO: 查询小区楼宇信息功能
            printf("查询小区楼宇信息功能待实现\n");
            printf("按任意键继续...");
            getchar();
            break;
        default:
            show_error("无效的选择");
        }
    }
}

// 业主信息排序界面
void show_owner_sort_screen(Database *db, const char *token)
{
    int choice;
    bool exit_flag = false;

    while (!exit_flag)
    {
        system("clear || cls");
        printf("===== 信息排序 =====\n");
        printf("1. 按缴费日期排序\n");
        printf("2. 按缴费金额排序\n");
        printf("3. 按缴费类型排序\n");
        printf("0. 返回上级菜单\n");
        printf("请选择操作 [0-3]: ");

        scanf("%d", &choice);
        getchar(); // 消耗换行符

        switch (choice)
        {
        case 0:
            exit_flag = true;
            break;
        case 1:
            // TODO: 按缴费日期排序功能
            printf("按缴费日期排序功能待实现\n");
            printf("按任意键继续...");
            getchar();
            break;
        case 2:
            // TODO: 按缴费金额排序功能
            printf("按缴费金额排序功能待实现\n");
            printf("按任意键继续...");
            getchar();
            break;
        case 3:
            // TODO: 按缴费类型排序功能
            printf("按缴费类型排序功能待实现\n");
            printf("按任意键继续...");
            getchar();
            break;
        default:
            show_error("无效的选择");
        }
    }
}

// 业主信息统计界面
void show_owner_statistics_screen(Database *db, const char *token)
{
    int choice;
    bool exit_flag = false;

    while (!exit_flag)
    {
        system("clear || cls");
        printf("===== 信息统计 =====\n");
        printf("1. 统计总缴费金额\n");
        printf("2. 统计年度缴费情况\n");
        printf("3. 统计各类型缴费金额\n");
        printf("0. 返回上级菜单\n");
        printf("请选择操作 [0-3]: ");

        scanf("%d", &choice);
        getchar(); // 消耗换行符

        switch (choice)
        {
        case 0:
            exit_flag = true;
            break;
        case 1:
            // TODO: 统计总缴费金额功能
            printf("统计总缴费金额功能待实现\n");
            printf("按任意键继续...");
            getchar();
            break;
        case 2:
            // TODO: 统计年度缴费情况功能
            printf("统计年度缴费情况功能待实现\n");
            printf("按任意键继续...");
            getchar();
            break;
        case 3:
            // TODO: 统计各类型缴费金额功能
            printf("统计各类型缴费金额功能待实现\n");
            printf("按任意键继续...");
            getchar();
            break;
        default:
            show_error("无效的选择");
        }
    }
}

// 业主系统维护界面
void show_owner_maintenance_screen(Database *db, const char *token)
{
    int choice;
    bool exit_flag = false;

    while (!exit_flag)
    {
        system("clear || cls");
        printf("===== 系统维护 =====\n");
        printf("1. 修改密码\n");
        printf("0. 返回上级菜单\n");
        printf("请选择操作 [0-1]: ");

        scanf("%d", &choice);
        getchar(); // 消耗换行符

        switch (choice)
        {
        case 0:
            exit_flag = true;
            break;
        case 1:
            show_change_password_screen(db, token, USER_OWNER);
            break;
        default:
            show_error("无效的选择");
        }
    }
}
