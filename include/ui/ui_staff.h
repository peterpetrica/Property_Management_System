#ifndef UI_STAFF_H
#define UI_STAFF_H

#include "db/database.h"
#include "auth/auth.h"
#include "models/user.h" // 添加这行以引入Staff类型定义

// 显示服务人员主界面
void show_staff_main_screen(Database *db, const char *user_id, UserType user_type);

// 物业服务管理界面
void show_service_management_screen(Database *db, const char *user_id, UserType user_type);

// 服务人员信息查询界面
void show_staff_query_screen(Database *db, const char *user_id, UserType user_type);

// 服务人员信息排序界面
void show_staff_sort_screen(Database *db, const char *user_id, UserType user_type);

// 服务人员信息统计界面
void show_staff_statistics_screen(Database *db, const char *user_id, UserType user_type);

// 服务人员系统维护界面
void show_staff_maintenance_screen(Database *db, const char *user_id, UserType user_type);

// 添加这些函数声明
void show_staff_personal_info_screen(Database *db, const char *user_id, UserType user_type);
void modify_personal_info_screen(Database *db, const char *user_id, UserType user_type);

// 添加函数声明
void send_payment_reminder(Database *db, const char *user_id, double unpaid_amount, const char *fee_types);

// 显示缴费信息相关函数声明
void case2_handler(Database *db);
void show_current_fee_standards(Database *db);
void get_current_fee_standards_info(Database *db, char *buffer, size_t buffer_size);

// 用于查询和统计的辅助函数
int query_all_staff(Database *db, Staff *staff_list, int max_count);
int count_all_staff(Database *db);
bool backup_database(Database *db);
bool restore_database(Database *db);

#endif /* UI_STAFF_H */
