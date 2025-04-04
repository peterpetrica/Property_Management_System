/**
 * @file ui_admin.c
 * @brief 物业服务管理系统管理员界面实现
 *
 * 本文件包含所有管理员操作界面的实现，包括信息管理、服务分配、
 * 信息查询、信息排序、统计分析、系统维护和费用管理等功能模块。
 *
 * @author @ZhuLaoliu888 @abbydu626 @Fxiansheng-ai @peterpetrica
 * @date 2025-03
 */
#include "ui/ui_admin.h"
#include "ui/ui_login.h"
#include "models/building.h"
#include "models/apartment.h"
#include "models/user.h"
#include "models/transaction.h"
#include "models/service.h"
#include "db/db_query.h"
#include "db/db_utils.h"
#include "utils/utils.h"
#include "utils/file_ops.h"
#include "utils/console.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h> 

/**
 * @brief 显示管理员主界面
 *
 * 显示管理员系统的主菜单，提供各功能模块的入口，包括信息管理、
 * 服务分配、信息查询、信息排序、信息统计、系统维护和费用管理等功能。
 *
 * @param db 数据库连接指针
 * @param user_id 当前登录用户的ID
 * @param user_type 当前登录用户的类型
 */
void show_admin_main_screen(Database *db, const char *user_id, UserType user_type)
{
    while (1)
    {
        clear_screen();
        printf("\n=== 物业服务管理系统主页面 ===\n");
        printf("1. 信息管理界面\n");
        printf("2. 服务分配界面\n");
        printf("3. 信息查询界面\n");
        printf("4. 信息排序界面\n");
        printf("5. 信息统计界面\n");
        printf("6. 系统维护界面\n");
        printf("7. 费用管理界面\n");
        printf("8. 退出系统\n");
        printf("请输入选项: ");

        int choice;
        scanf("%d", &choice);
        getchar();

        switch (choice)
        {
        case 1:
            show_info_management_screen(db, user_id, user_type);
            break;
        case 2:
            show_service_assignment_screen(db, user_id, user_type);
            break;
        case 3:
            show_info_query_screen(db, user_id, user_type);
            break;
        case 4:
            show_info_sort_screen(db, user_id, user_type);
            break;
        case 5:
            show_info_statistics_screen(db, user_id, user_type);
            break;
        case 6:
            show_system_maintenance_screen(db, user_id, user_type);
            break;
        case 7:
            manage_fee_standards_screen(db, user_id, user_type);
            break;
        case 8:
            printf("退出系统。\n");
            return;
        default:
            printf("无效选项，请重新输入。\n");
            pause_console();
            break;
        }
    }
}

/**
 * @brief 业主信息链表节点结构
 */
typedef struct OwnerNode
{
    char name[100];
    char phone[20];
    char email[100];
    char building[50];
    char room[20];
    char floor[10];
    struct OwnerNode *next;
} OwnerNode;

/**
 * @brief 服务人员信息链表节点结构
 */
typedef struct StaffNode
{
    char name[100];
    char service_type[50];
    int building_count;
    char buildings[255];
    struct StaffNode *next;
} StaffNode;

/**
 * @brief 创建一个新的业主节点
 *
 * @param name 业主姓名
 * @param phone 电话号码
 * @param email 电子邮箱
 * @param building 所在楼宇
 * @param room 房间号
 * @param floor 楼层
 * @return OwnerNode* 新创建的节点指针
 */
OwnerNode *create_owner_node_by_admin(const char *name, const char *phone, const char *email,
                                      const char *building, const char *room, const char *floor)
{
    OwnerNode *node = (OwnerNode *)malloc(sizeof(OwnerNode));
    if (node == NULL)
    {
        printf("内存分配失败\n");
        return NULL;
    }

    strncpy(node->name, name ? name : "未知", sizeof(node->name) - 1);
    strncpy(node->phone, phone ? phone : "未登记", sizeof(node->phone) - 1);
    strncpy(node->email, email ? email : "未登记", sizeof(node->email) - 1);
    strncpy(node->building, building ? building : "未分配", sizeof(node->building) - 1);
    strncpy(node->room, room ? room : "--", sizeof(node->room) - 1);
    strncpy(node->floor, floor ? floor : "--", sizeof(node->floor) - 1);

    node->next = NULL;
    return node;
}

/**
 * @brief 创建一个新的服务人员节点
 *
 * @param name 服务人员姓名
 * @param service_type 服务类型
 * @param building_count 负责楼宇数量
 * @param buildings 负责楼宇列表
 * @return StaffNode* 新创建的节点指针
 */
StaffNode *create_staff_node(const char *name, const char *service_type,
                             int building_count, const char *buildings)
{
    StaffNode *node = (StaffNode *)malloc(sizeof(StaffNode));
    if (node == NULL)
    {
        printf("内存分配失败\n");
        return NULL;
    }

    strncpy(node->name, name ? name : "未知", sizeof(node->name) - 1);
    strncpy(node->service_type, service_type ? service_type : "未知", sizeof(node->service_type) - 1);
    node->building_count = building_count;
    strncpy(node->buildings, buildings ? buildings : "无", sizeof(node->buildings) - 1);

    node->next = NULL;
    return node;
}

/**
 * @brief 保存业主信息到文件
 *
 * @param head 业主信息链表头指针
 * @param filename 要保存的文件名
 */
void save_owners_to_file(OwnerNode *head, const char *filename)
{
    FILE *file = fopen(filename, "w");
    if (!file)
    {
        printf("无法打开文件 %s\n", filename);
        return;
    }
    fprintf(file, "%-15s %-15s %-25s %-15s %-10s %-6s\n",
            "姓名", "电话", "邮箱", "所在楼宇", "房间号", "楼层");
    fprintf(file, "--------------------------------------------------------------------------------\n");

    for (OwnerNode *node = head; node != NULL; node = node->next)
    {
        fprintf(file, "%-15s %-15s %-25s %-15s %-10s %-6s\n",
                node->name, node->phone, node->email,
                node->building, node->room, node->floor);
    }
    fclose(file);
    printf("业主信息已保存到 %s\n", filename);
}

/**
 * @brief 保存服务人员信息到文件
 *
 * @param head 服务人员信息链表头指针
 * @param filename 要保存的文件名
 */
void save_staff_to_file(StaffNode *head, const char *filename)
{
    FILE *file = fopen(filename, "w");
    if (!file)
    {
        printf("无法打开文件 %s\n", filename);
        return;
    }
    fprintf(file, "%-15s %-15s %-12s %-40s\n",
            "姓名", "服务类型", "负责楼宇数", "负责楼宇");
    fprintf(file, "--------------------------------------------------------------------------------\n");

    for (StaffNode *node = head; node != NULL; node = node->next)
    {
        fprintf(file, "%-15s %-15s %-12d %-40s\n",
                node->name, node->service_type, node->building_count, node->buildings);
    }
    fclose(file);
    printf("服务人员信息已保存到 %s\n", filename);
}

/**
 * @brief 管理楼宇信息
 *
 * 提供添加、删除、修改和查看楼宇信息的功能界面
 *
 * @param db 数据库连接指针
 * @param user_id 当前登录用户的ID
 */
void manage_buildings(Database *db, const char *user_id);

/**
 * @brief 管理住户信息
 *
 * 提供添加、删除、修改和查看住户信息的功能界面
 *
 * @param db 数据库连接指针
 * @param user_id 当前登录用户的ID
 */
void manage_apartments(Database *db, const char *user_id);

/**
 * @brief 管理用户信息
 *
 * 提供添加、删除、修改和查看系统用户信息的功能界面
 *
 * @param db 数据库连接指针
 * @param user_id 当前登录用户的ID
 * @param user_type 当前登录用户的类型
 */
void manage_users(Database *db, const char *user_id, UserType user_type);

/**
 * @brief 管理服务项目信息
 *
 * 提供添加、删除、修改和查看服务项目的功能界面
 *
 * @param db 数据库连接指针
 * @param user_id 当前登录用户的ID
 */
void manage_services(Database *db, const char *user_id);

/**
 * @brief 显示费用标准管理界面
 *
 * 提供查看和修改各类费用标准的功能
 *
 * @param db 数据库连接指针
 * @param user_id 当前登录用户的ID
 * @param user_type 当前登录用户的类型
 */
void manage_fee_standards_screen(Database *db, const char *user_id, UserType user_type);

/**
 * @brief 显示定期费用生成界面
 *
 * 提供按周期自动生成物业费用账单的功能
 *
 * @param db 数据库连接指针
 * @param user_id 当前登录用户的ID
 * @param user_type 当前登录用户的类型
 */
void generate_periodic_fees_screen(Database *db, const char *user_id, UserType user_type);

/**
 * @brief 更新费用标准
 *
 * 更新系统中的费用标准信息
 *
 * @param db 数据库连接指针
 * @param user_id 当前登录用户的ID
 * @param user_type 当前登录用户的类型
 * @param standard 要更新的费用标准信息指针
 * @return 更新成功返回true，否则返回false
 */
bool update_fee_standard(Database *db, const char *user_id, UserType user_type, FeeStandard *standard);

/**
 * @brief 显示信息管理界面
 *
 * 提供楼宇管理、住户管理和用户管理的入口
 *
 * @param db 数据库连接指针
 * @param user_id 当前登录用户的ID
 * @param user_type 当前登录用户的类型
 */
void show_info_management_screen(Database *db, const char *user_id, UserType user_type)
{
    while (1)
    {
        int choice;
        clear_screen();
        printf("\n=== 信息管理界面 ===\n");
        printf("1. 楼宇管理\n");
        printf("2. 住户管理\n");
        printf("3. 用户管理\n");
        printf("4. 返回主菜单\n");
        printf("请输入选项: ");
        scanf("%d", &choice);
        getchar();

        switch (choice)
        {
        case 1:
            manage_buildings(db, user_id);
            break;
        case 2:
            manage_apartments(db, user_id);
            break;
        case 3:
            manage_users(db, user_id, user_type);
            break;
        case 4:
            return;
        default:
            printf("无效选项，请重新输入。\n");
            pause_console();
            continue;
        }
    }
}

/**
 * @brief 管理楼宇信息
 *
 * 提供添加、删除、修改和查看楼宇信息的功能
 *
 * @param db 数据库连接指针
 * @param user_id 当前登录用户的ID
 */
void manage_buildings(Database *db, const char *user_id)
{
    int choice;
    char sql[512];
    QueryResult result;

    clear_screen();
    printf("\n=== 楼宇管理 ===\n");
    printf("1. 添加楼宇\n");
    printf("2. 删除楼宇\n");
    printf("3. 修改楼宇信息\n");
    printf("4. 查看所有楼宇\n");
    printf("5. 返回上一级\n");
    printf("请输入选项: ");
    scanf("%d", &choice);
    getchar();

    switch (choice)
    {
    case 1:
    {
        char name[100], address[255];
        int floors;
        char building_id[37];

        printf("请输入楼宇名称: ");
        fgets(name, sizeof(name), stdin);
        trim_newline(name);
        printf("请输入楼宇地址: ");
        fgets(address, sizeof(address), stdin);
        trim_newline(address);
        printf("请输入楼层数: ");
        scanf("%d", &floors);
        getchar();

        generate_uuid(building_id);

        snprintf(sql, sizeof(sql),
                 "INSERT INTO buildings (building_id, building_name, address, floors_count) "
                 "VALUES ('%s', '%s', '%s', %d);",
                 building_id, name, address, floors);

        if (execute_update(db, sql))
            printf("楼宇添加成功！ID: %s\n", building_id);
        else
            printf("楼宇添加失败。\n");
        break;
    }

    case 2:
    {
        char building_name[100];
        printf("请输入要删除的楼宇名称: ");
        fgets(building_name, sizeof(building_name), stdin);
        trim_newline(building_name);

        snprintf(sql, sizeof(sql),
                 "SELECT COUNT(*) FROM buildings WHERE building_name = '%s';",
                 building_name);
        if (!execute_query(db, sql, &result) || atoi(result.rows[0].values[0]) == 0)
        {
            printf("删除失败，楼宇不存在。\n");
            free_query_result(&result);
            break;
        }
        free_query_result(&result);

        snprintf(sql, sizeof(sql),
                 "DELETE FROM buildings WHERE building_name = '%s';",
                 building_name);
        if (execute_update(db, sql))
            printf("楼宇删除成功！\n");
        else
            printf("删除失败，可能是权限不足。\n");
        break;
    }

    case 3:
    {
        char building_name[100];
        char new_name[100], new_address[255];
        printf("请输入要修改的楼宇名: ");
        scanf("%s", &building_name);
        getchar();

        snprintf(sql, sizeof(sql),
                 "SELECT COUNT(*) FROM buildings WHERE building_name = %s;",
                 building_name);
        if (!execute_query(db, sql, &result) || atoi(result.rows[0].values[0]) == 0)
        {
            printf("修改失败，楼宇不存在。\n");
            free_query_result(&result);
            break;
        }
        free_query_result(&result);

        printf("请输入新楼宇名称: ");
        fgets(new_name, sizeof(new_name), stdin);
        trim_newline(new_name);
        printf("请输入新楼宇地址: ");
        fgets(new_address, sizeof(new_address), stdin);
        trim_newline(new_address);

        snprintf(sql, sizeof(sql),
                 "UPDATE buildings SET building_name = '%s', address = '%s' "
                 "WHERE building_name = %s;",
                 new_name, new_address, building_name);
        if (execute_update(db, sql))
            printf("楼宇信息更新成功！\n");
        else
            printf("更新失败，可能是权限不足。\n");
        break;
    }
    case 4:
    {
        snprintf(sql, sizeof(sql),
                 "SELECT building_id, building_name, address, floors_count "
                 "FROM buildings ORDER BY building_id;");

        if (execute_query(db, sql, &result))
        {
            printf("\n=== 楼宇列表 ===\n");
            printf("%-20s %-30s %-10s\n",
                   "楼宇名称", "地址", "楼层数");
            printf("--------------------------------------------------------\n");

            for (int i = 0; i < result.row_count; i++)
            {
                printf("%-20s %-30s %-10s\n",
                       result.rows[i].values[1],
                       result.rows[i].values[2],
                       result.rows[i].values[3]);
            }

            printf("--------------------------------------------------------\n");
            printf("共 %d 条记录\n", result.row_count);

            free_query_result(&result);
        }
        else
        {
            printf("查询失败。\n");
        }
        break;
    }
    case 5:
        return;
    default:
        printf("无效选项。\n");
    }

    pause_console();
    manage_buildings(db, user_id);
}

/**
 * @brief 管理住户信息
 *
 * 提供添加、删除、修改和查看住户信息的功能
 *
 * @param db 数据库连接指针
 * @param user_id 当前登录用户的ID
 */
void manage_apartments(Database *db, const char *user_id)
{
    int choice;
    char sql[512];
    QueryResult result;

    clear_screen();
    printf("\n=== 住户管理 ===\n");
    printf("1. 添加住户\n");
    printf("2. 删除住户\n");
    printf("3. 修改住户信息\n");
    printf("4. 查看所有住户\n");
    printf("5. 返回上一级\n");
    printf("请输入选项: ");
    scanf("%d", &choice);
    getchar();

    switch (choice)
    {
    case 1:
    {
        char building_name[41], owner_username[41];
        int room_number, floor;
        float area_sqm;
        char room_id[37];

        generate_uuid(room_id);

        printf("请输入楼宇名称: ");
        fgets(building_name, sizeof(building_name), stdin);
        trim_newline(building_name);

        printf("请输入房间号: ");
        scanf("%d", &room_number);
        getchar();
        printf("请输入楼层: ");
        scanf("%d", &floor);
        getchar();
        printf("请输入房屋面积(平方米): ");
        scanf("%f", &area_sqm);
        getchar();
        printf("请输入业主用户名: ");
        fgets(owner_username, sizeof(owner_username), stdin);
        trim_newline(owner_username);

        snprintf(sql, sizeof(sql),
                 "SELECT building_id FROM buildings WHERE building_name = '%s';",
                 building_name);
        if (!execute_query(db, sql, &result) || result.row_count == 0)
        {
            printf("错误：楼宇名称不存在。\n");
            free_query_result(&result);
            break;
        }
        char building_id[41];
        strncpy(building_id, result.rows[0].values[0], sizeof(building_id));
        free_query_result(&result);

        snprintf(sql, sizeof(sql),
                 "SELECT user_id FROM users WHERE username = '%s';",
                 owner_username);
        if (!execute_query(db, sql, &result) || result.row_count == 0)
        {
            printf("错误：业主用户名不存在。\n");
            free_query_result(&result);
            break;
        }
        char owner_id[41];
        strncpy(owner_id, result.rows[0].values[0], sizeof(owner_id));
        free_query_result(&result);

        snprintf(sql, sizeof(sql),
                 "INSERT INTO rooms (room_id, building_id, room_number, floor, area_sqm, owner_id, status) "
                 "VALUES ('%s', '%s', %d, %d, %.2f, '%s', '已售');",
                 room_id, building_id, room_number, floor, area_sqm, owner_id);

        if (execute_update(db, sql))
            printf("住户添加成功！房间ID: %s\n", room_id);
        else
            printf("住户添加失败。\n");
        break;
    }
    case 2:
    {
        char building_name[41], room_number[20];

        printf("请输入楼宇名称: ");
        fgets(building_name, sizeof(building_name), stdin);
        trim_newline(building_name);

        printf("请输入房间号: ");
        fgets(room_number, sizeof(room_number), stdin);
        trim_newline(room_number);

        snprintf(sql, sizeof(sql),
                 "SELECT building_id FROM buildings WHERE building_name = '%s';",
                 building_name);
        if (!execute_query(db, sql, &result) || result.row_count == 0)
        {
            printf("错误：楼宇名称不存在。\n");
            free_query_result(&result);
            break;
        }
        char building_id[41]; // 修复变量名
        strncpy(building_id, result.rows[0].values[0], sizeof(building_id));
        free_query_result(&result);

        snprintf(sql, sizeof(sql),
                 "SELECT room_id FROM rooms WHERE building_id = '%s' AND room_number = '%s';",
                 building_id, room_number);
        if (!execute_query(db, sql, &result) || result.row_count == 0)
        {
            printf("错误：指定楼宇中不存在该房间号，无法删除。\n");
            free_query_result(&result);
            break;
        }

        char room_id_str[41];
        strncpy(room_id_str, result.rows[0].values[0], sizeof(room_id_str) - 1);
        room_id_str[sizeof(room_id_str) - 1] = '\0';
        free_query_result(&result);

        snprintf(sql, sizeof(sql),
                 "DELETE FROM rooms WHERE room_id = '%s';", room_id_str);

        if (execute_update(db, sql))
            printf("住户删除成功！\n");
        else
            printf("删除失败。\n");
        break;
    }
    case 3:
    {
        int room_id;
        char new_owner_id[41];

        printf("请输入要修改的房间ID: ");
        scanf("%d", &room_id);
        getchar();
        printf("请输入新业主ID: ");
        fgets(new_owner_id, sizeof(new_owner_id), stdin);
        trim_newline(new_owner_id);

        snprintf(sql, sizeof(sql),
                 "SELECT COUNT(*) FROM rooms WHERE room_id = %d;", room_id);
        if (!execute_query(db, sql, &result) || atoi(result.rows[0].values[0]) == 0)
        {
            printf("错误：房间ID不存在，无法修改。\n");
            free_query_result(&result);
            break;
        }
        free_query_result(&result);

        snprintf(sql, sizeof(sql),
                 "SELECT COUNT(*) FROM owners WHERE owner_id = '%s';",
                 new_owner_id);
        if (!execute_query(db, sql, &result) || atoi(result.rows[0].values[0]) == 0)
        {
            printf("错误：业主ID不存在。\n");
            free_query_result(&result);
            break;
        }
        free_query_result(&result);

        snprintf(sql, sizeof(sql),
                 "UPDATE rooms SET owner_id = '%s', status = '已售' WHERE room_id = %d;",
                 new_owner_id, room_id);

        if (execute_update(db, sql))
            printf("住户信息更新成功！\n");
        else
            printf("更新失败。\n");
        break;
    }
    case 4:
    {
        snprintf(sql, sizeof(sql),
                 "SELECT r.room_id, b.building_name, r.room_number, r.floor, r.area_sqm, u.name AS owner_name, r.status "
                 "FROM rooms r "
                 "LEFT JOIN buildings b ON r.building_id = b.building_id "
                 "LEFT JOIN users u ON r.owner_id = u.user_id "
                 "ORDER BY b.building_name, r.floor, r.room_number;");
        if (execute_query(db, sql, &result))
        {
            printf("\n=== 住户列表 ===\n");
            printf("%-20s %-15s %-8s %-12s %-20s %-12s\n",
                   "楼宇名称", "房间号", "楼层", "面积(m²)", "业主姓名", "状态");
            printf("---------------------------------------------------------------------------------\n");

            for (int i = 0; i < result.row_count; i++)
            {
                int floor = atoi(result.rows[i].values[3]);
                float area = atof(result.rows[i].values[4]);

                printf("%-20s %-15s %-8d %-12.2f %-20s %-12s\n",
                       result.rows[i].values[1],
                       result.rows[i].values[2],
                       floor,
                       area,
                       result.rows[i].values[5] ? result.rows[i].values[5] : "未分配",
                       result.rows[i].values[6]);
            }

            printf("---------------------------------------------------------------------------------\n");
            printf("共 %d 条记录\n", result.row_count);

            free_query_result(&result);
        }
        else
        {
            printf("查询失败。\n");
        }
        break;
    }
    case 5:
        return;
    default:
        printf("无效选项。\n");
    }

    pause_console();
    manage_apartments(db, user_id);
}

/**
 * @brief 管理用户信息
 *
 * 提供添加、删除、修改和查看用户信息的功能
 *
 * @param db 数据库连接指针
 * @param user_id 当前登录用户的ID
 * @param user_type 当前登录用户的类型
 */
void manage_users(Database *db, const char *user_id, UserType user_type)
{
    int choice;
    char sql[512];
    QueryResult result;

    clear_screen();
    printf("\n=== 用户管理 ===\n");
    printf("1. 添加用户\n");
    printf("2. 删除用户\n");
    printf("3. 修改用户信息\n");
    printf("4. 查看所有用户\n");
    printf("5. 返回上一级\n");
    printf("请输入选项: ");
    scanf("%d", &choice);
    clear_input_buffer();

    switch (choice)
    {
    case 1:
    {
        char username[100], password[100], name[100], phone[20], email[100];
    char new_user_id[41];
    int role_choice;
    char role_id[20];

    generate_uuid(new_user_id);
    
    // 修改这部分代码
    printf("请输入用户名: ");
    fgets(username, sizeof(username), stdin);
    trim_newline(username);  // 先获取输入再处理换行符

    printf("请输入密码: ");
    read_password(password, sizeof(password));

    printf("请输入姓名: ");
    fgets(name, sizeof(name), stdin);
    trim_newline(name);

    printf("请输入电话: ");
    fgets(phone, sizeof(phone), stdin);
    trim_newline(phone);

    printf("请输入邮箱: ");
    fgets(email, sizeof(email), stdin);
    trim_newline(email);

    printf("请选择角色(1-管理员 2-物业人员 3-业主): ");
    scanf("%d", &role_choice);
    getchar();  // 清除换行符

        switch (role_choice)
        {
        case 1:
            strcpy(role_id, "role_admin");
            break;
        case 2:
            strcpy(role_id, "role_staff");
            break;
        case 3:
            strcpy(role_id, "role_owner");
            break;
        default:
            printf("无效的角色选择\n");
            return;
        }

        char password_hash[256];
        hash_password(password, password_hash, sizeof(password_hash));
        snprintf(sql, sizeof(sql),
                 "INSERT INTO users (user_id, username, password_hash, name, "
                 "phone_number, email, role_id, status, registration_date) "
                 "VALUES ('%s', '%s', '%s', '%s', '%s', '%s', '%s', '1', datetime('now'));",
                 new_user_id, username, password_hash, name, phone, email, role_id);

        if (execute_update(db, sql))
            printf("用户添加成功！新用户ID: %s\n", new_user_id);
        else
            printf("用户添加失败。\n");
        break;
    }
    case 2:
    {
        char username[100];
        printf("请输入要删除的用户名: ");
        fgets(username, sizeof(username), stdin);
        trim_newline(username);

        snprintf(sql, sizeof(sql), "SELECT COUNT(*) FROM users WHERE username = '%s';", username);
        if (!execute_query(db, sql, &result) || atoi(result.rows[0].values[0]) == 0)
        {
            printf("错误：用户名不存在，无法删除。\n");
            free_query_result(&result);
            break;
        }
        free_query_result(&result);

        snprintf(sql, sizeof(sql), "DELETE FROM users WHERE username = '%s';", username);
        if (execute_update(db, sql))
            printf("用户删除成功！\n");
        else
            printf("删除失败。\n");
        break;
    }
    case 3:
    {
        char username[100], modify_user_id[41];
        char new_password[100], new_name[100], new_phone[20], new_email[100];
        int modify_choice;

        printf("请输入要修改的用户名: ");
        fgets(username, sizeof(username), stdin);
        trim_newline(username);

        snprintf(sql, sizeof(sql), "SELECT user_id FROM users WHERE username = '%s';", username);
        if (!execute_query(db, sql, &result) || result.row_count == 0)
        {
            printf("错误：用户名不存在，无法修改。\n");
            free_query_result(&result);
            break;
        }

        strncpy(modify_user_id, result.rows[0].values[0], sizeof(modify_user_id) - 1);
        free_query_result(&result);

        snprintf(sql, sizeof(sql),
                 "SELECT username, name, phone_number, email, role_id FROM users WHERE user_id = '%s';",
                 modify_user_id);

        if (execute_query(db, sql, &result) && result.row_count > 0)
        {
            printf("\n当前用户信息：\n");
            printf("用户名: %s\n", result.rows[0].values[0]);
            printf("姓名: %s\n", result.rows[0].values[1]);
            printf("电话: %s\n", result.rows[0].values[2]);
            printf("邮箱: %s\n", result.rows[0].values[3]);
            printf("角色: %s\n", result.rows[0].values[4]);
            free_query_result(&result);
        }

        printf("\n请选择要修改的信息：\n");
        printf("1. 密码\n");
        printf("2. 姓名\n");
        printf("3. 电话\n");
        printf("4. 邮箱\n");
        printf("请输入选项(1-4): ");
        scanf("%d", &modify_choice);
        getchar();

        switch (modify_choice)
        {
        case 1:
            printf("请输入新密码: ");
            read_password(new_password, sizeof(new_password));
            char new_password_hash[256];
            hash_password(new_password, new_password_hash, sizeof(new_password_hash));

            snprintf(sql, sizeof(sql),
                     "UPDATE users SET password_hash = '%s' WHERE user_id = '%s';",
                     new_password_hash, modify_user_id);

            if (execute_update(db, sql))
                printf("用户密码更新成功！\n");
            else
                printf("更新失败。\n");
            break;

        case 2:
            do
            {
                printf("请输入新姓名: ");
                fgets(new_name, sizeof(new_name), stdin);
                trim_newline(new_name);

                bool valid = true;
                if (strlen(new_name) > 20)
                {
                    valid = false;
                    printf("不合法的名字，请重新输入\n");
                }
                for (int i = 0; new_name[i] != '\0'; i++)
                {
                    if (new_name[i] >= '0' && new_name[i] <= '9')
                    {
                        printf("不合法的名字，请重新输入\n");
                        valid = false;
                        break;
                    }
                }
                if (valid)
                    break;
            } while (1);

            snprintf(sql, sizeof(sql),
                     "UPDATE users SET name = '%s' WHERE user_id = '%s';",
                     new_name, modify_user_id);

            if (execute_update(db, sql))
                printf("用户姓名更新成功！\n");
            else
                printf("更新失败。\n");
            break;

        case 3:
            do
            {
                printf("请输入新电话号码: ");
                fgets(new_phone, sizeof(new_phone), stdin);
                trim_newline(new_phone);

                bool valid = true;
                if (strlen(new_phone) != 11)
                {
                    valid = false;
                    printf("不合法的电话号码，请重新输入\n");
                }
                else
                {
                    for (int i = 0; new_phone[i] != '\0'; i++)
                    {
                        if (new_phone[i] < '0' || new_phone[i] > '9')
                        {
                            valid = false;
                            printf("不合法的电话号码，请重新输入\n");
                            break;
                        }
                    }
                }
                if (valid)
                    break;
            } while (1);

            snprintf(sql, sizeof(sql),
                     "UPDATE users SET phone_number = '%s' WHERE user_id = '%s';",
                     new_phone, modify_user_id);

            if (execute_update(db, sql))
                printf("用户电话更新成功！\n");
            else
                printf("更新失败。\n");
            break;

        case 4:
            do
            {
                printf("请输入新邮箱: ");
                fgets(new_email, sizeof(new_email), stdin);
                trim_newline(new_email);

                bool valid = true;
                if (strlen(new_email) < 6)
                {
                    printf("电子邮箱长度不能少于6个字符，请重新输入\n");
                    valid = false;
                    continue;
                }
                if (strchr(new_email, '@') == NULL)
                {
                    printf("不合法的邮箱地址，请重新输入\n");
                    valid = false;
                    continue;
                }
                bool has_digit = false;
                for (int i = 0; new_email[i] != '\0'; i++)
                {
                    if (new_email[i] >= '0' && new_email[i] <= '9')
                    {
                        has_digit = true;
                        break;
                    }
                }
                if (!has_digit)
                {
                    printf("电子邮箱地址必须包含至少一个数字，请重新输入\n");
                    valid = false;
                    continue;
                }
                if (valid)
                    break;
            } while (1);

            snprintf(sql, sizeof(sql),
                     "UPDATE users SET email = '%s' WHERE user_id = '%s';",
                     new_email, modify_user_id);

            if (execute_update(db, sql))
                printf("用户邮箱更新成功！\n");
            else
                printf("更新失败。\n");
            break;

        default:
            printf("无效选项，操作取消。\n");
        }
        break;
    }
        // ...existing code...

    case 4: // 查看所有用户
    {
        snprintf(sql, sizeof(sql),
                 "SELECT username, name, phone_number, email, "
                 "CASE role_id "
                 "   WHEN 'role_admin' THEN '管理员' "
                 "   WHEN 'role_staff' THEN '物业人员' "
                 "   WHEN 'role_owner' THEN '业主' "
                 "   ELSE '未知' END as role_name, "
                 "CASE status "
                 "   WHEN '1' THEN '正常' "
                 "   WHEN '0' THEN '禁用' "
                 "   ELSE '未知' END as status_name, "
                 "registration_date "
                 "FROM users;");

        if (execute_query(db, sql, &result))
        {
            printf("\n=== 用户列表 ===\n");
            printf("%-15s %-15s %-15s %-25s %-12s %-10s %-20s\n",
                   "用户名", "姓名", "电话", "邮箱", "角色", "状态", "注册日期");
            printf("----------------------------------------------------------------------------------------------------------\n");

            for (int i = 0; i < result.row_count; i++)
            {
                char formatted_date[30] = {0};
                time_t timestamp;

                // 尝试转换日期字符串为时间戳
                if (result.rows[i].values[6] && strlen(result.rows[i].values[6]) > 0)
                {
                    if (strstr(result.rows[i].values[6], "-")) // ISO格式日期
                    {
                        struct tm tm = {0};
                        if (strptime(result.rows[i].values[6], "%Y-%m-%d %H:%M:%S", &tm) != NULL)
                        {
                            timestamp = mktime(&tm);
                            strftime(formatted_date, sizeof(formatted_date),
                                     "%Y-%m-%d %H:%M:%S", localtime(&timestamp));
                        }
                        else
                        {
                            strncpy(formatted_date, "格式错误", sizeof(formatted_date) - 1);
                        }
                    }
                    else // UNIX时间戳
                    {
                        timestamp = (time_t)atoll(result.rows[i].values[6]);
                        if (timestamp > 0)
                        {
                            strftime(formatted_date, sizeof(formatted_date),
                                     "%Y-%m-%d %H:%M:%S", localtime(&timestamp));
                        }
                        else
                        {
                            strncpy(formatted_date, "无效时间戳", sizeof(formatted_date) - 1);
                        }
                    }
                }
                else
                {
                    strncpy(formatted_date, "未设置", sizeof(formatted_date) - 1);
                }

                printf("%-15s %-15s %-15s %-25s %-12s %-10s %-20s\n",
                       result.rows[i].values[0], // username
                       result.rows[i].values[1], // name
                       result.rows[i].values[2], // phone_number
                       result.rows[i].values[3], // email
                       result.rows[i].values[4], // role_name
                       result.rows[i].values[5], // status_name
                       formatted_date);          // formatted registration_date
            }
            printf("----------------------------------------------------------------------------------------------------------\n");
            printf("共 %d 条记录\n", result.row_count);

            free_query_result(&result);
        }
        else
        {
            printf("查询失败。\n");
        }
        break;
    }

        // ...existing code...
    case 5:
        return;
    default:
        printf("无效选项。\n");
    }

    pause_console();
    manage_users(db, user_id, user_type);
}

/**
 * @brief 显示服务分配界面
 *
 * 提供分配服务人员到楼宇、取消分配以及查看分配情况的功能
 *
 * @param db 数据库连接指针
 * @param user_id 当前登录用户的ID
 * @param user_type 当前登录用户的类型
 */
void show_service_assignment_screen(Database *db, const char *user_id, UserType user_type)
{
    while (1)
    {
        clear_screen();
        printf("\n=== 服务分配界面 ===\n");
        printf("1. 分配服务人员到楼宇\n");
        printf("2. 取消服务人员分配\n");
        printf("3. 查看服务人员分配情况\n");
        printf("4. 返回主菜单\n");
        printf("请输入选项: ");

        int choice;
        scanf("%d", &choice);
        getchar();

        switch (choice)
        {
        case 1:
        {
            char building_name[100];
            char staff_name[100];

            printf("\n请输入楼宇名称: ");
            fgets(building_name, sizeof(building_name), stdin);
            trim_newline(building_name);

            QueryResult result;
            char sql[512];
            snprintf(sql, sizeof(sql),
                     "SELECT building_id FROM buildings WHERE building_name = '%s'", building_name);

            if (execute_query(db, sql, &result))
            {
                if (result.row_count == 0)
                {
                    printf("未找到该楼宇名称。\n");
                    free_query_result(&result);
                    break;
                }

                char building_id[41];
                strcpy(building_id, result.rows[0].values[0]);
                free_query_result(&result);

                printf("\n请输入服务人员姓名或用户名: ");
                fgets(staff_name, sizeof(staff_name), stdin);
                trim_newline(staff_name);

                snprintf(sql, sizeof(sql),
                         "SELECT user_id FROM users WHERE (name = '%s' OR username = '%s') AND role_id = 'role_staff'",
                         staff_name, staff_name);

                if (execute_query(db, sql, &result))
                {
                    if (result.row_count == 0)
                    {
                        printf("未找到该服务人员。\n");
                        free_query_result(&result);
                        break;
                    }

                    char staff_user_id[41];
                    strcpy(staff_user_id, result.rows[0].values[0]);
                    free_query_result(&result);

                    if (assign_staff_to_building(db, user_id, user_type, staff_user_id, building_id))
                    {
                        printf("服务人员分配成功！\n");
                    }
                    else
                    {
                        printf("服务人员分配失败。\n");
                    }
                }
                else
                {
                    printf("查询失败。\n");
                }
            }
            else
            {
                printf("查询失败。\n");
            }

            break;
        }
        case 2:
        {
            char building_name[100];
            char staff_username[100];

            printf("\n请输入楼宇名称: ");
            fgets(building_name, sizeof(building_name), stdin);
            trim_newline(building_name);

            QueryResult result;
            char sql[512];
            snprintf(sql, sizeof(sql),
                     "SELECT building_id FROM buildings WHERE building_name = '%s'", building_name);

            if (execute_query(db, sql, &result))
            {
                if (result.row_count == 0)
                {
                    printf("未找到该楼宇名称。\n");
                    free_query_result(&result);
                    break;
                }

                char building_id[41];
                strcpy(building_id, result.rows[0].values[0]);
                free_query_result(&result);

                printf("\n请输入服务人员用户名: ");
                fgets(staff_username, sizeof(staff_username), stdin);
                trim_newline(staff_username);

                snprintf(sql, sizeof(sql),
                         "SELECT sa.staff_id "
                         "FROM service_areas sa "
                         "JOIN staff s ON sa.staff_id = s.staff_id "
                         "JOIN users u ON s.user_id = u.user_id "
                         "WHERE u.username = '%s' AND sa.building_id = '%s'",
                         staff_username, building_id);

                if (execute_query(db, sql, &result))
                {
                    if (result.row_count == 0)
                    {
                        printf("该楼宇没有分配该服务人员。\n");
                        free_query_result(&result);
                        break;
                    }

                    char staff_id[41];
                    strcpy(staff_id, result.rows[0].values[0]);
                    free_query_result(&result);

                    if (unassign_staff_from_building(db, user_id, user_type, staff_id, building_id))
                    {
                        printf("取消分配成功！\n");
                    }
                    else
                    {
                        printf("取消分配失败。\n");
                    }
                }
                else
                {
                    printf("查询失败。\n");
                }
            }
            else
            {
                printf("查询失败。\n");
            }

            break;
        }

        case 3:
        {
            QueryResult result;
            char sql[512];

            snprintf(sql, sizeof(sql),
                     "SELECT u.name as staff_name, u.username as staff_username, b.building_name "
                     "FROM service_areas sa "
                     "JOIN staff s ON sa.staff_id = s.staff_id "
                     "JOIN users u ON s.user_id = u.user_id "
                     "JOIN buildings b ON sa.building_id = b.building_id "
                     "ORDER BY b.building_name, u.name");

            if (execute_query(db, sql, &result))
            {
                printf("\n=== 服务人员分配情况 ===\n");
                printf("%-20s %-20s %-20s\n",
                       "服务人员姓名", "服务人员用户名", "楼宇名称");
                printf("------------------------------------------------------------\n");

                for (int i = 0; i < result.row_count; i++)
                {
                    printf("%-20s %-20s %-20s\n",
                           result.rows[i].values[0],
                           result.rows[i].values[1],
                           result.rows[i].values[2]);
                }
                printf("------------------------------------------------------------\n");
                printf("共 %d 条记录\n", result.row_count);

                free_query_result(&result);
            }
            else
            {
                printf("查询失败。\n");
            }
            break;
        }

        case 4:
            return;

        default:
            printf("无效选项，请重新输入。\n");
        }

        printf("\n按Enter键继续...");
        getchar();
    }
}

/**
 * @brief 显示信息查询界面
 *
 * 提供业主信息和服务人员信息的查询功能
 *
 * @param db 数据库连接指针
 * @param user_id 当前登录用户的ID
 * @param user_type 当前登录用户的类型
 */
void show_info_query_screen(Database *db, const char *user_id, UserType user_type)
{
    int choice;
    char sql[1024];

    do
    {
        clear_screen();
        printf("\n=== 信息查询 ===\n");
        printf("1. 业主信息查询\n");
        printf("2. 服务人员信息查询\n");
        printf("0. 返回主菜单\n");
        printf("\n请选择: ");

        scanf("%d", &choice);
        getchar();

        QueryResult result;

        switch (choice)
        {
        case 1: // 业主信息查询
        {
            char owner_name[100];
            printf("\n请输入业主姓名(支持模糊查询): ");
            fgets(owner_name, sizeof(owner_name), stdin);
            trim_newline(owner_name);

            snprintf(sql, sizeof(sql),
                     "SELECT u.name, u.phone_number, u.email, "
                     "b.building_name, r.room_number, r.floor "
                     "FROM users u "
                     "LEFT JOIN rooms r ON u.user_id = r.owner_id "
                     "LEFT JOIN buildings b ON r.building_id = b.building_id "
                     "WHERE u.role_id = 'role_owner' AND u.name LIKE '%%%s%%' "
                     "ORDER BY u.name",
                     owner_name);

            if (execute_query(db, sql, &result))
            {
                printf("\n=== 业主信息查询结果 ===\n");
                printf("%-15s %-15s %-25s %-15s %-10s %-6s\n",
                       "姓名", "电话", "邮箱", "所在楼宇", "房间号", "楼层");
                printf("--------------------------------------------------------------------------------\n");

                for (int i = 0; i < result.row_count; i++)
                {
                    printf("%-15s %-15s %-25s %-15s %-10s %-6s\n",
                           result.rows[i].values[0],                                       // 姓名
                           result.rows[i].values[1] ? result.rows[i].values[1] : "未登记", // 电话
                           result.rows[i].values[2] ? result.rows[i].values[2] : "未登记", // 邮箱
                           result.rows[i].values[3] ? result.rows[i].values[3] : "未分配", // 楼宇
                           result.rows[i].values[4] ? result.rows[i].values[4] : "--",     // 房间号
                           result.rows[i].values[5] ? result.rows[i].values[5] : "--");    // 楼层
                }
                printf("--------------------------------------------------------------------------------\n");
                printf("共 %d 条记录\n", result.row_count);
                free_query_result(&result);
            }
            break;
        }

        case 2: // 服务人员信息查询
        {
            char staff_name[100];
            printf("\n请输入服务人员姓名(支持模糊查询): ");
            fgets(staff_name, sizeof(staff_name), stdin);
            trim_newline(staff_name);

            snprintf(sql, sizeof(sql),
                     "SELECT u.name, st.type_name, u.phone_number, "
                     "COUNT(DISTINCT sa.building_id) as building_count, "
                     "GROUP_CONCAT(b.building_name) as buildings "
                     "FROM users u "
                     "JOIN staff s ON u.user_id = s.user_id "
                     "JOIN staff_types st ON s.staff_type_id = st.staff_type_id "
                     "LEFT JOIN service_areas sa ON s.staff_id = sa.staff_id "
                     "LEFT JOIN buildings b ON sa.building_id = b.building_id "
                     "WHERE u.role_id = 'role_staff' AND u.name LIKE '%%%s%%' "
                     "GROUP BY u.user_id "
                     "ORDER BY u.name",
                     staff_name);

            if (execute_query(db, sql, &result))
            {
                printf("\n=== 服务人员查询结果 ===\n");
                printf("%-15s %-15s %-15s %-12s %-30s\n",
                       "姓名", "服务类型", "联系电话", "负责楼宇数", "负责楼宇");
                printf("--------------------------------------------------------------------------------\n");

                for (int i = 0; i < result.row_count; i++)
                {
                    printf("%-15s %-15s %-15s %-12s %-30s\n",
                           result.rows[i].values[0],                                    // 姓名
                           result.rows[i].values[1],                                    // 服务类型
                           result.rows[i].values[2],                                    // 电话
                           result.rows[i].values[3],                                    // 楼宇数量
                           result.rows[i].values[4] ? result.rows[i].values[4] : "无"); // 楼宇列表
                }
                printf("--------------------------------------------------------------------------------\n");
                printf("共 %d 条记录\n", result.row_count);
                free_query_result(&result);
            }
            break;
        }

        case 0:
            return;

        default:
            printf("\n无效的选择，请重新输入!\n");
        }

        printf("\n按Enter键继续...");
        getchar();

    } while (1);
}

/**
 * @brief 显示信息排序界面
 *
 * 提供按缴费时间、缴费金额和业主姓名排序的功能
 *
 * @param db 数据库连接指针
 * @param user_id 当前登录用户的ID
 * @param user_type 当前登录用户的类型
 */
void show_info_sort_screen(Database *db, const char *user_id, UserType user_type)
{
    int choice;
    char sql[1024];

    do
    {
        clear_screen();
        printf("\n=== 信息排序 ===\n");
        printf("1. 按姓名排序查看业主信息\n");
        printf("2. 按负责楼宇数量排序查看服务人员\n");
        printf("0. 返回主菜单\n");
        printf("\n请选择: ");

        scanf("%d", &choice);
        getchar();

        QueryResult result;

        switch (choice)
        {
        case 1: // 按姓名排序查看业主信息
        {
            snprintf(sql, sizeof(sql),
                     "SELECT u.name, u.phone_number, u.email, "
                     "b.building_name, r.room_number, r.floor "
                     "FROM users u "
                     "LEFT JOIN rooms r ON u.user_id = r.owner_id "
                     "LEFT JOIN buildings b ON r.building_id = b.building_id "
                     "WHERE u.role_id = 'role_owner' "
                     "ORDER BY u.name ASC");

            if (execute_query(db, sql, &result))
            {
                printf("\n=== 业主信息(按姓名排序) ===\n");
                printf("%-15s %-15s %-25s %-15s %-10s %-6s\n",
                       "姓名", "电话", "邮箱", "所在楼宇", "房间号", "楼层");
                printf("--------------------------------------------------------------------------------\n");

                for (int i = 0; i < result.row_count; i++)
                {
                    printf("%-15s %-15s %-25s %-15s %-10s %-6s\n",
                           result.rows[i].values[0],                                       // 姓名
                           result.rows[i].values[1] ? result.rows[i].values[1] : "未登记", // 电话
                           result.rows[i].values[2] ? result.rows[i].values[2] : "未登记", // 邮箱
                           result.rows[i].values[3] ? result.rows[i].values[3] : "未分配", // 楼宇
                           result.rows[i].values[4] ? result.rows[i].values[4] : "--",     // 房间号
                           result.rows[i].values[5] ? result.rows[i].values[5] : "--");    // 楼层
                }
                printf("--------------------------------------------------------------------------------\n");
                printf("共 %d 条记录\n", result.row_count);
                free_query_result(&result);

                // 提供保存到文件选项
                printf("\n是否要保存排序结果到文件? (y/n): ");
                char save_choice;
                scanf(" %c", &save_choice);
                getchar();

                if (save_choice == 'y' || save_choice == 'Y')
                {
                    char filename[100];
                    printf("请输入文件名: ");
                    fgets(filename, sizeof(filename), stdin);
                    trim_newline(filename);

                    // 创建链表存储业主信息
                    OwnerNode *head = NULL;
                    OwnerNode *current = NULL;

                    // 将查询结果转换为链表
                    for (int i = 0; i < result.row_count; i++)
                    {
                        OwnerNode *node = create_owner_node_by_admin(
                            result.rows[i].values[0], // 姓名
                            result.rows[i].values[1], // 电话
                            result.rows[i].values[2], // 邮箱
                            result.rows[i].values[3], // 楼宇
                            result.rows[i].values[4], // 房间号
                            result.rows[i].values[5]  // 楼层
                        );

                        if (head == NULL)
                        {
                            head = node;
                            current = node;
                        }
                        else
                        {
                            current->next = node;
                            current = node;
                        }
                    }

                    // 保存到文件
                    save_owners_to_file(head, filename);

                    // 释放链表
                    while (head != NULL)
                    {
                        OwnerNode *temp = head;
                        head = head->next;
                        free(temp);
                    }
                }
            }
            break;
        }

        case 2: // 按负责楼宇数量排序查看服务人员
        {
            snprintf(sql, sizeof(sql),
                     "SELECT u.name, st.type_name, "
                     "COUNT(DISTINCT sa.building_id) as building_count, "
                     "GROUP_CONCAT(b.building_name) as buildings "
                     "FROM users u "
                     "JOIN staff s ON u.user_id = s.user_id "
                     "JOIN staff_types st ON s.staff_type_id = st.staff_type_id "
                     "LEFT JOIN service_areas sa ON s.staff_id = sa.staff_id "
                     "LEFT JOIN buildings b ON sa.building_id = b.building_id "
                     "WHERE u.role_id = 'role_staff' "
                     "GROUP BY u.user_id "
                     "ORDER BY building_count DESC");

            if (execute_query(db, sql, &result))
            {
                printf("\n=== 服务人员(按负责楼宇数量排序) ===\n");
                printf("%-15s %-15s %-12s %-40s\n",
                       "姓名", "服务类型", "负责楼宇数", "负责楼宇");
                printf("--------------------------------------------------------------------------------\n");

                // 提供保存到文件选项
                printf("\n是否要保存排序结果到文件? (y/n): ");
                char save_choice;
                scanf(" %c", &save_choice);
                getchar();

                if (save_choice == 'y' || save_choice == 'Y')
                {
                    char filename[100];
                    printf("请输入文件名: ");
                    fgets(filename, sizeof(filename), stdin);
                    trim_newline(filename);

                    // 创建链表存储结果
                    StaffNode *head = NULL;
                    StaffNode *current = NULL;

                    // 将查询结果转换为链表
                    for (int i = 0; i < result.row_count; i++)
                    {
                        StaffNode *node = create_staff_node(
                            result.rows[i].values[0],                                  // 姓名
                            result.rows[i].values[1],                                  // 服务类型
                            atoi(result.rows[i].values[2]),                            // 楼宇数量
                            result.rows[i].values[3] ? result.rows[i].values[3] : "无" // 楼宇列表
                        );

                        if (head == NULL)
                        {
                            head = node;
                            current = node;
                        }
                        else
                        {
                            current->next = node;
                            current = node;
                        }
                    }

                    // 保存到文件
                    save_staff_to_file(head, filename);

                    // 释放链表
                    while (head != NULL)
                    {
                        StaffNode *temp = head;
                        head = head->next;
                        free(temp);
                    }
                }

                // 显示结果
                for (int i = 0; i < result.row_count; i++)
                {
                    printf("%-15s %-15s %-12s %-40s\n",
                           result.rows[i].values[0],                                    // 姓名
                           result.rows[i].values[1],                                    // 服务类型
                           result.rows[i].values[2],                                    // 楼宇数量
                           result.rows[i].values[3] ? result.rows[i].values[3] : "无"); // 楼宇列表
                }
                printf("--------------------------------------------------------------------------------\n");
                printf("共 %d 条记录\n", result.row_count);
                free_query_result(&result);
            }
            break;
        }

        case 0:
            return;

        default:
            printf("\n无效的选择，请重新输入!\n");
        }

        printf("\n按Enter键继续...");
        getchar();

    } while (1);
}

/**
 * @brief 显示信息统计界面
 *
 * 提供楼宇统计、住户统计、费用统计和服务统计功能
 *
 * @param db 数据库连接指针
 * @param user_id 当前登录用户的ID
 * @param user_type 当前登录用户的类型
 */
void show_info_statistics_screen(Database *db, const char *user_id, UserType user_type)
{
    while (1)
    {
        clear_screen();
        printf("\n=== 信息统计界面 ===\n");
        printf("1. 楼宇统计\n");
        printf("2. 住户统计\n");
        printf("0. 返回主菜单\n");
        printf("请输入选项: ");

        int choice;
        scanf("%d", &choice);
        getchar();

        QueryResult result;
        char sql[1024];

        switch (choice)
        {
        case 1:
        {
            snprintf(sql, sizeof(sql),
                     "SELECT COUNT(*) as total_buildings, "
                     "SUM(floors_count) as total_floors "
                     "FROM buildings");

            if (execute_query(db, sql, &result))
            {
                printf("\n楼宇统计信息：\n");
                printf("------------------------\n");
                printf("总楼宇数：%s\n", result.rows[0].values[0]);
                printf("总楼层数：%s\n", result.rows[0].values[1]);
                free_query_result(&result);

                snprintf(sql, sizeof(sql),
                         "SELECT b.building_name, COUNT(r.room_id) as room_count "
                         "FROM buildings b "
                         "LEFT JOIN rooms r ON b.building_id = r.building_id "
                         "GROUP BY b.building_id "
                         "ORDER BY b.building_name");

                if (execute_query(db, sql, &result))
                {
                    printf("\n各楼宇房间统计：\n");
                    printf("------------------------\n");
                    printf("%-20s %-10s\n", "楼宇名称", "房间数");
                    for (int i = 0; i < result.row_count; i++)
                    {
                        printf("%-20s %-10s\n",
                               result.rows[i].values[0],
                               result.rows[i].values[1]);
                    }
                    free_query_result(&result);
                }
            }
            break;
        }

        case 2:
        {
            snprintf(sql, sizeof(sql),
                     "SELECT COUNT(*) as total_owners, "
                     "COUNT(DISTINCT building_id) as buildings_with_owners "
                     "FROM users u "
                     "LEFT JOIN rooms r ON u.user_id = r.owner_id "
                     "WHERE u.role_id = 3");

            if (execute_query(db, sql, &result))
            {
                printf("\n住户统计信息：\n");
                printf("------------------------\n");
                printf("总业主数：%s\n", result.rows[0].values[0]);
                printf("入住楼宇数：%s\n", result.rows[0].values[1]);
                free_query_result(&result);

                snprintf(sql, sizeof(sql),
                         "SELECT b.building_name, "
                         "COUNT(r.room_id) as total_rooms, "
                         "COUNT(r.owner_id) as occupied_rooms, "
                         "ROUND(CAST(COUNT(r.owner_id) AS FLOAT) / COUNT(r.room_id) * 100, 2) as occupancy_rate "
                         "FROM buildings b "
                         "LEFT JOIN rooms r ON b.building_id = r.building_id "
                         "GROUP BY b.building_id "
                         "ORDER BY b.building_name");

                if (execute_query(db, sql, &result))
                {
                    printf("\n楼宇入住率统计：\n");
                    printf("------------------------\n");
                    printf("%-20s %-10s %-10s %-10s\n",
                           "楼宇名称", "总房间数", "已入住", "入住率(%)");
                    for (int i = 0; i < result.row_count; i++)
                    {
                        printf("%-20s %-10s %-10s %-10s\n",
                               result.rows[i].values[0],
                               result.rows[i].values[1],
                               result.rows[i].values[2],
                               result.rows[i].values[3]);
                    }
                    free_query_result(&result);
                }
            }
            break;
        }
        case 0:
            return;

        default:
            printf("无效选项，请重新输入\n");
        }

        printf("\n按Enter键继续...");
        getchar();
    }
}

/**
 * @brief 显示系统维护界面
 *
 * 提供数据库备份、恢复和重置等系统维护功能
 *
 * @param db 数据库连接指针
 * @param user_id 当前登录用户的ID
 * @param user_type 当前登录用户的类型
 */
void show_system_maintenance_screen(Database *db, const char *user_id, UserType user_type)
{
    // 检查用户权限
    if (user_type != USER_ADMIN) {
        printf("权限不足，无法访问系统维护功能\n");
        pause_console();
        return;
    }

    while (1) {
        clear_screen();
        printf("\n=== 系统维护界面 ===\n");
        printf("1. 数据库备份\n");
        printf("2. 数据库恢复\n");
        printf("0. 返回主菜单\n");
        printf("\n请输入选项: ");

        int choice;
        scanf("%d", &choice);
        getchar(); // 清除换行符

        char filepath[256];
        char confirm;

        switch (choice) {
            case 1: // 数据库备份
            {
                printf("\n=== 数据库备份 ===\n");
                printf("请输入备份文件保存路径 (默认为 backup.db): ");
                fgets(filepath, sizeof(filepath), stdin);
                trim_newline(filepath);
                
                if (strlen(filepath) == 0) {
                    strcpy(filepath, "backup.db");
                }

                // 创建备份文件
                sqlite3 *backup_db;
                sqlite3_backup *backup;
                
                if (sqlite3_open(filepath, &backup_db) != SQLITE_OK) {
                    printf("错误：无法创建备份文件\n");
                    break;
                }

                backup = sqlite3_backup_init(backup_db, "main", db->db, "main");
                
                if (backup) {
                    if (sqlite3_backup_step(backup, -1) == SQLITE_DONE) {
                        printf("数据库备份成功！备份文件：%s\n", filepath);
                    } else {
                        printf("数据库备份失败\n");
                    }
                    sqlite3_backup_finish(backup);
                } else {
                    printf("备份初始化失败\n");
                }
                
                sqlite3_close(backup_db);
                break;
            }

            case 2: // 数据库恢复
{
    printf("\n=== 数据库恢复 ===\n");
    printf("请输入备份文件路径: ");
    fgets(filepath, sizeof(filepath), stdin);
    trim_newline(filepath);
    
    // 去除路径中的引号
    char *start = filepath;
    char *end = filepath + strlen(filepath) - 1;
    
    // 去除开头的引号
    if (*start == '\'' || *start == '"') {
        start++;
    }
    
    // 去除结尾的引号
    if (*end == '\'' || *end == '"') {
        *end = '\0';
    }
    
    // 复制处理后的路径
    char clean_path[256];
    strncpy(clean_path, start, sizeof(clean_path)-1);
    clean_path[sizeof(clean_path)-1] = '\0';

    // 检查文件是否存在
    if (access(clean_path, F_OK) == -1) {
        printf("错误：备份文件不存在\n");
        printf("尝试访问的路径：%s\n", clean_path);
        break;
    }

    printf("\n警告：恢复操作将覆盖当前所有数据！\n");
    printf("确定要继续吗？(y/n): ");
    scanf(" %c", &confirm);
    getchar();

    if (confirm == 'y' || confirm == 'Y') {
        sqlite3 *backup_db;
        sqlite3_backup *backup;

        // 尝试打开备份文件
        if (sqlite3_open(clean_path, &backup_db) != SQLITE_OK) {
            printf("错误：无法打开备份文件：%s\n", sqlite3_errmsg(backup_db));
            sqlite3_close(backup_db);
            break;
        }

        // 初始化备份
        backup = sqlite3_backup_init(db->db, "main", backup_db, "main");
        
        if (backup) {
            // 执行恢复
            int rc = sqlite3_backup_step(backup, -1);
            sqlite3_backup_finish(backup);
            
            if (rc == SQLITE_DONE) {
                printf("数据库恢复成功！\n");
            } else {
                printf("数据库恢复失败：%s\n", sqlite3_errmsg(db->db));
            }
        } else {
            printf("恢复初始化失败：%s\n", sqlite3_errmsg(db->db));
        }
        
        // 关闭备份数据库连接
        sqlite3_close(backup_db);
    }
    break;
}


            case 0: // 返回主菜单
                return;

            default:
                printf("无效选项，请重新输入\n");
        }

        printf("\n按Enter键继续...");
        getchar();
    }
}

/**
 * @brief 显示费用标准管理界面
 *
 * 提供查看和修改各类费用标准的功能
 *
 * @param db 数据库连接指针
 * @param user_id 当前登录用户的ID
 * @param user_type 当前登录用户的类型
 */
void manage_fee_standards_screen(Database *db, const char *user_id, UserType user_type)
{
    if (user_type != USER_ADMIN)
    {
        printf("权限不足，无法访问此功能\n");
        return;
    }

    int choice;
    while (1)
    {
        system("clear||cls");
        printf("\n===== 费用标准管理 =====\n");
        printf("1. 查看现有费用标准\n");
        printf("2. 修改费用标准\n");
        printf("3. 生成物业费账单\n");
        printf("0. 返回上一级\n");
        printf("请输入您的选择: ");
        scanf("%d", &choice);
        getchar();

        switch (choice)
        {
        case 1:
        {
            printf("\n===== 现有费用标准 =====\n");

            char query[512];
            QueryResult result;

            snprintf(query, sizeof(query),
                     "SELECT standard_id, fee_type, price_per_unit, unit, effective_date, end_date "
                     "FROM fee_standards "
                     "ORDER BY fee_type, effective_date DESC");

            if (!execute_query(db, query, &result))
            {
                printf("查询费用标准失败\n");
                printf("按任意键返回...");
                getchar();
                break;
            }

            if (result.row_count == 0)
            {
                printf("暂无费用标准记录\n");
                free_query_result(&result);
                printf("按任意键返回...");
                getchar();
                break;
            }

            printf("%-12s %-15s %-12s %-10s %-15s %-15s\n",
                   "标准ID", "费用类型", "单价", "单位", "生效日期", "终止日期");

            char fee_type_str[20];
            struct tm tm_info;
            char start_date_str[20], end_date_str[20];

            for (int i = 0; i < result.row_count; i++)
            {
                int fee_type = atoi(result.rows[i].values[1]);
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

                time_t effective_date = (time_t)atol(result.rows[i].values[4]);
                localtime_r(&effective_date, &tm_info);
                strftime(start_date_str, sizeof(start_date_str), "%Y-%m-%d", &tm_info);

                time_t end_date = (time_t)atol(result.rows[i].values[5]);
                if (end_date == 0)
                {
                    strcpy(end_date_str, "无限期");
                }
                else
                {
                    localtime_r(&end_date, &tm_info);
                    strftime(end_date_str, sizeof(end_date_str), "%Y-%m-%d", &tm_info);
                }

                printf("%-12s %-15s %-12.2f %-10s %-15s %-15s\n",
                       result.rows[i].values[0],
                       fee_type_str,
                       atof(result.rows[i].values[2]),
                       result.rows[i].values[3],
                       start_date_str,
                       end_date_str);
            }

            free_query_result(&result);
            printf("\n按任意键返回...");
            getchar();
        }
        break;

        case 2:
        {
            printf("\n===== 修改费用标准 =====\n");

            int fee_type_choice;
            printf("请选择要修改的费用类型:\n");
            printf("1. 物业费\n");
            printf("2. 停车费\n");
            printf("3. 水费\n");
            printf("4. 电费\n");
            printf("5. 燃气费\n");
            printf("请输入(1-5): ");
            scanf("%d", &fee_type_choice);
            getchar();

            FeeStandard standard;
            memset(&standard, 0, sizeof(FeeStandard));

            switch (fee_type_choice)
            {
            case 1:
                standard.fee_type = TRANS_PROPERTY_FEE;
                break;
            case 2:
                standard.fee_type = TRANS_PARKING_FEE;
                break;
            case 3:
                standard.fee_type = TRANS_WATER_FEE;
                break;
            case 4:
                standard.fee_type = TRANS_ELECTRICITY_FEE;
                break;
            case 5:
                standard.fee_type = TRANS_GAS_FEE;
                break;
            default:
                printf("无效选择\n");
                printf("按任意键返回...");
                getchar();
                continue;
            }

            char query[512];
            QueryResult result;

            snprintf(query, sizeof(query),
                     "SELECT standard_id, fee_type, price_per_unit, unit "
                     "FROM fee_standards WHERE fee_type = %d",
                     standard.fee_type);

            if (!execute_query(db, query, &result) || result.row_count == 0)
            {
                printf("未找到该费用类型的费用标准\n");
                printf("按任意键返回...");
                getchar();
                break;
            }

            strcpy(standard.standard_id, result.rows[0].values[0]);
            standard.price_per_unit = atof(result.rows[0].values[2]);
            strcpy(standard.unit, result.rows[0].values[3]);

            printf("当前费用类型: ");
            switch (standard.fee_type)
            {
            case TRANS_PROPERTY_FEE:
                printf("物业费\n");
                break;
            case TRANS_PARKING_FEE:
                printf("停车费\n");
                break;
            case TRANS_WATER_FEE:
                printf("水费\n");
                break;
            case TRANS_ELECTRICITY_FEE:
                printf("电费\n");
                break;
            case TRANS_GAS_FEE:
                printf("燃气费\n");
                break;
            default:
                printf("其他费用\n");
                break;
            }

            printf("当前单价: %.2f %s\n", standard.price_per_unit, standard.unit);
            printf("请输入新的单价: ");
            scanf("%f", &standard.price_per_unit);
            getchar();

            snprintf(query, sizeof(query),
                     "UPDATE fee_standards SET price_per_unit = %.2f, unit = '%s' WHERE standard_id = '%s'",
                     standard.price_per_unit, standard.unit, standard.standard_id);

            if (execute_update(db, query))
            {
                printf("费用标准更新成功\n");
            }
            else
            {
                printf("费用标准更新失败\n");
            }

            printf("按任意键返回...");
            getchar();
        }
        break;
        case 3:
            generate_property_fees(db, user_id, user_type);
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

/**
 * @brief 更新费用标准
 *
 * 更新系统中的费用标准信息
 *
 * @param db 数据库连接指针
 * @param user_id 当前登录用户的ID
 * @param user_type 当前登录用户的类型
 * @param standard 要更新的费用标准信息指针
 * @return 更新成功返回true，否则返回false
 */
bool update_fee_standard(Database *db, const char *user_id, UserType user_type, FeeStandard *standard)
{
    char query[1024];

    snprintf(query, sizeof(query),
             "UPDATE fee_standards SET fee_type = %d, price_per_unit = %.2f, unit = '%s', "
             "effective_date = %ld, end_date = %ld WHERE standard_id = '%s'",
             standard->fee_type, standard->price_per_unit, standard->unit,
             standard->effective_date, standard->end_date, standard->standard_id);

    if (execute_update(db, query))
    {
        return true;
    }
    return false;
}

/**
 * @brief 显示房屋分配管理界面
 *
 * 提供房屋分配给业主的功能，显示待分配的业主和可用房屋，
 * 并允许管理员进行房屋分配操作
 *
 * @param db 数据库连接指针
 * @param admin_id 当前登录管理员的ID
 */
void show_room_assignment_screen(Database *db, const char *admin_id)
{
    clear_screen();
    printf("\n=== 房屋分配管理 ===\n");

    const char *unassigned_query =
        "SELECT u.user_id, u.name, u.phone_number "
        "FROM users u "
        "LEFT JOIN rooms r ON u.user_id = r.owner_id "
        "WHERE u.role_id = 'role_owner' AND r.room_id IS NULL "
        "ORDER BY u.user_id";

    printf("\n待分配房屋的业主:\n");
    printf("%-8s %-12s %-15s\n", "用户ID", "姓名", "电话");
    printf("--------------------------------\n");

    sqlite3_stmt *stmt;
    if (sqlite3_prepare_v2(db->db, unassigned_query, -1, &stmt, NULL) == SQLITE_OK)
    {
        while (sqlite3_step(stmt) == SQLITE_ROW)
        {
            printf("%-8d %-12s %-15s\n",
                   sqlite3_column_int(stmt, 0),
                   sqlite3_column_text(stmt, 1),
                   sqlite3_column_text(stmt, 2));
        }
        sqlite3_finalize(stmt);
    }

    /**
     * @brief 查询并显示所有可用房屋信息
     *
     * 从数据库中选择未分配给业主的房屋信息（room_id IS NULL且status=0）
     * 按照楼号、楼层和房号排序显示
     */
    const char *available_query =
        "SELECT r.room_id, b.building_name, r.room_number, r.floor, r.area_sqm "
        "FROM rooms r "
        "JOIN buildings b ON r.building_id = b.building_id "
        "WHERE r.owner_id IS NULL AND r.status = 0 "
        "ORDER BY b.building_name, r.floor, r.room_number";

    printf("\n可用房屋:\n");
    printf("%-8s %-8s %-8s %-6s %-8s\n",
           "房屋ID", "楼号", "房号", "楼层", "面积");
    printf("----------------------------------------\n");

    if (sqlite3_prepare_v2(db->db, available_query, -1, &stmt, NULL) == SQLITE_OK)
    {
        while (sqlite3_step(stmt) == SQLITE_ROW)
        {
            printf("%-8s %-8s %-8s %-6d %-8.2f\n",
                   sqlite3_column_text(stmt, 0),
                   sqlite3_column_text(stmt, 1),
                   sqlite3_column_text(stmt, 2),
                   sqlite3_column_int(stmt, 3),
                   sqlite3_column_double(stmt, 4));
        }
        sqlite3_finalize(stmt);
    }

    int user_id;
    char room_id[10];
    printf("\n请输入要分配的业主ID: ");
    scanf("%d", &user_id);
    printf("请输入要分配的房屋ID: ");
    scanf("%s", room_id);

    char update_query[256];
    snprintf(update_query, sizeof(update_query),
             "UPDATE rooms SET owner_id = %d, status = 1 "
             "WHERE room_id = '%s' AND owner_id IS NULL",
             user_id, room_id);

    if (execute_update(db, update_query))
    {
        printf("\n✓ 房屋分配成功!\n");
    }
    else
    {
        printf("\n✗ 房屋分配失败,请检查输入信息\n");
    }

    printf("\n按Enter键继续...");
    getchar();
    getchar();
}
