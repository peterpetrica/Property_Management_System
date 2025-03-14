#include "ui/ui_staff.h"
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

// 显示服务人员主界面
void show_staff_main_screen(Database *db, const char *token)
{
    int choice;
    bool exit_flag = false;

    while (!exit_flag)
    {
        system("clear || cls");
        printf("===== 物业管理系统 - 服务人员主界面 =====\n");
        printf("1. 物业服务管理\n");
        printf("2. 信息查询\n");
        printf("3. 信息排序\n");
        printf("4. 信息统计\n");
        printf("5. 系统维护\n");
        printf("0. 退出登录\n");
        printf("请选择操作 [0-5]: ");

        scanf("%d", &choice);
        getchar(); // 消耗换行符

        switch (choice)
        {
        case 1:
            show_service_management_screen(db, token);
            break;
        case 2:
            show_staff_query_screen(db, token);
            break;
        case 3:
            show_staff_sort_screen(db, token);
            break;
        case 4:
            show_staff_statistics_screen(db, token);
            break;
        case 5:
            show_staff_maintenance_screen(db, token);
            break;
        case 0:
            exit_flag = true;
            break;
        default:
            show_error("无效的选择");
        }
    }
}

// 物业服务管理界面
void show_service_management_screen(Database *db, const char *token)
{
    int choice;
    bool exit_flag = false;

    while (!exit_flag)
    {
        system("clear || cls");
        printf("===== 物业服务管理 =====\n");
        printf("1. 添加服务记录\n");
        printf("2. 查看我的服务记录\n");
        printf("3. 发送缴费提醒\n");
        printf("4. 查看负责区域\n");
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
            // TODO: 添加服务记录功能
            printf("添加服务记录功能待实现\n");
            printf("按任意键继续...");
            getchar();
            break;
        case 2:
            // TODO: 查看服务记录功能
            printf("查看服务记录功能待实现\n");
            printf("按任意键继续...");
            getchar();
            break;
        case 3:
            // TODO: 发送缴费提醒功能
            printf("发送缴费提醒功能待实现\n");
            printf("按任意键继续...");
            getchar();
            break;
        case 4:
            // TODO: 查看负责区域功能
            printf("查看负责区域功能待实现\n");
            printf("按任意键继续...");
            getchar();
            break;
        default:
            show_error("无效的选择");
        }
    }
}

// 服务人员信息查询界面
void show_staff_query_screen(Database *db, const char *token)
{
    int choice;
    bool exit_flag = false;

    while (!exit_flag)
    {
        system("clear || cls");
        printf("===== 信息查询 =====\n");
        printf("1. 查询负责区域的业主\n");
        printf("2. 查询负责区域的房屋\n");
        printf("3. 查询业主缴费情况\n");
        printf("4. 查询服务记录\n");
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
            // TODO: 查询负责区域的业主功能
            printf("查询负责区域的业主功能待实现\n");
            printf("按任意键继续...");
            getchar();
            break;
        case 2:
            // TODO: 查询负责区域的房屋功能
            printf("查询负责区域的房屋功能待实现\n");
            printf("按任意键继续...");
            getchar();
            break;
        case 3:
            // TODO: 查询业主缴费情况功能
            printf("查询业主缴费情况功能待实现\n");
            printf("按任意键继续...");
            getchar();
            break;
        case 4:
            // TODO: 查询服务记录功能
            printf("查询服务记录功能待实现\n");
            printf("按任意键继续...");
            getchar();
            break;
        default:
            show_error("无效的选择");
        }
    }
}

// 服务人员信息排序界面
void show_staff_sort_screen(Database *db, const char *token)
{
    int choice;
    bool exit_flag = false;

    while (!exit_flag)
    {
        system("clear || cls");
        printf("===== 信息排序 =====\n");
        printf("1. 按业主姓名排序\n");
        printf("2. 按房屋编号排序\n");
        printf("3. 按缴费情况排序\n");
        printf("4. 按服务时间排序\n");
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
            // TODO: 按业主姓名排序功能
            printf("按业主姓名排序功能待实现\n");
            printf("按任意键继续...");
            getchar();
            break;
        case 2:
            // TODO: 按房屋编号排序功能
            printf("按房屋编号排序功能待实现\n");
            printf("按任意键继续...");
            getchar();
            break;
        case 3:
            // TODO: 按缴费情况排序功能
            printf("按缴费情况排序功能待实现\n");
            printf("按任意键继续...");
            getchar();
            break;
        case 4:
            // TODO: 按服务时间排序功能
            printf("按服务时间排序功能待实现\n");
            printf("按任意键继续...");
            getchar();
            break;
        default:
            show_error("无效的选择");
        }
    }
}

// 服务人员信息统计界面
void show_staff_statistics_screen(Database *db, const char *token)
{
    int choice;
    bool exit_flag = false;

    while (!exit_flag)
    {
        system("clear || cls");
        printf("===== 信息统计 =====\n");
        printf("1. 统计负责区域的业主数量\n");
        printf("2. 统计负责区域的房屋数量\n");
        printf("3. 统计缴费业主数量\n");
        printf("4. 统计未缴费业主数量\n");
        printf("5. 统计服务记录数量\n");
        printf("0. 返回上级菜单\n");
        printf("请选择操作 [0-5]: ");

        scanf("%d", &choice);
        getchar(); // 消耗换行符

        switch (choice)
        {
        case 0:
            exit_flag = true;
            break;
        case 1:
            // TODO: 统计负责区域的业主数量功能
            printf("统计负责区域的业主数量功能待实现\n");
            printf("按任意键继续...");
            getchar();
            break;
        case 2:
            // TODO: 统计负责区域的房屋数量功能
            printf("统计负责区域的房屋数量功能待实现\n");
            printf("按任意键继续...");
            getchar();
            break;
        case 3:
            // TODO: 统计缴费业主数量功能
            printf("统计缴费业主数量功能待实现\n");
            printf("按任意键继续...");
            getchar();
            break;
        case 4:
            // TODO: 统计未缴费业主数量功能
            printf("统计未缴费业主数量功能待实现\n");
            printf("按任意键继续...");
            getchar();
            break;
        case 5:
            // TODO: 统计服务记录数量功能
            printf("统计服务记录数量功能待实现\n");
            printf("按任意键继续...");
            getchar();
            break;
        default:
            show_error("无效的选择");
        }
    }
}

// 服务人员系统维护界面
void show_staff_maintenance_screen(Database *db, const char *token)
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
            show_change_password_screen(db, token, USER_STAFF);
            break;
        default:
            show_error("无效的选择");
        }
    }
}
