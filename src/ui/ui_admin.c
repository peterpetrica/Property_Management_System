//////////////////////////////////////////////////////////////
// 文件名: ui_admin.c
//
// 功能: 实现物业管理系统的管理员界面相关功能
//
// 主要模块:
// 1. 管理员主界面 - 显示管理员可以访问的所有功能入口
// 2. 信息管理界面 - 提供对楼盘、住户、用户等基本信息的增删改查功能
// 3. 服务分配界面 - 管理员分配物业服务请求给相应工作人员
// 4. 信息查询界面 - 按照不同条件查询系统内的各类信息
// 5. 信息排序界面 - 根据不同条件对查询结果进行排序展示
// 6. 信息统计界面 - 对系统内数据进行统计分析并呈现
// 7. 系统维护界面 - 系统备份、恢复和维护相关功能
//
// 预期实现: 为管理员提供友好的命令行交互界面，展示菜单选项，
// 接收用户输入，调用相应的数据库操作函数，并展示操作结果。
// 确保系统操作安全性并提供适当的错误处理和用户反馈。
//////////////////////////////////////////////////////////////

#include "ui/ui_admin.h"
#include "ui/ui_login.h"
#include "models/building.h"
#include "models/apartment.h"
#include "models/user.h"
#include "models/transaction.h"
#include "models/service.h"
#include "db/db_query.h"
#include "utils/utils.h"
#include "utils/file_ops.h"
#include "utils/console.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
void show_info_management_screen(Database *db, const char *user_id, UserType user_type);
void show_service_assignment_screen(Database *db, const char *user_id, UserType user_type);
void show_info_query_screen(Database *db, const char *user_id, UserType user_type);
void show_info_sort_screen(Database *db, const char *user_id, UserType user_type);
void show_info_statistics_screen(Database *db, const char *user_id, UserType user_type);
void show_system_maintenance_screen(Database *db, const char *user_id, UserType user_type);
void show_apartment_test_screen(Database *db, const char *user_id, UserType user_type);

// 在文件顶部添加这个函数声明
void clear_screen()
{
    system("clear||cls");
}

// 显示管理员主界面
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
    printf("8. 退出系统\n");
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
    case 7:
        show_apartment_test_screen(db, user_id, user_type);
        break;
    case 8:
        printf("退出系统。\n");
        exit(0);
    default:
        printf("无效选项，请重新输入。\n");
    }
}

// 信息管理界面
void show_info_management_screen(Database *db, const char *user_id, UserType user_type)
{
    // TODO: 实现信息管理界面功能
}

// 服务分配界面
void show_service_assignment_screen(Database *db, const char *user_id, UserType user_type)
{
    // TODO: 实现服务分配界面功能
}

// 信息查询界面
void show_info_query_screen(Database *db, const char *user_id, UserType user_type)
{
    // TODO: 实现信息查询界面功能
}

// 信息排序界面
void show_info_sort_screen(Database *db, const char *user_id, UserType user_type)
{
    // TODO: 实现信息排序界面功能
}

// 信息统计界面
void show_info_statistics_screen(Database *db, const char *user_id, UserType user_type)
{
    // TODO: 实现信息统计界面功能
}

// 系统维护界面
void show_system_maintenance_screen(Database *db, const char *user_id, UserType user_type)
{
    // TODO: 实现系统维护界面功能
}

// 房屋管理功能测试界面
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
        getchar(); // 清除输入缓冲区中的换行符

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

            printf("请输入楼宇ID: ");
            scanf("%s", room.building_id);
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
            // 移除换行符
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

            printf("状态 [%s]: ", room.status); // 应该是字符数组，不需要修改
            fgets(buffer, sizeof(buffer), stdin);
            buffer[strcspn(buffer, "\n")] = 0;
            if (buffer[0] != '\0')
                strncpy(room.status, buffer, sizeof(room.status) - 1); // 应该是字符数组，不需要修改

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
                printf("状态: %s\n", room.status); // 应该是字符数组，不需要修改
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
            char building_id[64];
            printf("请输入楼宇ID: ");
            scanf("%s", building_id);
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