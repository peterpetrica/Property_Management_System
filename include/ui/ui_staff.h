#ifndef UI_STAFF_H
#define UI_STAFF_H

#include "db/database.h"
#include "auth/auth.h"
#include <stddef.h>

// 主界面函数
void show_staff_main_screen(Database *db, const char *user_id, UserType user_type);

// 区域管理相关函数
void show_area_management_menu(Database *db, const char *user_id);
void show_staff_areas(Database *db, const char *staff_id);
void show_area_owners(Database *db, const char *user_id);

// 查询相关函数
void show_staff_query_screen(Database *db, const char *user_id, UserType user_type);
void show_user_query_menu(Database *db);
void case2_handler(Database *db);
void query_owner_payment_by_year(Database *db, const char *owner_name, int year);
void query_owner_all_payments(Database *db, const char *owner_name);
void query_user_by_name(Database *db);
void query_user_by_room(Database *db);
void query_user_payment_status(Database *db);
void show_owner_payment_status(Database *db);
void show_user_detail(Database *db, const char *user_id);
void show_owner_payment_query(Database *db);

// 排序相关函数
void show_sorted_owners_menu(Database *db);
void show_sorted_owners_by(Database *db, const char *sort_criteria);
void show_staff_sort_screen(Database *db, const char *user_id, UserType user_type);
void show_all_users(Database *db);
void show_paid_users(Database *db);
void show_unpaid_users(Database *db);

// 统计相关函数
void show_statistics_menu(Database *db);
void show_payment_statistics(Database *db);
void show_yearly_statistics(Database *db, int year);
void show_current_statistics(Database *db);
void show_unpaid_analysis(Database *db);

// 缴费提醒相关函数
void send_payment_reminders(Database *db);
void send_payment_reminder_screen(Database *db, const char *user_id);
void send_bulk_payment_reminders(Database *db);
void send_payment_reminder(Database *db, const char *user_id, double unpaid_amount, const char *fee_types);
void show_payment_reminder_menu(Database *db, const char *user_id);
void show_reminder_history(Database *db);
// 缴费管理相关函数
void show_payment_management_menu(Database *db);
void show_current_fee_standards(Database *db);
void get_current_fee_standards_info(Database *db, char *buffer, size_t buffer_size);

// 系统维护相关函数
void show_staff_maintenance_screen(Database *db, const char *user_id, UserType user_type);
void modify_personal_info_screen(Database *db, const char *user_id, UserType user_type);

// 工具函数
void clear_staff_screen(void);
int wait_for_user(void);
// 新增函数声明
void show_payment_details(Database *db, const char *user_id, int status);
#endif /* UI_STAFF_H */