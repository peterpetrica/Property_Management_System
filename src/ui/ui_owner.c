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
#include "auth/auth.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

bool query_username_by_user_id(Database *db, const char *user_id, char *username);
void query_due_payments(Database *db, const char *user_id);
bool update_overdue_transactions(Database *db);

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
    system("cls");
    printf("=====缴费记录=====\n");
    const char *query = "SELECT * FROM transactions WHERE user_id=?;";
    sqlite3_stmt *stmt;
    int rc = sqlite3_prepare_v2(db->db, query, -1, &stmt, NULL);
    if (rc != SQLITE_OK)
    {
        printf("SQLite错误: %s\n", sqlite3_errmsg(db->db));
        return;
    }
    sqlite3_bind_text(stmt, 1, user_id, -1, SQLITE_STATIC);
    printf("交易ID\t\t金额\t\t日期\n");
    while ((rc = sqlite3_step(stmt)) == SQLITE_ROW)
    {
        const char *transaction_id = sqlite3_column_text(stmt, 0);
        double amount = sqlite3_column_double(stmt, 1);
        const char *date = sqlite3_column_text(stmt, 2);
        printf("%s\t%.2f\t%s\n", transaction_id, amount, date);
    }
    sqlite3_finalize(stmt);
    printf("按任意键返回...\n");
    clear_input_buffer();
}

// 处理缴费流程
void process_payment_screen(Database *db, const char *user_id)
{
    system("cls");
    printf("=====缴费流程=====\n");
    double amount;
    printf("请输入缴费金额: ");
    if (scanf("%lf", &amount) != 1)
    {
        printf("输入错误，请重试\n");
        clear_input_buffer();
        return;
    }
    const char *query = "INSERT INTO transactions(transaction_id,amount,date) VALUES(?,?,?);";
    sqlite3_stmt *stmt;
    int rc = sqlite3_prepare_v2(db->db, query, -1, &stmt, NULL);
    if (rc != SQLITE_OK)
    {
        fprintf(stderr, "无法准备插入语句: %s\n", sqlite3_errmsg(db->db));
        return;
    }
    sqlite3_bind_text(stmt, 1, generate_transaction_id(), -1, SQLITE_STATIC);
    sqlite3_bind_double(stmt, 2, amount);
    if (sqlite3_step(stmt) != SQLITE_DONE)
    {
        fprintf(stderr, "缴费失败: %s\n", sqlite3_errmsg(db->db));
    }
    else
    {
        printf("缴费成功！金额: %.2f\n", amount);
    }
    sqlite3_finalize(stmt);
    printf("按任意键返回...\n");
    clear_input_buffer();
}

// 查询应缴费用
void query_due_amount(Database *db, const char *user_id)
{
    system("cls");
    printf("=====查询应缴费用=====\n");
    const char *query = "SELECT SUM(amount) FROM transactions WHERE user_id=?;";
    sqlite3_stmt *stmt;
    int rc = sqlite3_prepare_v2(db->db, query, -1, &stmt, NULL);
    if (rc != SQLITE_OK)
    {
        fprintf(stderr, "无法准备查询语句: %s\n", sqlite3_errmsg(db->db));
        return;
    }
    sqlite3_bind_text(stmt, 1, user_id, -1, SQLITE_STATIC);
    if (sqlite3_step(stmt) == SQLITE_ROW)
    {
        double total_amount = sqlite3_column_double(stmt, 0);
        printf("应缴费用: %.2f\n", total_amount);
    }
    else
    {
        printf("未找到任何记录\n");
    }
    sqlite3_finalize(stmt);
    printf("按任意键返回...\n");
    clear_input_buffer();
}

// 查询剩余费用
void query_remaining_balance(Database *db, const char *user_id)
{
    system("cls");
    printf("=====查询剩余费用=====\n");
    const char *query = "SELECT SUM(amount) FROM transactions WHERE user_id=?;";
    sqlite3_stmt *stmt;
    int rc = sqlite3_prepare_v2(db->db, query, -1, &stmt, NULL);
    if (rc != SQLITE_OK)
    {
        fprintf(stderr, "无法准备查询语句: %s\n", sqlite3_errmsg(db->db));
        return;
    }
    sqlite3_bind_text(stmt, 1, user_id, -1, SQLITE_STATIC);
    if (sqlite3_step(stmt) == SQLITE_ROW)
    {
        double total_amount = sqlite3_column_double(stmt, 0);
        printf("剩余费用: %.2f\n", total_amount);
    }
    else
    {
        printf("查询失败或无剩余费用\n");
    }
    sqlite3_finalize(stmt);
    printf("按任意键返回...\n");
    clear_input_buffer();
}

// 查询小区基本信息
void query_community_info(Database *db)
{
    system("cls");
    printf("=====查询小区基本信息=====\n");
    const char *query = "SELECT * FROM community_info;";
    sqlite3_stmt *stmt;
    int rc = sqlite3_prepare_v2(db->db, query, -1, &stmt, NULL);
    if (rc != SQLITE_OK)
    {
        fprintf(stderr, "无法准备查询语句: %s\n", sqlite3_errmsg(db->db));
        return;
    }
    while ((rc = sqlite3_step(stmt)) == SQLITE_ROW)
    {
        const char *name = sqlite3_column_text(stmt, 0);
        const char *address = sqlite3_column_text(stmt, 1);
        const char *phone_number = sqlite3_column_text(stmt, 2);
        printf("小区名称: %s\n", name);
        printf("地址: %s\n", address);
        printf("联系电话: %s\n", phone_number);
    }
    sqlite3_finalize(stmt);
    printf("按任意键返回...\n");
    clear_input_buffer();
}

// 查询收费信息
void query_fee_info(Database *db, const char *user_id)
{
    system("cls");
    printf("=====查询收费信息=====\n");
    const char *query = "SELECT * FROM fee_info;";
    sqlite3_stmt *stmt;
    int rc = sqlite3_prepare_v2(db->db, query, -1, &stmt, NULL);
    if (rc != SQLITE_OK)
    {
        fprintf(stderr, "无法准备查询语句: %s\n", sqlite3_errmsg(db->db));
        return;
    }
    sqlite3_bind_text(stmt, 1, user_id, -1, SQLITE_STATIC);
    printf("费用ID\t\t费用名称\t\t费用金额\n");
    while ((rc = sqlite3_step(stmt)) == SQLITE_ROW)
    {
        const char *fee_id = sqlite3_column_text(stmt, 0);
        const char *fee_name = sqlite3_column_text(stmt, 1);
        double fee_amount = sqlite3_column_double(stmt, 2);
        printf("%s\t%s\t%.2f\n", fee_id, fee_name, fee_amount);
    }
    sqlite3_finalize(stmt);
    printf("按任意键返回...\n");
    clear_input_buffer();
}

// 查询服务人员信息
void query_service_staff_info(Database *db, const char *user_id)
{
    system("cls");
    printf("=====查询服务人员信息=====\n");
    const char *query = "SELECT * FROM service_staff_info;";
    sqlite3_stmt *stmt;
    int rc = sqlite3_prepare_v2(db->db, query, -1, &stmt, NULL);
    if (rc != SQLITE_OK)
    {
        fprintf(stderr, "无法准备查询语句: %s\n", sqlite3_errmsg(db->db));
        return;
    }
    sqlite3_bind_text(stmt, 1, user_id, -1, SQLITE_STATIC);
    printf("服务人员ID\t\t姓名\t\t联系电话\n");
    while ((rc = sqlite3_step(stmt)) == SQLITE_ROW)
    {
        const char *staff_id = sqlite3_column_text(stmt, 0);
        const char *name = sqlite3_column_text(stmt, 1);
        const char *phone_number = sqlite3_column_text(stmt, 2);
        printf("%s\t%s\t%s\n", staff_id, name, phone_number);
    }
    sqlite3_finalize(stmt);
    printf("按任意键返回...\n");
    clear_input_buffer();
}

// 修改用户名
bool change_username(Database *db, const char *user_id, char *username)
{
    if (db == NULL || user_id == NULL || username == NULL)
    {
        fprintf(stderr, "无效的数据库或输入参数\n");
        return false;
    }
    const char *sql = "UPDATE users SET username = ? WHERE id = ?;";
    sqlite3_stmt *stmt;
    int rc;
    // 预处理 SQL 语句
    rc = sqlite3_prepare_v2(db->db, sql, -1, &stmt, 0);
    if (rc != SQLITE_OK)
    {
        fprintf(stderr, "SQL 预处理失败: %s\n", sqlite3_errmsg(db->db));
        return false;
    }
    // 绑定参数
    sqlite3_bind_text(stmt, 1, username, -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 2, user_id, -1, SQLITE_STATIC);
    // 执行 SQL 语句
    rc = sqlite3_step(stmt);
    if (rc != SQLITE_DONE)
    {
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
            show_owner_personal_info_screen(db, user_id, user_type); // 修正函数名称和语法错误
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
void show_owner_personal_info_screen(Database *db, const char *user_id, UserType user_type)
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
        system("cls");
        printf("\n===== 缴费管理 =====\n");
        printf("1. 查看缴费记录\n");
        printf("2. 缴纳费用\n");
        printf("3. 查询应缴费用\n");
        printf("4. 查询物业费标准\n");
        printf("0. 返回上一级\n");
        printf("请输入您的选择: ");
        scanf("%d", &choice);
        getchar(); // 清除输入缓冲区中的换行符

        switch (choice)
        {
        case 1:
            show_payment_history(db, user_id);
            break;
        case 2:
            process_payment_screen(db, user_id);
            break;
        case 3:
            query_due_payments(db, user_id);
            break;
        case 4:
            query_fee_info(db, user_id);
            break;
        case 0:
            return;
        default:
            printf("无效的选择，请重新输入\n");
            printf("按任意键继续...");
            getchar();
            break;
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
// 按 ID 升序排序的比较函数 - 重命名以避免与user.c冲突
int compare_owner_id_asc(const void *a, const void *b)
{
    Owner *ownerA = *(Owner **)a;
    Owner *ownerB = *(Owner **)b;
    return ownerA->user_id - ownerB->user_id;
}

// 按 ID 降序排序的比较函数 - 重命名
int compare_owner_id_desc(const void *a, const void *b)
{
    Owner *ownerA = *(Owner **)a;
    Owner *ownerB = *(Owner **)b;
    return ownerB->user_id - ownerA->user_id;
}

// 按姓名升序排序的比较函数 - 重命名
int compare_owner_name_asc(const void *a, const void *b)
{
    Owner *ownerA = *(Owner **)a;
    Owner *ownerB = *(Owner **)b;
    return strcmp(ownerA->name, ownerB->name);
}

// 按姓名降序排序的比较函数 - 重命名
int compare_owner_name_desc(const void *a, const void *b)
{
    Owner *ownerA = *(Owner **)a;
    Owner *ownerB = *(Owner **)b;
    return strcmp(ownerB->name, ownerA->name);
}
void show_owner_sort_screen(Database *db, const char *user_id, UserType user_type)
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
            // 按 ID 升序排序 - 更新函数名称
            {
                sort_owners(db, compare_owner_id_asc);
                display_owners(db);
                break;
            }

        case 2:
            // 按 ID 降序排序 - 更新函数名称
            {
                sort_owners(db, compare_owner_id_desc);
                display_owners(db);
                break;
            }

        case 3:
            // 按姓名升序排序 - 更新函数名称
            {
                sort_owners(db, compare_owner_name_asc);
                display_owners(db);
                break;
            }

        case 4:
            // 按姓名降序排序 - 更新函数名称
            {
                sort_owners(db, compare_owner_name_desc);
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
void show_owner_statistics_screen(Database *db, const char *user_id, UserType user_type)
{
    // TODO: 实现业主信息统计界面显示和操作逻辑
}

// 业主系统维护界面
void show_owner_maintenance_screen(Database *db, const char *user_id, UserType user_type)
{
    // TODO: 实现业主系统维护界面显示和操作逻辑
}

// 兼容旧接口
void show_owner_main_screen(Database *db, const char *user_id, UserType user_type)
{
    main_screen_owner(db, user_id, user_type);
}

/**
 * 查询应缴费用
 *
 * @param db 数据库连接
 * @param user_id 用户ID
 */
void query_due_payments(Database *db, const char *user_id)
{
    system("cls");

    // 先更新所有交易的逾期状态
    update_overdue_transactions(db);

    // 查询用户所有未支付的费用总额
    char query[512];
    QueryResult result;

    snprintf(query, sizeof(query),
             "SELECT SUM(amount) FROM transactions "
             "WHERE user_id = '%s' AND (status = %d OR status = %d)",
             user_id, TRANS_UNPAID, TRANS_OVERDUE);

    if (!execute_query(db, query, &result))
    {
        printf("查询应缴费用失败\n");
        return;
    }

    float total_amount = 0;
    if (result.row_count > 0 && result.rows[0].values[0] != NULL)
    {
        total_amount = atof(result.rows[0].values[0]);
    }

    printf("\n===== 应缴费用 =====\n");
    printf("总应缴金额: %.2f 元\n", total_amount);

    // 按费用类型统计
    free_query_result(&result);

    snprintf(query, sizeof(query),
             "SELECT fee_type, SUM(amount) FROM transactions "
             "WHERE user_id = '%s' AND (status = %d OR status = %d) "
             "GROUP BY fee_type",
             user_id, TRANS_UNPAID, TRANS_OVERDUE);

    if (!execute_query(db, query, &result))
    {
        printf("查询费用类型统计失败\n");
        return;
    }

    printf("\n费用类型明细:\n");

    for (int i = 0; i < result.row_count; i++)
    {
        int fee_type = atoi(result.rows[i].values[0]);
        float amount = atof(result.rows[i].values[1]);

        char fee_type_str[20];
        switch (fee_type)
        {
        case TRANS_PROPERTY_FEE:
            strcpy(fee_type_str, "物业费");
            break;
        case TRANS_PARKING_FEE:
            strcpy(fee_type_str, "停车费");
            break;
        case TRANS_WATER_FEE:
            strcpy(fee_type_str, "水费");
            break;
        case TRANS_ELECTRICITY_FEE:
            strcpy(fee_type_str, "电费");
            break;
        case TRANS_GAS_FEE:
            strcpy(fee_type_str, "燃气费");
            break;
        default:
            strcpy(fee_type_str, "其他费用");
            break;
        }

        printf("%s: %.2f 元\n", fee_type_str, amount);
    }

    free_query_result(&result);

    // 查询逾期费用
    snprintf(query, sizeof(query),
             "SELECT SUM(amount) FROM transactions "
             "WHERE user_id = '%s' AND status = %d",
             user_id, TRANS_OVERDUE);

    if (!execute_query(db, query, &result))
    {
        printf("查询逾期费用失败\n");
        return;
    }

    float overdue_amount = 0;
    if (result.row_count > 0 && result.rows[0].values[0] != NULL)
    {
        overdue_amount = atof(result.rows[0].values[0]);
    }

    if (overdue_amount > 0)
    {
        printf("\n注意: 您有 %.2f 元的费用已逾期未付，请尽快缴纳！\n", overdue_amount);
    }

    free_query_result(&result);
    printf("\n按任意键返回...");
    getchar();
}
