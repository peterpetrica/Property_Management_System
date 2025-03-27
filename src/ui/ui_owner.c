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
#include <sys/time.h>

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
    system("clear || cls");
    printf("=====缴费记录=====\n");

    const char *query = "SELECT transaction_id, amount, payment_date FROM transactions WHERE user_id=?";
    sqlite3_stmt *stmt;

    if (sqlite3_prepare_v2(db->db, query, -1, &stmt, NULL) != SQLITE_OK)
    {
        printf("SQL错误: %s\n", sqlite3_errmsg(db->db));
        return;
    }

    sqlite3_bind_text(stmt, 1, user_id, -1, SQLITE_STATIC);

    printf("交易ID\t\t金额\t\t日期\n");
    printf("--------------------------------\n");

    int found = 0;
    while (sqlite3_step(stmt) == SQLITE_ROW)
    {
        found = 1;

        // 交易ID
        const unsigned char *transaction_id = sqlite3_column_text(stmt, 0);

        // 金额
        double amount = sqlite3_column_double(stmt, 1);

        // 日期处理（假设存的是时间戳）
        time_t pay_date = sqlite3_column_int64(stmt, 2);
        char date_buf[20];
        strftime(date_buf, sizeof(date_buf), "%Y-%m-%d", localtime(&pay_date));

        printf("%-16s￥%-10.2f%s\n",
               transaction_id ? (const char *)transaction_id : "NULL",
               amount,
               date_buf);
    }

    if (!found)
    {
        printf("\n⚠️ 当前用户暂无缴费记录。\n");
    }

    sqlite3_finalize(stmt);
    printf("\n按任意键返回...");
    clear_input_buffer();
}

double query_total_fee(Database *db, const char *user_id)
{
    // 1. 定义所有变量
    const char *query = "SELECT SUM(amount) FROM transactions WHERE user_id=?;";
    sqlite3_stmt *stmt;
    double total_fee = -1.0;

    // 2. 准备语句
    if (sqlite3_prepare_v2(db->db, query, -1, &stmt, NULL) != SQLITE_OK)
    {
        fprintf(stderr, "准备查询失败: %s\n", sqlite3_errmsg(db->db));
        return -1.0;
    }

    // 3. 绑定参数
    sqlite3_bind_text(stmt, 1, user_id, -1, SQLITE_STATIC);

    // 4. 执行查询
    int rc = sqlite3_step(stmt);
    if (rc == SQLITE_ROW)
    {
        total_fee = sqlite3_column_double(stmt, 0);
    }
    else if (rc != SQLITE_DONE)
    {
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
    if (total < 0)
    {
        printf("❌ 查询失败: 请检查数据库或用户记录。\n");
    }
    else if (total == 0)
    {
        printf("用户 %s 暂无缴费记录。\n", user_id);
    }
    else
    {
        printf("用户 %s 的累计缴费总额：￥%.2f\n", user_id, total);
    }

    printf("\n按任意键返回...");
    clear_input_buffer();
}
void process_payment_screen(Database *db, const char *user_id)
{
    // 清屏并显示标题
    system("clear || cls");
    printf("======== 费用缴纳 ========\n\n");

    // 费用类型配置
    const char *fee_types[] = {"物业费", "水电费", "停车费", "维修基金"};
    const int valid_days[] = {365, 30, 30, 365}; // 各类型默认有效期（天）

    // 费用类型选择
    printf("请选择费用类型:\n");
    for (int i = 0; i < sizeof(fee_types) / sizeof(fee_types[0]); i++)
    {
        printf("%d. %s\n", i + 1, fee_types[i]);
    }
    printf("选择(1-%d): ", (int)(sizeof(fee_types) / sizeof(fee_types[0])));

    int fee_type;
    if (scanf("%d", &fee_type) != 1 ||
        fee_type < 1 ||
        fee_type > (int)(sizeof(fee_types) / sizeof(fee_types[0])))
    {
        printf("! 无效选择\n");
        clear_input_buffer();
        return;
    }
    clear_input_buffer();

    // 金额输入处理
    double amount = 0;
    do
    {
        printf("\n请输入%s金额（1-100000元）: ", fee_types[fee_type - 1]);
        if (scanf("%lf", &amount) != 1)
        {
            printf("! 请输入有效数字\n");
            clear_input_buffer();
            continue;
        }

        if (amount <= 0)
        {
            printf("! 金额必须大于0\n");
        }
        else if (amount > 100000)
        {
            printf("! 单笔缴费不得超过10万元\n");
        }
        else
        {
            break;
        }
        clear_input_buffer();
    } while (1);

    // 生成交易信息
    time_t now = time(NULL);
    time_t period_start = now;
    time_t period_end = now + (valid_days[fee_type - 1] * 24 * 60 * 60);

    // ===== 修改的交易ID生成部分 =====
    char trans_id[25] = {0}; // 稍微增大缓冲区
    struct timeval tv;
    gettimeofday(&tv, NULL); // 获取毫秒级时间
    struct tm *tm_pay = localtime(&now);

    // 生成更唯一的交易ID（带毫秒和5位随机数）
    snprintf(trans_id, sizeof(trans_id), "%c%04d%02d%02d%03d%05d",
             fee_types[fee_type - 1][0], // 费用类型首字母
             tm_pay->tm_year + 1900,     // 年
             tm_pay->tm_mon + 1,         // 月
             tm_pay->tm_mday,            // 日
             (int)(tv.tv_usec / 1000),   // 毫秒
             rand() % 100000);           // 5位随机数

    // 数据库操作（带重试机制）
    int retry_count = 0;
    bool success = false;
    sqlite3_stmt *stmt = NULL;

    do
    {
        const char *query = "INSERT INTO transactions "
                            "(transaction_id, user_id, amount, payment_date, fee_type, due_date, period_start, period_end) "
                            "VALUES (?, ?, ?, ?, ?, ?, ?, ?)";

        if (sqlite3_prepare_v2(db->db, query, -1, &stmt, NULL) != SQLITE_OK)
        {
            fprintf(stderr, "数据库错误: %s\n", sqlite3_errmsg(db->db));
            break;
        }

        // 绑定参数
        sqlite3_bind_text(stmt, 1, trans_id, -1, SQLITE_STATIC);
        sqlite3_bind_text(stmt, 2, user_id, -1, SQLITE_STATIC);
        sqlite3_bind_double(stmt, 3, amount);
        sqlite3_bind_int64(stmt, 4, (sqlite3_int64)now);
        sqlite3_bind_text(stmt, 5, fee_types[fee_type - 1], -1, SQLITE_STATIC);
        sqlite3_bind_int64(stmt, 6, (sqlite3_int64)period_end);
        sqlite3_bind_int64(stmt, 7, (sqlite3_int64)period_start);
        sqlite3_bind_int64(stmt, 8, (sqlite3_int64)period_end);

        int rc = sqlite3_step(stmt);
        if (rc == SQLITE_DONE)
        {
            success = true;
            break;
        }

        // 如果是唯一性冲突且还有重试次数
        if (rc == SQLITE_CONSTRAINT &&
            strstr(sqlite3_errmsg(db->db), "transaction_id") &&
            retry_count < 2)
        {
            // 重新生成更唯一的ID
            gettimeofday(&tv, NULL);
            snprintf(trans_id, sizeof(trans_id), "%c%04d%02d%02d%03d%05d",
                     fee_types[fee_type - 1][0],
                     tm_pay->tm_year + 1900,
                     tm_pay->tm_mon + 1,
                     tm_pay->tm_mday,
                     (int)(tv.tv_usec / 1000),
                     rand() % 100000);
            retry_count++;
            sqlite3_finalize(stmt);
            continue;
        }

        fprintf(stderr, "缴费失败: %s\n", sqlite3_errmsg(db->db));
        break;
    } while (1);

    if (stmt)
        sqlite3_finalize(stmt);
    // ===== 修改的交易ID生成部分结束 =====

    // 显示结果（格式化日期用于显示）
    char display_date[20];
    strftime(display_date, sizeof(display_date), "%Y-%m-%d", localtime(&now));

    char display_due_date[20];
    strftime(display_due_date, sizeof(display_due_date), "%Y-%m-%d", localtime(&period_end));

    printf("\n════════════════════════\n");
    if (success)
    {
        printf("✅ 缴费成功\n");
        printf("────────────────────────\n");
        printf("交易号:    %s\n", trans_id);
        printf("用户ID:    %s\n", user_id);
        printf("费用类型:  %s\n", fee_types[fee_type - 1]);
        printf("缴费金额:  ￥%.2f\n", amount);
        printf("缴费日期:  %s\n", display_date);
        printf("有效期至:  %s\n", display_due_date);
    }
    else
    {
        printf("! 缴费失败，请稍后重试\n");
    }
    printf("════════════════════════\n");

    printf("\n按任意键返回主菜单...");
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
    const char *check_sql = "SELECT 1 FROM sqlite_master WHERE type='table' AND name='community_info'";
    sqlite3_stmt *check_stmt;
    if (sqlite3_prepare_v2(db->db, check_sql, -1, &check_stmt, NULL) != SQLITE_OK)
    {
        fprintf(stderr, "数据库错误: %s\n", sqlite3_errmsg(db->db));
        return;
    }

    if (sqlite3_step(check_stmt) != SQLITE_ROW)
    {
        printf("\n⚠️ 小区信息表未初始化，请联系物业管理人员\n");
        sqlite3_finalize(check_stmt);
        printf("\n按任意键返回...");
        clear_input_buffer();
        return;
    }
    sqlite3_finalize(check_stmt);
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
    system("clear || cls"); // 修改为跨平台清屏
    printf("===== 物业费标准 =====\n");

    // 先检查表是否存在
    const char *check_table = "SELECT 1 FROM sqlite_master WHERE type='table' AND name='fee_info'";
    sqlite3_stmt *check_stmt;
    if (sqlite3_prepare_v2(db->db, check_table, -1, &check_stmt, NULL) != SQLITE_OK)
    {
        fprintf(stderr, "❌ 检查表失败: %s\n", sqlite3_errmsg(db->db));
        return;
    }

    if (sqlite3_step(check_stmt) != SQLITE_ROW)
    {
        printf("\n⚠️ 物业费标准表不存在，请联系管理员初始化数据\n");
        sqlite3_finalize(check_stmt);
        printf("\n按任意键返回...");
        clear_input_buffer();
        return;
    }
    sqlite3_finalize(check_stmt);

    // 原查询逻辑
    const char *query = "SELECT fee_id, fee_name, fee_amount FROM fee_info;";
    sqlite3_stmt *stmt;

    // ... [剩余代码保持不变] ...
}
// 查询服务人员信息
void query_service_staff_info(Database *db, const char *user_id)
{
    // 1. 跨平台清屏（静默模式，不显示错误）
    system("cls");

    // 2. 更友好的界面标题
    printf("\n════════ 为您服务的工作人员 ════════\n\n");

    // 3. 简化查询逻辑（仅展示，不绑定用户ID）
    const char *query = "SELECT staff_id, name, phone FROM service_staff_info LIMIT 20"; // 限制显示数量
    sqlite3_stmt *stmt;

    if (sqlite3_prepare_v2(db->db, query, -1, &stmt, NULL) != SQLITE_OK)
    {
        // 4. 更人性化的错误提示
        printf("\n⚠️ 暂未查询到服务人员信息\n");
    }
    else
    {
        // 5. 美化表格输出
        printf("┌────────────┬──────────┬───────────────┐\n");
        printf("│   %-8s │ %-8s │ %-12s │\n", "工号", "姓名", "联系电话");
        printf("├────────────┼──────────┼───────────────┤\n");

        while (sqlite3_step(stmt) == SQLITE_ROW)
        {
            printf("│ %-10s │ %-8s │ %-13s │\n",
                   sqlite3_column_text(stmt, 0) ? (const char *)sqlite3_column_text(stmt, 0) : "NULL",
                   sqlite3_column_text(stmt, 1) ? (const char *)sqlite3_column_text(stmt, 1) : "NULL",
                   sqlite3_column_text(stmt, 2) ? (const char *)sqlite3_column_text(stmt, 2) : "NULL");
        }
        printf("-------------------------------------\n");
    }

    // 6. 统一的退出提示
    printf("\n────────────────────────────────────\n");
    printf("按任意键返回主菜单...");

    // 7. 安全释放资源
    if (stmt)
        sqlite3_finalize(stmt);
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
            scanf("%s", username);
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
        case 2:
            process_payment_screen(db, user_id);
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
