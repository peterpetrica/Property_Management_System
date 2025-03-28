/*
 * 文件名: ui_staff.c
 * 功能: 实现物业管理系统中服务人员相关的用户界面
 *
 * 描述:
 * 本文件负责物业服务人员界面的所有交互功能，包括:
 * - 服务人员主界面
 * - 物业服务管理（创建、更新、删除服务请求等）
 * - 服务人员信息查询（按各种条件查找）
 * - 服务人员信息排序（按不同字段排序显示）
 * - 服务人员数据统计（统计各类服务数据）
 * - 系统维护功能
 *
 * 每个界面函数负责显示菜单、接收用户输入并调用相应的业务逻辑函数
 */

#include "ui/ui_staff.h"
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

// 清除输入缓冲区
void clear_staff_input_buffer()
{
    int c;
    while ((c = getchar()) != '\n' && c != EOF)
        ;
}

// 清屏函数
void clear_staff_screen()
{
#ifdef _WIN32
    // system("clear||cls");
#else
    system("clear");
#endif
}

// 等待用户按键返回
int wait_for_user()
{
    printf("按任意键返回主菜单...\n");
    getchar();
    getchar(); // 等待用户输入
    return 0;  // Return an integer as per the declaration
}

// 显示服务人员主界面
void show_staff_main_screen(Database *db, const char *user_id, UserType user_type)
{
    if (user_type != USER_STAFF)
    {
        printf("无权访问\n");
        return;
    }
    int choice = 0;
    do
    {
        clear_staff_screen(); // 修复清屏命令兼容性
        printf("欢迎来到服务人员主页面\n");
        printf("1.显示个人信息\n");
        printf("2.修改个人信息\n");
        printf("3.服务人员信息查询\n");
        printf("4.服务人员信息排序\n");
        printf("5.服务人员数据统计\n");
        printf("0.退出\n");
        printf("请选择: \n");
        if (scanf("%d", &choice) != 1)
        {
            printf("输入错误，请重试\n");
            clear_staff_input_buffer(); // 清空缓冲区
            continue;
        }
        switch (choice)
        {
        case 1:
            show_staff_personal_info_screen(db, user_id, user_type); // 修复语法错误
            break;
        case 2:
            modify_personal_info_screen(db, user_id, user_type);
            break;
        case 3:
            show_staff_query_screen(db, user_id, user_type);
            break;
        case 4:
            show_staff_sort_screen(db, user_id, user_type);
            break;
        case 5:
            show_staff_statistics_screen(db, user_id, user_type);
            break;
        case 6:
            show_staff_maintenance_screen(db, user_id, user_type);
            break;
        case 0:
            printf("退出服务人员主界面\n");
            break;
        default:
            printf("无效选择，请重试\n");
        }
    } while (choice != 0);
}

// 重命名为 show_staff_personal_info_screen
void show_staff_personal_info_screen(Database *db, const char *user_id, UserType user_type)
{
    clear_staff_screen();
    printf("个人信息界面\n");

    Staff staff;
    memset(&staff, 0, sizeof(Staff));

    if (!get_staff_by_id(db, user_id, &staff))
    {
        printf("获取个人信息失败\n");
        wait_for_user();
        return;
    }
    printf("姓名: %s\n", staff.name);
    printf("联系方式: %s\n", staff.phone_number);
    printf("服务类型: %s\n", staff.staff_type_id);

    wait_for_user();
}

// 修改个人信息界面
void modify_personal_info_screen(Database *db, const char *user_id, UserType user_type)
{
    clear_staff_screen();
    printf("修改个人信息界面\n");
    Staff staff;

    if (!get_staff_by_id(db, user_id, &staff))
    {
        printf("获取个人信息失败\n");
        wait_for_user();
        return;
    }
    const char *service_types[] = {"保洁", "保安", "管理员", "维修工", "园艺师"};
    const int service_type_count = sizeof(service_types) / sizeof(service_types[0]);
    int choice = 0;
    do
    {
        clear_staff_screen();
        printf("当前信息:\n");
        printf("1. 姓名: %s\n", staff.name);
        printf("2. 联系方式: %s\n", staff.phone_number);
        printf("3. 服务类型: %s\n", staff.staff_type_id);
        printf("0. 返回主菜单\n");
        printf("请选择要修改的选项: ");

        if (scanf("%d", &choice) != 1)
        {
            printf("输入错误，请重试\n");
            clear_staff_input_buffer(); // 清空缓冲区
            continue;
        }
        switch (choice)
        {
        case 1:
        {
            char new_name[40];
            do
            {
                printf("当前姓名: %s\n", staff.name);
                printf("请输入新的姓名: ");
                scanf("%s", new_name);
                int valid = 1;
                for (int i = 0; new_name[i] != '\0'; i++)
                {
                    if (new_name[i] >= '0' && new_name[i] <= '9')
                    {
                        valid = 0;
                        break;
                    }
                }
                if (!valid)
                {
                    printf("不合法名字，请重新输入\n");
                }
                else if (strcmp(staff.name, new_name) == 0)
                {
                    printf("新姓名不能与原姓名相同，请重新输入\n");
                }
                else
                {
                    strcpy(staff.name, new_name);
                    break;
                }
            } while (1);
            break;
        }
        case 2:
        {
            char new_phone_number[40];
            do
            {
                printf("当前联系方式: %s\n", staff.phone_number);
                printf("请输入新的联系方式: ");
                scanf("%s", new_phone_number);
                int valid = 1;
                if (strlen(new_phone_number) != 11)
                {
                    valid = 0;
                }
                else
                {
                    for (int i = 0; new_phone_number[i] != '\0'; i++)
                    {
                        if (new_phone_number[i] < '0' || new_phone_number[i] > '9')
                        {
                            valid = 0;
                            break;
                        }
                    }
                }
                if (!valid)
                {
                    printf("不合法联系方式，请重新输入\n");
                }
                else if (strcmp(staff.phone_number, new_phone_number) == 0)
                {
                    printf("新联系方式不能与原联系方式相同，请重新输入\n");
                }
                else
                {
                    strcpy(staff.phone_number, new_phone_number);
                    break;
                }
            } while (1);
            break;
        }
        case 3:
        {
            do
            {
                printf("当前服务类型: %s\n", staff.staff_type_id);
                printf("可选服务类型：\n");
                for (int i = 0; i < service_type_count; i++)
                {
                    printf("%d. %s\n", i + 1, service_types[i]);
                }
                printf("请输入服务类型编号(1-%d): ", service_type_count);
                int choice;
                if (scanf("%d", &choice) != 1 || choice < 1 || choice > service_type_count)
                {
                    printf("输入错误，请重试\n");
                    clear_staff_input_buffer(); // 清空缓冲区
                    continue;
                }
                const char *selected_service_type = service_types[choice - 1];
                if (strcmp(staff.staff_type_id, selected_service_type) == 0)
                {
                    printf("新服务类型不能与原服务类型相同，请重新输入\n");
                }
                else
                {
                    strcpy(staff.staff_type_id, selected_service_type);
                    break;
                }
            } while (1);
            break;
        }
        case 0:
            printf("返回主菜单...\n");
            return;
        default:
            printf("无效选择，请重试\n");
            break;
        }
    } while (choice != 0);
    // 更新信息
    if (update_staff(db, user_id, user_type, &staff))
    {
        printf("个人信息更新成功\n");
        if (!get_staff_by_id(db, user_id, &staff))
        {
            printf("获取个人信息失败\n");
            wait_for_user();
            return;
        }
    }
    else
    {
        printf("个人信息更新失败，请重试\n");
    }
    wait_for_user();
}

// 服务人员信息查询界面
void show_staff_query_screen(Database *db, const char *user_id, UserType user_type)
{
    int choice;
    while (1)
    {
        clear_staff_screen();
        printf("\n===== 服务人员信息查询 =====\n\n");
        printf("1. 查询用户基本信息\n");
        printf("2. 查询用户缴费情况\n");
        printf("3. 查询用户房屋信息\n");
        printf("0. 返回上一级\n");
        printf("\n请输入您的选择: ");

        if (scanf("%d", &choice) != 1)
        {
            clear_staff_input_buffer();
            printf("输入无效，请重新输入\n");
            continue;
        }
        clear_staff_input_buffer();

        switch (choice)
        {
        case 1:
        {
            clear_staff_screen();
            printf("\n===== 用户基本信息 =====\n\n");
            const char *query =
                "SELECT u.user_id, u.username, u.name, u.phone_number, u.email "
                "FROM users u "
                "WHERE u.role_id = 'role_owner' "
                "ORDER BY u.user_id;";

            sqlite3_stmt *stmt;
            if (sqlite3_prepare_v2(db->db, query, -1, &stmt, NULL) == SQLITE_OK)
            {
                // 修改表头格式，使用更紧凑的布局
                printf("--------------------------------------------------------------------------------------------------------\n");
                printf("| %-8s | %-12s | %-12s | %-12s | %-25s |\n",
                       "ID", "用户名", "姓名", "电话", "邮箱");
                printf("--------------------------------------------------------------------------------------------------------\n");

                int found = 0;
                while (sqlite3_step(stmt) == SQLITE_ROW)
                {
                    found = 1;
                    const char *full_id = (const char *)sqlite3_column_text(stmt, 0);
                    char short_id[9]; // 只显示ID的前8位
                    strncpy(short_id, full_id, 8);
                    short_id[8] = '\0';

                    // 使用格式化的表格样式
                    printf("| %-8s | %-12s | %-12s | %-12s | %-25s |\n",
                           short_id,
                           sqlite3_column_text(stmt, 1) ? (char *)sqlite3_column_text(stmt, 1) : "N/A",
                           sqlite3_column_text(stmt, 2) ? (char *)sqlite3_column_text(stmt, 2) : "N/A",
                           sqlite3_column_text(stmt, 3) ? (char *)sqlite3_column_text(stmt, 3) : "N/A",
                           sqlite3_column_text(stmt, 4) ? (char *)sqlite3_column_text(stmt, 4) : "N/A");
                    printf("--------------------------------------------------------------------------------------------------------\n");
                }

                if (!found)
                {
                    printf("\n⚠️ 未找到任何用户信息\n");
                }

                sqlite3_finalize(stmt);
            }
        }
        break;

        case 2:
            case2_handler(db);
            break;

        case 3:
            // 查询用户房屋信息
            {
                clear_staff_screen();
                printf("\n===== 用户房屋信息 =====\n\n");
                const char *query =
                    "SELECT u.user_id, u.username, b.building_name, r.room_number, r.floor, r.area_sqm "
                    "FROM users u "
                    "LEFT JOIN rooms r ON u.user_id = r.owner_id "
                    "LEFT JOIN buildings b ON r.building_id = b.building_id "
                    "WHERE u.role_id = 'role_owner' "
                    "ORDER BY b.building_name, r.room_number;";

                sqlite3_stmt *stmt;
                if (sqlite3_prepare_v2(db->db, query, -1, &stmt, NULL) == SQLITE_OK)
                {
                    printf("%-12s%-16s%-16s%-16s%-16s%-16s\n",
                           "用户ID", "用户名", "楼号", "房间号", "楼层", "面积(㎡)");
                    printf("------------------------------------------------------------------------\n");

                    int found = 0;
                    while (sqlite3_step(stmt) == SQLITE_ROW)
                    {
                        found = 1;
                        printf("%-12s%-16s%-16s%-16s%-16d%-16.2f\n",
                               sqlite3_column_text(stmt, 0),
                               sqlite3_column_text(stmt, 1),
                               sqlite3_column_text(stmt, 2),
                               sqlite3_column_text(stmt, 3),
                               sqlite3_column_int(stmt, 4),
                               sqlite3_column_double(stmt, 5));
                    }

                    if (!found)
                    {
                        printf("\n未找到任何用户房屋信息\n");
                    }

                    sqlite3_finalize(stmt);
                }
            }
            break;

        case 0:
            return;

        default:
            printf("无效的选择，请重新输入\n");
        }

        printf("\n按Enter键继续...");
        getchar();
    }
}

// 比较函数：按姓名比较服务人员
int compare_staff_by_name(const void *a, const void *b)
{
    const Staff *staff1 = (const Staff *)a;
    const Staff *staff2 = (const Staff *)b;
    return strcmp(staff1->name, staff2->name);
}

// 服务人员信息排序界面
void show_staff_sort_screen(Database *db, const char *user_id, UserType user_type)
{
    clear_staff_screen();
    printf("服务人员信息排序界面\n");

    Staff staff_list[100]; // 定义 staff_list 数组
    int count = query_all_staff(db, staff_list, 100);
    if (count <= 0)
    {
        printf("没有找到任何服务人员信息\n");
    }
    else
    {
        qsort(staff_list, count, sizeof(Staff), compare_staff_by_name);
        printf("按姓名排序后的服务人员信息:\n");
        for (int i = 0; i < count; i++)
        {
            printf("%d. 姓名: %s, 联系方式: %s, 服务类型: %s\n",
                   i + 1, staff_list[i].name, staff_list[i].phone_number, staff_list[i].staff_type_id);
        }
    }

    printf("按任意键返回主菜单...\n");
    getchar();
    getchar(); // 等待用户输入
}

// 服务人员信息统计界面
void show_staff_statistics_screen(Database *db, const char *user_id, UserType user_type)
{
    clear_staff_screen();
    printf("服务人员数据统计界面\n");

    int total_staff = count_all_staff(db);
    printf("服务人员总数: %d\n", total_staff);
    const char *query = "SELECT staff_type_id, COUNT(*) FROM staff GROUP BY staff_type_id";
    sqlite3_stmt *stmt;
    int rc = sqlite3_prepare_v2(db->db, query, -1, &stmt, NULL);
    if (rc != SQLITE_OK)
    {
        printf("SQLite错误: %s\n", sqlite3_errmsg(db->db));
        return;
    }
    printf("服务人员类型统计:\n");
    while ((rc = sqlite3_step(stmt)) == SQLITE_ROW)
    {
        const char *staff_type = sqlite3_column_text(stmt, 0);
        int count = sqlite3_column_int(stmt, 1);
        printf("服务人员类型: %s, 人数: %d\n", staff_type, count);
    }
    sqlite3_finalize(stmt);
    wait_for_user();
}

// 服务人员系统维护界面
void show_staff_maintenance_screen(Database *db, const char *user_id, UserType user_type)
{
    clear_staff_screen();
    printf("服务人员系统维护界面\n");

    printf("1. 数据备份\n");
    printf("2. 数据恢复\n");
    printf("0. 返回主菜单\n");
    int choice;
    if (scanf("%d", &choice) != 1)
    {
        printf("输入错误，请重试\n");
        clear_staff_input_buffer(); // 清空缓冲区
        return;
    }

    switch (choice)
    {
    case 1:
        if (backup_database(db))
        {
            printf("数据备份成功\n");
        }
        else
        {
            printf("数据备份失败\n");
        }
        break;
    case 2:
        if (restore_database(db))
        {
            printf("数据恢复成功\n");
        }
        else
        {
            printf("数据恢复失败\n");
        }
        break;
    case 0:
        return;
    default:
        printf("无效选择，请重试\n");
    }

    wait_for_user();
}

// 显示当前费用标准
void show_current_fee_standards(Database *db)
{
    printf("\n===== 当前收费标准 =====\n\n");
    const char *query =
        "SELECT fee_type, price_per_unit, unit, "
        "(CASE "
        "  WHEN fee_type = 1 THEN '物业费' "
        "  WHEN fee_type = 2 THEN '停车费' "
        "  WHEN fee_type = 3 THEN '水费' "
        "  WHEN fee_type = 4 THEN '电费' "
        "  WHEN fee_type = 5 THEN '燃气费' "
        "  ELSE '其他费用' END) as type_name "
        "FROM fee_standards "
        "WHERE end_date = 0 OR end_date > strftime('%s','now') "
        "ORDER BY fee_type;";

    sqlite3_stmt *stmt;
    if (sqlite3_prepare_v2(db->db, query, -1, &stmt, NULL) == SQLITE_OK)
    {
        printf("┌────────────┬──────────┬─────────────┐\n");
        printf("│ %-10s │ %-8s │ %-11s │\n",
               "费用类型", "单价", "计费单位");
        printf("├────────────┼──────────┼─────────────┤\n");

        while (sqlite3_step(stmt) == SQLITE_ROW)
        {
            printf("│ %-10s │ %8.2f │ %-11s │\n",
                   sqlite3_column_text(stmt, 3),
                   sqlite3_column_double(stmt, 1),
                   sqlite3_column_text(stmt, 2));
        }
        printf("└────────────┴──────────┴─────────────┘\n");
        sqlite3_finalize(stmt);
    }
}

// 修改查询用户缴费情况函数
void case2_handler(Database *db)
{
    clear_staff_screen();
    show_current_fee_standards(db);

    printf("\n===== 用户缴费情况 =====\n\n");
    const char *query =
        "WITH room_fees AS ("
        "    SELECT r.owner_id, "
        "           u.username, "
        "           u.name, "
        "           r.room_number, "
        "           r.area_sqm, "
        "           fs.fee_type, "
        "           fs.price_per_unit, "
        "           CASE "
        "               WHEN fs.fee_type = 1 THEN r.area_sqm * fs.price_per_unit "
        "               WHEN fs.fee_type = 2 THEN fs.price_per_unit "
        "               ELSE 0 "
        "           END as fee_amount "
        "    FROM users u "
        "    JOIN rooms r ON u.user_id = r.owner_id "
        "    CROSS JOIN fee_standards fs "
        "    WHERE u.role_id = 'role_owner' "
        "    AND fs.fee_type IN (1, 2) " // 只计算物业费和停车费
        "    AND (fs.end_date = 0 OR fs.end_date > strftime('%s','now'))"
        "), user_summary AS ("
        "    SELECT rf.owner_id, "
        "           rf.username, "
        "           rf.name, "
        "           rf.room_number, "
        "           SUM(rf.fee_amount) as monthly_due, "
        "           GROUP_CONCAT("
        "               CASE "
        "                   WHEN rf.fee_type = 1 THEN '物业费:' || printf('%.2f', rf.fee_amount) "
        "                   WHEN rf.fee_type = 2 THEN '停车费:' || printf('%.2f', rf.fee_amount) "
        "               END, '; '"
        "           ) as fee_details, "
        "           COALESCE((SELECT SUM(amount) FROM transactions "
        "                    WHERE user_id = rf.owner_id "
        "                    AND status = 1 " // 已支付
        "                    AND period_start >= strftime('%s','now','start of month')"
        "                    AND period_end < strftime('%s','now','start of month','+1 month')"
        "           ), 0) as paid_amount, "
        "           COALESCE((SELECT SUM(amount) FROM transactions "
        "                    WHERE user_id = rf.owner_id "
        "                    AND status IN (0, 2) " // 未支付或逾期
        "                    AND period_start >= strftime('%s','now','start of month')"
        "                    AND period_end < strftime('%s','now','start of month','+1 month')"
        "           ), 0) as unpaid_amount "
        "    FROM room_fees rf "
        "    GROUP BY rf.owner_id, rf.username, rf.name, rf.room_number"
        ")"
        "SELECT * FROM user_summary "
        "ORDER BY username;";

    sqlite3_stmt *stmt;
    if (sqlite3_prepare_v2(db->db, query, -1, &stmt, NULL) == SQLITE_OK)
    {
        printf("┌───────┬──────────┬──────────┬──────────┬────────────┬────────────┬────────────┬───────────────────────┐\n");
        printf("│ %-5s │ %-8s │ %-8s │ %-8s │ %-10s │ %-10s │ %-10s │ %-19s │\n",
               "序号", "用户名", "姓名", "房号", "月应缴", "已缴金额", "未缴金额", "费用明细");
        printf("├───────┼──────────┼──────────┼──────────┼────────────┼────────────┼────────────┼───────────────────────┤\n");

        int row_num = 0;
        double total_monthly = 0;
        double total_paid = 0;
        double total_unpaid = 0;

        while (sqlite3_step(stmt) == SQLITE_ROW)
        {
            row_num++;
            const char *username = (const char *)sqlite3_column_text(stmt, 1);
            const char *name = (const char *)sqlite3_column_text(stmt, 2);
            const char *room_number = (const char *)sqlite3_column_text(stmt, 3);
            double monthly_fee = sqlite3_column_double(stmt, 4);
            const char *fee_details = (const char *)sqlite3_column_text(stmt, 5);
            double paid = sqlite3_column_double(stmt, 6);
            double unpaid = sqlite3_column_double(stmt, 7);

            // 转换费用明细格式
            char formatted_details[100] = {0};
            int pos = 0;
            const char *p = fee_details;
            while (*p && pos < 99)
            {
                if (*p == '1')
                {
                    pos += snprintf(formatted_details + pos, 100 - pos, "物业费:%.2f;", atof(strchr(p, ':') + 1));
                }
                else if (*p == '2')
                {
                    pos += snprintf(formatted_details + pos, 100 - pos, "停车费:%.2f;", atof(strchr(p, ':') + 1));
                }
                p = strchr(p, ';');
                if (!p)
                    break;
                p++;
            }

            printf("│ %-5d │ %-8s │ %-8s │ %-8s │ %10.2f │ %10.2f │ %10.2f │ %-19s │\n",
                   row_num,
                   username ? username : "N/A",
                   name ? name : "N/A",
                   room_number ? room_number : "N/A",
                   monthly_fee,
                   paid,
                   unpaid,
                   formatted_details);

            total_monthly += monthly_fee;
            total_paid += paid;
            total_unpaid += unpaid;
        }

        if (row_num > 0)
        {
            printf("├───────┴──────────┴──────────┴──────────┼────────────┼────────────┼────────────┼───────────────────────┤\n");
            printf("│ 总计                                   │ %10.2f │ %10.2f │ %10.2f │                    │\n",
                   total_monthly, total_paid, total_unpaid);
            printf("└──────────────────────────────────────┴────────────┴────────────┴────────────┴───────────────────────┘\n");
        }
        else
        {
            printf("└───────┴──────────┴──────────┴──────────┴────────────┴────────────┴────────────┴───────────────────────┘\n");
            printf("\n未找到任何用户缴费信息\n");
        }

        sqlite3_finalize(stmt);
    }
}

// 发送缴费提醒函数
void send_payment_reminder(Database *db, const char *user_id, double unpaid_amount, const char *fee_types)
{
    char reminder[512];
    char username[100];
    time_t now = time(NULL);
    struct tm *t = localtime(&now);

    // 获取用户名
    if (!query_username_by_user_id(db, user_id, username))
    {
        strcpy(username, "业主");
    }

    // 获取当前费用标准信息
    char fee_standards[256] = "";
    get_current_fee_standards_info(db, fee_standards, sizeof(fee_standards));

    // 生成提醒内容
    snprintf(reminder, sizeof(reminder),
             "尊敬的%s:\n\n"
             "您目前有以下费用未缴纳：\n"
             "总金额：%.2f元\n"
             "费用类型：%s\n\n"
             "请您及时缴纳费用。如已缴费请忽略此提醒。\n\n"
             "发送时间：%d-%02d-%02d\n"
             "物业服务中心",
             username, unpaid_amount, fee_types,
             t->tm_year + 1900, t->tm_mon + 1, t->tm_mday);

    // 保存提醒到数据库
    char query[1024];
    snprintf(query, sizeof(query),
             "INSERT INTO payment_reminders (user_id, reminder_content, send_time, status) "
             "VALUES ('%s', '%s', %ld, 0)",
             user_id, reminder, (long)now);

    if (execute_update(db, query))
    {
        printf("\n已成功向用户 %s 发送缴费提醒\n", username);
    }
    else
    {
        printf("\n发送提醒失败\n");
    }
}

// 获取当前费用标准信息
void get_current_fee_standards_info(Database *db, char *buffer, size_t buffer_size)
{
    const char *query =
        "SELECT fee_type, price_per_unit, unit "
        "FROM fee_standards "
        "WHERE end_date = 0 OR end_date > strftime('%s','now') "
        "ORDER BY fee_type;";

    sqlite3_stmt *stmt;
    if (sqlite3_prepare_v2(db->db, query, -1, &stmt, NULL) == SQLITE_OK)
    {
        char *curr_pos = buffer;
        int remaining = buffer_size;

        while (sqlite3_step(stmt) == SQLITE_ROW)
        {
            const char *type_str;
            int fee_type = sqlite3_column_int(stmt, 0);

            switch (fee_type)
            {
            case TRANS_PROPERTY_FEE:
                type_str = "物业费";
                break;
            case TRANS_PARKING_FEE:
                type_str = "停车费";
                break;
            case TRANS_WATER_FEE:
                type_str = "水费";
                break;
            case TRANS_ELECTRICITY_FEE:
                type_str = "电费";
                break;
            case TRANS_GAS_FEE:
                type_str = "燃气费";
                break;
            default:
                type_str = "其他费用";
                break;
            }

            int written = snprintf(curr_pos, remaining, "%s: %.2f %s\n",
                                   type_str,
                                   sqlite3_column_double(stmt, 1),
                                   sqlite3_column_text(stmt, 2));

            if (written >= remaining)
                break;
            curr_pos += written;
            remaining -= written;
        }
        sqlite3_finalize(stmt);
    }
}
