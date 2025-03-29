#ifndef UI_STAFF_H
#define UI_STAFF_H

#include "db/database.h"
#include "auth/auth.h"
#include <stddef.h>

// 主界面函数
void show_staff_main_screen(Database *db, const char *user_id, UserType user_type);

// 查询相关函数
void show_staff_query_screen(Database *db, const char *user_id, UserType user_type);
void case2_handler(Database *db);
void query_owner_payment_by_year(Database *db, const char *owner_name, int year);
void query_owner_all_payments(Database *db, const char *owner_name);
void query_user_by_name(Database *db);
void query_user_by_room(Database *db);
void query_user_payment_status(Database *db);
void show_user_query_menu(Database *db);
void show_owner_payment_status(Database *db);

// 统计相关函数
void show_payment_statistics(Database *db);
void show_yearly_statistics(Database *db, int year);
void show_current_statistics(Database *db);
void show_unpaid_analysis(Database *db);
void show_statistics_menu(Database *db);

// 用户显示相关函数
void show_all_users(Database *db);
void show_paid_users(Database *db);
void show_unpaid_users(Database *db);
void show_sorted_owners(Database *db);

// 其他辅助函数
void display_owners(Database *db);
void show_assigned_areas(Database *db, const char *staff_id);
void send_payment_reminders(Database *db);
void send_payment_reminder(Database *db, const char *user_id, double unpaid_amount, const char *fee_types);
void get_current_fee_standards_info(Database *db, char *buffer, size_t buffer_size);

#endif /* UI_STAFF_H */
