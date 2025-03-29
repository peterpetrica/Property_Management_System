#ifndef UI_STAFF_H
#define UI_STAFF_H

#include "db/database.h"
#include "auth/auth.h"
#include "models/user.h"

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

#endif /* UI_STAFF_H */
