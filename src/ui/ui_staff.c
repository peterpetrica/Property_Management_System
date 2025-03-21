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
 
 // 显示服务人员主界面
 void show_staff_main_screen(Database *db, const char *token,UserType user_type)
 {
     if (user_type != USER_STAFF)
     {
         printf("无权访问\n");
         return;
     }
     int choice = 0;
     do
     {
         system("cls");
         printf("欢迎来到服务人员主页面\n");
         printf("1.显示个人信息\n");
         printf("2.修改个人信息\n");
         printf("3.服务人员信息查询\n");
         printf("4.服务人员信息排序\n");
         printf("5.服务人员数据统计\n");
         printf("0.退出\n");
         printf("请选择\n");
         scanf("%d", &choice);
         switch (choice)
         {
         case 1:
             show_personal_info_screen(db, token);
             break;
         case 2:
             modify_personal_info_screen(db, token);
             break;
         case 3:
             show_staff_query_screen(db, token);
             break;
         case 4:
             show_staff_sort_screen(db, token);
             break;
         case 5:
             show_staff_statistics_screen(db, token);
             break;
         case 0:
             printf("退出服务人员主界面\n");
             break;
         default:
             printf("无效选择，请重试\n");
         }
     }while(choice != 0);
 }
 
 void show_personal_info_screen(Database* db, const char* token)
 {
     system("cls");
     printf("个人信息界面\n");
     Staff staff;
     if(!get_staff_by_id(db, token, &staff))
     {
         printf("获取个人信息失败\n");
         return;
     }
     printf("姓名: %s\n", staff.name);
     printf("联系方式: %s\n", staff.phone);
     printf("服务类型: %s\n", staff.service_type);
     printf("按任意键返回主菜单...\n");
     getchar();
     getchar(); // 等待用户输入
 }
 // 修改个人信息界面
 void modify_personal_info_screen(Database *db, const char *token)
 {
     system("cls");
     printf("修改个人信息界面\n");
     Staff staff;
     
     if (!get_staff_by_id(db, token, &staff))
     {
         printf("获取个人信息失败\n");
         return;
     }
     printf("当前姓名: %s\n", staff.name);
     printf("请输入新的姓名: ");
     scanf("%s", staff.name);
 
     printf("当前联系方式: %s\n", staff.phone);
     printf("请输入新的联系方式: ");
     scanf("%s", staff.phone);
 
     if (update_staff(db, token, &staff))
     {
         printf("个人信息更新成功\n");
     }
     else
     {
         printf("个人信息更新失败，请重试\n");
     }
 
     printf("按任意键返回主菜单...\n");
     getchar();
     getchar(); // 等待用户输入
 }
 // 服务人员信息查询界面
 void show_staff_query_screen(Database *db, const char *token)
 {
     system("cls");
     printf("服务人员信息查询界面\n");
 
     Staff staff_list[100];
     int count = query_all_staff(db, staff_list, 100);
     if (count <= 0)
     {
         printf("没有找到任何服务人员信息\n");
     }
     else
     {
         printf("服务人员信息列表:\n");
         for (int i = 0; i < count; i++)
         {
             printf("%d. 姓名: %s, 联系方式: %s, 服务类型: %s\n",
                    i + 1, staff_list[i].name, staff_list[i].phone, staff_list[i].service_type);
         }
     }
 
     printf("按任意键返回主菜单...\n");
     getchar();
     getchar(); // 等待用户输入
 }
 
 // 服务人员信息排序界面
 void show_staff_sort_screen(Database *db, const char *token)
 {
     system("cls");
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
                    i + 1, staff_list[i].name, staff_list[i].phone, staff_list[i].service_type);
         }
     }
 
     printf("按任意键返回主菜单...\n");
     getchar();
     getchar(); // 等待用户输入
 }
 
 // 服务人员信息统计界面
 void show_staff_statistics_screen(Database *db, const char *token)
 {
     system("cls");
     printf("服务人员数据统计界面\n");
 
     int total_staff = count_all_staff(db);
     printf("服务人员总数: %d\n", total_staff);
 
     printf("按任意键返回主菜单...\n");
     getchar();
     getchar(); // 等待用户输入
 }
 
 // 服务人员系统维护界面
 void show_staff_maintenance_screen(Database *db, const char *token)
 {
     system("cls");
     printf("服务人员系统维护界面\n");
 
     printf("1. 数据备份\n");
     printf("2. 数据恢复\n");
     printf("0. 返回主菜单\n");
     int choice;
     scanf("%d", &choice);
 
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
 
     printf("按任意键返回主菜单...\n");
     getchar();
     getchar(); // 等待用户输入
 }
 