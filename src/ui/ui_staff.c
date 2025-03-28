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
    while ((c = getchar()) != '\n' && c != EOF);
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
            clear_staff_input_buffer(); // 清空缓冲区
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
                clear_staff_input_buffer(); // 清空缓冲区
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
                        char short_id[9];  // 只显示ID的前8位
                        strncpy(short_id, full_id, 8);
                        short_id[8] = '\0';

                        // 使用格式化的表格样式
                        printf("| %-8s | %-12s | %-12s | %-12s | %-25s |\n",
                            short_id,
                            sqlite3_column_text(stmt, 1) ? (char*)sqlite3_column_text(stmt, 1) : "N/A",
                            sqlite3_column_text(stmt, 2) ? (char*)sqlite3_column_text(stmt, 2) : "N/A",
                            sqlite3_column_text(stmt, 3) ? (char*)sqlite3_column_text(stmt, 3) : "N/A",
                            sqlite3_column_text(stmt, 4) ? (char*)sqlite3_column_text(stmt, 4) : "N/A");
                        printf("--------------------------------------------------------------------------------------------------------\n");
                    }
                    
                    if (!found) {
                        printf("\n⚠️ 未找到任何用户信息\n");
                    }
                    
                    sqlite3_finalize(stmt);
                }
            }
            break;

        case 2:
            // 查询用户缴费情况
            {
                clear_staff_screen();
                printf("\n===== 用户缴费情况 =====\n\n");
                const char *query = 
                    "SELECT u.user_id, u.username, "
                    "COUNT(t.transaction_id) as total_trans, "
                    "SUM(CASE WHEN t.status = 1 THEN t.amount ELSE 0 END) as paid_amount, "
                    "SUM(CASE WHEN t.status = 0 THEN t.amount ELSE 0 END) as unpaid_amount "
                    "FROM users u "
                    "LEFT JOIN transactions t ON u.user_id = t.user_id "
                    "WHERE u.role_id = 'role_owner' "
                    "GROUP BY u.user_id, u.username;";

                sqlite3_stmt *stmt;
                if (sqlite3_prepare_v2(db->db, query, -1, &stmt, NULL) == SQLITE_OK)
                {
                    printf("%-12s%-16s%-16s%-16s%-16s\n", 
                           "用户ID", "用户名", "交易总数", "已缴金额", "未缴金额");
                    printf("------------------------------------------------------------------------\n");
                    
                    int found = 0;
                    while (sqlite3_step(stmt) == SQLITE_ROW)
                    {
                        found = 1;
                        printf("%-12s%-16s%-16d%-16.2f%-16.2f\n",
                            sqlite3_column_text(stmt, 0),
                            sqlite3_column_text(stmt, 1),
                            sqlite3_column_int(stmt, 2),
                            sqlite3_column_double(stmt, 3),
                            sqlite3_column_double(stmt, 4));
                    }
                    
                    if (!found) {
                        printf("\n未找到任何用户缴费信息\n");
                    }
                    
                    sqlite3_finalize(stmt);
                }
            }
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
