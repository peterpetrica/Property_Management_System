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
        char building_id[41];
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
    getchar();

    switch (choice)
    {
    case 1:
    {
        char username[100], password[100], name[100], phone[20], email[100];
        char new_user_id[41];
        int role_choice;
        char role_id[20];

        generate_uuid(new_user_id);

        printf("请输入用户名: ");
        fgets(username, sizeof(username), stdin);
        trim_newline(username);

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
        getchar();

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
    case 4:
    {
        snprintf(sql, sizeof(sql), "SELECT username, name, phone_number, email, role_id, status, registration_date FROM users;");
        if (execute_query(db, sql, &result))
        {
            printf("\n=== 用户列表 ===\n");
            printf("%-15s %-15s %-15s %-25s %-12s %-10s %-20s\n",
                   "用户名", "姓名", "电话", "邮箱", "角色ID", "状态", "注册日期");
            printf("----------------------------------------------------------------------------------------------------------\n");

            for (int i = 0; i < result.row_count; i++)
            {
                char formatted_date[30] = {0};

                time_t date_time = parse_time(result.rows[i].values[6]);

                if (date_time != (time_t)-1)
                {
                    format_time(date_time, formatted_date, sizeof(formatted_date));
                }
                else
                {
                    strncpy(formatted_date, result.rows[i].values[6], sizeof(formatted_date) - 1);
                }

                printf("%-15s %-15s %-15s %-25s %-12s %-10s %-20s\n",
                       result.rows[i].values[0],
                       result.rows[i].values[1],
                       result.rows[i].values[2],
                       result.rows[i].values[3],
                       result.rows[i].values[4],
                       result.rows[i].values[5],
                       formatted_date);
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
 * 提供楼盘信息查询和业主模糊查询功能
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
        printf("========================\n");
        printf("  信息查询\n");
        printf("========================\n");
        printf("1. 查询楼盘信息\n");
        printf("2. 模糊查询业主姓名\n");
        printf("0. 返回主菜单\n");
        printf("请选择操作: ");
        scanf("%d", &choice);
        getchar();

        switch (choice)
        {
        case 1:
        {
            QueryResult result;
            char check_sql[256];
            snprintf(check_sql, sizeof(check_sql),
                     "SELECT name FROM sqlite_master "
                     "WHERE type='table' AND name='buildings';");

            if (!execute_query(db, check_sql, &result) || result.row_count == 0)
            {
                printf("查询楼盘信息失败，'buildings' 表不存在。\n");
                free_query_result(&result);
                break;
            }
            free_query_result(&result);

            snprintf(sql, sizeof(sql),
                     "SELECT building_id, building_name, address, floors_count "
                     "FROM buildings ORDER BY building_id;");

            if (execute_query(db, sql, &result))
            {
                printf("\n=== 楼盘列表 ===\n");
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
                printf("共 %d 栋楼\n", result.row_count);
                free_query_result(&result);
            }
            else
            {
                printf("查询楼盘信息失败。\n");
            }
            break;
        }
        case 2:
        {
            char name[100];
            printf("请输入业主姓名（支持模糊查询）: ");
            fgets(name, sizeof(name), stdin);
            trim_newline(name);

            QueryResult result;

            snprintf(sql, sizeof(sql),
                     "SELECT user_id, name, phone_number "
                     "FROM users "
                     "WHERE role_id = 'role_owner' "
                     "AND name LIKE '%%%s%%'",
                     name);

            if (execute_query(db, sql, &result))
            {
                if (result.row_count > 0)
                {
                    printf("\n=== 查询结果 ===\n");
                    printf("%-20s %-20s %-15s\n",
                           "用户ID", "姓名", "联系电话");
                    printf("------------------------------------------------\n");

                    for (int i = 0; i < result.row_count; i++)
                    {
                        printf("%-20s %-20s %-15s\n",
                               result.rows[i].values[0],
                               result.rows[i].values[1],
                               result.rows[i].values[2]);
                    }
                    printf("------------------------------------------------\n");
                    printf("共找到 %d 条记录\n", result.row_count);
                }
                else
                {
                    printf("\n未找到符合条件的业主。\n");
                }
                free_query_result(&result);
            }
            else
            {
                printf("查询失败。\n");
            }
            break;
        }
        case 0:
            printf("返回主菜单...\n");
            return;

        default:
            printf("无效选项，请重新选择。\n");
            break;
        }

        printf("\n按Enter键继续...");
        getchar();
    } while (1);
}

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct Node
{
    char owner_name[100];
    char payment_amount[100];
    char payment_date[100];
    struct Node *next;
} Node;

Node *create_node(const char *owner_name, const char *payment_amount, const char *payment_date)
{
    Node *new_node = (Node *)malloc(sizeof(Node));
    if (new_node)
    {
        strcpy(new_node->owner_name, owner_name);
        strcpy(new_node->payment_amount, payment_amount);
        strcpy(new_node->payment_date, payment_date);
        new_node->next = NULL;
    }
    return new_node;
}

/**
 * @brief 将排序结果保存到文件
 *
 * 将链表中的排序结果数据保存到指定文件
 *
 * @param head 链表头节点指针
 * @param filename 目标文件名
 */
void save_to_file(Node *head, const char *filename)
{
    FILE *file = fopen(filename, "w");
    if (!file)
    {
        printf("无法打开文件进行写入\n");
        return;
    }

    fprintf(file, "Owner Name               Payment Amount     Payment Date\n");
    fprintf(file, "---------------------------------------------------------\n");

    Node *current = head;
    while (current)
    {
        fprintf(file, "%-25s %-18s %-15s\n", current->owner_name, current->payment_amount, current->payment_date);
        current = current->next;
    }

    fclose(file);
    printf("数据已成功保存到 %s\n", filename);
}

/**
 * @brief 释放链表内存
 *
 * 释放链表占用的所有内存
 *
 * @param head 链表头节点指针
 */
void free_linked_list(Node *head)
{
    Node *current = head;
    while (current)
    {
        Node *temp = current;
        current = current->next;
        free(temp);
    }
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
    Node *head = NULL;
    Node *tail = NULL;

    do
    {
        clear_screen();
        printf("========================\n");
        printf("  信息排序\n");
        printf("========================\n");
        printf("1. 按缴费时间排序\n");
        printf("2. 按缴费金额排序\n");
        printf("3. 按姓名升序排序业主信息\n");
        printf("0. 返回主菜单\n");
        printf("请选择操作: ");
        scanf("%d", &choice);
        getchar();

        char sql[1024];
        QueryResult result;

        switch (choice)
        {
        case 1:
            snprintf(sql, sizeof(sql),
                     "SELECT u.name AS owner_name, "
                     "t.amount AS payment_amount, "
                     "datetime(t.payment_date, 'unixepoch') AS payment_date "
                     "FROM transactions t "
                     "JOIN users u ON t.user_id = u.user_id "
                     "ORDER BY t.payment_date ASC");
            break;

        case 2:
            snprintf(sql, sizeof(sql),
                     "SELECT u.name AS owner_name, "
                     "t.amount AS payment_amount, "
                     "datetime(t.payment_date, 'unixepoch') AS payment_date "
                     "FROM transactions t "
                     "JOIN users u ON t.user_id = u.user_id "
                     "ORDER BY t.amount ASC");
            break;

        case 3:
            snprintf(sql, sizeof(sql),
                     "SELECT u.name AS owner_name, "
                     "u.phone_number AS phone, "
                     "u.email AS email "
                     "FROM users u "
                     "WHERE u.role_id = 3 "
                     "ORDER BY u.name ASC");
            break;

        case 0:
            printf("返回主菜单...\n");
            free_linked_list(head);
            return;

        default:
            printf("无效选项，请重新选择。\n");
            continue;
        }

        if (execute_query(db, sql, &result))
        {
            printf("\n查询结果：\n");
            printf("----------------------------------------\n");

            for (int i = 0; i < result.column_count; i++)
            {
                printf("%-20s", result.column_names[i]);
            }
            printf("\n");
            printf("----------------------------------------\n");

            for (int i = 0; i < result.row_count; i++)
            {
                Node *new_node = create_node(result.rows[i].values[0], result.rows[i].values[1], result.rows[i].values[2]);
                if (tail == NULL)
                {
                    head = new_node;
                    tail = head;
                }
                else
                {
                    tail->next = new_node;
                    tail = new_node;
                }
            }

            printf("----------------------------------------\n");
            printf("共 %d 条记录\n", result.row_count);
            free_query_result(&result);

            save_to_file(head, "sorted_info.txt");
        }
        else
        {
            printf("查询失败。\n");
        }

        printf("\n按Enter键继续...");
        getchar();

    } while (choice != 0);

    free_linked_list(head);
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
        printf("3. 费用统计\n");
        printf("4. 服务统计\n");
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

        case 3:
        {
            snprintf(sql, sizeof(sql),
                     "SELECT t.fee_type, "
                     "COUNT(*) as bill_count, "
                     "SUM(t.amount) as total_amount, "
                     "AVG(t.amount) as avg_amount "
                     "FROM transactions t "
                     "GROUP BY t.fee_type");

            if (execute_query(db, sql, &result))
            {
                printf("\n费用统计信息：\n");
                printf("------------------------\n");
                printf("%-15s %-10s %-15s %-15s\n",
                       "费用类型", "账单数", "总金额", "平均金额");
                for (int i = 0; i < result.row_count; i++)
                {
                    printf("%-15s %-10s %-15s %-15s\n",
                           result.rows[i].values[0],
                           result.rows[i].values[1],
                           result.rows[i].values[2],
                           result.rows[i].values[3]);
                }
                free_query_result(&result);
            }
            break;
        }

        case 4:
        {
            snprintf(sql, sizeof(sql),
                     "SELECT u.name as staff_name, "
                     "COUNT(DISTINCT sa.building_id) as building_count "
                     "FROM users u "
                     "LEFT JOIN service_areas sa ON u.user_id = sa.staff_id "
                     "WHERE u.role_id = 2 "
                     "GROUP BY u.user_id");

            if (execute_query(db, sql, &result))
            {
                printf("\n服务人员工作量统计：\n");
                printf("------------------------\n");
                printf("%-20s %-15s\n", "服务人员", "负责楼宇数");
                for (int i = 0; i < result.row_count; i++)
                {
                    printf("%-20s %-15s\n",
                           result.rows[i].values[0],
                           result.rows[i].values[1]);
                }
                free_query_result(&result);
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
    while (1)
    {
        clear_screen();
        printf("\n=== 系统维护界面 ===\n");
        printf("1. 数据库备份\n");
        printf("2. 数据库恢复\n");
        printf("3. 数据库重置\n");
        printf("0. 返回主菜单\n");
        printf("请输入选项: ");

        int choice;
        scanf("%d", &choice);
        getchar();

        switch (choice)
        {
        case 1:
        {
            char backup_path[256];
            printf("\n请输入备份文件保存路径: ");
            fgets(backup_path, sizeof(backup_path), stdin);
            trim_newline(backup_path);

            if (backup_database(db))
            {
                printf("数据库备份成功!\n");
            }
            else
            {
                printf("数据库备份失败!\n");
            }
            break;
        }

        case 2:
        {
            char backup_path[256];
            printf("\n请输入备份文件路径: ");
            fgets(backup_path, sizeof(backup_path), stdin);
            trim_newline(backup_path);

            printf("\n警告:数据库恢复将覆盖当前所有数据,确定要继续吗?(y/n): ");
            char confirm = getchar();
            getchar();

            if (confirm == 'y' || confirm == 'Y')
            {
                if (restore_database(db))
                {
                    printf("数据库恢复成功!\n");
                }
                else
                {
                    printf("数据库恢复失败!\n");
                }
            }
            break;
        }

        case 3:
        {
            printf("\n警告:数据库重置将清空所有数据,确定要继续吗?(y/n): ");
            char confirm = getchar();
            getchar();

            if (confirm == 'y' || confirm == 'Y')
            {
                if (clean_database(db))
                {
                    if (db_init_tables(db))
                    {
                        printf("数据库重置成功!\n");
                    }
                    else
                    {
                        printf("数据库表初始化失败!\n");
                    }
                }
                else
                {
                    printf("数据库清理失败!\n");
                }
            }
            break;
        }

        case 0:
            return;

        default:
            printf("无效选项,请重新输入\n");
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
