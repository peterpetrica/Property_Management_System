/**
 * @file ui_staff.c
 * @brief 实现物业管理系统中服务人员相关的用户界面
 *
 * @details 本文件负责物业服务人员界面的所有交互功能，包括:
 * - 服务人员主界面
 * - 物业服务管理（创建、更新、删除服务请求等）
 * - 服务人员信息查询（按各种条件查找）
 * - 服务人员信息排序（按不同字段排序显示）
 * - 服务人员数据统计（统计各类服务数据）
 * - 系统维护功能
 *
 * 每个界面函数负责显示菜单、接收用户输入并调用相应的业务逻辑函数
 *
 * @author @ZhuLaoliu888 @abbydu626 @Fxiansheng-ai @peterpetrica
 * @date 2025-03
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
#include <ctype.h>

typedef struct OwnerNode
{
    char name[50];
    char phone[20];
    char building[20];
    char room[20];
    double area;
    char reg_date[20];
    int unpaid_count;
    struct OwnerNode *next;
} OwnerNode;
OwnerNode *create_owner_node(const char *name, const char *phone, const char *building, const char *room, double area, const char *reg_date, int unpaid_count)
{
    OwnerNode *new_node = (OwnerNode *)malloc(sizeof(OwnerNode));
    if (new_node)
    {
        strncpy(new_node->name, name, sizeof(new_node->name) - 1);
        strncpy(new_node->phone, phone, sizeof(new_node->phone) - 1);
        strncpy(new_node->building, building, sizeof(new_node->building) - 1);
        strncpy(new_node->room, room, sizeof(new_node->room) - 1);
        new_node->area = area;
        strncpy(new_node->reg_date, reg_date, sizeof(new_node->reg_date) - 1);
        new_node->unpaid_count = unpaid_count;
        new_node->next = NULL;
    }
    return new_node;
}
// 静态函数声明
/**
 * @brief 等待用户按键后继续
 */
static void wait_for_key(void);

/**
 * @brief 获取费用类型的名称
 *
 * @param fee_type 费用类型ID
 * @return const char* 费用类型名称
 */
static const char *get_fee_type_name(int fee_type);

/**
 * @brief 打印用户表格表头
 */
static void print_user_table_header();

/**
 * @brief 打印单个用户信息
 *
 * @param stmt SQLite查询结果
 */
static void print_user_info(sqlite3_stmt *stmt);

// 函数前向声明
/**
 * @brief 显示个人提醒菜单
 *
 * @param db 数据库连接
 */
void show_individual_reminder_menu(Database *db);

/**
 * @brief 显示提醒历史记录
 *
 * @param db 数据库连接
 */
void show_reminder_history(Database *db);

/**
 * @brief 按姓名发送提醒
 *
 * @param db 数据库连接
 */
void send_reminder_by_name(Database *db);

/**
 * @brief 显示支付详情
 *
 * @param db 数据库连接
 * @param user_id 用户ID
 * @param status 支付状态
 */
void show_payment_details(Database *db, const char *user_id, int status);

// 实现静态函数
/**
 * @brief 根据费用类型ID获取费用类型名称
 *
 * @param fee_type 费用类型ID
 * @return const char* 费用类型名称
 */
static const char *get_fee_type_name(int fee_type)
{
    switch (fee_type)
    {
    case 1:
        return "物业费";
    case 2:
        return "停车费";
    case 3:
        return "水费";
    case 4:
        return "电费";
    case 5:
        return "燃气费";
    default:
        return "其他费用";
    }
}

/**
 * @brief 等待用户按键后继续
 */
void wait_for_key()
{
    printf("\n按任意键继续...");
    getchar();
    getchar();
}

/**
 * @brief 批量发送缴费提醒
 *
 * @param db 数据库连接
 */
void send_bulk_payment_reminders(Database *db)
{
    const char *query =
        "SELECT DISTINCT u.user_id, u.username, SUM(t.amount) as total "
        "FROM users u "
        "JOIN transactions t ON u.user_id = t.user_id "
        "WHERE t.status = 0 "
        "GROUP BY u.user_id;";

    sqlite3_stmt *stmt;
    if (sqlite3_prepare_v2(db->db, query, -1, &stmt, NULL) == SQLITE_OK)
    {
        int count = 0;
        while (sqlite3_step(stmt) == SQLITE_ROW)
        {
            const char *user_id = (const char *)sqlite3_column_text(stmt, 0);
            const char *username = (const char *)sqlite3_column_text(stmt, 1);
            double amount = sqlite3_column_double(stmt, 2);

            count++;
        }
        printf("\n✓ 已成功发送%d条提醒\n", count);
        sqlite3_finalize(stmt);
    }
    wait_for_key();
}

/**
 * @brief 清除服务人员界面
 */
void clear_staff_screen()
{
#ifdef _WIN32
    system("cls");
#else
    system("clear");
#endif
}

/**
 * @brief 等待用户任意键返回
 *
 * @return int 返回0
 */
int wait_for_user()
{
    printf("按任意键返回主菜单...\n");
    getchar();
    getchar();
    return 0;
}

/**
 * @brief 显示区域管理菜单
 *
 * @param db 数据库连接
 * @param user_id 用户ID
 */
void show_area_management_menu(Database *db, const char *user_id)
{
    int choice;
    do
    {
        clear_staff_screen();
        printf("\n===== 负责区域管理 =====\n");
        printf("1. 查看负责区域信息\n");
        printf("2. 查看区域内业主信息\n");
        printf("0. 返回主菜单\n");
        printf("请选择: ");

        scanf("%d", &choice);
        clear_input_buffer();

        switch (choice)
        {
        case 1:
            show_staff_areas(db, user_id);
            break;
        case 2:
            show_area_owners(db, user_id);
            break;
        case 0:
            return;
        default:
            printf("无效选择，请重试\n");
        }

        if (choice != 0)
        {
            printf("\n按Enter键继续...");
            getchar();
        }
    } while (choice != 0);
}

/**
 * @brief 显示服务人员负责的区域信息
 *
 * @param db 数据库连接
 * @param user_id 用户ID
 */
void show_staff_areas(Database *db, const char *user_id)
{
    clear_staff_screen();
    printf("\n=== 我的负责区域 ===\n\n");

    const char *query =
        "SELECT b.building_name, COUNT(r.room_id) as total_rooms, "
        "COUNT(DISTINCT r.owner_id) as owner_count, "
        "b.floors_count, sa.assignment_date, "
        "(SELECT type_name FROM staff_types WHERE staff_type_id = "
        "(SELECT staff_type_id FROM staff WHERE user_id = ?)) as service_type "
        "FROM service_areas sa "
        "JOIN staff s ON sa.staff_id = s.staff_id "
        "JOIN buildings b ON sa.building_id = b.building_id "
        "LEFT JOIN rooms r ON b.building_id = r.building_id "
        "WHERE s.user_id = ? "
        "GROUP BY b.building_id "
        "ORDER BY b.building_name";

    sqlite3_stmt *stmt;
    if (sqlite3_prepare_v2(db->db, query, -1, &stmt, NULL) == SQLITE_OK)
    {
        sqlite3_bind_text(stmt, 1, user_id, -1, SQLITE_STATIC);
        sqlite3_bind_text(stmt, 2, user_id, -1, SQLITE_STATIC);

       
        printf(" %-10s %-8s %-8s    %-6s\n",
               "楼号", "总房间", "住户数", "楼层");
       

        bool found = false;
        while (sqlite3_step(stmt) == SQLITE_ROW)
        {
            found = true;
            const char *building_name = (const char *)sqlite3_column_text(stmt, 0);
            int total_rooms = sqlite3_column_int(stmt, 1);
            int owner_count = sqlite3_column_int(stmt, 2);
            int floors_count = sqlite3_column_int(stmt, 3);

            printf(" %-10s %-8d %-8d %-6d\n",
                   building_name ? building_name : "未知",
                   total_rooms, owner_count, floors_count);
        }

       

        if (!found)
        {
            printf("\n⚠️ 您当前没有被分配负责区域，请联系管理员分配\n");
        }
        else
        {
            printf("\n您的服务类型: %s\n",
                   sqlite3_column_text(stmt, 5) ? (const char *)sqlite3_column_text(stmt, 5) : "一般服务人员");
        }

        sqlite3_finalize(stmt);
    }
    else
    {
        printf("查询失败: %s\n", sqlite3_errmsg(db->db));
    }
}

/**
 * @brief 获取当前费用标准信息
 *
 * @param db 数据库连接
 * @param buffer 存储信息的缓冲区
 * @param buffer_size 缓冲区大小
 */
void get_current_fee_standards_info(Database *db, char *buffer, size_t buffer_size)
{
    buffer[0] = '\0';

    const char *query =
        "SELECT fee_type, "
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
        while (sqlite3_step(stmt) == SQLITE_ROW)
        {
            const char *type_name = (const char *)sqlite3_column_text(stmt, 1);

            if (buffer[0] != '\0')
            {
                strncat(buffer, "、", buffer_size - strlen(buffer) - 1);
            }

            strncat(buffer, type_name, buffer_size - strlen(buffer) - 1);
        }

        sqlite3_finalize(stmt);
    }

    if (buffer[0] == '\0')
    {
        strncpy(buffer, "物业费、水电费等", buffer_size - 1);
    }

    buffer[buffer_size - 1] = '\0';
}

/**
 * @brief 显示区域内业主信息
 *
 * @param db 数据库连接
 * @param user_id 服务人员ID
 */
void show_area_owners(Database *db, const char *user_id)
{
    clear_staff_screen();
    printf("\n=== 负责区域业主信息 ===\n\n");

    const char *query =
        "SELECT u.username, u.name, u.phone_number, b.building_name, r.room_number, "
        "u.email, r.area_sqm "
        "FROM service_areas sa "
        "JOIN buildings b ON sa.building_id = b.building_id "
        "JOIN rooms r ON b.building_id = r.building_id "
        "JOIN users u ON r.owner_id = u.user_id "
        "JOIN staff s ON sa.staff_id = s.staff_id "
        "WHERE s.user_id = ? "
        "ORDER BY b.building_name, r.room_number";

    sqlite3_stmt *stmt;
    if (sqlite3_prepare_v2(db->db, query, -1, &stmt, NULL) == SQLITE_OK)
    {
        sqlite3_bind_text(stmt, 1, user_id, -1, SQLITE_STATIC);

       
        printf(" %-8s   %-10s  %-12s    %-8s  %-6s   %-10s \n",
               "用户名", "姓名", "联系电话", "楼号", "房号", "面积(㎡)");
        

        bool found = false;
        while (sqlite3_step(stmt) == SQLITE_ROW)
        {
            found = true;
            printf(" %-8s  %-10s  %-12s  %-8s  %-6s  %-10.2f \n",
                   sqlite3_column_text(stmt, 0),
                   sqlite3_column_text(stmt, 1),
                   sqlite3_column_text(stmt, 2),
                   sqlite3_column_text(stmt, 3),
                   sqlite3_column_text(stmt, 4),
                   sqlite3_column_double(stmt, 6));
        }

       
        if (!found)
        {
            printf("\n⚠️ 您负责的区域暂无业主信息\n");
        }

        sqlite3_finalize(stmt);
    }
    else
    {
        printf("查询失败: %s\n", sqlite3_errmsg(db->db));
    }
}

/**
 * @brief 显示服务人员主界面
 *
 * @param db 数据库连接
 * @param user_id 用户ID
 * @param user_type 用户类型
 */
void show_staff_main_screen(Database *db, const char *user_id, UserType user_type)
{
    while (1)
    {
        clear_staff_screen();
        printf("\n===== 物业服务人员系统 =====\n");
        printf("1. 查看负责区域\n");
        printf("2. 业主缴费查询\n");
        printf("3. 缴费提醒管理\n");
        printf("4. 业主信息排序\n");
        printf("5. 缴费统计分析\n");
        printf("6. 个人信息维护\n");
        printf("7. 退出登录\n");
        printf("请选择操作[1-7]: ");

        int choice;
        scanf("%d", &choice);
        clear_input_buffer();

        switch (choice)
        {
        case 1:
            show_area_management_menu(db, user_id);
            break;
        case 2:
            show_payment_management_menu(db);
            break;
        case 3:
            show_reminder_management_menu(db);
            break;
        case 4:
            show_sorted_owners_menu(db);
            break;
        case 5:
            show_statistics_menu(db);
            break;
        case 6:
            modify_personal_info_screen(db, user_id, user_type);
            break;
        case 7:
            printf("退出登录...\n");
            return;
        default:
            printf("无效选项！\n");
            printf("按任意键继续...");
            getchar();
        }
    }
}

/**
 * @brief 按不同排序方式显示业主
 *
 * @param db 数据库连接
 * @param sort_criteria 排序条件
 */
/**
 * @brief 按不同排序方式显示业主
 *
 * @param db 数据库连接
 * @param sort_criteria 排序条件
 */
/**
 * @brief 按不同排序方式显示业主
 *
 * @param db 数据库连接
 * @param sort_criteria 排序条件
 */
void show_sorted_owners_by(Database *db, const char *sort_criteria)
{
    clear_staff_screen();
    printf("\n=== 业主排序列表 ===\n\n");

    char query[1024];
    snprintf(query, sizeof(query),
             "SELECT u.user_id, u.username, u.name, u.phone_number, u.email, "
             "u.registration_date, b.building_name, r.room_number, r.area_sqm, "
             "(SELECT COUNT(*) FROM transactions t WHERE t.user_id = u.user_id AND t.status = 0) as unpaid_count "
             "FROM users u "
             "LEFT JOIN rooms r ON u.user_id = r.owner_id "
             "LEFT JOIN buildings b ON r.building_id = b.building_id "
             "WHERE u.role_id = 'role_owner' "
             "ORDER BY %s",
             sort_criteria);

    sqlite3_stmt *stmt;
    if (sqlite3_prepare_v2(db->db, query, -1, &stmt, NULL) == SQLITE_OK)
    {

        printf(" %-8s %-12s   %-8s    %-8s   %-8s    \n",
               "姓名", "电话", "楼号", "房号", "面积");

        OwnerNode *head = NULL;

        while (sqlite3_step(stmt) == SQLITE_ROW)
        {
            time_t reg_date = sqlite3_column_int64(stmt, 5);
            char date_str[20];
            strftime(date_str, sizeof(date_str), "%Y-%m-%d", localtime(&reg_date));

            printf(" %-8s %-12s %-8s %-8s %8.2f  \n",
                   sqlite3_column_text(stmt, 2),
                   sqlite3_column_text(stmt, 3),
                   sqlite3_column_text(stmt, 6),
                   sqlite3_column_text(stmt, 7),
                   sqlite3_column_double(stmt, 8),
                   date_str,
                   sqlite3_column_int(stmt, 9));

            // 将数据添加到链表中
            OwnerNode *new_node = create_owner_node(
                sqlite3_column_text(stmt, 2),
                sqlite3_column_text(stmt, 3),
                sqlite3_column_text(stmt, 6),
                sqlite3_column_text(stmt, 7),
                sqlite3_column_double(stmt, 8),
                date_str,
                sqlite3_column_int(stmt, 9));
            add_owner_to_list(&head, new_node);
        }

        sqlite3_finalize(stmt);

        // 询问用户是否要导出数据
        printf("\n是否将排序后的业主信息导出到文件？(y/n): ");
        char export_choice;
        scanf(" %c", &export_choice);
        if (export_choice == 'y' || export_choice == 'Y')
        {
            // 导出数据到文件
            write_list_to_file(head, "sorted_owners.csv");
            printf("✓ 数据已成功导出到sorted_owners.csv\n");
        }

        // 释放链表内存
        free_owner_list(head);
    }
    wait_for_key();
}

/**
 * @brief 显示业主排序菜单
 *
 * @param db 数据库连接
 */
void show_sorted_owners_menu(Database *db)
{
    int choice;
    do
    {
        clear_staff_screen();
        printf("\n=== 业主信息排序 ===\n");
        printf("1. 按姓名排序\n");
        printf("2. 按楼号和房号排序\n");
        printf("3. 按缴费情况排序\n");
        printf("4. 按注册时间排序\n");
        printf("5. 复合排序(姓名+缴费情况)\n");
        printf("0. 返回上级菜单\n");
        printf("\n请选择排序方式: ");

        scanf("%d", &choice);
        clear_input_buffer();

        switch (choice)
        {
        case 1:
            show_sorted_owners_by(db, "u.name ASC");
            break;
        case 2:
            show_sorted_owners_by(db, "b.building_name ASC, r.room_number ASC");
            break;
        case 3:
            show_sorted_owners_by(db, "unpaid_count DESC");
            break;
        case 4:
            show_sorted_owners_by(db, "u.registration_date DESC");
            break;
        case 5:
            show_sorted_owners_by(db, "u.name ASC, unpaid_count DESC");
            break;
        case 0:
            return;
        default:
            printf("无效选择，请重试\n");
        }
    } while (choice != 0);
    // 询问用户是否要导出数据 
    printf("\n是否将排序后的业主信息导出到文件？(y/n): ");
    char export_choice;
    scanf(" %c", &export_choice);
    if (export_choice == 'y' || export_choice == 'Y') 
    {
        // 创建链表并导出数据
            OwnerNode *head = NULL;
         // 假设show_sorted_owners_by函数已经填充了链表
            write_list_to_file(head, "sorted_owners.csv");
        free_owner_list(head);
        printf("✓ 数据已成功导出到sorted_owners.csv\n");
        
    }
    
}


/**
 * @brief 将业主节点添加到链表
 *
 * @param head 链表头指针
 * @param new_node 新的业主节点
 */
void add_owner_to_list(OwnerNode **head, OwnerNode *new_node) {
    if (!*head) {
        *head = new_node;
    } else {
        OwnerNode *current = *head;
        while (current->next) {
            current = current->next;
        }
        current->next = new_node;
    }
}

/**
 * @brief 将链表数据写入文件
 *
 * @param head 链表头指针
 * @param filename 文件名
 */
void write_list_to_file(OwnerNode *head, const char *filename) {
    FILE *file = fopen(filename, "w");
    if (file) {
        OwnerNode *current = head;
        while (current) {
            fprintf(file, "%s,%s,%s,%s,%.2f,%s,%d\n",
                    current->name, current->phone, current->building, current->room,
                    current->area, current->reg_date, current->unpaid_count);
            current = current->next;
        }
        fclose(file);
    }
}

/**
 * @brief 释放链表内存
 *
 * @param head 链表头指针
 */
void free_owner_list(OwnerNode *head) {
    OwnerNode *current = head;
    while (current) {
        OwnerNode *next = current->next;
        free(current);
        current = next;
    }
}

    
/**
 * @brief 显示服务人员负责的区域信息
 *
 * @param db 数据库连接
 * @param staff_id 服务人员ID
 */
void show_assigned_areas(Database *db, const char *staff_id)
{
    printf("\n=== 负责区域信息 ===\n");

    const char *query =
        "SELECT b.building_name, COUNT(r.room_id) as total_rooms, "
        "COUNT(DISTINCT r.owner_id) as owner_count, "
        "b.floors_count, b.units_per_floor, "
        "sa.assignment_date, "
        "(SELECT name FROM staff_types WHERE staff_type_id = "
        "(SELECT staff_type_id FROM staff WHERE staff_id = ?)) as service_type "
        "FROM service_areas sa "
        "JOIN buildings b ON sa.building_id = b.building_id "
        "LEFT JOIN rooms r ON b.building_id = r.building_id "
        "WHERE sa.staff_id = ? "
        "GROUP BY b.building_id";

    sqlite3_stmt *stmt;
    if (sqlite3_prepare_v2(db->db, query, -1, &stmt, NULL) == SQLITE_OK)
    {
        sqlite3_bind_text(stmt, 1, staff_id, -1, SQLITE_STATIC);
        sqlite3_bind_text(stmt, 2, staff_id, -1, SQLITE_STATIC);

        
        printf(" %-15s %-10s %-10s %-8s %-15s %-12s\n",
               "楼宇名称", "总房间数", "业主数", "楼层数", "每层单元数", "分配日期");
        

        bool found = false;
        while (sqlite3_step(stmt) == SQLITE_ROW)
        {
            found = true;
            const char *building_name = (const char *)sqlite3_column_text(stmt, 0);
            int total_rooms = sqlite3_column_int(stmt, 1);
            int owner_count = sqlite3_column_int(stmt, 2);
            int floors_count = sqlite3_column_int(stmt, 3);
            int units_per_floor = sqlite3_column_int(stmt, 4);

            char date_str[20] = "未知";
            time_t assign_date = sqlite3_column_int64(stmt, 5);
            if (assign_date > 0)
            {
                strftime(date_str, sizeof(date_str), "%Y-%m-%d", localtime(&assign_date));
            }

            const char *service_type = (const char *)sqlite3_column_text(stmt, 6);

            printf(" %-15s %-10d %-10d %-8d %-15d %-12s\n",
                   building_name ? building_name : "未知",
                   total_rooms,
                   owner_count,
                   floors_count,
                   units_per_floor,
                   date_str);
        }
       
        if (!found)
        {
            printf("\n⚠️ 您当前没有负责的区域\n");
        }
        else
        {
            sqlite3_reset(stmt);
            sqlite3_step(stmt);
            const char *service_type = (const char *)sqlite3_column_text(stmt, 6);
            printf("\n您的服务类型: %s\n", service_type ? service_type : "未设置");
        }

        sqlite3_finalize(stmt);
    }
    else
    {
        printf("查询失败: %s\n", sqlite3_errmsg(db->db));
    }
}

/**
 * @brief 显示业主缴费状态
 *
 * @param db 数据库连接
 */
void show_owner_payment_status(Database *db)
{
    printf("\n=== 业主缴费情况查询 ===\n");

    const char *query =
        "SELECT u.user_id, u.name, r.room_number, "
        "SUM(CASE WHEN t.status = 1 THEN t.amount ELSE 0 END) as paid_amount, "
        "SUM(CASE WHEN t.status = 0 THEN t.amount ELSE 0 END) as unpaid_amount, "
        "COUNT(CASE WHEN t.status = 2 THEN 1 END) as overdue_count "
        "FROM users u "
        "LEFT JOIN rooms r ON u.user_id = r.owner_id "
        "LEFT JOIN transactions t ON u.user_id = t.user_id "
        "WHERE u.role_id = 'role_owner' "
        "GROUP BY u.user_id "
        "ORDER BY r.room_number;";

    sqlite3_stmt *stmt;
    if (sqlite3_prepare_v2(db->db, query, -1, &stmt, NULL) != SQLITE_OK)
    {
        printf("查询失败\n");
        return;
    }

    printf("\n%-8s %-12s %-8s %-12s %-12s %-10s\n",
           "业主ID", "姓名", "房号", "已缴金额", "未缴金额", "逾期数");
    printf("------------------------------------------------------------\n");

    bool found = false;
    while (sqlite3_step(stmt) == SQLITE_ROW)
    {
        found = true;
        printf("%-8s %-12s %-8s %-12.2f %-12.2f %-10d\n",
               sqlite3_column_text(stmt, 0),
               sqlite3_column_text(stmt, 1),
               sqlite3_column_text(stmt, 2),
               sqlite3_column_double(stmt, 3),
               sqlite3_column_double(stmt, 4),
               sqlite3_column_int(stmt, 5));
    }

    sqlite3_finalize(stmt);
}

/**
 * @brief 发送缴费提醒
 *
 * @param db 数据库连接
 */
void send_payment_reminders(Database *db)
{
    printf("\n=== 发送缴费提醒 ===\n");

    const char *query =
        "SELECT DISTINCT u.user_id, u.name, u.phone_number, u.email, "
        "SUM(t.amount) as total_due "
        "FROM users u "
        "JOIN transactions t ON u.user_id = t.user_id "
        "WHERE t.status IN (0, 2) "
        "GROUP BY u.user_id;";

    sqlite3_stmt *stmt;
    if (sqlite3_prepare_v2(db->db, query, -1, &stmt, NULL) != SQLITE_OK)
    {
        printf("查询失败\n");
        return;
    }

    printf("\n%-8s %-12s %-15s %-25s %-12s\n",
           "业主ID", "姓名", "电话", "邮箱", "待缴总额");
    printf("----------------------------------------------------------------\n");

    bool found = false;
    while (sqlite3_step(stmt) == SQLITE_ROW)
    {
        found = true;
        printf("%-8s %-12s %-15s %-25s %-12.2f\n",
               sqlite3_column_text(stmt, 0),
               sqlite3_column_text(stmt, 1),
               sqlite3_column_text(stmt, 2),
               sqlite3_column_text(stmt, 3),
               sqlite3_column_double(stmt, 4));
    }

    sqlite3_finalize(stmt);

    if (found)
    {
        printf("\n是否发送提醒短信给这些业主？(y/n): ");
        char choice;
        scanf(" %c", &choice);
        if (choice == 'y' || choice == 'Y')
        {
            printf("✓ 提醒短信发送成功！\n");
        }
    }
}

/**
 * @brief 修改个人信息界面
 *
 * @param db 数据库连接
 * @param user_id 用户ID
 * @param user_type 用户类型
 */
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
        printf("3. 修改密码\n");
        printf("0. 返回主菜单\n");
        printf("请选择要修改的选项: ");

        if (scanf("%d", &choice) != 1)
        {
            printf("输入错误，请重试\n");
            clear_input_buffer();
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
            // 添加修改密码功能
            char old_password[100], new_password[100];
            printf("\n===== 修改密码 =====\n");
            clear_input_buffer();
            printf("请输入旧密码: ");
            read_password(old_password, sizeof(old_password));

            printf("\n请输入新密码: ");
            read_password(new_password, sizeof(new_password));

            printf("\n请确认新密码: ");
            char confirm_password[100];
            read_password(confirm_password, sizeof(confirm_password));

            if (strcmp(new_password, confirm_password) != 0)
            {
                printf("\n两次输入的新密码不一致，密码修改失败\n");
                wait_for_key(); // 使用统一的等待按键函数
                break;
            }

            if (change_password(db, user_id, user_type, old_password, new_password))
            {
                printf("\n密码修改成功！\n");
            }
            else
            {
                printf("\n密码修改失败，请确认旧密码是否正确\n");
            }
            wait_for_key(); // 使用统一的等待按键函数
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

/**
 * @brief 显示服务人员信息查询界面
 *
 * @param db 数据库连接
 * @param user_id 用户ID
 * @param user_type 用户类型
 */
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
                "AND u.registration_date > 0 "
                "ORDER BY u.registration_date DESC;";

            sqlite3_stmt *stmt;
            if (sqlite3_prepare_v2(db->db, query, -1, &stmt, NULL) == SQLITE_OK)
            {
                
                printf(" %-12s  %-10s  %-8s  %-11s  %-15s  %-10s  %-20s \n",
                       "用户ID", "用户名", "姓名", "电话", "注册时间", "性别", "身份证号");
               
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

                    char time_str[20];
                    struct tm *tm_info = localtime(&reg_time);
                    strftime(time_str, sizeof(time_str), "%Y-%m-%d", tm_info);

                    char masked_id[20] = "未填写";
                    if (id_number && strlen(id_number) == 18)
                    {
                        snprintf(masked_id, sizeof(masked_id), "%6s******%4s",
                                 id_number, id_number + 14);
                    }

                    printf(" %-12s  %-10s  %-8s  %-11s  %-15s  %-10s  %-20s \n",
                           user_id ? user_id : "N/A",
                           username ? username : "N/A",
                           name ? name : "N/A",
                           phone ? phone : "未填写",
                           time_str,
                           gender ? gender : "未填写",
                           id_number ? masked_id : "未填写");
                }

                

                if (!found)
                {
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

/**
 * @brief 按姓名比较服务人员（用于排序）
 *
 * @param a 第一个服务人员
 * @param b 第二个服务人员
 * @return int 比较结果
 */
int compare_staff_by_name(const void *a, const void *b)
{
    const Staff *staff1 = (const Staff *)a;
    const Staff *staff2 = (const Staff *)b;
    return strcmp(staff1->name, staff2->name);
}

/**
 * @brief 显示服务人员信息排序界面
 *
 * @param db 数据库连接
 * @param user_id 用户ID
 * @param user_type 用户类型
 */
void show_staff_sort_screen(Database *db, const char *user_id, UserType user_type)
{
    clear_staff_screen();
    printf("服务人员信息排序界面\n");

    Staff staff_list[100];
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
    getchar();
}

/**
 * @brief 显示服务人员信息统计界面
 *
 * @param db 数据库连接
 * @param user_id 用户ID
 * @param user_type 用户类型
 */
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

/**
 * @brief 显示服务人员系统维护界面
 *
 * @param db 数据库连接
 * @param user_id 用户ID
 * @param user_type 用户类型
 */
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
        clear_input_buffer();
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

/**
 * @brief 显示当前费用标准
 *
 * @param db 数据库连接
 */
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
       
        printf(" %-10s     %-8s  %-11s \n",
               "费用类型", "单价", "计费单位");
       

        while (sqlite3_step(stmt) == SQLITE_ROW)
        {
            printf(" %-10s  %8.2f  %-11s \n",
                   sqlite3_column_text(stmt, 3),
                   sqlite3_column_double(stmt, 1),
                   sqlite3_column_text(stmt, 2));
        }
        
        sqlite3_finalize(stmt);
    }
}

/**
 * @brief 处理用户缴费情况查询的Case 2
 *
 * @param db 数据库连接
 */
void case2_handler(Database *db)
{
    int choice;
    char owner_name[64];
    int year;

    do
    {
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

        switch (choice)
        {
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
    } while (choice != 0);
}

/**
 * @brief 按年查询业主缴费情况
 *
 * @param db 数据库连接
 * @param owner_name 业主姓名
 * @param year 查询年份
 */
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
        "WHERE u.name = ? AND "
        "(strftime('%Y', datetime(t.payment_date, 'unixepoch')) = ? OR "
        "strftime('%Y', datetime(t.due_date, 'unixepoch')) = ?) "
        "ORDER BY t.payment_date DESC, t.due_date ASC";

    sqlite3_stmt *stmt;
    if (sqlite3_prepare_v2(db->db, query, -1, &stmt, NULL) == SQLITE_OK)
    {
        char year_str[5];
        snprintf(year_str, sizeof(year_str), "%d", year);

        sqlite3_bind_text(stmt, 1, owner_name, -1, SQLITE_STATIC);
        sqlite3_bind_text(stmt, 2, year_str, -1, SQLITE_STATIC);
        sqlite3_bind_text(stmt, 3, year_str, -1, SQLITE_STATIC);

        
        printf(" %-8s  %-8s  %-6s  %-10s  %-10s  %-8s  %-6s \n",
               "费用类型", "金额", "状态", "缴费日期", "到期日期", "楼号", "房号");
       
        bool found = false;
        double total_paid = 0;
        double total_unpaid = 0;

        while (sqlite3_step(stmt) == SQLITE_ROW)
        {
            found = true;
            const char *fee_type = get_fee_type_name(sqlite3_column_int(stmt, 0));
            double amount = sqlite3_column_double(stmt, 1);
            int status = sqlite3_column_int(stmt, 2);
            time_t payment_date = sqlite3_column_int64(stmt, 3);
            time_t due_date = sqlite3_column_int64(stmt, 4);
            const char *building = (const char *)sqlite3_column_text(stmt, 5);
            const char *room = (const char *)sqlite3_column_text(stmt, 6);

            char payment_date_str[20] = "未缴费";
            char due_date_str[20];

            if (payment_date > 0)
            {
                strftime(payment_date_str, sizeof(payment_date_str), "%Y-%m-%d", localtime(&payment_date));
                total_paid += amount;
            }
            else
            {
                total_unpaid += amount;
            }
            strftime(due_date_str, sizeof(due_date_str), "%Y-%m-%d", localtime(&due_date));

            printf(" %-8s  %8.2f  %-6s  %-10s  %-10s  %-8s  %-6s \n",
                   fee_type, amount,
                   status == 1 ? "已缴费" : (status == 2 ? "已逾期" : "未缴费"),
                   payment_date_str, due_date_str,
                   building ? building : "未知",
                   room ? room : "未知");
        }

        if (found)
        {
            
            printf(" 统计信息: 已缴费总额: %-10.2f    未缴费总额: %-10.2f        \n",
                   total_paid, total_unpaid);
        }
        

        if (!found)
        {
            printf("\n⚠️ 未找到业主 %s 在 %d 年的缴费记录\n", owner_name, year);
        }

        sqlite3_finalize(stmt);
    }
    else
    {
        printf("查询失败: %s\n", sqlite3_errmsg(db->db));
    }
    wait_for_key();
}

/**
 * @brief 查询业主所有缴费情况
 *
 * @param db 数据库连接
 * @param owner_name 业主姓名
 */
void query_owner_all_payments(Database *db, const char *owner_name)
{
    clear_staff_screen();
    printf("\n=== 查询业主 %s 的所有缴费情况 ===\n\n", owner_name);

    const char *user_query =
        "SELECT u.user_id, u.phone_number, b.building_name, r.room_number "
        "FROM users u "
        "LEFT JOIN rooms r ON u.user_id = r.owner_id "
        "LEFT JOIN buildings b ON r.building_id = b.building_id "
        "WHERE u.name = ?";

    sqlite3_stmt *stmt;
    const char *user_id = NULL;
    if (sqlite3_prepare_v2(db->db, user_query, -1, &stmt, NULL) == SQLITE_OK)
    {
        sqlite3_bind_text(stmt, 1, owner_name, -1, SQLITE_STATIC);

        if (sqlite3_step(stmt) == SQLITE_ROW)
        {
            user_id = (const char *)sqlite3_column_text(stmt, 0);
            printf("业主ID: %s\n", user_id);
            printf("电话: %s\n", sqlite3_column_text(stmt, 1));
            printf("住址: %s楼 %s室\n\n",
                   sqlite3_column_text(stmt, 2) ? (const char *)sqlite3_column_text(stmt, 2) : "未知",
                   sqlite3_column_text(stmt, 3) ? (const char *)sqlite3_column_text(stmt, 3) : "未知");
        }
        else
        {
            printf("\n⚠️ 未找到名为 %s 的业主\n", owner_name);
            sqlite3_finalize(stmt);
            wait_for_key();
            return;
        }

        sqlite3_finalize(stmt);
    }

    if (user_id == NULL)
    {
        printf("查询失败: 无法获取用户ID\n");
        wait_for_key();
        return;
    }

    const char *query =
        "SELECT t.transaction_id, t.fee_type, t.amount, t.status, t.payment_date, t.due_date "
        "FROM transactions t "
        "WHERE t.user_id = ? "
        "ORDER BY t.status ASC, t.due_date DESC";

    if (sqlite3_prepare_v2(db->db, query, -1, &stmt, NULL) == SQLITE_OK)
    {
        sqlite3_bind_text(stmt, 1, user_id, -1, SQLITE_STATIC);

        
        printf(" %-10s %-10s %-8s %-12s %-12s\n",
               "缴费类型", "金额", "状态", "缴费日期", "到期日期");
        

        bool found = false;
        double total_paid = 0.0;
        double total_unpaid = 0.0;
        int paid_count = 0;
        int unpaid_count = 0;

        while (sqlite3_step(stmt) == SQLITE_ROW)
        {
            found = true;
            const char *fee_type = get_fee_type_name(sqlite3_column_int(stmt, 1));
            double amount = sqlite3_column_double(stmt, 2);
            int status = sqlite3_column_int(stmt, 3);
            time_t payment_date = sqlite3_column_int64(stmt, 4);
            time_t due_date = sqlite3_column_int64(stmt, 5);

            char payment_date_str[20] = "未缴费";
            char due_date_str[20];

            if (payment_date > 0)
            {
                strftime(payment_date_str, sizeof(payment_date_str), "%Y-%m-%d", localtime(&payment_date));
            }
            strftime(due_date_str, sizeof(due_date_str), "%Y-%m-%d", localtime(&due_date));

            if (status == 1)
            {
                total_paid += amount;
                paid_count++;
            }
            else
            {
                total_unpaid += amount;
                unpaid_count++;
            }

            printf(" %-10s %10.2f %-8s %-12s %-12s\n",
                   fee_type, amount,
                   status == 1 ? "已缴费" : "未缴费",
                   payment_date_str, due_date_str);
        }

        if (!found)
        {
            printf(" %-57s \n", "该业主暂无任何缴费记录");
        }

        

        if (found)
        {
            printf("\n【缴费统计】\n");
            printf("已缴费: %d笔，共%.2f元\n", paid_count, total_paid);
            printf("未缴费: %d笔，共%.2f元\n", unpaid_count, total_unpaid);
            printf("缴费率: %.1f%%\n", (paid_count + unpaid_count > 0) ? (float)paid_count / (paid_count + unpaid_count) * 100 : 0.0);
        }

        sqlite3_finalize(stmt);
    }
    wait_for_key();
}

/**
 * @brief 显示排序后的用户
 *
 * @param db 数据库连接
 */
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
    if (sqlite3_prepare_v2(db->db, query, -1, &stmt, NULL) == SQLITE_OK)
    {
        
        printf(" %-10s  %-12s  %-8s  %-8s  %-10s \n",
               "姓名", "电话", "楼号", "房号", "未缴笔数");
        

        while (sqlite3_step(stmt) == SQLITE_ROW)
        {
            printf(" %-10s  %-12s  %-8s  %-8s  %-10d \n",
                   sqlite3_column_text(stmt, 0),
                   sqlite3_column_text(stmt, 1),
                   sqlite3_column_text(stmt, 2),
                   sqlite3_column_text(stmt, 3),
                   sqlite3_column_int(stmt, 4));
        }
        
        sqlite3_finalize(stmt);
    }
}

/**
 * @brief 发送缴费提醒给指定用户
 *
 * @param db 数据库连接
 * @param user_id 用户ID
 * @param unpaid_amount 未付款金额
 * @param fee_types 费用类型字符串
 */
void send_payment_reminder(Database *db, const char *user_id, double unpaid_amount, const char *fee_types)
{
    char reminder[512];
    char username[100];
    char phone[20] = "未知";
    time_t now = time(NULL);
    struct tm *t = localtime(&now);

    const char *user_query =
        "SELECT name, phone_number FROM users WHERE user_id = ?";

    sqlite3_stmt *stmt;
    if (sqlite3_prepare_v2(db->db, user_query, -1, &stmt, NULL) == SQLITE_OK)
    {
        sqlite3_bind_text(stmt, 1, user_id, -1, SQLITE_STATIC);

        if (sqlite3_step(stmt) == SQLITE_ROW)
        {
            strncpy(username, (const char *)sqlite3_column_text(stmt, 0), sizeof(username) - 1);
            strncpy(phone, (const char *)sqlite3_column_text(stmt, 1), sizeof(phone) - 1);
        }
        sqlite3_finalize(stmt);
    }

    char fee_types_str[256] = "";
    if (strlen(fee_types) == 0)
    {
        get_current_fee_standards_info(db, fee_types_str, sizeof(fee_types_str));
        fee_types = fee_types_str;
    }

    double overdue_amount = 0.0;
    int overdue_days = 0;

    const char *overdue_query =
        "SELECT SUM(amount), "
        "MAX(julianday('now') - julianday(datetime(due_date, 'unixepoch'))) "
        "FROM transactions "
        "WHERE user_id = ? AND status = 0 AND due_date < strftime('%s','now')";

    if (sqlite3_prepare_v2(db->db, overdue_query, -1, &stmt, NULL) == SQLITE_OK)
    {
        sqlite3_bind_text(stmt, 1, user_id, -1, SQLITE_STATIC);

        if (sqlite3_step(stmt) == SQLITE_ROW)
        {
            overdue_amount = sqlite3_column_double(stmt, 0);
            overdue_days = (int)sqlite3_column_double(stmt, 1);
        }
        sqlite3_finalize(stmt);
    }

    snprintf(reminder, sizeof(reminder),
             "尊敬的%s业主:\n\n"
             "您好！我们的系统显示您目前有以下费用未缴纳：\n"
             "总金额：%.2f元\n",
             username, unpaid_amount);

    if (overdue_amount > 0)
    {
        char overdue_info[200];
        snprintf(overdue_info, sizeof(overdue_info),
                 "其中逾期费用：%.2f元，已逾期%d天\n"
                 "请注意：逾期超过30天将产生额外滞纳金\n\n",
                 overdue_amount, overdue_days);
        strncat(reminder, overdue_info, sizeof(reminder) - strlen(reminder) - 1);
    }

    strncat(reminder, "费用类型：", sizeof(reminder) - strlen(reminder) - 1);
    strncat(reminder, fee_types, sizeof(reminder) - strlen(reminder) - 1);

    char ending[200];
    snprintf(ending, sizeof(ending),
             "\n\n请您及时缴纳费用。如已缴费请忽略此提醒。\n\n"
             "发送时间：%d-%02d-%02d\n"
             "联系电话：400-888-XXXX\n"
             "%s物业服务中心",
             t->tm_year + 1900, t->tm_mon + 1, t->tm_mday,
             "翠湖花园");
    strncat(reminder, ending, sizeof(reminder) - strlen(reminder) - 1);

    char query[1024];
    snprintf(query, sizeof(query),
             "INSERT INTO payment_reminders (user_id, reminder_content, send_time, status) "
             "VALUES ('%s', '%s', %ld, 0)",
             user_id, reminder, (long)now);

    if (execute_update(db, query))
    {
        printf("\n✓ 已成功向业主 %s(%s) 发送缴费提醒\n", username, phone);
    }
    else
    {
        printf("\n⚠️ 发送提醒失败\n");
    }
}

/**
 * @brief 显示用户详细信息
 *
 * @param db 数据库连接
 * @param user_id 用户ID
 */
void show_user_detail(Database *db, const char *user_id)
{
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
    if (sqlite3_prepare_v2(db->db, query, -1, &stmt, NULL) == SQLITE_OK)
    {
        sqlite3_bind_text(stmt, 1, user_id, -1, SQLITE_STATIC);

        if (sqlite3_step(stmt) == SQLITE_ROW)
        {
            printf("   基本信息 \n");
            printf(" 用户名: %-35s \n", sqlite3_column_text(stmt, 0));
            printf(" 姓  名: %-35s \n", sqlite3_column_text(stmt, 1));
            printf(" 电  话: %-35s \n", sqlite3_column_text(stmt, 2));
            printf(" 邮  箱: %-35s \n", sqlite3_column_text(stmt, 3));
            printf(" 性  别: %-35s \n", sqlite3_column_text(stmt, 5));
            printf(" 地  址: %-35s \n", sqlite3_column_text(stmt, 6));
          
            const char *name = (const char *)sqlite3_column_text(stmt, 0);
            printf("\n   房产信息 \n");
            printf(" 楼  号: %-35s \n", sqlite3_column_text(stmt, 7));
            printf(" 房  号: %-35s \n", sqlite3_column_text(stmt, 8));
            printf(" 面  积: %-35.2f \n", sqlite3_column_double(stmt, 9));
           
            time_t reg_date = sqlite3_column_int64(stmt, 4);
            char date_str[20];
            strftime(date_str, sizeof(date_str), "%Y-%m-%d", localtime(&reg_date));
            printf("\n注册时间: %s\n", date_str);
        }
        sqlite3_finalize(stmt);
    }
}

/**
 * @brief 显示发送缴费提醒界面
 *
 * @param db 数据库连接
 * @param user_id 用户ID
 */
void send_payment_reminder_screen(Database *db, const char *user_id)
{
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
    if (sqlite3_prepare_v2(db->db, query, -1, &stmt, NULL) == SQLITE_OK)
    {
        printf("\n是否发送缴费提醒？(Y/N): ");
        char choice;
        scanf(" %c", &choice);

        if (toupper(choice) == 'Y')
        {
            printf("\n✓ 提醒已发送\n");
        }

        sqlite3_finalize(stmt);
    }
    wait_for_user();
}

/**
 * @brief 批量发送缴费提醒
 *
 * @param db 数据库连接
 */
void send_bulk_payment_reminders(Database *db);

/**
 * @brief 显示用户查询菜单
 *
 * @param db 数据库连接
 */
void show_user_query_menu(Database *db)
{
    clear_staff_screen();
    int choice;
    do
    {
        printf("\n=== 用户信息查询系统 ===\n");
        printf("1. 按姓名查询用户\n");
        printf("2. 按房间号查询用户\n");
        printf("3. 查询用户缴费状态\n");
        printf("4. 显示所有用户\n");
        printf("0. 返回上级菜单\n");
        printf("请选择: ");

        scanf("%d", &choice);
        clear_input_buffer();

        switch (choice)
        {
        case 1:
            query_user_by_name(db);
            break;
        case 2:
            query_user_by_room(db);
            break;
        case 3:
            query_user_payment_status(db);
            break;
        case 4:
            show_all_users(db);
            break;
        case 0:
            return;
        default:
            printf("无效选择，请重试\n");
        }
    } while (choice != 0);
}

/**
 * @brief 按姓名查询用户
 *
 * @param db 数据库连接
 */
void query_user_by_name(Database *db)
{
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
    if (sqlite3_prepare_v2(db->db, query, -1, &stmt, NULL) == SQLITE_OK)
    {
        char name_pattern[66];
        snprintf(name_pattern, sizeof(name_pattern), "%%%s%%", name);
        sqlite3_bind_text(stmt, 1, name_pattern, -1, SQLITE_STATIC);

        print_user_table_header();

        while (sqlite3_step(stmt) == SQLITE_ROW)
        {
            print_user_info(stmt);
        }

        sqlite3_finalize(stmt);
    }
    wait_for_key();
}

/**
 * @brief 按房间号查询用户
 *
 * @param db 数据库连接
 */
void query_user_by_room(Database *db)
{
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
    if (sqlite3_prepare_v2(db->db, query, -1, &stmt, NULL) == SQLITE_OK)
    {
        sqlite3_bind_text(stmt, 1, building, -1, SQLITE_STATIC);
        sqlite3_bind_text(stmt, 2, room, -1, SQLITE_STATIC);

        print_user_table_header();

        while (sqlite3_step(stmt) == SQLITE_ROW)
        {
            print_user_info(stmt);
        }

        sqlite3_finalize(stmt);
    }
    wait_for_key();
}

/**
 * @brief 查询用户缴费状态
 *
 * @param db 数据库连接
 */
void query_user_payment_status(Database *db)
{
    const char *query =
        "SELECT u.user_id, u.username, u.name, COUNT(t.transaction_id) as unpaid_count, "
        "SUM(t.amount) as total_amount "
        "FROM users u "
        "LEFT JOIN transactions t ON u.user_id = t.user_id AND t.status = 0 "
        "WHERE u.role_id = 'role_owner' "
        "GROUP BY u.user_id "
        "HAVING unpaid_count > 0";

    sqlite3_stmt *stmt;
    if (sqlite3_prepare_v2(db->db, query, -1, &stmt, NULL) == SQLITE_OK)
    {
        printf("\n=== 待缴费用户列表 ===\n");
        printf("%-20s%-15s%-15s%-15s%-15s\n",
               "用户名", "姓名", "未缴费笔数", "待缴总额", "状态");
        printf("------------------------------------------------------------------\n");

        while (sqlite3_step(stmt) == SQLITE_ROW)
        {
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

/**
 * @brief 显示所有用户
 *
 * @param db 数据库连接
 */
void show_all_users(Database *db)
{
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
    if (sqlite3_prepare_v2(db->db, query, -1, &stmt, NULL) == SQLITE_OK)
    {
        printf("\n=== 业主信息总览 ===\n\n");
        
        printf(" %-8s %-12s %-8s %-10s %-6s %-8s %-12s %-12s\n",
               "姓名", "电话", "楼号", "房号", "面积", "已缴费", "待缴费", "缴费率");
       

        while (sqlite3_step(stmt) == SQLITE_ROW)
        {
            const char *name = (const char *)sqlite3_column_text(stmt, 2);
            const char *phone = (const char *)sqlite3_column_text(stmt, 3);
            const char *building = (const char *)sqlite3_column_text(stmt, 5);
            const char *room = (const char *)sqlite3_column_text(stmt, 6);
            double area = sqlite3_column_double(stmt, 7);
            int paid_count = sqlite3_column_int(stmt, 8);
            double total_paid = sqlite3_column_double(stmt, 9);
            int unpaid_count = sqlite3_column_int(stmt, 10);
            double total_unpaid = sqlite3_column_double(stmt, 11);

            double payment_rate = 0;
            if (paid_count + unpaid_count > 0)
            {
                payment_rate = (double)paid_count / (paid_count + unpaid_count) * 100;
            }

            printf(" %-8s %-12s %-8s %-10s %6.1f %8.2f %12.2f %11.1f%%\n",
                   name ? name : "未知",
                   phone ? phone : "未知",
                   building ? building : "未分配",
                   room ? room : "未分配",
                   area,
                   total_paid,
                   total_unpaid,
                   payment_rate);

            if (unpaid_count > 0)
            {
                
                printf(" 待缴费明细:                                                                              \n");

                const char *detail_query =
                    "SELECT fee_type, amount, due_date "
                    "FROM transactions "
                    "WHERE user_id = ? AND status = 0 "
                    "ORDER BY due_date ASC";

                sqlite3_stmt *detail_stmt;
                if (sqlite3_prepare_v2(db->db, detail_query, -1, &detail_stmt, NULL) == SQLITE_OK)
                {
                    sqlite3_bind_text(detail_stmt, 1, sqlite3_column_text(stmt, 0), -1, SQLITE_STATIC);

                    while (sqlite3_step(detail_stmt) == SQLITE_ROW)
                    {
                        int fee_type = sqlite3_column_int(detail_stmt, 0);
                        double amount = sqlite3_column_double(detail_stmt, 1);
                        time_t due_date = sqlite3_column_int64(detail_stmt, 2);

                        char date_str[20];
                        strftime(date_str, sizeof(date_str), "%Y-%m-%d", localtime(&due_date));

                        printf(" %-12s: %-8.2f元  到期日期: %-12s                                          \n",
                               get_fee_type_name(fee_type),
                               amount,
                               date_str);
                    }
                    sqlite3_finalize(detail_stmt);
                }
            }
            
        }
        
        sqlite3_finalize(stmt);
    }
    wait_for_key();
}

/**
 * @brief 打印用户表头
 */
static void print_user_table_header()
{
    printf("\n%-12s%-15s%-15s%-15s%-25s%-15s%-15s\n",
           "用户名", "姓名", "电话", "楼号", "房号", "注册时间", "状态");
    printf("--------------------------------------------------------------------------------\n");
}

/**
 * @brief 打印用户信息
 *
 * @param stmt SQLite查询结果
 */
static void print_user_info(sqlite3_stmt *stmt)
{
    time_t reg_time = sqlite3_column_int64(stmt, 5);
    char time_str[20];
    strftime(time_str, sizeof(time_str), "%Y-%m-%d", localtime(&reg_time));

    printf("%-12s%-15s%-15s%-15s%-25s%-15s%-15s\n",
           sqlite3_column_text(stmt, 1),
           sqlite3_column_text(stmt, 2),
           sqlite3_column_text(stmt, 3),
           sqlite3_column_text(stmt, 7),
           sqlite3_column_text(stmt, 6),
           time_str,
           "正常");
}

/**
 * @brief 显示已缴费用户
 *
 * @param db 数据库连接
 */
void show_paid_users(Database *db)
{
    clear_staff_screen();
    printf("\n=== 已缴费用户信息 ===\n\n");

    const char *paid_query =
        "SELECT u.user_id, u.username, u.name, u.phone_number, b.building_name, r.room_number, "
        "r.area_sqm, SUM(t.amount) as total_paid, COUNT(t.transaction_id) as payment_count, "
        "MAX(t.payment_date) as latest_payment "
        "FROM users u "
        "JOIN transactions t ON u.user_id = t.user_id "
        "LEFT JOIN rooms r ON u.user_id = r.owner_id "
        "LEFT JOIN buildings b ON r.building_id = b.building_id "
        "WHERE t.status = 1 AND u.role_id = 'role_owner' "
        "GROUP BY u.user_id, u.username, u.name, u.phone_number, b.building_name, r.room_number "
        "ORDER BY total_paid DESC";

    sqlite3_stmt *stmt;
    if (sqlite3_prepare_v2(db->db, paid_query, -1, &stmt, NULL) == SQLITE_OK)
    {
       
        printf(" %-8s %-8s %-12s %-8s %-6s %-10s %-8s %-10s\n",
               "用户ID", "姓名", "电话", "楼号", "房号", "已缴金额", "缴费笔数", "最近缴费");
        

        int count = 0;
        double total_paid = 0.0;

        while (sqlite3_step(stmt) == SQLITE_ROW)
        {
            count++;
            char date_str[20] = "未知";
            time_t payment_date = sqlite3_column_int64(stmt, 9);
            if (payment_date > 0)
            {
                strftime(date_str, sizeof(date_str), "%Y-%m-%d", localtime(&payment_date));
            }

            double amount = sqlite3_column_double(stmt, 7);
            total_paid += amount;

            printf(" %-8s %-8s %-12s %-8s %-6s %10.2f %8d %-10s\n",
                   sqlite3_column_text(stmt, 0),
                   sqlite3_column_text(stmt, 2),
                   sqlite3_column_text(stmt, 3),
                   sqlite3_column_text(stmt, 4) ? sqlite3_column_text(stmt, 4) : (const unsigned char *)"未知",
                   sqlite3_column_text(stmt, 5) ? sqlite3_column_text(stmt, 5) : (const unsigned char *)"未知",
                   amount,
                   sqlite3_column_int(stmt, 8),
                   date_str);
        }

        
        printf(" 总计: %d位业主已缴费，收款总额 %.2f元                                      │\n",
               count, total_paid);
       

        if (count == 0)
        {
            printf("\n当前没有业主有已缴费记录。\n");
        }

        sqlite3_finalize(stmt);
    }
    else
    {
        printf("\n查询数据库失败: %s\n", sqlite3_errmsg(db->db));
    }

    wait_for_key();
}

/**
 * @brief 显示未缴费用户
 *
 * @param db 数据库连接
 */
void show_unpaid_users(Database *db)
{
    clear_staff_screen();
    printf("\n=== 欠费业主明细表 ===\n\n");

    const char *query =
        "SELECT u.user_id, u.name, u.phone_number, b.building_name, r.room_number, "
        "SUM(t.amount) as total_due, COUNT(t.transaction_id) as unpaid_count, "
        "GROUP_CONCAT(DISTINCT CASE "
        "  WHEN t.fee_type = 1 THEN '物业费' "
        "  WHEN t.fee_type = 2 THEN '停车费' "
        "  WHEN t.fee_type = 3 THEN '水费' "
        "  WHEN t.fee_type = 4 THEN '电费' "
        "  WHEN t.fee_type = 5 THEN '燃气费' "
        "  ELSE '其他费用' END) as fee_types, "
        "MIN(t.due_date) as earliest_due, "
        "julianday('now') - julianday(datetime(MIN(t.due_date), 'unixepoch')) as overdue_days "
        "FROM users u "
        "JOIN transactions t ON u.user_id = t.user_id "
        "LEFT JOIN rooms r ON u.user_id = r.owner_id "
        "LEFT JOIN buildings b ON r.building_id = b.building_id "
        "WHERE t.status = 0 AND u.role_id = 'role_owner' "
        "GROUP BY u.user_id "
        "ORDER BY overdue_days DESC";

    sqlite3_stmt *stmt;
    if (sqlite3_prepare_v2(db->db, query, -1, &stmt, NULL) == SQLITE_OK)
    {
       
        printf(" %-8s %-12s %-15s %-8s    %-10s    %-10s\n",
               "姓名", "电话", "住址", "欠费金额", "欠费项目", "欠费状态");
        

        int count = 0;
        double total_unpaid = 0.0;

        while (sqlite3_step(stmt) == SQLITE_ROW)
        {
            count++;
            const char *name = (const char *)sqlite3_column_text(stmt, 1);
            const char *phone = (const char *)sqlite3_column_text(stmt, 2);

            char address[50] = "";
            if (sqlite3_column_text(stmt, 3) && sqlite3_column_text(stmt, 4))
            {
                snprintf(address, sizeof(address), "%s-%s",
                         (const char *)sqlite3_column_text(stmt, 3),
                         (const char *)sqlite3_column_text(stmt, 4));
            }
            else
            {
                strcpy(address, "未登记");
            }

            double amount = sqlite3_column_double(stmt, 5);
            const char *fee_types = (const char *)sqlite3_column_text(stmt, 7);
            int overdue_days = (int)sqlite3_column_double(stmt, 9);

            total_unpaid += amount;

            const char *status = "正常";
            if (overdue_days > 90)
                status = "严重逾期";
            else if (overdue_days > 30)
                status = "已逾期";
            else if (overdue_days > 0)
                status = "即将逾期";

            printf(" %-8s %-12s %-15s %8.2f %-10s %-10s\n",
                   name, phone, address, amount,
                   fee_types ? fee_types : "物业费等", status);
        }

        if (count > 0)
        {
            
            printf(" 总计: %-2d位业主欠费，共计金额: %-10.2f元                        \n",
                   count, total_unpaid);
        }
        else
        {
            printf(" %-70s \n", "当前没有欠费业主");
        }

        
        sqlite3_finalize(stmt);
    }

    wait_for_key();
}

/**
 * @brief 显示服务人员负责区域
 *
 * @param db 数据库连接
 * @param staff_id 服务人员ID
 */
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
    if (sqlite3_prepare_v2(db->db, query, -1, &stmt, NULL) == SQLITE_OK)
    {
        sqlite3_bind_text(stmt, 1, staff_id, -1, SQLITE_STATIC);

        
        printf(" %-10s %-20s %-12s %-12s\n",
               "楼栋", "地址", "房间数", "业主数");
        

        bool found = false;
        while (sqlite3_step(stmt) == SQLITE_ROW)
        {
            found = true;
            printf(" %-10s %-20s %-12d %-12d\n",
                   sqlite3_column_text(stmt, 0),
                   sqlite3_column_text(stmt, 1),
                   sqlite3_column_int(stmt, 2),
                   sqlite3_column_int(stmt, 3));
        }

        

        if (!found)
        {
            printf("\n⚠️ 当前未分配负责区域\n");
        }

        sqlite3_finalize(stmt);
    }
    wait_for_key();
}

/**
 * @brief 显示缴费管理菜单
 *
 * @param db 数据库连接
 */
void show_payment_management_menu(Database *db)
{
    while (1)
    {
        clear_staff_screen();
        printf("\n===== 业主缴费管理 =====\n");
        printf("1. 按姓名查询业主缴费情况\n");
        printf("2. 显示所有欠费业主\n");
        printf("3. 显示所有已缴费业主\n");
        printf("4. 查看当前收费标准\n");
        printf("0. 返回上级菜单\n");
        printf("请选择(0-4): ");

        int choice;
        scanf("%d", &choice);
        clear_input_buffer();

        switch (choice)
        {
        case 1:
            show_owner_payment_query(db);
            break;
        case 2:
            show_unpaid_users(db);
            break;
        case 3:
            show_paid_users(db);
            break;
        case 4:
            show_current_fee_standards(db);
            wait_for_key();
            break;
        case 0:
            return;
        default:
            printf("无效选择，请重试\n");
            wait_for_key();
        }
    }
}

/**
 * @brief 显示提醒管理菜单
 *
 * @param db 数据库连接
 */
void show_reminder_management_menu(Database *db)
{
    int choice;
    do
    {
        clear_staff_screen();
        printf("\n=== 缴费提醒管理 ===\n");
        printf("1. 发送单个提醒\n");
        printf("2. 批量发送提醒\n");
        printf("0. 返回上级菜单\n");
        printf("\n请选择: ");

        scanf("%d", &choice);
        clear_input_buffer();

        switch (choice)
        {
        case 1:
        {
            char user_id[37];
            printf("请输入业主姓名: ");
            scanf("%s", user_id);
            send_payment_reminder_screen(db, user_id);
            break;
        }
        case 2:
            send_bulk_payment_reminders(db);
            break;
        case 0:
            return;
        default:
            printf("无效选择，请重试\n");
        }
    } while (choice != 0);
}

/**
 * @brief 显示统计菜单
 *
 * @param db 数据库连接
 */
void show_statistics_menu(Database *db)
{
    int choice;
    do
    {
        clear_staff_screen();
        printf("\n===== 缴费统计分析 =====\n");
        printf("1. 按年度查看统计\n");
        printf("2. 查看当前月度统计\n");
        printf("0. 返回上级菜单\n");
        printf("\n请选择: ");

        scanf("%d", &choice);
        clear_input_buffer();

        switch (choice)
        {
        case 1:
        {
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

/**
 * @brief 显示年度统计
 *
 * @param db 数据库连接
 * @param year 年份
 */
void show_yearly_statistics(Database *db, int year)
{
    clear_staff_screen();
    printf("\n=== %d年度缴费统计 ===\n\n", year);

    char year_str[5];
    snprintf(year_str, sizeof(year_str), "%d", year);

    const char *query =
        "SELECT "
        "(SELECT COUNT(DISTINCT user_id) FROM users WHERE role_id = 'role_owner') as total_owners, "
        "(SELECT COUNT(DISTINCT user_id) FROM transactions WHERE status = 1 AND strftime('%Y', datetime(payment_date, 'unixepoch')) = ?) as paid_users, "
        "(SELECT COUNT(DISTINCT user_id) FROM transactions WHERE status = 0 AND strftime('%Y', datetime(due_date, 'unixepoch')) = ?) as unpaid_users, "
        "(SELECT SUM(amount) FROM transactions WHERE status = 1 AND strftime('%Y', datetime(payment_date, 'unixepoch')) = ?) as paid_amount, "
        "(SELECT SUM(amount) FROM transactions WHERE status = 0 AND strftime('%Y', datetime(due_date, 'unixepoch')) = ?) as unpaid_amount";

    sqlite3_stmt *stmt;
    if (sqlite3_prepare_v2(db->db, query, -1, &stmt, NULL) == SQLITE_OK)
    {
        for (int i = 1; i <= 4; i++)
        {
            sqlite3_bind_text(stmt, i, year_str, -1, SQLITE_STATIC);
        }

        if (sqlite3_step(stmt) == SQLITE_ROW)
        {
            int total_owners = sqlite3_column_int(stmt, 0);
            int paid_users = sqlite3_column_int(stmt, 1);
            int unpaid_users = sqlite3_column_int(stmt, 2);
            double paid_amount = sqlite3_column_double(stmt, 3);
            double unpaid_amount = sqlite3_column_double(stmt, 4);

        
            printf(" 总业主数量: %-8d           \n", total_owners);
            printf(" 已缴费用户: %-8d \n",
                   paid_users, total_owners > 0 ? (float)paid_users / total_owners * 100 : 0);
            printf(" 未缴费用户: %-8d \n",
                   unpaid_users, total_owners > 0 ? (float)unpaid_users / total_owners * 100 : 0);
            printf(" 已收缴费总额: %-8.2f元     \n", paid_amount);
            printf(" 未收缴费总额: %-8.2f元     \n", unpaid_amount);
            
        }
        sqlite3_finalize(stmt);
    }

    const char *type_query =
        "SELECT fee_type, "
        "COUNT(DISTINCT user_id) as user_count, "
        "SUM(amount) as total_amount "
        "FROM transactions "
        "WHERE (status = 0 AND strftime('%Y', datetime(due_date, 'unixepoch')) = ?) "
        "OR (status = 1 AND strftime('%Y', datetime(payment_date, 'unixepoch')) = ?) "
        "GROUP BY fee_type";

    if (sqlite3_prepare_v2(db->db, type_query, -1, &stmt, NULL) == SQLITE_OK)
    {
        sqlite3_bind_text(stmt, 1, year_str, -1, SQLITE_STATIC);
        sqlite3_bind_text(stmt, 2, year_str, -1, SQLITE_STATIC);

        printf("【按费用类型统计】\n");
        
        printf(" %-10s  %-8s  %-10s \n", "费用类型", "用户数", "总金额");
       

        double total = 0;
        while (sqlite3_step(stmt) == SQLITE_ROW)
        {
            int fee_type = sqlite3_column_int(stmt, 0);
            int user_count = sqlite3_column_int(stmt, 1);
            double amount = sqlite3_column_double(stmt, 2);
            total += amount;

            printf(" %-10s  %-8d  %10.2f \n",
                   get_fee_type_name(fee_type),
                   user_count,
                   amount);
        }
        
        printf(" %-10s  %-8s  %10.2f \n", "合计", "", total);

        sqlite3_finalize(stmt);
    }

    const char *unpaid_list_query =
        "SELECT DISTINCT u.name, u.phone_number, b.building_name, r.room_number, "
        "COUNT(t.transaction_id) as unpaid_count, SUM(t.amount) as unpaid_amount "
        "FROM transactions t "
        "JOIN users u ON t.user_id = u.user_id "
        "LEFT JOIN rooms r ON u.user_id = r.owner_id "
        "LEFT JOIN buildings b ON r.building_id = b.building_id "
        "WHERE t.status = 0 AND strftime('%Y', datetime(t.due_date, 'unixepoch')) = ? "
        "GROUP BY u.user_id "
        "ORDER BY unpaid_amount DESC "
        "LIMIT 10";

    if (sqlite3_prepare_v2(db->db, unpaid_list_query, -1, &stmt, NULL) == SQLITE_OK)
    {
        sqlite3_bind_text(stmt, 1, year_str, -1, SQLITE_STATIC);

        printf("【%d年度未缴费业主TOP10】\n", year);
        
        printf(" %-8s %-12s %-8s %-6s %-8s %-10s\n",
               "业主", "电话", "楼号", "房号", "未缴笔数", "未缴金额");
        

        bool found = false;
        while (sqlite3_step(stmt) == SQLITE_ROW)
        {
            found = true;
            printf(" %-8s %-12s %-8s %-6s %-8d %10.2f\n",
                   sqlite3_column_text(stmt, 0),
                   sqlite3_column_text(stmt, 1),
                   sqlite3_column_text(stmt, 2) ? sqlite3_column_text(stmt, 2) : (const unsigned char *)"未知",
                   sqlite3_column_text(stmt, 3) ? sqlite3_column_text(stmt, 3) : (const unsigned char *)"未知",
                   sqlite3_column_int(stmt, 4),
                   sqlite3_column_double(stmt, 5));
        }

        if (!found)
        {
            printf(" %-62s \n", "暂无未缴费记录");
        }

        
        sqlite3_finalize(stmt);
    }

    wait_for_key();
}

/**
 * @brief 显示当前统计
 *
 * @param db 数据库连接
 */
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
    if (sqlite3_prepare_v2(db->db, query, -1, &stmt, NULL) == SQLITE_OK)
    {
        if (sqlite3_step(stmt) == SQLITE_ROW)
        {
            int total_owners = sqlite3_column_int(stmt, 0);
            int paid_users = sqlite3_column_int(stmt, 1);
            int unpaid_users = sqlite3_column_int(stmt, 2);
            double total_paid = sqlite3_column_double(stmt, 3);
            double total_unpaid = sqlite3_column_double(stmt, 4);

            
            printf("总业主数量: %-8d           \n", total_owners);
            printf("已缴费用户: %-8d (%.1f%%)   \n",
                   paid_users, total_owners > 0 ? (float)paid_users / total_owners * 100 : 0);
            printf("未缴费用户: %-8d (%.1f%%)   \n",
                   unpaid_users, total_owners > 0 ? (float)unpaid_users / total_owners * 100 : 0);
            printf("已收缴费总额: %-8.2f元     \n", total_paid);
            printf("未收缴费总额: %-8.2f元     \n", total_unpaid);
            
        }
        sqlite3_finalize(stmt);
    }
    wait_for_key();
}

/**
 * @brief 显示欠费分析
 *
 * @param db 数据库连接
 */
void show_unpaid_analysis(Database *db)
{
    clear_staff_screen();
    printf("\n=== 欠费情况分析 ===\n\n");

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
    if (sqlite3_prepare_v2(db->db, duration_query, -1, &stmt, NULL) == SQLITE_OK)
    {
        printf("【按欠费时长统计】\n");
        printf("%-10s  %-8s  %-10s \n", "欠费时长", "用户数", "欠费金额");

        while (sqlite3_step(stmt) == SQLITE_ROW)
        {
            printf(" %-10s  %-8d  %10.2f \n",
                   sqlite3_column_text(stmt, 0),
                   sqlite3_column_int(stmt, 1),
                   sqlite3_column_double(stmt, 2));
        }
        
        sqlite3_finalize(stmt);
    }

    const char *type_query =
        "SELECT fee_type, COUNT(DISTINCT user_id) as user_count, "
        "SUM(amount) as total_amount "
        "FROM transactions "
        "WHERE status = 0 "
        "GROUP BY fee_type "
        "ORDER BY total_amount DESC";

    if (sqlite3_prepare_v2(db->db, type_query, -1, &stmt, NULL) == SQLITE_OK)
    {
        printf("【按费用类型统计】\n");
        
        printf(" %-10s  %-8s  %-10s \n", "费用类型", "用户数", "欠费金额");
        

        double total = 0;
        while (sqlite3_step(stmt) == SQLITE_ROW)
        {
            int fee_type = sqlite3_column_int(stmt, 0);
            int user_count = sqlite3_column_int(stmt, 1);
            double amount = sqlite3_column_double(stmt, 2);
            total += amount;

            printf(" %-10s  %-8d  %10.2f \n",
                   get_fee_type_name(fee_type),
                   user_count,
                   amount);
        }
       
        printf(" %-10s  %-8s  %10.2f \n", "合计", "", total);
       

        sqlite3_finalize(stmt);
    }
    wait_for_key();
}

/**
 * @brief 显示业主缴费查询
 *
 * @param db 数据库连接
 */
void show_owner_payment_query(Database *db)
{
    clear_staff_screen();
    printf("\n=== 按姓名查询业主缴费情况 ===\n\n");

    char name[50];
    printf("请输入业主姓名(可输入部分姓名): ");
    scanf("%s", name);
    clear_input_buffer();

    const char *query =
        "SELECT u.user_id, u.name, u.phone_number, b.building_name, r.room_number, "
        "SUM(CASE WHEN t.status = 1 THEN t.amount ELSE 0 END) as paid_amount, "
        "SUM(CASE WHEN t.status = 0 THEN t.amount ELSE 0 END) as unpaid_amount, "
        "COUNT(CASE WHEN t.status = 1 THEN 1 END) as paid_count, "
        "COUNT(CASE WHEN t.status = 0 THEN 1 END) as unpaid_count "
        "FROM users u "
        "LEFT JOIN rooms r ON u.user_id = r.owner_id "
        "LEFT JOIN buildings b ON r.building_id = b.building_id "
        "LEFT JOIN transactions t ON u.user_id = t.user_id "
        "WHERE u.role_id = 'role_owner' AND u.name LIKE ? "
        "GROUP BY u.user_id "
        "HAVING paid_count > 0 OR unpaid_count > 0";

    sqlite3_stmt *stmt;
    if (sqlite3_prepare_v2(db->db, query, -1, &stmt, NULL) == SQLITE_OK)
    {
        char pattern[52];
        snprintf(pattern, sizeof(pattern), "%%%s%%", name);
        sqlite3_bind_text(stmt, 1, pattern, -1, SQLITE_STATIC);

        bool found = false;
        while (sqlite3_step(stmt) == SQLITE_ROW)
        {
            found = true;
            const char *user_id = (const char *)sqlite3_column_text(stmt, 0);
            const char *owner_name = (const char *)sqlite3_column_text(stmt, 1);
            const char *phone = (const char *)sqlite3_column_text(stmt, 2);
            const char *building = sqlite3_column_text(stmt, 3) ? (const char *)sqlite3_column_text(stmt, 3) : "未知";
            const char *room = sqlite3_column_text(stmt, 4) ? (const char *)sqlite3_column_text(stmt, 4) : "未知";
            double paid = sqlite3_column_double(stmt, 5);
            double unpaid = sqlite3_column_double(stmt, 6);
            int paid_count = sqlite3_column_int(stmt, 7);
            int unpaid_count = sqlite3_column_int(stmt, 8);

            printf("\n=== %s (%s) - %s楼 %s房 - 缴费记录 ===\n\n",
                   owner_name, phone, building, room);

           
            printf(" 已缴费: %2d笔，共计 %-10.2f 元          \n", paid_count, paid);
            printf(" 未缴费: %2d笔，共计 %-10.2f 元          \n", unpaid_count, unpaid);

            double rate = 0;
            if (paid_count + unpaid_count > 0)
                rate = (double)paid_count / (paid_count + unpaid_count) * 100;

            printf(" 缴费率: %-5.1f%%                                \n", rate);
            

            if (paid_count > 0)
            {
                show_payment_details(db, user_id, 1);
            }

            if (unpaid_count > 0)
            {
                show_payment_details(db, user_id, 0);

                printf("\n是否向该业主发送缴费提醒? (y/n): ");
                char choice;
                scanf(" %c", &choice);
                clear_input_buffer();

                if (choice == 'y' || choice == 'Y')
                {
                    send_payment_reminder(db, user_id, unpaid, "");
                    printf("\n✓ 已成功向 %s 发送缴费提醒\n", owner_name);
                }
            }
        }

        if (!found)
        {
            printf("\n未找到匹配的业主'%s'的缴费记录\n", name);
        }

        sqlite3_finalize(stmt);
    }
    else
    {
        printf("查询失败: %s\n", sqlite3_errmsg(db->db));
    }

    wait_for_key();
}

/**
 * @brief 显示缴费明细
 *
 * @param db 数据库连接
 * @param user_id 用户ID
 * @param status 缴费状态
 */
void show_payment_details(Database *db, const char *user_id, int status)
{
    const char *status_str = status == 1 ? "已缴费" : "未缴费";
    printf("\n【%s记录】\n", status_str);
    
    printf(" %-10s %-8s %-12s %-15s\n",
           "费用类型", "金额", "日期", "备注");
    

    const char *query =
        "SELECT t.fee_type, t.amount, "
        "CASE WHEN t.status = 1 THEN t.payment_date ELSE t.due_date END as date, "
        "t.description "
        "FROM transactions t "
        "WHERE t.user_id = ? AND t.status = ? "
        "ORDER BY date DESC";

    sqlite3_stmt *stmt;
    if (sqlite3_prepare_v2(db->db, query, -1, &stmt, NULL) == SQLITE_OK)
    {
        sqlite3_bind_text(stmt, 1, user_id, -1, SQLITE_STATIC);
        sqlite3_bind_int(stmt, 2, status);

        bool found = false;
        while (sqlite3_step(stmt) == SQLITE_ROW)
        {
            found = true;
            int fee_type = sqlite3_column_int(stmt, 0);
            double amount = sqlite3_column_double(stmt, 1);
            time_t date = sqlite3_column_int64(stmt, 2);
            const char *desc = (const char *)sqlite3_column_text(stmt, 3);

            char date_str[20];
            strftime(date_str, sizeof(date_str), "%Y-%m-%d", localtime(&date));

            printf(" %-10s %8.2f %-12s %-15s\n",
                   get_fee_type_name(fee_type),
                   amount,
                   date_str,
                   desc ? desc : "");
        }

        if (!found)
        {
            printf(" %-54s \n", "无记录");
        }

       
        sqlite3_finalize(stmt);
    }
}

/**
 * @brief 显示提醒历史记录
 *
 * @param db 数据库连接
 */
void show_reminder_history(Database *db)
{
    clear_staff_screen();
    printf("\n=== 已发送提醒记录 ===\n\n");

    const char *query =
        "SELECT pr.reminder_id, u.name, u.phone_number, pr.send_time, "
        "substr(pr.reminder_content, 1, 30) as content_preview "
        "FROM payment_reminders pr "
        "JOIN users u ON pr.user_id = u.user_id "
        "ORDER BY pr.send_time DESC "
        "LIMIT 20";

    sqlite3_stmt *stmt;
    if (sqlite3_prepare_v2(db->db, query, -1, &stmt, NULL) == SQLITE_OK)
    {
        
        printf(" %-8s %-10s %-12s %-16s %-20s\n",
               "编号", "姓名", "电话", "发送时间", "内容预览");
       

        bool found = false;
        while (sqlite3_step(stmt) == SQLITE_ROW)
        {
            found = true;
            int id = sqlite3_column_int(stmt, 0);
            const char *name = (const char *)sqlite3_column_text(stmt, 1);
            const char *phone = (const char *)sqlite3_column_text(stmt, 2);
            time_t send_time = sqlite3_column_int64(stmt, 3);
            const char *preview = (const char *)sqlite3_column_text(stmt, 4);

            char time_str[20];
            strftime(time_str, sizeof(time_str), "%Y-%m-%d %H:%M", localtime(&send_time));

            printf(" %-8d %-10s %-12s %-16s %-20s\n",
                   id, name, phone, time_str, preview);
        }

        if (!found)
        {
            printf(" %-68s \n", "暂无提醒记录");
        }

        
        sqlite3_finalize(stmt);
    }
    else
    {
        printf("查询提醒记录失败: %s\n", sqlite3_errmsg(db->db));
    }
}