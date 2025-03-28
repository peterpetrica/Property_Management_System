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
    int found = 0;
    while ((rc = sqlite3_step(stmt)) == SQLITE_ROW)
    {   found = 1;
        const char *transaction_id = sqlite3_column_text(stmt, 0);
        double amount = sqlite3_column_double(stmt, 1);
        const char *date = sqlite3_column_text(stmt, 2);
        printf("%s\t%.2f\t%s\n", transaction_id, amount, date);
    } 
    if (!found) 
    {
        printf("\n⚠️ 当前用户暂无缴费记录。\n");
    }
    sqlite3_finalize(stmt);
    printf("\n------------------------------------\n");
    printf("按任意键返回...\n");
    clear_input_buffer();
}
double query_total_fee(Database *db, const char *user_id) 
{
    // 1. 定义所有变量
    const char *query = "SELECT SUM(amount) FROM transactions WHERE user_id=?;";
    sqlite3_stmt *stmt;
    double total_fee = -1.0;
    
    // 2. 准备语句
    if (sqlite3_prepare_v2(db->db, query, -1, &stmt, NULL) != SQLITE_OK) {
        fprintf(stderr, "准备查询失败: %s\n", sqlite3_errmsg(db->db));
        return -1.0;
    }
    
    // 3. 绑定参数
    sqlite3_bind_text(stmt, 1, user_id, -1, SQLITE_STATIC);
    
    // 4. 执行查询
    int rc = sqlite3_step(stmt);
    if (rc == SQLITE_ROW) {
        total_fee = sqlite3_column_double(stmt, 0);
    } else if (rc != SQLITE_DONE) {
        fprintf(stderr, "查询执行失败: %s\n", sqlite3_errmsg(db->db));
    }
    
    // 5. 释放资源
    sqlite3_finalize(stmt);
    
    // 6. 返回结果
    return total_fee;
}

void show_total_fee(Database *db, const char *user_id) 
{
    system("cls");
    printf("===== 缴费总金额 =====\n");
    double total = query_total_fee(db, user_id);
    if (total < 0) {
        printf("❌ 查询失败: 请检查数据库或用户记录。\n");
    } else if (total == 0) {
        printf("用户 %s 暂无缴费记录。\n", user_id);
    } else {
        printf("用户 %s 的累计缴费总额：￥%.2f\n", user_id, total);
    }
    
    printf("\n按任意键返回...");
    clear_input_buffer();
}

void process_payment_screen(Database *db, const char *user_id) 
{
    system("cls");
    printf("=====缴费流程=====\n");
    
    // 获取缴费金额
    double amount;
    printf("请输入缴费金额: ");
    if (scanf("%lf", &amount) != 1 || amount <= 0) {
        printf("输入错误，金额必须为正数\n");
        clear_input_buffer();
        return;
    }
    clear_input_buffer();
    
    // 获取费用类型
    char fee_type[50] = {0}; // 初始化为全0
    printf("请输入费用类型(如:物业费/水电费等): ");
    if (scanf("%49s", fee_type) != 1 || strlen(fee_type) == 0) {
        printf("输入错误，请重试\n");
        clear_input_buffer();
        return;
    }
    clear_input_buffer();
    
    // 准备SQL语句
    const char *query = "INSERT INTO transactions(transaction_id, user_id, amount, payment_date, fee_type) VALUES(?,?,?,?,?);";
    sqlite3_stmt *stmt = NULL;
    
    int rc = sqlite3_prepare_v2(db->db, query, -1, &stmt, NULL);
    if (rc != SQLITE_OK) {
        fprintf(stderr, "无法准备插入语句: %s\n", sqlite3_errmsg(db->db));
        return;
    }
    
    // 生成交易ID和日期
    char *trans_id = generate_transaction_id();
    char *current_date = get_current_date();
    
    // 检查生成的ID和日期是否有效
    if (!trans_id || !current_date) {
        fprintf(stderr, "生成交易信息失败\n");
        free(trans_id);
        free(current_date);
        sqlite3_finalize(stmt);
        return;
    }
    
    // 绑定参数
    sqlite3_bind_text(stmt, 1, trans_id, -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 2, user_id, -1, SQLITE_STATIC);
    sqlite3_bind_double(stmt, 3, amount);
    sqlite3_bind_text(stmt, 4, current_date, -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 5, fee_type, -1, SQLITE_STATIC);
    
    // 执行SQL
    rc = sqlite3_step(stmt);
    if (rc != SQLITE_DONE) {
        fprintf(stderr, "缴费失败: %s\n", sqlite3_errmsg(db->db));
    } else {
        printf("缴费成功！交易号: %s, 金额: %.2f, 类型: %s\n", 
              trans_id, amount, fee_type);
    }
    
    // 释放资源
    sqlite3_finalize(stmt);
    free(trans_id);
    free(current_date);
    
    printf("按任意键返回...\n");
    getchar();
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
    printf("===== 物业费标准 =====\n");

    const char *query = "SELECT fee_id, fee_name, fee_amount FROM fee_info;";
    sqlite3_stmt *stmt;
    
    // 准备查询
    int rc = sqlite3_prepare_v2(db->db, query, -1, &stmt, NULL);
    if (rc != SQLITE_OK) {
        fprintf(stderr, "❌ 查询失败: %s\n", sqlite3_errmsg(db->db));
        return;
    }
    // 打印表头
    printf("费用ID\t\t费用名称\t\t费用金额\n");
    printf("--------------------------------------------\n");

    // 遍历结果
    int found = 0;
    while ((rc = sqlite3_step(stmt)) == SQLITE_ROW) {
        found = 1;
        const char *fee_id = (const char *)sqlite3_column_text(stmt, 0);
        const char *fee_name = (const char *)sqlite3_column_text(stmt, 1);
        double fee_amount = sqlite3_column_double(stmt, 2);
        printf("%-8s\t%-16s\t￥%.2f\n", fee_id, fee_name, fee_amount);
    }

    // 检查是否无数据
    if (!found) {
        printf("\n⚠️ 当前无物业费标准数据。\n");
    }

    // 释放资源
    sqlite3_finalize(stmt);
    printf("\n--------------------------------------------\n");
    printf("按任意键返回...");
    clear_input_buffer();
}

// 查询服务人员信息
void query_service_staff_info(Database *db, const char *user_id)
{
    system("cls");
    printf("===== 用户信息查询 =====\n\n");

    // 修改查询语句，获取用户的详细信息和缴费情况
    const char *query = 
        "SELECT "
        "   u.user_id, "
        "   u.username, "
        "   u.name, "
        "   u.phone_number, "
        "   u.email, "
        "   r.room_number, "
        "   b.building_name, "
        "   COUNT(t.transaction_id) as total_transactions, "
        "   SUM(CASE WHEN t.status = 1 THEN t.amount ELSE 0 END) as paid_amount "
        "FROM users u "
        "LEFT JOIN rooms r ON u.user_id = r.owner_id "
        "LEFT JOIN buildings b ON r.building_id = b.building_id "
        "LEFT JOIN transactions t ON u.user_id = t.user_id "
        "WHERE u.role_id = 'role_owner' "
        "GROUP BY u.user_id, u.username, u.name, u.phone_number, u.email, r.room_number, b.building_name "
        "ORDER BY b.building_name, r.room_number;";

    sqlite3_stmt *stmt;
    int rc = sqlite3_prepare_v2(db->db, query, -1, &stmt, NULL);
    if (rc != SQLITE_OK)
    {
        fprintf(stderr, "查询失败: %s\n", sqlite3_errmsg(db->db));
        return;
    }

    // 打印表头
    printf("%-10s%-15s%-15s%-15s%-20s%-15s%-15s%-12s%-12s\n", 
           "用户ID", "用户名", "姓名", "电话", "邮箱", "楼号", "房间号", "交易次数", "已缴金额");
    printf("------------------------------------------------------------------------------------------------\n");

    int found = 0;
    while ((rc = sqlite3_step(stmt)) == SQLITE_ROW)
    {
        found = 1;
        const char *user_id = (const char *)sqlite3_column_text(stmt, 0);
        const char *username = (const char *)sqlite3_column_text(stmt, 1);
        const char *name = (const char *)sqlite3_column_text(stmt, 2);
        const char *phone = (const char *)sqlite3_column_text(stmt, 3);
        const char *email = (const char *)sqlite3_column_text(stmt, 4);
        const char *room_number = (const char *)sqlite3_column_text(stmt, 5);
        const char *building_name = (const char *)sqlite3_column_text(stmt, 6);
        int total_trans = sqlite3_column_int(stmt, 7);
        double paid_amount = sqlite3_column_double(stmt, 8);

        printf("%-10s%-15s%-15s%-15s%-20s%-15s%-15s%-12d%-12.2f\n",
            user_id ? user_id : "N/A",
            username ? username : "未知",
            name ? name : "未知",
            phone ? phone : "未知",
            email ? email : "未知",
            building_name ? building_name : "未分配",
            room_number ? room_number : "未分配",
            total_trans,
            paid_amount);

        // 查询未缴费信息
        const char *unpaid_query = 
            "SELECT COUNT(*) as count, SUM(amount) as total "
            "FROM transactions "
            "WHERE user_id = ? AND status = 0;";
        
        sqlite3_stmt *unpaid_stmt;
        if (sqlite3_prepare_v2(db->db, unpaid_query, -1, &unpaid_stmt, NULL) == SQLITE_OK)
        {
            sqlite3_bind_text(unpaid_stmt, 1, user_id, -1, SQLITE_STATIC);
            if (sqlite3_step(unpaid_stmt) == SQLITE_ROW)
            {
                int unpaid_count = sqlite3_column_int(unpaid_stmt, 0);
                double unpaid_amount = sqlite3_column_double(unpaid_stmt, 1);
                if (unpaid_count > 0)
                {
                    printf("    ⚠️ 待缴费: %d笔, 总金额: %.2f元\n", unpaid_count, unpaid_amount);
                }
            }
            sqlite3_finalize(unpaid_stmt);
        }
        printf("------------------------------------------------------------------------------------------------\n");
    }

    if (!found)
    {
        printf("\n⚠️ 暂无用户信息\n");
    }

    sqlite3_finalize(stmt);
    printf("\n按任意键返回...");
    clear_input_buffer();
    getchar();
}

// 修改用户名
bool change_username(Database *db, const char *user_id, char *username)
{
    if (db == NULL || user_id == NULL || username == NULL)
    {
        fprintf(stderr, "无效的数据库或输入参数\n");
        return false;
    }
    const char *sql = "UPDATE users SET username = ? WHERE user_id = ?;";
    sqlite3_stmt *stmt;
    int rc;
    // 预处理 SQL 语句`
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
    getchar();
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
            printf("请输入用户名\n");
            scanf("%s",username);
            if (change_username(db, user_id, username))
            {
                printf("用户名更改成功\n");
            }
            else
            {
                printf("用户名修改失败，请重试。\n");
            }
            break;
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
        printf("5.查询缴费总金额\n");
        printf("0. 返回上一级\n");
        printf("请输入您的选择: ");
        scanf("%d", &choice);
        getchar(); // 清除输入缓冲区中的换行符

        switch (choice)
        {
        case 1:
            show_payment_history(db, user_id);
            break;
        case 2:process_payment_screen
            (db, user_id);
            break;
        case 3:
            query_due_payments(db, user_id);
            break;
        case 4:
            query_fee_info(db, user_id);
            break;
        case 5:
            show_total_fee(db, user_id);  
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

// 查询特定业主的缴费信息
void query_owner_payment_info(Database *db, const char *user_id) {
    system("cls");
    printf("===== 查询业主缴费信息 =====\n");

    const char *query = "SELECT transaction_id, fee_type, amount, payment_date, status FROM transactions WHERE user_id = ?;";
    sqlite3_stmt *stmt;
    int rc = sqlite3_prepare_v2(db->db, query, -1, &stmt, NULL);
    if (rc != SQLITE_OK) {
        fprintf(stderr, "SQLite错误: %s\n", sqlite3_errmsg(db->db));
        return;
    }

    sqlite3_bind_text(stmt, 1, user_id, -1, SQLITE_STATIC);

    printf("交易ID\t费用类型\t金额\t支付日期\t状态\n");
    printf("-------------------------------------------------\n");
    int found = 0;
    while ((rc = sqlite3_step(stmt)) == SQLITE_ROW) {
        found = 1;
        const char *transaction_id = sqlite3_column_text(stmt, 0);
        int fee_type = sqlite3_column_int(stmt, 1);
        double amount = sqlite3_column_double(stmt, 2);
        const char *payment_date = sqlite3_column_text(stmt, 3);
        int status = sqlite3_column_int(stmt, 4);

        printf("%s\t%d\t\t%.2f\t%s\t%d\n", transaction_id, fee_type, amount, payment_date ? payment_date : "未支付", status);
    }

    if (!found) {
        printf("\n⚠️ 当前业主暂无缴费记录。\n");
    }

    sqlite3_finalize(stmt);
    printf("\n按任意键返回...\n");
    clear_input_buffer();
}

// 查询所有业主的缴费情况
void query_all_owners_payment_info(Database *db) {
    system("cls");
    printf("===== 查询所有业主缴费情况 =====\n");

    const char *query = "SELECT u.user_id, u.name, SUM(t.amount) AS total_paid FROM users u "
                        "LEFT JOIN transactions t ON u.user_id = t.user_id "
                        "WHERE u.role_id = 'role_owner' GROUP BY u.user_id, u.name;";
    sqlite3_stmt *stmt;
    int rc = sqlite3_prepare_v2(db->db, query, -1, &stmt, NULL);
    if (rc != SQLITE_OK) {
        fprintf(stderr, "SQLite错误: %s\n", sqlite3_errmsg(db->db));
        return;
    }

    printf("业主ID\t姓名\t\t累计缴费金额\n");
    printf("-----------------------------------------\n");
    while ((rc = sqlite3_step(stmt)) == SQLITE_ROW) {
        const char *user_id = sqlite3_column_text(stmt, 0);
        const char *name = sqlite3_column_text(stmt, 1);
        double total_paid = sqlite3_column_double(stmt, 2);

        printf("%s\t%s\t\t%.2f\n", user_id, name, total_paid);
    }

    sqlite3_finalize(stmt);
    printf("\n按任意键返回...\n");
    clear_input_buffer();
}
