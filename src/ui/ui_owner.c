/*
 * 文件: ui_owner.c
 * 功能: 实现物业管理系统中业主用户界面的各种功能
 *
 * 该文件包含业主界面的所有UI组件和交互逻辑，包括：
 * - 业主主界面
 * - 个人信息管理
 * - 缴费管理
 * - 业主信息查询
 * - 业主信息排序
 * - 业主信息统计
 * - 系统维护等功能
 *
 * 每个界面函数接收数据库连接和用户令牌作为参数，用于身份验证和数据操作
 */

#include "ui/ui_owner.h"
#include "ui/ui_login.h"
#include "models/building.h"
#include "models/apartment.h"
#include "models/user.h"
#include "models/transaction.h"
#include "models/service.h"
#include "db/db_query.h"
#include "utils/utils.h"
#include "utils/console.h"
#include "auth/auth.h" // 确保包含auth.h
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
// 函数声明
bool query_username_by_user_id(Database *db, const char *user_id, char *username);

// 清除输入缓冲区
void clear_input_buffer()
{
    int c;
    while ((c = getchar()) != '\n' && c != EOF)
        ;
}

// 显示缴费记录
void show_payment_history(Database *db, const char *user_id)
{
    printf("显示缴费记录功能待实现\n");
    // TODO: 实现显示缴费记录的功能
}

// 处理缴费流程
void process_payment_screen(Database *db, const char *user_id)
{
    printf("缴费流程功能待实现\n");
    // TODO: 实现缴费流程的功能
}

// 查询应缴费用
void query_due_amount(Database *db, const char *user_id)
{
    printf("查询应缴费用功能待实现\n");
    // TODO: 实现查询应缴费用的功能
}

// 查询剩余费用
void query_remaining_balance(Database *db, const char *user_id)
{
    printf("查询剩余费用功能待实现\n");
    // TODO: 实现查询剩余费用的功能
}

// 查询小区基本信息
void query_community_info(Database *db)
{
    printf("查询小区基本信息功能待实现\n");
    // TODO: 实现查询小区基本信息的功能
}

// 查询收费信息
void query_fee_info(Database *db, const char *user_id)
{
    printf("查询收费信息功能待实现\n");
    // TODO: 实现查询收费信息的功能
}

// 查询服务人员信息
void query_service_staff_info(Database *db, const char *user_id)
{
    printf("查询服务人员信息功能待实现\n");
    // TODO: 实现查询服务人员信息的功能
}

// 修改用户名
bool change_username(Database *db, const char *user_id, char *username)
{
    if (db == NULL || user_id == NULL || username == NULL)
    {
        fprintf(stderr, "无效的数据库或输入参数\n");
        return false;
    }
const char* sql = "UPDATE users SET username = ? WHERE id = ?;";
sqlite3_stmt* stmt;
int rc;
// 预处理 SQL 语句
rc = sqlite3_prepare_v2(db->db, sql, -1, &stmt, 0);
if (rc != SQLITE_OK) {
    fprintf(stderr, "SQL 预处理失败: %s\n", sqlite3_errmsg(db->db));
    return false;
}
// 绑定参数
sqlite3_bind_text(stmt, 1, username, -1, SQLITE_STATIC);
sqlite3_bind_text(stmt, 2, user_id, -1, SQLITE_STATIC);
// 执行 SQL 语句
rc = sqlite3_step(stmt);
if (rc != SQLITE_DONE) {
    fprintf(stderr, "用户名更新失败: %s\n", sqlite3_errmsg(db->db));
    sqlite3_finalize(stmt);
    return false;
}
// 释放资源
sqlite3_finalize(stmt);
printf("用户名修改成功\n");
return true;
}

// 修改密码
void handle_change_password(Database *db, const char *user_id, UserType user_type)
{
    char old_password[100];
    char new_password[100];
    printf("请输入旧密码: ");
    read_password(old_password, sizeof(old_password));
    printf("请输入新密码: ");
    read_password(new_password, sizeof(new_password));

    if (change_password(db, user_id, user_type, old_password, new_password))
    {
        printf("密码修改成功!\n");
    }
    else
    {
        printf("密码修改失败，请确认旧密码是否正确。\n");
    }
}

// 主界面
void main_screen_owner(Database *db, const char *user_id, UserType user_type)
{
    char username[100];
    query_username_by_user_id(db, user_id, username);
    printf("您好，%s\n", username);

    int number;
    while (1)
    {
        printf("请输入你要进入的页面\n");
        printf("1--更改信息     2--缴费管理      3--信息查询    4--信息排序     0--退出\n");
        scanf("%d", &number);
        if (number == 0)
        {
            printf("退出系统\n");
            break;
        }

        if (number == 1)
        {
            show_personal_info_screen(db, user_id, user_type);
        }
        else if (number == 2)
        {
            show_payment_management_screen(db, user_id, user_type);
        }
        else if (number == 3)
        {
            show_owner_query_screen(db, user_id, user_type);
        }
        else if (number == 4)
        {
            show_owner_sort_screen(db, user_id, user_type);
        }
        else
        {
            printf("输入的数据错误，请重新输入\n");
        }
    }
}

// 个人信息管理界面
void show_personal_info_screen(Database *db, const char *user_id, UserType user_type)
{
    char username[100];
    query_username_by_user_id(db, user_id, username);

    printf("更改您的用户名---1    更改您的密码---2    退出---0\n");
    int num;
    while (1)
    {
        scanf("%d", &num);
        if (num == 1)
        {
            if (change_username(db, user_id, username))
            {
                printf("用户名更改成功\n");
            }
            else
            {
                printf("用户名修改失败，请重试。\n");
            }
        }
        else if (num == 2)
        {
            handle_change_password(db, user_id, user_type);
        }
        else if (num == 0)
        {
            printf("退出个人信息管理\n");
            break;
        }
        else
        {
            printf("输入错误，请重新输入。\n");
        }
    }
}

// 缴费管理界面
void show_payment_management_screen(Database *db, const char *user_id, UserType user_type)
{
    int choice;
    while (1)
    {
        printf("\n===== 缴费管理 =====\n");
        printf("1--查看缴费记录\n");
        printf("2--缴纳费用\n");
        printf("3--查询应缴费用\n");
        printf("4--查询剩余费用\n");
        printf("0--返回上一级\n");
        printf("请输入您的选择: ");
        if (scanf("%d", &choice) != 1)
        {
            clear_input_buffer(); // 缓冲区
            continue;
        }
        if (choice == 1)
        {
            show_payment_history(db, user_id);
        } // 显示缴费记录 }
        else if (choice == 2)
        {
            process_payment_screen(db, user_id);
        } // 进入缴费流程 }
        else if (choice == 3)
        {
            query_due_amount(db, user_id);
        } // 查询应缴费用
        else if (choice == 4)
        {
            query_remaining_balance(db, user_id);
        } // 查询剩余费用 }
        else if (choice == 0)
        {
            printf("返回上一级菜单。\n");
            break;
        }
        else
        {
            printf("输入错误，请重新输入。\n");
        }
    }
}

// 业主信息查询界面
void show_owner_query_screen(Database *db, const char *user_id, UserType user_type)
{
    int choice;
    while (1)
    {
        printf("\n===== 业主信息查询 =====\n");
        printf("1--查询小区基本信息\n");
        printf("2--查询收费信息\n");
        printf("3--查询为您服务的工作人员信息\n");
        printf("0--返回上一级\n");
        printf("请输入您的选择: ");

        if (scanf("%d", &choice) != 1)
        {
            clear_input_buffer(); // 临时存储
            printf("输入无效，请重新输入。\n");
            continue;
        }

        // 根据用户的选择使用 switch-case 进行处理
        switch (choice)
        {
        case 1:
            // 用户选择查询小区基本信息
            {
                query_community_info(db);
                break;
            } // 退出当前 case，继续循环显示菜单

        case 2:
            // 用户选择查询收费信息，传入 user_id 以便获取针对该业主的收费详情
            {
                query_fee_info(db, user_id);
                break;
            }

        case 3:
            // 用户选择查询为其服务的工作人员信息
            {
                query_service_staff_info(db, user_id);
                break;
            }

        case 0:
            // 用户选择返回上一级菜单
            {
                printf("返回上一级菜单。\n");
                return;
            } // 直接退出函数

        default:
            // 如果输入的选项不在预期内，则提示错误
            {
                printf("输入错误，请重新输入。\n");
                break;
            }
        }
    }
}

// 业主信息排序界面
// //需要的函数
// 按 ID 升序排序的比较函数
int compare_id_asc(const void* a, const void* b)
{
    Owner* ownerA = *(Owner**)a;
    Owner* ownerB = *(Owner**)b;
    return ownerA->user_id - ownerB->user_id;
}

// 按 ID 降序排序的比较函数
int compare_id_desc(const void* a, const void* b)
{
    Owner* ownerA = *(Owner**)a;
    Owner* ownerB = *(Owner**)b;
    return ownerB->user_id - ownerA->user_id;
}

// 按姓名升序排序的比较函数
int compare_name_asc(const void* a, const void* b)
{
    Owner* ownerA = *(Owner**)a;
    Owner* ownerB = *(Owner**)b;
    return strcmp(ownerA->name, ownerB->name);
}

// 按姓名降序排序的比较函数
int compare_name_desc(const void* a, const void* b)
{
    Owner* ownerA = *(Owner**)a;
    Owner* ownerB = *(Owner**)b;
    return strcmp(ownerB->name, ownerA->name);
}
void show_owner_sort_screen(Database* db, const char* user_id, UserType user_type)
{
    int choice;
    while (1)
    {
        printf("\n===== 业主信息排序 =====\n");
        printf("1--按 ID 升序\n");
        printf("2--按 ID 降序\n");
        printf("3--按姓名升序\n");
        printf("4--按姓名降序\n");
        printf("0--返回上一级\n");
        printf("请输入您的选择: ");

        if (scanf("%d", &choice) != 1)
        {
            clear_input_buffer();
            printf("输入无效，请重新输入。\n");
            continue;
        }
        // 根据用户的选择进行处理
        switch (choice)
        {
        case 1:
            // 按 ID 升序排序
        {
            sort_owners(db, compare_id_asc);
            display_owners(db);
            break;
        }

        case 2:
            // 按 ID 降序排序
        {
            sort_owners(db, compare_id_desc);
            display_owners(db);
            break;
        }

        case 3:
            // 按姓名升序排序
        {
            sort_owners(db, compare_name_asc);
            display_owners(db);
            break;
        }

        case 4:
            // 按姓名降序排序
        {
            sort_owners(db, compare_name_desc);
            display_owners(db);
            break;
        }

        case 0:
            // 返回上一级菜单
        {
            printf("返回上一级菜单。\n");
            return;
        }

        }
    }
}
    // TODO: 实现业主信息排序界面显示和操作逻辑
// 业主信息统计界面
    void show_owner_statistics_screen(Database * db, const char* user_id, UserType user_type)
{
    // TODO: 实现业主信息统计界面显示和操作逻辑
}

// 业主系统维护界面
void show_owner_maintenance_screen(Database * db, const char* user_id, UserType user_type)
{
    // TODO: 实现业主系统维护界面显示和操作逻辑
}

// 兼容旧接口
void show_owner_main_screen(Database * db, const char* user_id, UserType user_type)
{
    main_screen_owner(db, user_id, user_type);
}
