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

// 显示缴费记录
void show_payment_history(Database *db, const char *user_id)
{
    clear_screen();
    printf("\n====== 缴费记录 ======\n");

    const char *query = 
        "SELECT t.transaction_id, "
        "CASE t.fee_type "
        "WHEN 1 THEN '物业费' "
        "WHEN 2 THEN '停车费' "
        "WHEN 3 THEN '水费' "
        "WHEN 4 THEN '电费' "
        "WHEN 5 THEN '燃气费' "
        "ELSE '其他' END as fee_type, "
        "t.amount, "
        "t.payment_date "
        "FROM transactions t "
        "WHERE t.user_id = ? "
        "AND t.payment_date IS NOT NULL "
        "AND t.payment_date > 0 "
        "AND t.status = 1 "
        "AND t.amount > 0 "  // 确保金额为正数
        "ORDER BY t.payment_date DESC";

    sqlite3_stmt *stmt;
    if (sqlite3_prepare_v2(db->db, query, -1, &stmt, NULL) != SQLITE_OK)
    {
        printf("SQL错误: %s\n", sqlite3_errmsg(db->db));
        return;
    }

    sqlite3_bind_text(stmt, 1, user_id, -1, SQLITE_STATIC);

    printf("\n%-20s %-10s %-15s %-15s\n",
           "交易编号", "费用类型", "缴费金额", "缴费日期");
    printf("----------------------------------------------------------------------\n");

    int found = 0;
    double total_paid = 0.0;
    
    while (sqlite3_step(stmt) == SQLITE_ROW)
    {
        const unsigned char *transaction_id = sqlite3_column_text(stmt, 0);
        const unsigned char *fee_type = sqlite3_column_text(stmt, 1);
        double amount = sqlite3_column_double(stmt, 2);
        time_t pay_date = sqlite3_column_int64(stmt, 3);
        
        // 验证日期的有效性
        if (pay_date > 0)
        {
            found = 1;
            char date_buf[20];
            strftime(date_buf, sizeof(date_buf), "%Y-%m-%d", localtime(&pay_date));

            // 缩短交易ID的显示长度
            char short_id[9] = {0};
            strncpy(short_id, (const char *)transaction_id, 8);

            printf("%-20s %-10s ￥%-14.2f %s\n",
                   short_id,
                   fee_type,
                   amount,
                   date_buf);
                   
            total_paid += amount;
        }
    }

    sqlite3_finalize(stmt);

    if (!found)
    {
        printf("\n⚠️ 当前用户暂无缴费记录。\n");
    }
    else
    {
        printf("----------------------------------------------------------------------\n");
        printf("累计已缴金额: ￥%.2f\n", total_paid);
    }

    printf("\n说明：显示所有已完成的缴费记录\n");
    printf("\n按Enter键返回...");
    getchar();
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
    // system("clear||cls");
    printf("===== 缴费总金额 =====\n");
    double total = query_total_fee(db, user_id);
    char username[100];
    query_username_by_user_id(db, user_id, username);

    if (total < 0)
    {
        printf("❌ 查询失败: 请检查数据库或用户记录。\n");
    }
    else if (total == 0)
    {
        printf("用户 %s 暂无缴费记录。\n", username);
    }
    else
    {
        printf("用户 %s 的累计缴费总额：￥%.2f\n", username, total);
    }

    printf("\n按任意键返回...");
    clear_input_buffer();
}
void process_payment_screen(Database *db, const char *user_id)
{
    clear_screen();
    printf("\n============= 费用缴纳 =============\n\n");

    const char *fee_types[] = {"物业费", "水电费", "停车费", "维修基金"};
    const int valid_days[] = {365, 30, 30, 365};

    printf("请选择费用类型:\n");
    for (int i = 0; i < sizeof(fee_types) / sizeof(fee_types[0]); i++)
    {
        printf("%d. %s\n", i + 1, fee_types[i]);
    }
    printf("\n选择(1-%d): ", (int)(sizeof(fee_types) / sizeof(fee_types[0])));

    int fee_type;
    if (scanf("%d", &fee_type) != 1 || fee_type < 1 || fee_type > (int)(sizeof(fee_types) / sizeof(fee_types[0])))
    {
        printf("❌ 无效选择\n");
        clear_input_buffer();
        return;
    }
    clear_input_buffer();

    const char *query_unpaid =
        "SELECT transaction_id, amount, due_date "
        "FROM transactions "
        "WHERE user_id = ? AND fee_type = ? AND status = 0 "
        "ORDER BY due_date ASC";

    sqlite3_stmt *stmt_unpaid;
    if (sqlite3_prepare_v2(db->db, query_unpaid, -1, &stmt_unpaid, NULL) == SQLITE_OK)
    {
        sqlite3_bind_text(stmt_unpaid, 1, user_id, -1, SQLITE_STATIC);
        sqlite3_bind_int(stmt_unpaid, 2, fee_type);

        struct UnpaidRecord
        {
            char transaction_id[37];
            double amount;
            time_t due_date;
        } unpaid_records[100];
        int record_count = 0;
        double total_unpaid = 0.0;

        while (sqlite3_step(stmt_unpaid) == SQLITE_ROW && record_count < 100)
        {
            const char *trans_id = (const char *)sqlite3_column_text(stmt_unpaid, 0);
            if (trans_id) {
                strncpy(unpaid_records[record_count].transaction_id,
                        trans_id,
                        sizeof(unpaid_records[record_count].transaction_id) - 1);
                unpaid_records[record_count].transaction_id[sizeof(unpaid_records[record_count].transaction_id) - 1] = '\0';
                
                unpaid_records[record_count].amount = sqlite3_column_double(stmt_unpaid, 1);
                unpaid_records[record_count].due_date = sqlite3_column_int64(stmt_unpaid, 2);
                total_unpaid += unpaid_records[record_count].amount;
                record_count++;
            }
        }
        sqlite3_finalize(stmt_unpaid);

        if (record_count > 0)
        {
            printf("\n=== 未缴%s记录 ===\n", fee_types[fee_type - 1]);
            printf("交易编号\t\t金额\t\t到期日期\n");
            printf("--------------------------------------------------\n");
            
            for (int i = 0; i < record_count; i++)
            {
                char date_str[20];
                strftime(date_str, sizeof(date_str), "%Y-%m-%d", 
                        localtime(&unpaid_records[i].due_date));
                printf("%-24s ￥%-10.2f %s\n",
                       unpaid_records[i].transaction_id,
                       unpaid_records[i].amount,
                       date_str);
            }
            printf("--------------------------------------------------\n");
            printf("总计: %d笔待缴费用，共计￥%.2f\n", record_count, total_unpaid);

            printf("\n是否一次性缴清所有欠费? (1-是/0-否): ");
            int choice;
            scanf("%d", &choice);
            clear_input_buffer();

            time_t now = time(NULL);
            
            if (choice == 1)
            {
                sqlite3_exec(db->db, "BEGIN TRANSACTION", 0, 0, 0);

                for (int i = 0; i < record_count; i++)
                {
                    const char *update_query =
                        "UPDATE transactions "
                        "SET status = 1, payment_date = ? "
                        "WHERE transaction_id = ?";

                    sqlite3_stmt *update_stmt;
                    if (sqlite3_prepare_v2(db->db, update_query, -1, &update_stmt, NULL) == SQLITE_OK)
                    {
                        sqlite3_bind_int64(update_stmt, 1, (sqlite3_int64)now);
                        sqlite3_bind_text(update_stmt, 2, unpaid_records[i].transaction_id, -1, SQLITE_STATIC);
                        sqlite3_step(update_stmt);
                        sqlite3_finalize(update_stmt);
                    }
                }

                sqlite3_exec(db->db, "COMMIT", 0, 0, 0);
                char display_date[20];
                strftime(display_date, sizeof(display_date), "%Y-%m-%d", localtime(&now));

                printf("\n════════════════════════\n");
                printf("✅ 批量缴费成功\n");
                printf("────────────────────────\n");
                printf("用户ID: %s\n", user_id);
                printf("费用类型: %s\n", fee_types[fee_type - 1]);
                printf("缴费总额: ￥%.2f\n", total_unpaid);
                printf("缴费日期: %s\n", display_date);
                printf("缴费笔数: %d\n", record_count);
            }
           // process_payment_screen 部分缴费部分的修改代码
else
{
    double amount;
    do
    {
        printf("\n请输入缴费金额（最大￥%.2f）: ", total_unpaid);
        if (scanf("%lf", &amount) != 1)
        {
            printf("❌ 请输入有效数字\n");
            clear_input_buffer();
            continue;
        }

        if (amount <= 0)
        {
            printf("❌ 金额必须大于0\n");
        }
        else if (amount > 100000)
        {
            printf("❌ 单笔缴费不得超过10万元\n");
        }
        else if (amount > total_unpaid)
        {
            printf("❌ 缴费金额不能超过应缴总额\n");
        }
        else
        {
            break;
        }
        clear_input_buffer();
    } while (1);

    sqlite3_exec(db->db, "BEGIN TRANSACTION", 0, 0, 0);
    double remaining_amount = amount;

    // 首先创建一条已支付记录
    char new_transaction_id[37];
    generate_uuid(new_transaction_id);
    
    const char *insert_query = 
        "INSERT INTO transactions "
        "(transaction_id, user_id, fee_type, amount, status, payment_date) "
        "VALUES (?, ?, ?, ?, 1, ?)";

    sqlite3_stmt *insert_stmt;
    if (sqlite3_prepare_v2(db->db, insert_query, -1, &insert_stmt, NULL) == SQLITE_OK)
    {
        sqlite3_bind_text(insert_stmt, 1, new_transaction_id, -1, SQLITE_STATIC);
        sqlite3_bind_text(insert_stmt, 2, user_id, -1, SQLITE_STATIC);
        sqlite3_bind_int(insert_stmt, 3, fee_type);
        sqlite3_bind_double(insert_stmt, 4, amount);
        sqlite3_bind_int64(insert_stmt, 5, (sqlite3_int64)now);
        sqlite3_step(insert_stmt);
        sqlite3_finalize(insert_stmt);
    }

    // 更新原始未支付记录
    for (int i = 0; i < record_count && remaining_amount > 0; i++)
    {
        double current_payment = (remaining_amount >= unpaid_records[i].amount)
                                   ? unpaid_records[i].amount
                                   : remaining_amount;

        if (current_payment >= unpaid_records[i].amount)
        {
            // 如果完全支付了这条记录
            const char *update_query =
                "UPDATE transactions "
                "SET status = 1, payment_date = ? "
                "WHERE transaction_id = ?";

            sqlite3_stmt *update_stmt;
            if (sqlite3_prepare_v2(db->db, update_query, -1, &update_stmt, NULL) == SQLITE_OK)
            {
                sqlite3_bind_int64(update_stmt, 1, (sqlite3_int64)now);
                sqlite3_bind_text(update_stmt, 2, unpaid_records[i].transaction_id, -1, SQLITE_STATIC);
                sqlite3_step(update_stmt);
                sqlite3_finalize(update_stmt);
            }
        }
        else
        {
            // 如果是部分支付
            const char *update_query =
                "UPDATE transactions "
                "SET amount = amount - ? "
                "WHERE transaction_id = ?";

            sqlite3_stmt *update_stmt;
            if (sqlite3_prepare_v2(db->db, update_query, -1, &update_stmt, NULL) == SQLITE_OK)
            {
                sqlite3_bind_double(update_stmt, 1, current_payment);
                sqlite3_bind_text(update_stmt, 2, unpaid_records[i].transaction_id, -1, SQLITE_STATIC);
                sqlite3_step(update_stmt);
                sqlite3_finalize(update_stmt);
            }
        }
        remaining_amount -= current_payment;
    }

    sqlite3_exec(db->db, "COMMIT", 0, 0, 0);
    char display_date[20];
    strftime(display_date, sizeof(display_date), "%Y-%m-%d", localtime(&now));

    printf("\n════════════════════════\n");
    printf("✅ 缴费成功\n");
    printf("────────────────────────\n");
    printf("用户ID: %s\n", user_id);
    printf("费用类型: %s\n", fee_types[fee_type - 1]);
    printf("缴费金额: ￥%.2f\n", amount);
    printf("缴费日期: %s\n", display_date);
    printf("剩余未缴: ￥%.2f\n", total_unpaid - amount);
}
            printf("════════════════════════\n");
        }
        else
        {
            printf("\n✓ 当前无未支付的%s。\n", fee_types[fee_type - 1]);
        }
    }

    printf("\n按Enter键返回主菜单...");
    getchar();
}
// 查询剩余费用
void query_remaining_balance(Database *db, const char *user_id)
{
    // system("clear||cls");
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
// 查询小区基本信息
void query_community_info(Database *db)
{
    printf("\n============= 小区基本信息 =============\n\n");
    printf("小区名称: 灰灰小区\n");
    printf("\n楼盘信息:\n");

    // 使用 QueryResult 结构体存储查询结果
    QueryResult result;
    const char *sql = 
        "SELECT building_id, building_name, address, floors_count "
        "FROM buildings ORDER BY building_id;";

    if (execute_query(db, sql, &result))
    {
        printf("%-20s %-30s %-10s\n",
               "楼宇名称", "地址", "楼层数");
        printf("--------------------------------------------------------\n");

        for (int i = 0; i < result.row_count; i++)
        {
            printf("%-20s %-30s %-10s\n",
                   result.rows[i].values[1],  // building_name
                   result.rows[i].values[2],  // address
                   result.rows[i].values[3]); // floors_count
        }

        printf("--------------------------------------------------------\n");
        printf("共 %d 栋楼\n", result.row_count);

        free_query_result(&result);
    }

    printf("\n住户情况: 目前总住户578人\n");
    printf("配套设施: 健身房、游泳池、儿童乐园\n");
    printf("物业服务: 24小时物业服务中心\n");
    printf("当前状态: 一切正常\n");
    printf("\n=======================================\n");
    
    printf("\n按任意键返回...");
    clear_input_buffer();
    getchar();
}
// 查询收费信息
void query_fee_info(Database *db, const char *user_id)
{
    // system("clear||cls");
    printf("===== 物业费标准 =====\n");

    // 定义费用类型
    const char *fee_type_names[] = {
        "其他费用",
        "物业费",
        "停车费",
        "水费",
        "电费",
        "燃气费",
    };

    int fee_types[] = {1, 2, 3, 4, 5}; // 对应物业费、停车费、水费、电费、燃气费
    int fee_count = sizeof(fee_types) / sizeof(fee_types[0]);

    // 打印表头
    printf("费用ID\t\t费用名称\t\t费用金额\t\t单位\n");
    printf("--------------------------------------------\n");

    // 遍历每种费用类型
    int found = 0;
    for (int i = 0; i < fee_count; i++)
    {
        FeeStandard standard;
        if (get_current_fee_standard(db, fee_types[i], &standard))
        {
            found = 1;

            // 格式化日期
            char effective_date[20] = {0};
            time_t eff_time = standard.effective_date;
            strftime(effective_date, sizeof(effective_date), "%Y-%m-%d", localtime(&eff_time));

            // 输出费用信息
            printf("%-8s\t%-16s\t￥%.2f\t\t/%s\n",
                   standard.standard_id,
                   fee_type_names[standard.fee_type],
                   standard.price_per_unit,
                   standard.unit);
        }
    }

    // 检查是否无数据
    if (!found)
    {
        printf("\n⚠️ 当前无物业费标准数据。\n");
    }

    printf("\n--------------------------------------------\n");
    printf("按任意键返回...");
    clear_input_buffer();
}

// 查询服务人员信息
void query_service_staff_info(Database *db, const char *user_id)
{
    printf("\n============= 服务人员信息 =============\n\n");

    const char *query =
        "SELECT "
        "   u.user_id, "
        "   u.username, "
        "   u.name, "
        "   u.phone_number, "
        "   u.email "
        "FROM users u "
        "WHERE u.role_id = 'role_staff' "
        "ORDER BY u.name;";

    sqlite3_stmt *stmt;
    if (sqlite3_prepare_v2(db->db, query, -1, &stmt, NULL) != SQLITE_OK)
    {
        fprintf(stderr, "查询失败: %s\n", sqlite3_errmsg(db->db));
        return;
    }

    int found = 0;
    while (sqlite3_step(stmt) == SQLITE_ROW)
    {
        found = 1;
        // 移除了 if (!found) 判断，直接显示数据
        printf("\n工号: %-20s\n", sqlite3_column_text(stmt, 0));
        printf("姓名: %-20s\n", sqlite3_column_text(stmt, 2));
        printf("联系电话: %-20s\n", sqlite3_column_text(stmt, 3));
        printf("电子邮箱: %-20s\n", sqlite3_column_text(stmt, 4));
        printf("\n-------------------------------------\n");
    }

    if (!found)
    {
        printf("\n⚠️ 暂无服务人员信息\n");
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
        printf("1--更改信息     2--缴费管理      3--信息查询      0--退出\n");
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
    while (1)
    {
        printf("更改您的用户名---1    更改您的密码---2    返回上一级---0\n");
        int num;

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
        }
        else if (num == 2)
        {
            handle_change_password(db, user_id, user_type);
        }
        else if (num == 0)
        {
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
        // system("clear||cls");
        printf("\n===== 缴费管理 =====\n");
        printf("1. 查看缴费记录\n");
        printf("2. 缴纳费用\n");
        printf("3. 查询应缴费用\n");
        printf("4.查询缴费总金额\n");
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
    clear_screen();
    update_overdue_transactions(db);

    printf("\n===== 应缴费用查询 =====\n\n");

    // 1. 查询未支付和逾期的费用总额（排除已支付的）
    const char *total_query =
        "SELECT fee_type, amount, due_date, status "
        "FROM transactions "
        "WHERE user_id = ? "
        "AND status != 1 " // 排除已支付的
        "ORDER BY fee_type";

    sqlite3_stmt *stmt;
    if (sqlite3_prepare_v2(db->db, total_query, -1, &stmt, NULL) != SQLITE_OK)
    {
        fprintf(stderr, "查询准备失败: %s\n", sqlite3_errmsg(db->db));
        return;
    }

    sqlite3_bind_text(stmt, 1, user_id, -1, SQLITE_STATIC);

    // 用于统计的变量
    double total_due = 0.0;
    double total_overdue = 0.0;

    printf("费用明细:\n");
    printf("----------------------------------------\n");
    printf("费用类型\t金额\t\t到期日\t\t状态\n");

    int found = 0;
    while (sqlite3_step(stmt) == SQLITE_ROW)
    {
        found = 1;
        const char *fee_type = (const char *)sqlite3_column_text(stmt, 0);
        double amount = sqlite3_column_double(stmt, 1);
        time_t due_date = sqlite3_column_int64(stmt, 2);
        int status = sqlite3_column_int(stmt, 3);

        // 格式化到期日期
        char date_str[20];
        strftime(date_str, sizeof(date_str), "%Y-%m-%d", localtime(&due_date));

        // 累计总额
        if (status == 0 || status == 2)
        {
            total_due += amount;
            if (status == 2)
            {
                total_overdue += amount;
            }
        }

        // 显示状态
        const char *status_str = (status == 0) ? "未支付" : (status == 2) ? "已逾期"
                                                                          : "未知";

        printf("%-12s\t￥%.2f\t%s\t%s\n",
               fee_type, amount, date_str, status_str);
    }

    sqlite3_finalize(stmt);

    if (!found)
    {
        printf("\n当前无待缴费用。\n");
    }
    else
    {
        printf("----------------------------------------\n");
        printf("总应缴金额: ￥%.2f\n", total_due);
        if (total_overdue > 0)
        {
            printf("⚠️ 其中逾期金额: ￥%.2f\n", total_overdue);
        }
    }

    printf("\n----------------------------------------\n");
    printf("说明: \n");
    printf("- 此处仅显示未支付和逾期的费用\n");
    printf("- 请及时缴纳费用，避免产生逾期\n");

    printf("\n按任意键返回...");
    getchar();
}
// 查询特定业主的缴费信息
void query_owner_payment_info(Database *db, const char *user_id)
{
    printf("\n===== 缴费记录查询 =====\n");

    const char *query =
        "SELECT CAST(transaction_id AS INTEGER) as id, "
        "fee_type, amount, payment_date, status "
        "FROM transactions WHERE user_id = ? "
        "ORDER BY payment_date DESC;";

    sqlite3_stmt *stmt;
    if (sqlite3_prepare_v2(db->db, query, -1, &stmt, NULL) != SQLITE_OK)
    {
        printf("查询失败: %s\n", sqlite3_errmsg(db->db));
        return;
    }

    sqlite3_bind_text(stmt, 1, user_id, -1, SQLITE_STATIC);

    printf("\n%-6s %-10s %-10s %-12s %-8s\n",
           "单号", "费用类型", "金额", "支付日期", "状态");
    printf("----------------------------------------\n");

    while (sqlite3_step(stmt) == SQLITE_ROW)
    {
        const char *status_text = sqlite3_column_int(stmt, 4) == 1 ? "已支付" : "未支付";

        printf("%-6d %-10d %-10.2f %-12s %-8s\n",
               sqlite3_column_int(stmt, 0),
               sqlite3_column_int(stmt, 1),
               sqlite3_column_double(stmt, 2),
               sqlite3_column_text(stmt, 3) ? (const char *)sqlite3_column_text(stmt, 3) : "未支付",
               status_text);
    }

    sqlite3_finalize(stmt);
}

// 查询所有业主的缴费情况
void query_all_owners_payment_info(Database *db)
{
    // system("clear||cls");
    printf("===== 查询所有业主缴费情况 =====\n");

    const char *query = "SELECT u.user_id, u.name, SUM(t.amount) AS total_paid FROM users u "
                        "LEFT JOIN transactions t ON u.user_id = t.user_id "
                        "WHERE u.role_id = 'role_owner' GROUP BY u.user_id, u.name;";
    sqlite3_stmt *stmt;
    int rc = sqlite3_prepare_v2(db->db, query, -1, &stmt, NULL);
    if (rc != SQLITE_OK)
    {
        fprintf(stderr, "SQLite错误: %s\n", sqlite3_errmsg(db->db));
        return;
    }

    printf("业主ID\t姓名\t\t累计缴费金额\n");
    printf("-----------------------------------------\n");
    while ((rc = sqlite3_step(stmt)) == SQLITE_ROW)
    {
        const char *user_id = sqlite3_column_text(stmt, 0);
        const char *name = sqlite3_column_text(stmt, 1);
        double total_paid = sqlite3_column_double(stmt, 2);

        printf("%s\t%s\t\t%.2f\n", user_id, name, total_paid);
    }

    sqlite3_finalize(stmt);
    printf("\n按任意键返回...\n");
    clear_input_buffer();
}
