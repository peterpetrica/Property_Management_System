/**
 * @file ui_admin.c
 * @brief 物业管理系统管理员界面功能实现
 *
 * 本文件实现了物业管理系统中管理员界面的所有相关功能：
 * - 管理员主界面
 * - 信息管理界面
 * - 服务分配界面
 * - 信息查询界面
 * - 信息排序界面
 * - 信息统计界面
 * - 系统维护界面
 * - 测试功能界面
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

// 添加缺少的函数声明
bool query_buildings(Database *db, QueryResult *result);
bool fuzzy_query_owner(Database *db, const char *pattern, QueryResult *result);
void pause_console();
void trim_newline(char *str);
void wait_for_key(); // 修改函数名，替换之前的pause
void print_query_result(QueryResult *result);

/**
 * @brief 清屏函数
 *
 * 清除控制台显示内容，适用于Windows和Unix/Linux系统
 */
void clear_screen()
{
    // system("clear||cls");
}

/**
 * @brief 显示管理员主界面
 *
 * 显示管理员系统的主菜单，提供各功能模块的入口
 *
 * @param db 数据库连接指针
 * @param user_id 当前登录用户的ID
 * @param user_type 当前登录用户的类型
 */
void show_admin_main_screen(Database *db, const char *user_id, UserType user_type)
{
    printf("\n=== 物业服务管理系统主页面 ===\n");
    printf("1. 信息管理界面\n");
    printf("2. 服务分配界面\n");
    printf("3. 信息查询界面\n");
    printf("4. 信息排序界面\n");
    printf("5. 信息统计界面\n");
    printf("6. 系统维护界面\n");
    printf("7. 房屋管理功能测试\n");
    printf("8. 楼宇管理功能测试\n"); // 添加新的菜单选项
    printf("9. 退出系统\n");
    printf("请输入选项: ");
    int choice;
    scanf("%d", &choice);
    getchar(); // 清除输入缓冲区中的换行符

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
    // 以下两项是测试用选单，暂时不要删
    case 7:
        show_apartment_test_screen(db, user_id, user_type);
        break;
    case 8:
        show_building_test_screen(db, user_id, user_type);
        break;
    case 9:
        printf("退出系统。\n");
        exit(0);
    default:
        printf("无效选项，请重新输入。\n");
    }
}

/**
 * @brief 显示信息管理界面
 *
 * 提供对系统基本信息（如楼盘、住户、用户等）的增删改查功能
 *
 * @param db 数据库连接指针
 * @param user_id 当前登录用户的ID
 * @param user_type 当前登录用户的类型
 */
void show_info_management_screen(Database *db, const char *user_id, UserType user_type)
{
    // TODO: 实现信息管理界面功能
}

/**
 * @brief 显示服务分配界面
 *
 * 管理员将物业服务请求分配给相应工作人员的界面
 *
 * @param db 数据库连接指针
 * @param user_id 当前登录用户的ID
 * @param user_type 当前登录用户的类型
 */
void show_service_assignment_screen(Database *db, const char *user_id, UserType user_type)
{
    // TODO: 实现服务分配界面功能
}

/**
 * @brief 显示系统维护界面
 *
 * 提供系统备份、恢复、用户信息更新和费用标准管理等功能
 *
 * @param db 数据库连接指针
 * @param user_id 当前登录用户的ID
 * @param user_type 当前登录用户的类型
 */
void show_system_maintenance_screen(Database *db, const char *user_id, UserType user_type)
{
    int choice;

    while (1)
    {
        clear_screen();
        printf("\n===== 系统维护 =====\n");
        printf("1. 费用标准管理\n");
        printf("2. 生成月度费用\n");
        printf("3. 数据备份\n");
        printf("4. 数据恢复\n");
        printf("0. 返回上一级\n");
        printf("\n请选择: ");

        scanf("%d", &choice);
        clear_input_buffer();

        switch (choice)
        {
        case 1:
            manage_fee_standards_screen(db, user_id, user_type);
            break;
        case 2:
            generate_monthly_fees_screen(db, user_id, user_type);
            break;
        case 3:
            backup_database(db);
            printf("数据备份完成\n");
            printf("按任意键继续...");
            getchar();
            break;
        case 4:
            restore_database(db);
            printf("数据恢复完成\n");
            printf("按任意键继续...");
            getchar();
            break;
        case 0:
            return;
        default:
            printf("无效的选择\n");
            printf("按任意键继续...");
            getchar();
            break;
        }
    }
}

////////////////////////////////////////////////
// 以下是各种功能测试的界面及其示例调用，请不要改动   //
//                                            //
//                                            //
//                                            //
//                                            //
////////////////////////////////////////////////

/**
 * @brief 显示房屋管理测试界面
 *
 * 提供添加、更新、删除、查询房屋信息等功能，用于测试
 *
 * @param db 数据库连接指针
 * @param user_id 当前登录用户的ID
 * @param user_type 当前登录用户的类型
 */
void show_apartment_test_screen(Database *db, const char *user_id, UserType user_type)
{
    while (1)
    {
        clear_screen();
        printf("\n=== 房屋管理功能测试 ===\n");
        printf("1. 添加房屋\n");
        printf("2. 更新房屋信息\n");
        printf("3. 删除房屋\n");
        printf("4. 获取房屋信息\n");
        printf("5. 查询特定楼宇的所有房屋\n");
        printf("6. 查询业主的所有房屋\n");
        printf("0. 返回主菜单\n");
        printf("请输入选项: ");

        int choice;
        scanf("%d", &choice);
        getchar();

        if (choice == 0)
        {
            show_admin_main_screen(db, user_id, user_type);
            return;
        }

        switch (choice)
        {
        case 1: // 添加房屋
        {
            Room room;
            memset(&room, 0, sizeof(Room));

            char building_name[64];
            printf("请输入楼宇名: ");
            scanf("%s", building_name);
            // 根据楼宇名查询楼宇ID
            if (!get_building_id_by_name(db, building_name, room.building_id))
            {
                printf("未找到此楼宇名！\n");
                getchar();
                break;
            }
            getchar();

            printf("请输入房间号: ");
            scanf("%s", room.room_number);
            getchar();

            printf("请输入楼层: ");
            scanf("%d", &room.floor);
            getchar();

            printf("请输入面积(平方米): ");
            scanf("%f", &room.area_sqm);
            getchar();

            printf("请输入业主ID(如果没有业主请直接回车): ");
            fgets(room.owner_id, sizeof(room.owner_id), stdin);
            room.owner_id[strcspn(room.owner_id, "\n")] = 0;

            printf("请输入状态(如 '已售', '在售', '空置'): ");
            scanf("%63s", room.status); // 修正格式指定符和使用数组大小限制
            getchar();

            if (add_room(db, user_id, user_type, &room))
            {
                printf("添加房屋成功！房屋ID: %s\n", room.room_id);
            }
            else
            {
                printf("添加房屋失败！\n");
            }
        }
        break;

        case 2: // 更新房屋信息
        {
            Room room;
            memset(&room, 0, sizeof(Room));

            printf("请输入要更新的房屋ID: ");
            scanf("%s", room.room_id);
            getchar();

            // 获取原房屋信息
            if (!get_room(db, room.room_id, &room))
            {
                printf("未找到此房屋ID！\n");
                break;
            }

            printf("当前房屋信息：\n");
            printf("楼宇ID: %s\n", room.building_id);
            printf("房间号: %s\n", room.room_number);
            printf("楼层: %d\n", room.floor);
            printf("面积: %.2f平方米\n", room.area_sqm);
            printf("业主ID: %s\n", room.owner_id[0] ? room.owner_id : "无");
            printf("状态: %s\n", room.status);

            printf("\n请输入新的房屋信息（直接回车保持不变）：\n");

            char buffer[256];
            printf("楼宇ID [%s]: ", room.building_id);
            fgets(buffer, sizeof(buffer), stdin);
            buffer[strcspn(buffer, "\n")] = 0;
            if (buffer[0] != '\0')
                strncpy(room.building_id, buffer, sizeof(room.building_id) - 1);

            printf("房间号 [%s]: ", room.room_number);
            fgets(buffer, sizeof(buffer), stdin);
            buffer[strcspn(buffer, "\n")] = 0;
            if (buffer[0] != '\0')
                strncpy(room.room_number, buffer, sizeof(room.room_number) - 1);

            printf("楼层 [%d]: ", room.floor);
            fgets(buffer, sizeof(buffer), stdin);
            buffer[strcspn(buffer, "\n")] = 0;
            if (buffer[0] != '\0')
                room.floor = atoi(buffer);

            printf("面积(平方米) [%.2f]: ", room.area_sqm);
            fgets(buffer, sizeof(buffer), stdin);
            buffer[strcspn(buffer, "\n")] = 0;
            if (buffer[0] != '\0')
                room.area_sqm = atof(buffer);

            printf("业主ID [%s]: ", room.owner_id[0] ? room.owner_id : "无");
            fgets(buffer, sizeof(buffer), stdin);
            buffer[strcspn(buffer, "\n")] = 0;
            if (buffer[0] != '\0')
                strncpy(room.owner_id, buffer, sizeof(room.owner_id) - 1);

            printf("状态 [%s]: ", room.status);
            fgets(buffer, sizeof(buffer), stdin);
            buffer[strcspn(buffer, "\n")] = 0;
            if (buffer[0] != '\0')
                strncpy(room.status, buffer, sizeof(room.status) - 1);
            if (update_room(db, user_id, user_type, &room))
            {
                printf("更新房屋信息成功！\n");
            }
            else
            {
                printf("更新房屋信息失败！\n");
            }
        }
        break;

        case 3: // 删除房屋
        {
            char room_id[64];
            printf("请输入要删除的房屋ID: ");
            scanf("%s", room_id);
            getchar();

            if (delete_room(db, user_id, user_type, room_id))
            {
                printf("删除房屋成功！\n");
            }
            else
            {
                printf("删除房屋失败！\n");
            }
        }
        break;

        case 4: // 获取房屋信息
        {
            char room_id[64];
            printf("请输入房屋ID: ");
            scanf("%s", room_id);
            getchar();

            Room room;
            if (get_room(db, room_id, &room))
            {
                printf("\n房屋信息：\n");
                printf("---------------------------------------\n");
                printf("房屋ID: %s\n", room.room_id);
                printf("楼宇ID: %s\n", room.building_id);
                printf("房间号: %s\n", room.room_number);
                printf("楼层: %d\n", room.floor);
                printf("面积: %.2f平方米\n", room.area_sqm);
                printf("业主ID: %s\n", room.owner_id[0] ? room.owner_id : "无");
                printf("状态: %s\n", room.status);
                printf("---------------------------------------\n");
            }
            else
            {
                printf("获取房屋信息失败！\n");
            }
        }
        break;

        case 5: // 获取特定楼宇的所有房屋
        {
            char building_name[64];
            printf("请输入楼宇名: ");
            scanf("%s", building_name);
            // 根据楼宇名查询楼宇ID
            char building_id[40];
            if (!get_building_id_by_name(db, building_name, building_id))
            {
                printf("未找到此楼宇名！\n");
                break;
            }
            getchar();

            QueryResult result;
            if (list_rooms_by_building(db, user_id, user_type, building_id, &result))
            {
                printf("\n该楼宇下的所有房屋：\n");
                printf("---------------------------------------\n");
                printf("%-10s %-10s %-10s %-5s %-8s %-20s %-10s\n",
                       "房屋ID", "楼宇ID", "房间号", "楼层", "面积", "业主ID/姓名", "状态");

                for (int i = 0; i < result.row_count; i++)
                {
                    printf("%-10s %-10s %-10s %-5s %-8s %-20s %-10s\n",
                           result.rows[i].values[0],                                                       // room_id
                           result.rows[i].values[1],                                                       // building_id
                           result.rows[i].values[2],                                                       // room_number
                           result.rows[i].values[3],                                                       // floor
                           result.rows[i].values[4],                                                       // area_sqm
                           result.rows[i].values[6] ? result.rows[i].values[6] : result.rows[i].values[5], // owner_name 或 owner_id
                           result.rows[i].values[7]);                                                      // status
                }
                printf("---------------------------------------\n");
                printf("共 %d 条记录\n", result.row_count);
                free_query_result(&result);
            }
            else
            {
                printf("查询楼宇房屋失败！\n");
            }
        }
        break;

        case 6: // 获取业主的所有房屋
        {
            char owner_id[64];
            printf("请输入业主ID: ");
            scanf("%s", owner_id);
            getchar();

            QueryResult result;
            if (get_owner_rooms(db, user_id, user_type, owner_id, &result))
            {
                printf("\n该业主拥有的所有房屋：\n");
                printf("---------------------------------------\n");
                printf("%-10s %-10s %-15s %-10s %-5s %-8s %-10s\n",
                       "房屋ID", "楼宇ID", "楼宇名称", "房间号", "楼层", "面积", "状态");

                for (int i = 0; i < result.row_count; i++)
                {
                    printf("%-10s %-10s %-15s %-10s %-5s %-8s %-10s\n",
                           result.rows[i].values[0],  // room_id
                           result.rows[i].values[1],  // building_id
                           result.rows[i].values[2],  // building_name
                           result.rows[i].values[3],  // room_number
                           result.rows[i].values[4],  // floor
                           result.rows[i].values[5],  // area_sqm
                           result.rows[i].values[6]); // status
                }
                printf("---------------------------------------\n");
                printf("共 %d 条记录\n", result.row_count);
                free_query_result(&result);
            }
            else
            {
                printf("查询业主房屋失败！\n");
            }
        }
        break;

        default:
            printf("无效选项，请重新输入。\n");
        }

        printf("\n按Enter键继续...");
        getchar();
    }
}

/**
 * @brief 显示楼宇管理测试界面
 *
 * 提供添加、修改、删除、查询楼宇信息等功能，用于测试
 *
 * @param db 数据库连接指针
 * @param user_id 当前登录用户的ID
 * @param user_type 当前登录用户的类型
 */
void show_building_test_screen(Database *db, const char *user_id, UserType user_type)
{
    // 楼宇管理测试界面实现
    clear_screen();
    printf("\n=== 楼宇管理功能测试 ===\n");
    printf("该功能正在开发中...\n");
    printf("按Enter键返回主菜单...");
    getchar();
    show_admin_main_screen(db, user_id, user_type);
}

/**
 * @brief 显示信息查询界面
 *
 * 提供按不同条件查询系统内各类信息的功能
 *
 * @param db 数据库连接指针
 * @param user_id 当前登录用户的ID
 * @param user_type 当前登录用户的类型
 */
void show_info_query_screen(Database *db, const char *user_id, UserType user_type)
{
    int choice;
    while (1)
    {
        clear_screen();
        printf("\n=== 信息查询界面 ===\n");
        printf("1. 查询所有楼盘信息\n");
        printf("2. 模糊查询业主姓名\n");
        printf("3. 返回主界面\n");
        printf("请输入选项: ");
        scanf("%d", &choice);
        getchar();

        QueryResult result;
        char pattern[100];

        if (choice == 3)
            break;

        switch (choice)
        {
        case 1:
            if (query_buildings(db, &result))
            {
                printf("\n--- 所有楼盘信息 ---\n");
                for (int i = 0; i < result.row_count; i++)
                {
                    printf("ID: %s | 名称: %s | 地址: %s | 管理员ID: %s\n",
                           result.rows[i].values[0], result.rows[i].values[1],
                           result.rows[i].values[2], result.rows[i].values[3]);
                }
            }
            else
            {
                printf("查询失败！\n");
            }
            free_query_result(&result);
            pause_console();
            break;
        case 2:
            printf("请输入业主姓名关键字: ");
            fgets(pattern, sizeof(pattern), stdin);
            trim_newline(pattern);

            if (fuzzy_query_owner(db, pattern, &result))
            {
                printf("\n--- 查询结果 ---\n");
                for (int i = 0; i < result.row_count; i++)
                {
                    printf("ID: %s | 姓名: %s | 联系方式: %s\n",
                           result.rows[i].values[0], result.rows[i].values[1],
                           result.rows[i].values[2]);
                }
            }
            else
            {
                printf("查询失败！\n");
            }
            free_query_result(&result);
            pause_console();
            break;
        default:
            printf("无效选项。\n");
            pause_console();
        }
    }
}

/**
 * @brief 显示信息排序界面
 *
 * 根据不同条件对查询结果进行排序展示的功能
 *
 * @param db 数据库连接指针
 * @param user_id 当前登录用户的ID
 * @param user_type 当前登录用户的类型
 */
void show_info_sort_screen(Database *db, const char *user_id, UserType user_type)
{
    clear_screen();
    printf("\n=== 信息排序界面 ===\n");

    int choice;
    printf("1. 业主缴费信息单属性排序\n");
    printf("2. 业主缴费信息多属性排序\n");
    printf("3. 按姓名升序排序业主信息\n");
    printf("4. 返回主界面\n");
    printf("请输入选项: ");
    scanf("%d", &choice);
    getchar();

    QueryResult result;
    char sql[1024];

    switch (choice)
    {
    case 1:
    {
        // 单属性排序
        printf("请选择排序字段: \n");
        printf("1. 缴费时间\n");
        printf("2. 缴费金额\n");
        printf("3. 业主姓名\n");
        printf("请输入: ");
        int attr;
        scanf("%d", &attr);
        getchar();

        char *order_by = NULL;
        if (attr == 1)
            order_by = "payment.payment_date";
        else if (attr == 2)
            order_by = "payment.amount";
        else if (attr == 3)
            order_by = "owner.name";
        else
        {
            printf("无效选项。\n");
            pause();
            return;
        }

        snprintf(sql, sizeof(sql),
                 "SELECT owner.name AS 业主姓名, payment.amount AS 缴费金额, payment.payment_date AS 缴费时间 "
                 "FROM payment "
                 "JOIN apartment ON payment.apartment_id = apartment.id "
                 "JOIN owner ON apartment.owner_id = owner.id "
                 "JOIN building ON apartment.building_id = building.id "
                 "WHERE building.manager_id = '%s' "
                 "ORDER BY %s ASC;",
                 user_id, order_by);

        if (execute_query(db, sql, &result))
        {
            print_query_result(&result);
            free_query_result(&result);
        }
        else
            printf("查询失败。\n");

        break;
    }
    case 2:
    {
        // 多属性排序
        printf("多属性排序：先按缴费金额降序，再按缴费时间升序。\n");

        snprintf(sql, sizeof(sql),
                 "SELECT owner.name AS 业主姓名, payment.amount AS 缴费金额, payment.payment_date AS 缴费时间 "
                 "FROM payment "
                 "JOIN apartment ON payment.apartment_id = apartment.id "
                 "JOIN owner ON apartment.owner_id = owner.id "
                 "JOIN building ON apartment.building_id = building.id "
                 "WHERE building.manager_id = '%s' "
                 "ORDER BY payment.amount DESC, payment.payment_date ASC;",
                 user_id);

        if (execute_query(db, sql, &result))
        {
            print_query_result(&result);
            free_query_result(&result);
        }
        else
            printf("查询失败。\n");

        break;
    }
    case 3:
    {
        // 按姓名升序
        snprintf(sql, sizeof(sql),
                 "SELECT owner.name AS 业主姓名, owner.phone AS 电话 "
                 "FROM owner "
                 "JOIN apartment ON owner.id = apartment.owner_id "
                 "JOIN building ON apartment.building_id = building.id "
                 "WHERE building.manager_id = '%s' "
                 "GROUP BY owner.id "
                 "ORDER BY owner.name ASC;",
                 user_id);

        if (execute_query(db, sql, &result))
        {
            print_query_result(&result);
            free_query_result(&result);
        }
        else
            printf("查询失败。\n");

        break;
    }
    case 4:
        return;
    default:
        printf("无效选项。\n");
    }

    pause();
    show_info_sort_screen(db, user_id, user_type);
}

/**
 * @brief 显示信息统计界面
 *
 * 对系统内数据进行统计分析并呈现的功能
 *
 * @param db 数据库连接指针
 * @param user_id 当前登录用户的ID
 * @param user_type 当前登录用户的类型
 */
void show_info_statistics_screen(Database *db, const char *user_id, UserType user_type)
{
    int choice;
    clear_screen();
    printf("\n=== 信息统计界面 ===\n");

    printf("1. 统计未缴费业主数量\n");
    printf("2. 按缴费状态统计（已缴费/未缴费）\n");
    printf("3. 统计某年所有未缴费的业主\n");
    printf("4. 统计截止某时间已缴费业主数量\n");
    printf("5. 返回主界面\n");
    printf("请输入选项: ");
    scanf("%d", &choice);
    getchar();

    QueryResult result;
    char sql[1024];

    switch (choice)
    {
    case 1:
    {
        // 统计未缴费业主数量
        snprintf(sql, sizeof(sql),
                 "SELECT COUNT(*) AS 未缴费业主数量 FROM owner "
                 "JOIN apartment ON owner.id = apartment.owner_id "
                 "JOIN payment ON apartment.id = payment.apartment_id "
                 "JOIN building ON apartment.building_id = building.id "
                 "WHERE building.manager_id = '%s' AND payment.status = '未缴费';",
                 user_id);

        if (execute_query(db, sql, &result))
        {
            print_query_result(&result);
            free_query_result(&result);
        }
        else
            printf("统计失败。\n");

        break;
    }
    case 2:
    {
        // 按缴费状态统计
        printf("请选择缴费状态统计:\n");
        printf("1. 已缴费业主数量\n");
        printf("2. 未缴费业主数量\n");
        printf("请输入选项: ");
        int status_choice;
        scanf("%d", &status_choice);
        getchar();

        char *status = (status_choice == 1) ? "已缴费" : "未缴费";
        snprintf(sql, sizeof(sql),
                 "SELECT COUNT(*) AS 业主数量 FROM owner "
                 "JOIN apartment ON owner.id = apartment.owner_id "
                 "JOIN payment ON apartment.id = payment.apartment_id "
                 "JOIN building ON apartment.building_id = building.id "
                 "WHERE building.manager_id = '%s' AND payment.status = '%s';",
                 user_id, status);

        if (execute_query(db, sql, &result))
        {
            print_query_result(&result);
            free_query_result(&result);
        }
        else
            printf("统计失败。\n");

        break;
    }
    case 3:
    {
        // 统计某年所有未缴费的业主
        int year;
        printf("请输入查询年份: ");
        scanf("%d", &year);
        getchar();

        snprintf(sql, sizeof(sql),
                 "SELECT COUNT(*) AS 未缴费业主数量 FROM owner "
                 "JOIN apartment ON owner.id = apartment.owner_id "
                 "JOIN payment ON apartment.id = payment.apartment_id "
                 "JOIN building ON apartment.building_id = building.id "
                 "WHERE building.manager_id = '%s' AND payment.status = '未缴费' "
                 "AND strftime('%%Y', payment.payment_date) = '%d';",
                 user_id, year);

        if (execute_query(db, sql, &result))
        {
            print_query_result(&result);
            free_query_result(&result);
        }
        else
            printf("统计失败。\n");

        break;
    }
    case 4:
    {
        // 统计截至某时间已缴费业主数量
        char date[20];
        printf("请输入截止日期（格式: YYYY-MM-DD）：");
        fgets(date, sizeof(date), stdin);
        trim_newline(date);

        snprintf(sql, sizeof(sql),
                 "SELECT COUNT(*) AS 已缴费业主数量 FROM owner "
                 "JOIN apartment ON owner.id = apartment.owner_id "
                 "JOIN payment ON apartment.id = payment.apartment_id "
                 "JOIN building ON apartment.building_id = building.id "
                 "WHERE building.manager_id = '%s' AND payment.status = '已缴费' "
                 "AND payment.payment_date <= '%s';",
                 user_id, date);

        if (execute_query(db, sql, &result))
        {
            print_query_result(&result);
            free_query_result(&result);
        }
        else
            printf("统计失败。\n");

        break;
    }
    case 5:
        return;
    default:
        printf("无效选项。\n");
    }

    pause();
    show_info_statistics_screen(db, user_id, user_type);
}

/**
 * @brief 生成周期性费用界面
 *
 * 按年月生成物业费、停车费等周期性费用的界面
 *
 * @param db 数据库连接指针
 * @param user_id 当前登录用户的ID
 * @param user_type 当前登录用户的类型
 */
void generate_periodic_fees_screen(Database *db, const char *user_id, UserType user_type)
{
    if (user_type != USER_ADMIN)
    {
        printf("权限不足，无法访问此功能\n");
        return;
    }

    int year, month, fee_type, due_days;

    // system("clear||cls");
    printf("\n===== 生成周期性费用 =====\n");
    printf("请输入年份: ");
    scanf("%d", &year);
    getchar(); // 清除输入缓冲区

    printf("请输入月份(1-12): ");
    scanf("%d", &month);
    getchar(); // 清除输入缓冲区

    if (month < 1 || month > 12)
    {
        printf("无效的月份\n");
        printf("按任意键返回...");
        getchar();
        return;
    }

    printf("请选择费用类型:\n");
    printf("1. 物业费\n");
    printf("2. 停车费\n");
    printf("3. 全部\n");
    printf("请输入(1-3): ");
    scanf("%d", &fee_type);
    getchar(); // 清除输入缓冲区

    printf("请输入付款截止天数(从月底开始计算): ");
    scanf("%d", &due_days);
    getchar(); // 清除输入缓冲区

    // 计算账单开始和结束日期
    struct tm start_tm = {0}, end_tm = {0};
    start_tm.tm_year = year - 1900;
    start_tm.tm_mon = month - 1;
    start_tm.tm_mday = 1;

    end_tm.tm_year = year - 1900;
    end_tm.tm_mon = month;
    end_tm.tm_mday = 0; // 这会自动调整为上个月的最后一天

    time_t period_start = mktime(&start_tm);
    time_t period_end = mktime(&end_tm);

    bool success = true;

    if (fee_type == 1 || fee_type == 3)
    {
        success &= generate_property_fees(db, period_start, period_end, due_days);
    }

    if (fee_type == 2 || fee_type == 3)
    {
        success &= generate_parking_fees(db, period_start, period_end, due_days);
    }

    if (success)
    {
        printf("成功生成%d年%d月的费用账单\n", year, month);
    }
    else
    {
        printf("生成费用账单时发生错误\n");
    }

    printf("按任意键返回...");
    getchar();
}

/**
 * @brief 费用标准管理界面
 *
 * 提供查看、添加费用标准和生成周期性费用的功能
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
        // system("clear||cls");
        printf("\n===== 费用标准管理 =====\n");
        printf("1. 查看现有费用标准\n");
        printf("2. 添加新费用标准\n");
        printf("3. 生成周期性费用\n");
        printf("0. 返回上一级\n");
        printf("请输入您的选择: ");
        scanf("%d", &choice);
        getchar(); // 清除输入缓冲区

        switch (choice)
        {
        case 1:
            // 查看费用标准实现
            {
                // system("clear||cls");
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
                    // 费用类型
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

                    // 生效日期
                    time_t effective_date = (time_t)atol(result.rows[i].values[4]);
                    localtime_r(&effective_date, &tm_info);
                    strftime(start_date_str, sizeof(start_date_str), "%Y-%m-%d", &tm_info);

                    // 终止日期
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
                           result.rows[i].values[0], // 标准ID
                           fee_type_str,
                           atof(result.rows[i].values[2]), // 单价
                           result.rows[i].values[3],       // 单位
                           start_date_str,
                           end_date_str);
                }

                free_query_result(&result);
                printf("\n按任意键返回...");
                getchar();
            }
            break;
        case 2:
            // 添加费用标准实现
            {
                // system("clear||cls");
                printf("\n===== 添加新费用标准 =====\n");

                FeeStandard standard;
                memset(&standard, 0, sizeof(FeeStandard));

                generate_uuid(standard.standard_id);

                printf("请选择费用类型:\n");
                printf("1. 物业费\n");
                printf("2. 停车费\n");
                printf("3. 水费\n");
                printf("4. 电费\n");
                printf("5. 燃气费\n");
                printf("99. 其他费用\n");
                printf("请输入: ");
                scanf("%d", &standard.fee_type);
                getchar(); // 清除输入缓冲区

                printf("请输入单价: ");
                scanf("%f", &standard.price_per_unit);
                getchar(); // 清除输入缓冲区

                printf("请输入计费单位(如元/平方米/月、元/个/月): ");
                scanf("%15s", standard.unit);
                getchar(); // 清除输入缓冲区

                printf("请输入生效日期(格式: YYYY-MM-DD): ");
                char date_str[20];
                scanf("%19s", date_str);
                getchar(); // 清除输入缓冲区

                // 解析日期
                struct tm tm_info = {0};
                if (sscanf(date_str, "%d-%d-%d", &tm_info.tm_year, &tm_info.tm_mon, &tm_info.tm_mday) == 3)
                {
                    tm_info.tm_year -= 1900; // 年份需要减去1900
                    tm_info.tm_mon -= 1;     // 月份是从0开始的
                    standard.effective_date = mktime(&tm_info);
                }
                else
                {
                    printf("日期格式错误\n");
                    printf("按任意键返回...");
                    getchar();
                    break;
                }

                printf("是否设置终止日期? (1-是, 0-无限期): ");
                int has_end_date;
                scanf("%d", &has_end_date);
                getchar(); // 清除输入缓冲区

                if (has_end_date)
                {
                    printf("请输入终止日期(格式: YYYY-MM-DD): ");
                    scanf("%19s", date_str);
                    getchar(); // 清除输入缓冲区

                    // 解析日期
                    struct tm end_tm = {0};
                    if (sscanf(date_str, "%d-%d-%d", &end_tm.tm_year, &end_tm.tm_mon, &end_tm.tm_mday) == 3)
                    {
                        end_tm.tm_year -= 1900; // 年份需要减去1900
                        end_tm.tm_mon -= 1;     // 月份是从0开始的
                        standard.end_date = mktime(&end_tm);
                    }
                    else
                    {
                        printf("日期格式错误\n");
                        printf("按任意键返回...");
                        getchar();
                        break;
                    }
                }
                else
                {
                    standard.end_date = 0; // 无限期
                }

                // 添加费用标准
                if (add_fee_standard(db, user_id, user_type, &standard))
                {
                    printf("成功添加费用标准\n");
                }
                else
                {
                    printf("添加费用标准失败\n");
                }

                printf("按任意键返回...");
                getchar();
            }
            break;
        case 3:
            generate_periodic_fees_screen(db, user_id, user_type);
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
 * @brief 生成月度费用界面
 *
 * 按年月生成物业费、停车费等周期性费用的界面
 *
 * @param db 数据库连接指针
 * @param user_id 当前登录用户的ID
 * @param user_type 当前登录用户的类型
 */
void generate_monthly_fees_screen(Database *db, const char *user_id, UserType user_type)
{
    clear_screen();
    printf("\n===== 生成月度费用 =====\n");

    int year, month;
    time_t now = time(NULL);
    struct tm *t = localtime(&now);

    printf("当前日期: %d年%d月\n\n", t->tm_year + 1900, t->tm_mon + 1);
    printf("请输入要生成的年份: ");
    scanf("%d", &year);
    printf("请输入要生成的月份(1-12): ");
    scanf("%d", &month);

    if (month < 1 || month > 12)
    {
        printf("无效的月份\n");
        wait_for_key();
        return;
    }

    // 设置账单周期
    struct tm start_tm = {0};
    start_tm.tm_year = year - 1900;
    start_tm.tm_mon = month - 1;
    start_tm.tm_mday = 1;

    struct tm end_tm = start_tm;
    end_tm.tm_mon++;

    time_t period_start = mktime(&start_tm);
    time_t period_end = mktime(&end_tm);

    printf("\n生成周期: %d年%d月1日 - %d年%d月1日\n",
           year, month,
           month == 12 ? year + 1 : year,
           month == 12 ? 1 : month + 1);

    int due_days;
    printf("请输入缴费截止天数(从下月1日起): ");
    scanf("%d", &due_days);

    printf("\n正在生成费用...\n");

    bool success = true;
    success &= generate_property_fees(db, period_start, period_end, due_days);
    success &= generate_parking_fees(db, period_start, period_end, due_days);
    success &= generate_utility_fees(db, period_start, period_end, due_days);

    if (success)
    {
        printf("\n✓ 成功生成%d年%d月的费用账单\n", year, month);
    }
    else
    {
        printf("\n✗ 生成费用账单时发生错误\n");
    }

    wait_for_key();
}
