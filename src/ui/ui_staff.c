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
#include "models/transaction.h"  // 添加此行
#include "models/service.h"
#include "db/db_query.h"
#include "utils/utils.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <ctype.h> // 添加这行

// 静态函数声明
static void wait_for_key(void);
static const char* get_fee_type_name(int fee_type); 
static void print_user_table_header();
static void print_user_info(sqlite3_stmt *stmt);

// 实现静态函数
static const char* get_fee_type_name(int fee_type) {
    switch(fee_type) {
        case 1: return "物业费";
        case 2: return "停车费";
        case 3: return "水费"; 
        case 4: return "电费";
        case 5: return "燃气费";
        default: return "其他费用";
    }
}

static void wait_for_key() {
    printf("\n按任意键继续...");
    getchar();
    getchar();
}

// 批量发送提醒函数声明 
void send_bulk_payment_reminders(Database *db) {
    const char *query = 
        "SELECT DISTINCT u.user_id, u.username, SUM(t.amount) as total "
        "FROM users u "
        "JOIN transactions t ON u.user_id = t.user_id "
        "WHERE t.status = 0 "
        "GROUP BY u.user_id;";

    sqlite3_stmt *stmt;
    if (sqlite3_prepare_v2(db->db, query, -1, &stmt, NULL) == SQLITE_OK) {
        int count = 0;
        while (sqlite3_step(stmt) == SQLITE_ROW) {
            const char *user_id = (const char*)sqlite3_column_text(stmt, 0);
            const char *username = (const char*)sqlite3_column_text(stmt, 1); 
            double amount = sqlite3_column_double(stmt, 2);
            
            send_payment_reminder(db, user_id, amount, "");
            count++;
        }
        printf("\n✓ 已成功发送%d条提醒\n", count);
        sqlite3_finalize(stmt);
    }
    wait_for_key();
}

// 清屏函数
void clear_staff_screen()
{
#ifdef _WIN32
    system("cls");
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
    return 0; // Return an integer as per the declaration
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
        clear_staff_screen();
        printf("\n┌───────────── 物业服务人员系统 ─────────────┐\n");
        printf("│                                              │\n");
        printf("│  1. 查看负责区域信息                        │\n");
        printf("│  2. 查询业主缴费情况                        │\n");
        printf("│  3. 发送缴费提醒                            │\n");
        printf("│  4. 显示业主排序列表                        │\n");
        printf("│  5. 查看缴费统计                            │\n");
        printf("│  0. 退出系统                                │\n");
        printf("│                                              │\n");
        printf("└──────────────────────────────────────────┘\n");
        printf("\n请选择功能 (0-5): ");

        if (scanf("%d", &choice) != 1)
        {
            printf("输入错误，请重试\n");
            clear_input_buffer();
            continue;
        }

        switch (choice)
        {
            case 1:
                show_staff_service_areas(db, user_id);
                break;
            case 2:
                show_owner_payment_query(db);
                break;
            case 3:
                show_payment_reminder_menu(db, user_id);  // 修改这里,传入user_id参数user_id);
                break;
            case 4:
                show_sorted_owners(db);
                break;
            case 5:
                show_payment_status_statistics(db);
                break;
            case 0:
                printf("退出系统\n");
                break;
            default:
                printf("无效选择，请重试\n");
        }
    } while (choice != 0);
}

void show_payment_status_statistics(Database *db)
{
    clear_staff_screen();
    printf("\n=== 缴费情况统计 ===\n\n");
    
    const char *query = 
        "SELECT "
        "(SELECT COUNT(DISTINCT u.user_id) FROM users u WHERE u.role_id = 'role_owner') as total_owners, "
        "(SELECT COUNT(DISTINCT t.user_id) FROM transactions t WHERE t.status = 1) as paid_count, "
        "(SELECT COUNT(DISTINCT t.user_id) FROM transactions t WHERE t.status = 0) as unpaid_count, "
        "(SELECT SUM(t.amount) FROM transactions t WHERE t.status = 1) as total_paid, "
        "(SELECT SUM(t.amount) FROM transactions t WHERE t.status = 0) as total_unpaid";

    sqlite3_stmt *stmt;
    if (sqlite3_prepare_v2(db->db, query, -1, &stmt, NULL) == SQLITE_OK) {
        if (sqlite3_step(stmt) == SQLITE_ROW) {
            int total_owners = sqlite3_column_int(stmt, 0);
            int paid_count = sqlite3_column_int(stmt, 1);
            int unpaid_count = sqlite3_column_int(stmt, 2);
            double total_paid = sqlite3_column_double(stmt, 3);
            double total_unpaid = sqlite3_column_double(stmt, 4);
            
            printf("┌─────────────────────────────────┐\n");
            printf("│ 总业主数量: %-8d             │\n", total_owners);
            printf("│ 已缴费业主: %-8d (%.1f%%)     │\n", 
                   paid_count, total_owners > 0 ? (float)paid_count/total_owners*100 : 0);
            printf("│ 未缴费业主: %-8d (%.1f%%)     │\n", 
                   unpaid_count, total_owners > 0 ? (float)unpaid_count/total_owners*100 : 0);
            printf("│ 已收费总额: %-8.2f元         │\n", total_paid);
            printf("│ 未收费总额: %-8.2f元         │\n", total_unpaid);
            printf("└─────────────────────────────────┘\n");
        }
        sqlite3_finalize(stmt);
    }
    
    wait_for_key();
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
    int choice=0;
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
            clear_input_buffer(); // 清空缓冲区
            continue;
        }
        switch(choice)
        {
            case 1:
            {
                char new_name[40];
                do
                {
                    printf("当前姓名: %s\n", staff.name);
                    printf("请输入新的姓名: ");
                    scanf("%s", new_name);
                    int valid=1;
                    for(int i=0;new_name[i]!='\0';i++)
                    {
                        if(new_name[i]>='0'&&new_name[i]<='9')
                        {
                            valid=0;
                            break;
                        }
                    }
                    if(!valid)
                    {
                        printf("不合法名字，请重新输入\n");
                    }
                    else if(strcmp(staff.name, new_name) == 0)
                    {
                        printf("新姓名不能与原姓名相同，请重新输入\n");
                    }
                    else
                    {
                        strcpy(staff.name, new_name);
                        break;
                    }
                }while(1);
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
                int valid=1;
                if(strlen(new_phone_number)!=11)
                {
                    valid=0;
                }
                else
                {
                    for(int i=0;new_phone_number[i]!='\0';i++)
                    {
                        if(new_phone_number[i]<'0'||new_phone_number[i]>'9')
                        {
                            valid=0;
                            break;
                        }
                    }
                }
                if(!valid)
                {
                    printf("不合法联系方式，请重新输入\n");
                }
                else if(strcmp(staff.phone_number, new_phone_number) == 0)
                {
                    printf("新联系方式不能与原联系方式相同，请重新输入\n");
                }
                else
                {
                    strcpy(staff.phone_number, new_phone_number);
                    break;
                }
            }while(1);
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
            printf("请输入服务类型编号(1-%d): ",service_type_count);
            int choice;
            if (scanf("%d", &choice) != 1||choice<1||choice>service_type_count)
            {
                printf("输入错误，请重试\n");
                clear_input_buffer(); // 清空缓冲区
                continue;
            }
            const char*selected_service_type=service_types[choice-1];
            if(strcmp(staff.staff_type_id, selected_service_type) == 0)
            {
                printf("新服务类型不能与原服务类型相同，请重新输入\n");
            }
            else
            {
                strcpy(staff.staff_type_id, selected_service_type);
                break;
            }
        }while(1);
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
    //更新信息
    if (update_staff(db, user_id, user_type, &staff))
    {
        printf("个人信息更新成功\n");
        if(!get_staff_by_id(db, user_id, &staff))
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
            clear_input_buffer();
            printf("输入无效，请重新输入\n");
            continue;
        }
        clear_input_buffer();

        switch (choice)
        {
        case 1:
            {
                clear_staff_screen();
                printf("\n===== 用户基本信息 =====\n\n");
                const char *query = 
                    "SELECT u.user_id, u.username, u.name, "
                    "u.phone_number, u.email, u.registration_date, "
                    "u.id_number, u.gender, u.address "
                    "FROM users u "
                    "WHERE u.role_id = 'role_owner' "
                    "AND u.registration_date > 0 "  // 只显示通过注册获得的用户
                    "ORDER BY u.registration_date DESC;";  // 按注册时间倒序排列

                sqlite3_stmt *stmt;
                if (sqlite3_prepare_v2(db->db, query, -1, &stmt, NULL) == SQLITE_OK)
                {
                    printf("┌────────────────────────────────────────────────────────────────────────────────────┐\n");
                    printf("│ %-12s │ %-10s │ %-8s │ %-11s │ %-15s │ %-10s │ %-20s │\n", 
                           "用户ID", "用户名", "姓名", "电话", "注册时间", "性别", "身份证号");
                    printf("├────────────────────────────────────────────────────────────────────────────────────┤\n");
                    
                    int found = 0;
                    while (sqlite3_step(stmt) == SQLITE_ROW)
                    {
                        found = 1;
                        const char *user_id = (const char *)sqlite3_column_text(stmt, 0);
                        const char *username = (const char *)sqlite3_column_text(stmt, 1);
                        const char *name = (const char *)sqlite3_column_text(stmt, 2);
                        const char *phone = (const char *)sqlite3_column_text(stmt, 3);
                        const char *email = (const char *)sqlite3_column_text(stmt, 4);
                        time_t reg_time = sqlite3_column_int64(stmt, 5);
                        const char *gender = (const char *)sqlite3_column_text(stmt, 7);
                        const char *id_number = (const char *)sqlite3_column_text(stmt, 6);
                        
                        // 格式化注册时间
                        char time_str[20];
                        struct tm *tm_info = localtime(&reg_time);
                        strftime(time_str, sizeof(time_str), "%Y-%m-%d", tm_info);

                        // 隐藏身份证号中间部分
                        char masked_id[20] = "未填写";
                        if (id_number && strlen(id_number) == 18) {
                            snprintf(masked_id, sizeof(masked_id), "%6s******%4s", 
                                    id_number, id_number + 14);
                        }

                        printf("│ %-12s │ %-10s │ %-8s │ %-11s │ %-15s │ %-10s │ %-20s │\n",
                            user_id ? user_id : "N/A",
                            username ? username : "N/A",
                            name ? name : "N/A",
                            phone ? phone : "未填写",
                            time_str,
                            gender ? gender : "未填写",
                            id_number ? masked_id : "未填写");
                    }
                    
                    printf("└────────────────────────────────────────────────────────────────────────────────────┘\n");
                    
                    if (!found) {
                        printf("\n⚠️ 未找到任何注册用户信息\n");
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
                    int found = 0;
                    while (sqlite3_step(stmt) == SQLITE_ROW)
                    {
                        found = 1;
                        printf("%-12s%-16s%-16s%-16s%-16d%-16.2f\n",
                            sqlite3_column_text(stmt, 0),
                            sqlite3_column_text(stmt, 1),
                            sqlite3_column_text(stmt, 2),
                            sqlite3_column_text(stmt, 3),
                            sqlite3_column_int(stmt, 4),     // 使用%d而不是%s
                            sqlite3_column_double(stmt, 5));
                    }
                    
                    if (!found) {
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
        clear_input_buffer(); // 清空缓冲区
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
    int choice;
    char owner_name[64];
    int year;
    
    do {
        clear_staff_screen();
        printf("\n===== 用户缴费信息查询 =====\n");
        printf("1. 查询某业主某年缴费情况\n");
        printf("2. 查询某业主所有缴费记录\n");
        printf("3. 显示所有已缴费用户\n");
        printf("4. 显示所有未缴费用户\n");
        printf("5. 查看当前收费标准\n");
        printf("0. 返回上级菜单\n");
        printf("\n请选择: ");
        
        scanf("%d", &choice);
        clear_input_buffer();
        
        switch(choice) {
            case 1:
                printf("\n请输入业主姓名: ");
                scanf("%s", owner_name);
                printf("请输入查询年份: ");
                scanf("%d", &year);
                query_owner_payment_by_year(db, owner_name, year);
                break;
                
            case 2:
                printf("\n请输入业主姓名: ");
                scanf("%s", owner_name);
                query_owner_all_payments(db, owner_name);
                break;
                
            case 3:
                show_paid_users(db);
                break;
                
            case 4:
                show_unpaid_users(db);
                break;
                
            case 5:
                show_current_fee_standards(db);
                break;
                
            case 0:
                return;
                
            default:
                printf("无效选择，请重试\n");
        }
        
        printf("\n按Enter键继续...");
        getchar();
    } while(choice != 0);
}

void query_owner_payment_by_year(Database *db, const char *owner_name, int year)
{
    clear_staff_screen();
    printf("\n=== 查询业主 %s 在 %d 年的缴费情况 ===\n\n", owner_name, year);

    const char *query =
        "SELECT t.fee_type, t.amount, t.status, t.payment_date, t.due_date, "
        "b.building_name, r.room_number "
        "FROM transactions t "
        "JOIN users u ON t.user_id = u.user_id "
        "LEFT JOIN rooms r ON t.room_id = r.room_id "
        "LEFT JOIN buildings b ON r.building_id = b.building_id "
        "WHERE u.name = ? AND strftime('%Y', datetime(t.payment_date, 'unixepoch')) = ? "
        "ORDER BY t.payment_date DESC";

    sqlite3_stmt *stmt;
    if (sqlite3_prepare_v2(db->db, query, -1, &stmt, NULL) == SQLITE_OK) {
        char year_str[5];
        snprintf(year_str, sizeof(year_str), "%d", year);

        sqlite3_bind_text(stmt, 1, owner_name, -1, SQLITE_STATIC);
        sqlite3_bind_text(stmt, 2, year_str, -1, SQLITE_STATIC);

        double total_paid = 0.0;
        double total_unpaid = 0.0;

        printf("┌──────────────────────────────────────────────────────────────────────┐\n");
        printf("│ %-8s │ %-8s │ %-6s │ %-10s │ %-10s │ %-8s │ %-6s │\n",
               "费用类型", "金额", "状态", "缴费日期", "到期日期", "楼号", "房号");
        printf("├──────────────────────────────────────────────────────────────────────┤\n");

        bool found = false;
        while (sqlite3_step(stmt) == SQLITE_ROW) {
            found = true;
            const char *fee_type = get_fee_type_name(sqlite3_column_int(stmt, 0));
            double amount = sqlite3_column_double(stmt, 1);
            int status = sqlite3_column_int(stmt, 2);
            time_t payment_date = sqlite3_column_int64(stmt, 3);
            time_t due_date = sqlite3_column_int64(stmt, 4);
            const char *building = sqlite3_column_text(stmt, 5);
            const char *room = sqlite3_column_text(stmt, 6);

            char payment_date_str[20] = "未缴费";
            char due_date_str[20];

            if (payment_date > 0) {
                strftime(payment_date_str, sizeof(payment_date_str), "%Y-%m-%d", localtime(&payment_date));
                total_paid += amount;
            } else {
                total_unpaid += amount;
            }
            strftime(due_date_str, sizeof(due_date_str), "%Y-%m-%d", localtime(&due_date));

            printf("│ %-8s │ %8.2f │ %-6s │ %-10s │ %-10s │ %-8s │ %-6s │\n",
                   fee_type, amount,
                   status == 1 ? "已缴费" : "未缴费",
                   payment_date_str, due_date_str,
                   building ? building : "N/A",
                   room ? room : "N/A");
        }

        if (found) {
            printf("├──────────────────────────────────────────────────────────────────────┤\n");
            printf("│ 统计信息: 已缴费总额: %-10.2f    未缴费总额: %-10.2f        │\n",
                   total_paid, total_unpaid);
        }
        printf("└──────────────────────────────────────────────────────────────────────┘\n");

        if (!found) {
            printf("\n⚠️ 未找到业主 %s 在 %d 年的缴费记录\n", owner_name, year);
        }

        sqlite3_finalize(stmt);
    }
    wait_for_key();
}

void query_owner_all_payments(Database *db, const char *owner_name)
{
    clear_staff_screen();
    printf("\n=== 查询业主 %s 的所有缴费情况 ===\n\n", owner_name);

    const char *query =
        "SELECT t.transaction_id, t.fee_type, t.amount, t.status, t.payment_date, t.due_date "
        "FROM transactions t "
        "JOIN users u ON t.user_id = u.user_id "
        "WHERE u.name = ? "
        "ORDER BY t.payment_date DESC";

    sqlite3_stmt *stmt;
    if (sqlite3_prepare_v2(db->db, query, -1, &stmt, NULL) == SQLITE_OK) {
        sqlite3_bind_text(stmt, 1, owner_name, -1, SQLITE_STATIC);

        printf("┌─────────────────────────────────────────────────────────────┐\n");
        printf("│ %-10s│ %-10s│ %-8s│ %-12s│ %-12s│\n",
               "缴费类型", "金额", "状态", "缴费日期", "到期日期");
        printf("├─────────────────────────────────────────────────────────────┤\n");

        bool found = false;
        while (sqlite3_step(stmt) == SQLITE_ROW) {
            found = true;
            const char *fee_type = get_fee_type_name(sqlite3_column_int(stmt, 1));
            double amount = sqlite3_column_double(stmt, 2);
            int status = sqlite3_column_int(stmt, 3);
            time_t payment_date = sqlite3_column_int64(stmt, 4);
            time_t due_date = sqlite3_column_int64(stmt, 5);

            char payment_date_str[20] = "未缴费";
            char due_date_str[20];

            if (payment_date > 0) {
                strftime(payment_date_str, sizeof(payment_date_str), "%Y-%m-%d", localtime(&payment_date));
            }
            strftime(due_date_str, sizeof(due_date_str), "%Y-%m-%d", localtime(&due_date));

            printf("│ %-10s│ %10.2f│ %-8s│ %-12s│ %-12s│\n",
                   fee_type, amount,
                   status == 1 ? "已缴费" : "未缴费",
                   payment_date_str, due_date_str);
        }
        printf("└─────────────────────────────────────────────────────────────┘\n");

        if (!found) {
            printf("\n⚠️ 未找到业主 %s 的缴费记录\n", owner_name);
        }

        sqlite3_finalize(stmt);
    }
    wait_for_key();
}

void show_sorted_users(Database *db)
{
    clear_staff_screen();
    printf("\n===== 用户排序显示 =====\n\n");
    
    const char *query = 
        "SELECT u.name, u.phone_number, b.building_name, r.room_number, "
        "(SELECT COUNT(*) FROM transactions t WHERE t.user_id = u.user_id AND t.status = 0) as unpaid_count "
        "FROM users u "
        "LEFT JOIN rooms r ON u.user_id = r.owner_id "
        "LEFT JOIN buildings b ON r.building_id = b.building_id "
        "WHERE u.role_id = 'role_owner' "
        "ORDER BY u.name ASC, unpaid_count DESC";

    sqlite3_stmt *stmt;
    if (sqlite3_prepare_v2(db->db, query, -1, &stmt, NULL) == SQLITE_OK) {
        printf("┌──────────────────────────────────────────────────────────────┐\n");
        printf("│ %-10s │ %-12s │ %-8s │ %-8s │ %-10s │\n",
               "姓名", "电话", "楼号", "房号", "未缴笔数");
        printf("├──────────────────────────────────────────────────────────────┤\n");

        while (sqlite3_step(stmt) == SQLITE_ROW) {
            printf("│ %-10s │ %-12s │ %-8s │ %-8s │ %-10d │\n",
                   sqlite3_column_text(stmt, 0),
                   sqlite3_column_text(stmt, 1),
                   sqlite3_column_text(stmt, 2),
                   sqlite3_column_text(stmt, 3),
                   sqlite3_column_int(stmt, 4));
        }
        printf("└──────────────────────────────────────────────────────────────┘\n");
        sqlite3_finalize(stmt);
    }
}

void show_payment_statistics(Database *db)
{
    clear_staff_screen();
    printf("\n===== 缴费统计信息 =====\n\n");
    
    // 1. 统计今年的缴费情况
    int current_year = 2024; // 可以根据需要获取当前年份
    
    const char *year_stats_query = 
        "SELECT "
        "(SELECT COUNT(DISTINCT user_id) FROM transactions WHERE status = 0 AND strftime('%Y', datetime(due_date, 'unixepoch')) = ?) as unpaid_count, "
        "(SELECT COUNT(DISTINCT user_id) FROM transactions WHERE status = 1 AND strftime('%Y', datetime(payment_date, 'unixepoch')) = ?) as paid_count, "
        "(SELECT COUNT(DISTINCT user_id) FROM users WHERE role_id = 'role_owner') as total_owners";

    sqlite3_stmt *stmt;
    if (sqlite3_prepare_v2(db->db, year_stats_query, -1, &stmt, NULL) == SQLITE_OK) {
        char year_str[5];
        snprintf(year_str, sizeof(year_str), "%d", current_year);
        
        sqlite3_bind_text(stmt, 1, year_str, -1, SQLITE_STATIC);
        sqlite3_bind_text(stmt, 2, year_str, -1, SQLITE_STATIC);
        
        if (sqlite3_step(stmt) == SQLITE_ROW) {
            int unpaid_count = sqlite3_column_int(stmt, 0);
            int paid_count = sqlite3_column_int(stmt, 1);
            int total_owners = sqlite3_column_int(stmt, 2);
            
            printf("【%d年度统计】\n", current_year);
            printf("总业主数量: %d\n", total_owners);
            printf("已缴费业主: %d (%.1f%%)\n", paid_count, 
                   total_owners > 0 ? (float)paid_count/total_owners*100 : 0);
            printf("未缴费业主: %d (%.1f%%)\n\n", unpaid_count,
                   total_owners > 0 ? (float)unpaid_count/total_owners*100 : 0);
        }
        sqlite3_finalize(stmt);
    }
    
    // 2. 分类型统计未缴费金额
    const char *unpaid_amount_query = 
        "SELECT fee_type, COUNT(DISTINCT user_id) as user_count, SUM(amount) as total_amount "
        "FROM transactions "
        "WHERE status = 0 "
        "GROUP BY fee_type "
        "ORDER BY total_amount DESC";

    if (sqlite3_prepare_v2(db->db, unpaid_amount_query, -1, &stmt, NULL) == SQLITE_OK) {
        printf("【未缴费金额统计】\n");
        printf("┌────────────┬──────────┬────────────┐\n");
        printf("│ %-10s │ %-8s │ %-10s │\n", "费用类型", "未缴人数", "未缴总额");
        printf("├────────────┼──────────┼────────────┤\n");
        
        double total_unpaid = 0;
        while (sqlite3_step(stmt) == SQLITE_ROW) {
            int fee_type = sqlite3_column_int(stmt, 0);
            int user_count = sqlite3_column_int(stmt, 1);
            double amount = sqlite3_column_double(stmt, 2);
            total_unpaid += amount;
            
            printf("│ %-10s │ %-8d │ %10.2f │\n",
                   get_fee_type_name(fee_type),
                   user_count,
                   amount);
        }
        printf("├────────────┼──────────┼────────────┤\n");
        printf("│ %-10s │ %-8s │ %10.2f │\n", "合计", "", total_unpaid);
        printf("└────────────┴──────────┴────────────┘\n");
        
        sqlite3_finalize(stmt);
    }
}

// 发送缴费提醒函数信息
void send_payment_reminder(Database *db, const char *user_id, double unpaid_amount, const char *fee_types) 
{
    char reminder[512]; 
    char username[100];
    time_t now = time(NULL);
    struct tm *t = localtime(&now);

    // 获取用户名
    if (!query_username_by_user_id(db, user_id, username)) {
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

    if (execute_update(db, query)) {
        printf("\n已成功向用户 %s 发送缴费提醒\n", username);
    } else {
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
    if (sqlite3_prepare_v2(db->db, query, -1, &stmt, NULL) == SQLITE_OK) {
        char *curr_pos = buffer;
        int remaining = buffer_size;

        while (sqlite3_step(stmt) == SQLITE_ROW) {
            const char *type_str;
            int fee_type = sqlite3_column_int(stmt, 0);

            switch (fee_type) {
                case TRANS_PROPERTY_FEE: type_str = "物业费"; break;
                case TRANS_PARKING_FEE: type_str = "停车费"; break;
                case TRANS_WATER_FEE: type_str = "水费"; break;
                case TRANS_ELECTRICITY_FEE: type_str = "电费"; break;
                case TRANS_GAS_FEE: type_str = "燃气费"; break;
                default: type_str = "其他费用"; break;
            }

            int written = snprintf(curr_pos, remaining, "%s: %.2f %s\n",
                                 type_str,
                                 sqlite3_column_double(stmt, 1),
                                 sqlite3_column_text(stmt, 2));

            if (written >= remaining) break;
            curr_pos += written;
            remaining -= written;
        }
        sqlite3_finalize(stmt);
    }
}

// 显示用户详细信息函数
void show_user_detail(Database *db, const char *user_id) {
    clear_staff_screen();
    printf("\n=== 用户详细信息 ===\n\n");
    
    const char *query = 
        "SELECT u.username, u.name, u.phone_number, u.email, "
        "u.registration_date, u.gender, u.address, "
        "b.building_name, r.room_number, r.area_sqm "
        "FROM users u "
        "LEFT JOIN rooms r ON u.user_id = r.owner_id "
        "LEFT JOIN buildings b ON r.building_id = b.building_id "
        "WHERE u.user_id = ?";

    sqlite3_stmt *stmt;
    if (sqlite3_prepare_v2(db->db, query, -1, &stmt, NULL) == SQLITE_OK) {
        sqlite3_bind_text(stmt, 1, user_id, -1, SQLITE_STATIC);
        
        if (sqlite3_step(stmt) == SQLITE_ROW) {
            printf("┌─ 基本信息 ───────────────────────────────┐\n");
            printf("│ 用户名: %-35s │\n", sqlite3_column_text(stmt, 0));
            printf("│ 姓  名: %-35s │\n", sqlite3_column_text(stmt, 1));
            printf("│ 电  话: %-35s │\n", sqlite3_column_text(stmt, 2));
            printf("│ 邮  箱: %-35s │\n", sqlite3_column_text(stmt, 3));
            printf("│ 性  别: %-35s │\n", sqlite3_column_text(stmt, 5));
            printf("│ 地  址: %-35s │\n", sqlite3_column_text(stmt, 6));
            printf("└─────────────────────────────────────────┘\n");
            const char *name = (const char *)sqlite3_column_text(stmt, 0);
            printf("\n┌─ 房产信息 ───────────────────────────────┐\n");
            printf("│ 楼  号: %-35s │\n", sqlite3_column_text(stmt, 7));
            printf("│ 房  号: %-35s │\n", sqlite3_column_text(stmt, 8));
            printf("│ 面  积: %-35.2f │\n", sqlite3_column_double(stmt, 9));
            printf("└─────────────────────────────────────────┘\n");
            time_t reg_date = sqlite3_column_int64(stmt, 4);
            char date_str[20];
            strftime(date_str, sizeof(date_str), "%Y-%m-%d", localtime(&reg_date));
            printf("\n注册时间: %s\n", date_str);
        }   
        sqlite3_finalize(stmt);
    }
}

// 添加缴费提醒操作函数
void send_payment_reminder_screen(Database *db, const char *user_id) {
    clear_staff_screen();
    printf("\n=== 发送缴费提醒 ===\n");

    const char *query = 
        "SELECT u.name, u.phone_number, t.amount, t.due_date, "
        "CASE t.fee_type "
        "    WHEN 1 THEN '物业费' "
        "    WHEN 2 THEN '停车费' "
        "    WHEN 3 THEN '水费' "
        "    WHEN 4 THEN '电费' "
        "    WHEN 5 THEN '燃气费' "
        "END as fee_type "
        "FROM transactions t "
        "JOIN users u ON t.user_id = u.user_id "
        "WHERE t.user_id = ? AND t.status = 0";

    sqlite3_stmt *stmt;
    if (sqlite3_prepare_v2(db->db, query, -1, &stmt, NULL) == SQLITE_OK) {
        sqlite3_bind_text(stmt, 1, user_id, -1, SQLITE_STATIC);
        printf("┌ 未缴费详情 ───────────────────────────────────────┐\n");
        double total_amount = 0;
        char fee_types[256] = "";
        
        while (sqlite3_step(stmt) == SQLITE_ROW) {
            const char *name = (const char *)sqlite3_column_text(stmt, 0);
            const char *phone = (const char *)sqlite3_column_text(stmt, 1);
            double amount = sqlite3_column_double(stmt, 2);
            time_t due_date = sqlite3_column_int64(stmt, 3);
            const char *fee_type = (const char *)sqlite3_column_text(stmt, 4);
            
            total_amount += amount;
            if (strlen(fee_types) > 0) {
                strcat(fee_types, "、");
            }
            strcat(fee_types, fee_type);
            
            char due_date_str[20];
            strftime(due_date_str, sizeof(due_date_str), "%Y-%m-%d", localtime(&due_date));
            
            printf("│ 费用类型: %-10s  金额: %-8.2f  到期: %-10s │\n",
                   fee_type, amount, due_date_str);
        }
        printf("└──────────────────────────────────────────────────┘\n");
        printf("\n是否发送缴费提醒？(Y/N): ");
        char choice;
        scanf(" %c", &choice);
        
        if (toupper(choice) == 'Y') {
            send_payment_reminder(db, user_id, total_amount, fee_types);
            printf("\n✓ 提醒已发送\n");
        }
        
        sqlite3_finalize(stmt);
    }
    wait_for_user();
}

// 添加函数声明
void show_user_detail(Database *db, const char *user_id);
void send_payment_reminder_screen(Database *db, const char *user_id);
void send_bulk_payment_reminders(Database *db);

void show_user_query_menu(Database *db) {
    clear_staff_screen();
    int choice;
    do {
        printf("\n=== 用户信息查询系统 ===\n");
        printf("1. 按姓名查询用户\n");
        printf("2. 按房间号查询用户\n");
        printf("3. 查询用户缴费状态\n");
        printf("4. 显示所有用户\n");
        printf("0. 返回上级菜单\n");
        printf("请选择: ");
        
        scanf("%d", &choice);
        clear_input_buffer();
        
        switch(choice) {
            case 1: query_user_by_name(db); break;
            case 2: query_user_by_room(db); break;
            case 3: query_user_payment_status(db); break;
            case 4: show_all_users(db); break;
            case 0: return;
            default: printf("无效选择，请重试\n");
        }
    } while(choice != 0);
}

void query_user_by_name(Database *db) {
    char name[64];
    printf("\n请输入要查询的用户姓名: ");
    scanf("%s", name);
    
    const char *query = 
        "SELECT u.user_id, u.username, u.name, u.phone_number, "
        "u.email, u.registration_date, r.room_number, b.building_name "
        "FROM users u "
        "LEFT JOIN rooms r ON u.user_id = r.owner_id "
        "LEFT JOIN buildings b ON r.building_id = b.building_id "
        "WHERE u.name LIKE ? AND u.role_id = 'role_owner'";

    sqlite3_stmt *stmt;
    if (sqlite3_prepare_v2(db->db, query, -1, &stmt, NULL) == SQLITE_OK) {
        char name_pattern[66];
        snprintf(name_pattern, sizeof(name_pattern), "%%%s%%", name);
        sqlite3_bind_text(stmt, 1, name_pattern, -1, SQLITE_STATIC);
        
        print_user_table_header();
        
        while (sqlite3_step(stmt) == SQLITE_ROW) {
            print_user_info(stmt);
        }
        
        sqlite3_finalize(stmt);
    }
    wait_for_key();
}

void query_user_by_room(Database *db) {
    char building[32], room[32];
    printf("\n请输入楼号: ");
    scanf("%s", building);
    printf("请输入房间号: ");
    scanf("%s", room);
    
    const char *query = 
        "SELECT u.user_id, u.username, u.name, u.phone_number, "
        "u.email, u.registration_date, r.room_number, b.building_name "
        "FROM users u "
        "JOIN rooms r ON u.user_id = r.owner_id "
        "JOIN buildings b ON r.building_id = b.building_id "
        "WHERE b.building_name = ? AND r.room_number = ?";

    sqlite3_stmt *stmt;
    if (sqlite3_prepare_v2(db->db, query, -1, &stmt, NULL) == SQLITE_OK) {
        sqlite3_bind_text(stmt, 1, building, -1, SQLITE_STATIC);
        sqlite3_bind_text(stmt, 2, room, -1, SQLITE_STATIC);
        
        print_user_table_header();
        
        while (sqlite3_step(stmt) == SQLITE_ROW) {
            print_user_info(stmt);
        }
        
        sqlite3_finalize(stmt);
    }
    wait_for_key();
}

void query_user_payment_status(Database *db) {
    const char *query = 
        "SELECT u.user_id, u.username, u.name, COUNT(t.transaction_id) as unpaid_count, "
        "SUM(t.amount) as total_amount "
        "FROM users u "
        "LEFT JOIN transactions t ON u.user_id = t.user_id AND t.status = 0 "
        "WHERE u.role_id = 'role_owner' "
        "GROUP BY u.user_id "
        "HAVING unpaid_count > 0";

    sqlite3_stmt *stmt;
    if (sqlite3_prepare_v2(db->db, query, -1, &stmt, NULL) == SQLITE_OK) {
        printf("\n=== 待缴费用户列表 ===\n");
        printf("%-20s%-15s%-15s%-15s%-15s\n", 
               "用户名", "姓名", "未缴费笔数", "待缴总额", "状态");
        printf("------------------------------------------------------------------\n");
        
        while (sqlite3_step(stmt) == SQLITE_ROW) {
            printf("%-20s%-15s%-15d%-15.2f%-15s\n",
                   sqlite3_column_text(stmt, 1),
                   sqlite3_column_text(stmt, 2),
                   sqlite3_column_int(stmt, 3),
                   sqlite3_column_double(stmt, 4),
                   "待缴费");
        }
        
        sqlite3_finalize(stmt);
    }
    wait_for_key();
}

void show_all_users(Database *db) {
    clear_staff_screen();
    
    const char *query = 
        "SELECT DISTINCT "
        "   u.user_id, u.username, u.name, u.phone_number, u.email, "
        "   b.building_name, r.room_number, r.area_sqm, "
        "   (SELECT COUNT(*) FROM transactions t WHERE t.user_id = u.user_id AND t.status = 1) as paid_count, "
        "   (SELECT COALESCE(SUM(amount), 0) FROM transactions t WHERE t.user_id = u.user_id AND t.status = 1) as total_paid, "
        "   (SELECT COUNT(*) FROM transactions t WHERE t.user_id = u.user_id AND t.status = 0) as unpaid_count, "
        "   (SELECT COALESCE(SUM(amount), 0) FROM transactions t WHERE t.user_id = u.user_id AND t.status = 0) as total_unpaid "
        "FROM users u "
        "LEFT JOIN rooms r ON u.user_id = r.owner_id "
        "LEFT JOIN buildings b ON r.building_id = b.building_id "
        "WHERE u.role_id = 'role_owner' "
        "ORDER BY b.building_name, r.room_number";

    sqlite3_stmt *stmt;
    if (sqlite3_prepare_v2(db->db, query, -1, &stmt, NULL) == SQLITE_OK) {
        printf("\n=== 业主信息总览 ===\n\n");
        printf("┌───────────────────────────────────────────────────────────────────────────────────────────┐\n");
        printf("│ %-8s│ %-12s│ %-8s│ %-10s│ %-6s│ %-8s│ %-12s│ %-12s│\n",
               "姓名", "电话", "楼号", "房号", "面积", "已缴费", "待缴费", "缴费率");
        printf("├───────────────────────────────────────────────────────────────────────────────────────────┤\n");

        while (sqlite3_step(stmt) == SQLITE_ROW) {
            const char *name = (const char*)sqlite3_column_text(stmt, 2);
            const char *phone = (const char*)sqlite3_column_text(stmt, 3);
            const char *building = (const char*)sqlite3_column_text(stmt, 5);
            const char *room = (const char*)sqlite3_column_text(stmt, 6);
            double area = sqlite3_column_double(stmt, 7);
            int paid_count = sqlite3_column_int(stmt, 8);
            double total_paid = sqlite3_column_double(stmt, 9);
            int unpaid_count = sqlite3_column_int(stmt, 10);
            double total_unpaid = sqlite3_column_double(stmt, 11);
            
            // 计算缴费率
            double payment_rate = 0;
            if (paid_count + unpaid_count > 0) {
                payment_rate = (double)paid_count / (paid_count + unpaid_count) * 100;
            }

            printf("│ %-8s│ %-12s│ %-8s│ %-10s│ %6.1f│ %8.2f│ %12.2f│ %11.1f%%│\n",
                   name ? name : "未知",
                   phone ? phone : "未知",
                   building ? building : "未分配",
                   room ? room : "未分配",
                   area,
                   total_paid,
                   total_unpaid,
                   payment_rate);

            // 如果有未缴费记录，显示详细信息
            if (unpaid_count > 0) {
                printf("├───────────────────────────────────────────────────────────────────────────────────────────┤\n");
                printf("│ 待缴费明细:                                                                              │\n");
                
                // 查询未缴费详情
                const char *detail_query = 
                    "SELECT fee_type, amount, due_date "
                    "FROM transactions "
                    "WHERE user_id = ? AND status = 0 "
                    "ORDER BY due_date ASC";
                
                sqlite3_stmt *detail_stmt;
                if (sqlite3_prepare_v2(db->db, detail_query, -1, &detail_stmt, NULL) == SQLITE_OK) {
                    sqlite3_bind_text(detail_stmt, 1, sqlite3_column_text(stmt, 0), -1, SQLITE_STATIC);
                    
                    while (sqlite3_step(detail_stmt) == SQLITE_ROW) {
                        int fee_type = sqlite3_column_int(detail_stmt, 0);
                        double amount = sqlite3_column_double(detail_stmt, 1);
                        time_t due_date = sqlite3_column_int64(detail_stmt, 2);
                        
                        char date_str[20];
                        strftime(date_str, sizeof(date_str), "%Y-%m-%d", localtime(&due_date));
                        
                        printf("│ %-12s: %-8.2f元  到期日期: %-12s                                          │\n",
                               get_fee_type_name(fee_type),
                               amount,
                               date_str);
                    }
                    sqlite3_finalize(detail_stmt);
                }
            }
            printf("├───────────────────────────────────────────────────────────────────────────────────────────┤\n");
        }
        printf("└───────────────────────────────────────────────────────────────────────────────────────────┘\n");
        
        sqlite3_finalize(stmt);
    }
    wait_for_key();
}

// 辅助函数：打印用户信息表头
static void print_user_table_header() {
    printf("\n%-12s%-15s%-15s%-15s%-25s%-15s%-15s\n",
           "用户名", "姓名", "电话", "楼号", "房号", "注册时间", "状态");
    printf("--------------------------------------------------------------------------------\n");
}

// 辅助函数：打印用户信息行
static void print_user_info(sqlite3_stmt *stmt) {
    time_t reg_time = sqlite3_column_int64(stmt, 5);
    char time_str[20];
    strftime(time_str, sizeof(time_str), "%Y-%m-%d", localtime(&reg_time));
    
    printf("%-12s%-15s%-15s%-15s%-25s%-15s%-15s\n",
           sqlite3_column_text(stmt, 1),  // username
           sqlite3_column_text(stmt, 2),  // name
           sqlite3_column_text(stmt, 3),  // phone
           sqlite3_column_text(stmt, 7),  // building_name
           sqlite3_column_text(stmt, 6),  // room_number
           time_str,                      // registration_date
           "正常");                       // status
}


void show_paid_users(Database *db) {
    clear_staff_screen();
    printf("\n=== 已缴费用户信息 ===\n");
    const char *paid_query = 
        "SELECT u.username, u.name, u.phone_number, b.building_name, r.room_number, "
        "r.area_sqm, SUM(t.amount) as total_paid, COUNT(t.transaction_id) as payment_count "
        "FROM users u "
        "JOIN transactions t ON u.user_id = t.user_id "
        "LEFT JOIN rooms r ON u.user_id = r.owner_id "
        "LEFT JOIN buildings b ON r.building_id = b.building_id "
        "WHERE t.status = 1 "
        "GROUP BY u.user_id";

    sqlite3_stmt *stmt;
    if (sqlite3_prepare_v2(db->db, paid_query, -1, &stmt, NULL) == SQLITE_OK) {
        printf("┌─────────────────────────────────────────────────────────────────────────────────┐\n");
        printf("│ %-10s│ %-8s│ %-12s│ %-8s│ %-6s│ %-8s│ %-10s│ %-8s│\n",
               "用户名", "姓名", "电话", "楼号", "房号", "面积", "已缴金额", "缴费次数");
        printf("├─────────────────────────────────────────────────────────────────────────────────┤\n");

        while (sqlite3_step(stmt) == SQLITE_ROW) {
            printf("│ %-10s│ %-8s│ %-12s│ %-8s│ %-6s│ %8.2f│ %10.2f│ %8d│\n",
                   sqlite3_column_text(stmt, 0),  // username
                   sqlite3_column_text(stmt, 1),  // name
                   sqlite3_column_text(stmt, 2),  // phone_number
                   sqlite3_column_text(stmt, 3),  // building_name
                   sqlite3_column_text(stmt, 4),  // room_number
                   sqlite3_column_double(stmt, 5),// area_sqm
                   sqlite3_column_double(stmt, 6),// total_paid
                   sqlite3_column_int(stmt, 7));  // payment_count
        }
        printf("└─────────────────────────────────────────────────────────────────────────────────┘\n");
        sqlite3_finalize(stmt);
    }
}

void show_unpaid_users(Database *db) {
    clear_staff_screen();
    printf("\n=== 待缴费用户信息 ===\n");
    const char *unpaid_query = 
        "SELECT u.username, u.name, u.phone_number, b.building_name, r.room_number, "
        "SUM(t.amount) as total_due, COUNT(t.transaction_id) as unpaid_count, "
        "MIN(t.due_date) as earliest_due "
        "FROM users u "
        "JOIN transactions t ON u.user_id = t.user_id "
        "LEFT JOIN rooms r ON u.user_id = r.owner_id "
        "LEFT JOIN buildings b ON r.building_id = b.building_id "
        "WHERE t.status = 0 "
        "GROUP BY u.user_id";

    sqlite3_stmt *stmt;
    if (sqlite3_prepare_v2(db->db, unpaid_query, -1, &stmt, NULL) == SQLITE_OK) {
        printf("┌─────────────────────────────────────────────────────────────────────────────────┐\n");
        printf("│ %-10s│ %-8s│ %-12s│ %-8s│ %-6s│ %-10s│ %-10s│ %-10s│\n",
               "用户名", "姓名", "电话", "楼号", "房号", "待缴金额", "未缴笔数", "最早到期");
        printf("├─────────────────────────────────────────────────────────────────────────────────┤\n");

        while (sqlite3_step(stmt) == SQLITE_ROW) {
            char date_str[20];
            time_t due_date = sqlite3_column_int64(stmt, 7);
            strftime(date_str, sizeof(date_str), "%Y-%m-%d", localtime(&due_date));

            printf("│ %-10s│ %-8s│ %-12s│ %-8s│ %-6s│ %10.2f│ %10d│ %-10s│\n",
                   sqlite3_column_text(stmt, 0),  // username
                   sqlite3_column_text(stmt, 1),  // name
                   sqlite3_column_text(stmt, 2),  // phone_number
                   sqlite3_column_text(stmt, 3),  // building_name
                   sqlite3_column_text(stmt, 4),  // room_number
                   sqlite3_column_double(stmt, 5),// total_due
                   sqlite3_column_int(stmt, 6),   // unpaid_count
                   date_str);                     // earliest_due
        }
        printf("└─────────────────────────────────────────────────────────────────────────────────┘\n");
        sqlite3_finalize(stmt);
    }
}

// 添加新函数：显示服务区域信息
void show_staff_service_areas(Database *db, const char *staff_id)
{
    clear_staff_screen();
    printf("\n=== 负责区域信息 ===\n\n");

    const char *query =
        "SELECT b.building_name, b.address, COUNT(r.room_id) as room_count, "
        "COUNT(DISTINCT r.owner_id) as owner_count "
        "FROM service_areas sa "
        "JOIN buildings b ON sa.building_id = b.building_id "
        "LEFT JOIN rooms r ON b.building_id = r.building_id "
        "WHERE sa.staff_id = ? "
        "GROUP BY b.building_id";

    sqlite3_stmt *stmt;
    if (sqlite3_prepare_v2(db->db, query, -1, &stmt, NULL) == SQLITE_OK) {
        sqlite3_bind_text(stmt, 1, staff_id, -1, SQLITE_STATIC);

        printf("┌────────────────────────────────────────────────────────────┐\n");
        printf("│ %-10s│ %-20s│ %-12s│ %-12s│\n",
               "楼栋", "地址", "房间数", "业主数");
        printf("├────────────────────────────────────────────────────────────┤\n");

        bool found = false;
        while (sqlite3_step(stmt) == SQLITE_ROW) {
            found = true;
            printf("│ %-10s│ %-20s│ %-12d│ %-12d│\n",
                   sqlite3_column_text(stmt, 0),
                   sqlite3_column_text(stmt, 1),
                   sqlite3_column_int(stmt, 2),
                   sqlite3_column_int(stmt, 3));
        }

        printf("└────────────────────────────────────────────────────────────┘\n");

        if (!found) {
            printf("\n⚠️ 当前未分配负责区域\n");
        }

        sqlite3_finalize(stmt);
    }
    wait_for_key();
}

// 添加新函数：缴费管理菜单
void show_payment_management_menu(Database *db)
{
    int choice;
    do {
        clear_staff_screen();
        printf("\n=== 业主费用管理 ===\n");
        printf("1. 查询业主缴费记录\n");
        printf("2. 查看未缴费业主\n");
        printf("3. 查看当前费用标准\n");
        printf("0. 返回上级菜单\n");
        printf("\n请选择: ");
        
        scanf("%d", &choice);
        clear_input_buffer();
        
        switch(choice) {
            case 1:
                case2_handler(db);
                break;
            case 2:
                show_unpaid_users(db);
                break;
            case 3:
                show_current_fee_standards(db);
                break;
            case 0:
                return;
            default:
                printf("无效选择，请重试\n");
        }
    } while(choice != 0);
}

// 添加新函数：提醒管理菜单 
void show_reminder_management_menu(Database *db)
{
    int choice;
    do {
        clear_staff_screen();
        printf("\n=== 缴费提醒管理 ===\n");
        printf("1. 发送单个提醒\n");
        printf("2. 批量发送提醒\n");
        printf("0. 返回上级菜单\n");
        printf("\n请选择: ");
        
        scanf("%d", &choice);
        clear_input_buffer();
        
        switch(choice) {
            case 1: {
                char user_id[37];
                printf("请输入业主ID: ");
                scanf("%s", user_id);
                send_payment_reminder_screen(db, user_id);
                break;
            }
            case 2:
                send_bulk_payment_reminders(db);
                break;
            case 0:
                return;
            default: printf("无效选择，请重试\n");
        }
    } while(choice != 0);
}

// 添加新函数：统计菜单
void show_statistics_menu(Database *db) 
{
    int choice;
    do {
        clear_staff_screen();
        printf("\n===== 缴费统计分析 =====\n");
        printf("1. 按年度查看统计\n");
        printf("2. 查看当前月度统计\n");
        printf("3. 查看欠费分析\n");
        printf("0. 返回上级菜单\n");
        printf("\n请选择: ");
        
        scanf("%d", &choice);
        clear_input_buffer();

        switch (choice) {
            case 1: {
                int year;
                printf("请输入要查询的年份: ");
                scanf("%d", &year);
                clear_input_buffer();
                show_yearly_statistics(db, year);
                break;
            }
            case 2:
                show_current_statistics(db);
                break;
            case 3:
                show_unpaid_analysis(db);
                break;
            case 0:
                printf("返回上级菜单\n");
                break;
            default:
                printf("无效的选择，请重试\n");
                pause_console();
                break;
        }
    } while (choice != 0);
}

// 添加新函数：年度统计
void show_yearly_statistics(Database *db, int year)
{
    clear_staff_screen();
    printf("\n=== %d年度缴费统计 ===\n\n", year);

    char year_str[5];
    snprintf(year_str, sizeof(year_str), "%d", year);

    const char *query = 
        "SELECT "
        "(SELECT COUNT(DISTINCT user_id) FROM transactions WHERE status = 1 AND strftime('%Y', datetime(payment_date, 'unixepoch')) = ?) as paid_users, "
        "(SELECT COUNT(DISTINCT user_id) FROM transactions WHERE status = 0 AND strftime('%Y', datetime(due_date, 'unixepoch')) = ?) as unpaid_users, "
        "(SELECT SUM(amount) FROM transactions WHERE status = 1 AND strftime('%Y', datetime(payment_date, 'unixepoch')) = ?) as paid_amount, "
        "(SELECT SUM(amount) FROM transactions WHERE status = 0 AND strftime('%Y', datetime(due_date, 'unixepoch')) = ?) as unpaid_amount";

    sqlite3_stmt *stmt;
    if (sqlite3_prepare_v2(db->db, query, -1, &stmt, NULL) == SQLITE_OK) {
        for(int i = 1; i <= 4; i++) {
            sqlite3_bind_text(stmt, i, year_str, -1, SQLITE_STATIC);
        }

        if (sqlite3_step(stmt) == SQLITE_ROW) {
            int paid_users = sqlite3_column_int(stmt, 0);
            int unpaid_users = sqlite3_column_int(stmt, 1);
            double paid_amount = sqlite3_column_double(stmt, 2);
            double unpaid_amount = sqlite3_column_double(stmt, 3);
            
            printf("┌─────────────────────────────────┐\n");
            printf("│ 已缴费用户数: %-8d           │\n", paid_users);
            printf("│ 未缴费用户数: %-8d           │\n", unpaid_users);
            printf("│ 已收缴费总额: %-8.2f元       │\n", paid_amount);
            printf("│ 未收缴费总额: %-8.2f元       │\n", unpaid_amount);
            printf("└─────────────────────────────────┘\n");
        }
        sqlite3_finalize(stmt);
    }
    wait_for_key();
}

// 添加两个缺失的统计函数实现
void show_current_statistics(Database *db)
{
    clear_staff_screen();
    printf("\n=== 当前缴费情况统计 ===\n\n");

    const char *query = 
        "SELECT "
        "(SELECT COUNT(DISTINCT user_id) FROM users WHERE role_id = 'role_owner') as total_owners, "
        "(SELECT COUNT(DISTINCT user_id) FROM transactions WHERE status = 1) as paid_users, "
        "(SELECT COUNT(DISTINCT user_id) FROM transactions WHERE status = 0) as unpaid_users, "
        "(SELECT SUM(amount) FROM transactions WHERE status = 1) as total_paid, "
        "(SELECT SUM(amount) FROM transactions WHERE status = 0) as total_unpaid";

    sqlite3_stmt *stmt;
    if (sqlite3_prepare_v2(db->db, query, -1, &stmt, NULL) == SQLITE_OK) {
        if (sqlite3_step(stmt) == SQLITE_ROW) {
            int total_owners = sqlite3_column_int(stmt, 0);
            int paid_users = sqlite3_column_int(stmt, 1);
            int unpaid_users = sqlite3_column_int(stmt, 2);
            double total_paid = sqlite3_column_double(stmt, 3);
            double total_unpaid = sqlite3_column_double(stmt, 4);
            
            printf("┌─────────────────────────────────┐\n");
            printf("│ 总业主数量: %-8d           │\n", total_owners);
            printf("│ 已缴费用户: %-8d (%.1f%%)   │\n", 
                   paid_users, total_owners > 0 ? (float)paid_users/total_owners*100 : 0);
            printf("│ 未缴费用户: %-8d (%.1f%%)   │\n", 
                   unpaid_users, total_owners > 0 ? (float)unpaid_users/total_owners*100 : 0);
            printf("│ 已收缴费总额: %-8.2f元     │\n", total_paid);
            printf("│ 未收缴费总额: %-8.2f元     │\n", total_unpaid);
            printf("└─────────────────────────────────┘\n");
        }
        sqlite3_finalize(stmt);
    }
    wait_for_key();
}

void show_unpaid_analysis(Database *db)
{
    clear_staff_screen();
    printf("\n=== 欠费情况分析 ===\n\n");

    // 1. 按欠费时长统计
    const char *duration_query =
        "SELECT "
        "CASE "
        "  WHEN julianday('now') - julianday(datetime(due_date, 'unixepoch')) <= 30 THEN '30天内' "
        "  WHEN julianday('now') - julianday(datetime(due_date, 'unixepoch')) <= 90 THEN '90天内' "
        "  ELSE '90天以上' "
        "END as overdue_period, "
        "COUNT(DISTINCT user_id) as user_count, "
        "SUM(amount) as total_amount "
        "FROM transactions "
        "WHERE status = 0 "
        "GROUP BY overdue_period "
        "ORDER BY MIN(due_date)";

    sqlite3_stmt *stmt;
    if (sqlite3_prepare_v2(db->db, duration_query, -1, &stmt, NULL) == SQLITE_OK) {
        printf("【按欠费时长统计】\n");
        printf("┌────────────┬──────────┬────────────┐\n");
        printf("│ %-10s │ %-8s │ %-10s │\n", "欠费时长", "用户数", "欠费金额");
        printf("├────────────┼──────────┼────────────┤\n");
        
        while (sqlite3_step(stmt) == SQLITE_ROW) {
            printf("│ %-10s │ %-8d │ %10.2f │\n",
                   sqlite3_column_text(stmt, 0),
                   sqlite3_column_int(stmt, 1),
                   sqlite3_column_double(stmt, 2));
        }
        printf("└────────────┴──────────┴────────────┘\n\n");
        sqlite3_finalize(stmt);
    }

    // 2. 按费用类型统计
    const char *type_query =
        "SELECT fee_type, COUNT(DISTINCT user_id) as user_count, "
        "SUM(amount) as total_amount "
        "FROM transactions "
        "WHERE status = 0 "
        "GROUP BY fee_type "
        "ORDER BY total_amount DESC";

    if (sqlite3_prepare_v2(db->db, type_query, -1, &stmt, NULL) == SQLITE_OK) {
        printf("【按费用类型统计】\n");
        printf("┌────────────┬──────────┬────────────┐\n");
        printf("│ %-10s │ %-8s │ %-10s │\n", "费用类型", "用户数", "欠费金额");
        printf("├────────────┼──────────┼────────────┤\n");
        
        double total = 0;
        while (sqlite3_step(stmt) == SQLITE_ROW) {
            int fee_type = sqlite3_column_int(stmt, 0);
            int user_count = sqlite3_column_int(stmt, 1);
            double amount = sqlite3_column_double(stmt, 2);
            total += amount;
            
            printf("│ %-10s │ %-8d │ %10.2f │\n",
                   get_fee_type_name(fee_type),
                   user_count,
                   amount);
        }
        printf("├────────────┼──────────┼────────────┤\n");
        printf("│ %-10s │ %-8s │ %10.2f │\n", "合计", "", total);
        printf("└────────────┴──────────┴────────────┘\n");
        
        sqlite3_finalize(stmt);
    }
    wait_for_key();
}

// 添加新函数：查询业主缴费情况
void show_owner_payment_query(Database *db)
{
    clear_staff_screen();
    printf("\n=== 查询业主缴费情况 ===\n\n");
    
    // 先查询并显示所有业主列表
    const char *query = 
    "SELECT DISTINCT u.user_id, u.username, u.name, u.phone_number, "
    "b.building_name, r.room_number, r.area_sqm "
    "FROM users u "
    "LEFT JOIN rooms r ON u.user_id = r.owner_id "
    "LEFT JOIN buildings b ON r.building_id = b.building_id "
    "WHERE u.role_id = 'role_owner' AND u.registration_date > 0 "
    "ORDER BY b.building_name, r.room_number";

    sqlite3_stmt *stmt;
    if (sqlite3_prepare_v2(db->db, query, -1, &stmt, NULL) == SQLITE_OK) {
        printf("┌────────────────────────────────────────────────────────────┐\n");
        printf("│ %-8s│ %-12s│ %-8s│ %-8s│ %-8s│ %-8s│\n",
               "姓名", "电话", "楼号", "房号", "面积", "用户ID");
        printf("├────────────────────────────────────────────────────────────┤\n");

        while (sqlite3_step(stmt) == SQLITE_ROW) {
            printf("│ %-8s│ %-12s│ %-8s│ %-8s│ %8.2f│ %-8s│\n",
                   sqlite3_column_text(stmt, 2),  // name
                   sqlite3_column_text(stmt, 3),  // phone
                   sqlite3_column_text(stmt, 4),  // building
                   sqlite3_column_text(stmt, 5),  // room
                   sqlite3_column_double(stmt, 6),// area
                   sqlite3_column_text(stmt, 0)); // user_id
        }
        printf("└────────────────────────────────────────────────────────────┘\n\n");
        sqlite3_finalize(stmt);
    }

    // 输入要查询的业主ID
    char owner_id[37];
    printf("请输入要查询的业主ID: ");
    scanf("%s", owner_id);

    // 查询该业主的所有费用信息
    const char *fee_query = 
        "SELECT t.fee_type, t.amount, t.status, t.payment_date, t.due_date, "
        "t.period_start, t.period_end "
        "FROM transactions t "
        "WHERE t.user_id = ? "
        "ORDER BY t.due_date DESC";

    if (sqlite3_prepare_v2(db->db, fee_query, -1, &stmt, NULL) == SQLITE_OK) {
        sqlite3_bind_text(stmt, 1, owner_id, -1, SQLITE_STATIC);

        printf("\n=== 费用记录 ===\n");
        printf("┌──────────────────────────────────────────────────────────┐\n");
        printf("│ %-10s│ %-8s│ %-8s│ %-12s│ %-12s│\n",
               "费用类型", "金额", "状态", "缴费日期", "到期日期");
        printf("├──────────────────────────────────────────────────────────┤\n");

        double total_paid = 0;
        double total_unpaid = 0;

        while (sqlite3_step(stmt) == SQLITE_ROW) {
            const char *fee_type = get_fee_type_name(sqlite3_column_int(stmt, 0));
            double amount = sqlite3_column_double(stmt, 1);
            int status = sqlite3_column_int(stmt, 2);
            time_t payment_date = sqlite3_column_int64(stmt, 3);
            time_t due_date = sqlite3_column_int64(stmt, 4);

            char payment_date_str[20] = "未缴费";
            char due_date_str[20];
            strftime(due_date_str, sizeof(due_date_str), "%Y-%m-%d", localtime(&due_date));

            if (status == 1) {
                strftime(payment_date_str, sizeof(payment_date_str), "%Y-%m-%d", localtime(&payment_date));
                total_paid += amount;
            } else {
                total_unpaid += amount;
            }

            printf("│ %-10s│ %8.2f│ %-8s│ %-12s│ %-12s│\n",
                   fee_type, amount,
                   status == 1 ? "已缴费" : "未缴费",
                   payment_date_str, due_date_str);
        }
        
        printf("├──────────────────────────────────────────────────────────┤\n");
        printf("│ 总计: 已缴费 %-8.2f元  待缴费 %-8.2f元              │\n",
               total_paid, total_unpaid);
        printf("└──────────────────────────────────────────────────────────┘\n");

        sqlite3_finalize(stmt);
    }
    wait_for_key();
}

// 添加新函数：显示业主排序列表
void show_sorted_owners(Database *db)  
{
    clear_staff_screen();
    printf("\n=== 业主排序列表 ===\n\n");
    
    const char *query = 
        "SELECT u.user_id, u.username, u.name, u.phone_number, u.email, "
        "u.registration_date, b.building_name, r.room_number, r.area_sqm, "
        "(SELECT COUNT(*) FROM transactions t WHERE t.user_id = u.user_id AND t.status = 0) as unpaid_count "
        "FROM users u "
        "LEFT JOIN rooms r ON u.user_id = r.owner_id "
        "LEFT JOIN buildings b ON r.building_id = b.building_id "
        "WHERE u.role_id = 'role_owner' "
        "ORDER BY u.name ASC";

    sqlite3_stmt *stmt;
    if (sqlite3_prepare_v2(db->db, query, -1, &stmt, NULL) == SQLITE_OK) {
        printf("┌──────────────────────────────────────────────────────────────────────┐\n");
        printf("│ %-8s│ %-12s│ %-8s│ %-8s│ %-8s│ %-12s│ %-8s│\n",
               "姓名", "电话", "楼号", "房号", "面积", "注册时间", "待缴费");
        printf("├──────────────────────────────────────────────────────────────────────┤\n");

        while (sqlite3_step(stmt) == SQLITE_ROW) {
            time_t reg_date = sqlite3_column_int64(stmt, 5);
            char date_str[20];
            strftime(date_str, sizeof(date_str), "%Y-%m-%d", localtime(&reg_date));

            printf("│ %-8s│ %-12s│ %-8s│ %-8s│ %8.2f│ %-12s│ %8d │\n",
                   sqlite3_column_text(stmt, 2),  // name
                   sqlite3_column_text(stmt, 3),  // phone
                   sqlite3_column_text(stmt, 6),  // building
                   sqlite3_column_text(stmt, 7),  // room
                   sqlite3_column_double(stmt, 8),// area
                   date_str,                      // registration_date
                   sqlite3_column_int(stmt, 9));  // unpaid_count
        }
        printf("└──────────────────────────────────────────────────────────────────────┘\n");
        sqlite3_finalize(stmt);
    }
    wait_for_key();
}

// 实现函数定义,匹配新的声明
void show_payment_reminder_menu(Database *db, const char *user_id)
{
    int choice;
    do {
        clear_staff_screen();
        printf("\n┌─────────── 缴费提醒管理 ─────────────┐\n");
        printf("│                                        │\n");
        printf("│  1. 查看待缴费业主列表                │\n");
        printf("│  2. 发送单个提醒                      │\n");
        printf("│  3. 批量发送提醒                      │\n");
        printf("│  0. 返回上级菜单                      │\n");
        printf("│                                        │\n");
        printf("└──────────────────────────────────────┘\n");
        printf("\n请选择操作 (0-3): ");
        
        scanf("%d", &choice);
        clear_input_buffer();
        
        switch(choice) {
            case 1:
                show_unpaid_users(db);
                break;
            case 2: {
                char owner_id[37];
                printf("请输入业主ID: ");
                scanf("%s", owner_id);
                send_payment_reminder_screen(db, owner_id);
                break;
            }
            case 3:
                send_bulk_payment_reminders(db);
                break;
            case 0:
                return;
            default: 
                printf("无效选择，请重试\n");
        }
        
        if (choice != 0) {
            printf("\n按Enter键继续...");
            getchar();
        }
    } while(choice != 0);
}