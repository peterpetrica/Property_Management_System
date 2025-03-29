#ifndef UI_STAFF_H
#define UI_STAFF_H

#include "db/database.h"
#include "auth/auth.h"
#include "models/user.h"
#include "models/transaction.h"  // 添加此行

// 主界面
void show_staff_main_screen(Database *db, const char *user_id, UserType user_type);

// 核心功能
void show_staff_service_areas(Database *db, const char *staff_id);
void show_payment_reminder_menu(Database *db, const char *user_id);
void show_owner_payment_query(Database *db);
void show_sorted_owners(Database *db);
void show_payment_statistics(Database *db);

// 辅助功能
void send_payment_reminder(Database *db, const char *user_id, double amount, const char *fee_types);
void send_bulk_payment_reminders(Database *db);
void query_owner_payment_by_year(Database *db, const char *owner_name, int year);
void query_owner_all_payments(Database *db, const char *owner_name);
void show_payment_status_statistics(Database *db);

// 添加缺失的函数声明
void case2_handler(Database *db);
void query_user_by_name(Database *db);
void query_user_by_room(Database *db);
void query_user_payment_status(Database *db);
void show_all_users(Database *db);
void show_paid_users(Database *db);
void show_unpaid_users(Database *db);
void show_yearly_statistics(Database *db, int year);
void show_current_statistics(Database *db);
void show_unpaid_analysis(Database *db);
void get_current_fee_standards_info(Database *db, char *buffer, size_t buffer_size);
bool get_current_fee_standard(Database *db, int fee_type, FeeStandard *standard);

#endif /* UI_STAFF_H */
