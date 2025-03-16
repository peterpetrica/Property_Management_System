#ifndef UI_OWNER_H
#define UI_OWNER_H

#include "db/database.h"
#include "auth/auth.h"

// 显示业主主界面
void show_owner_main_screen(Database *db, const char *token);

// 个人信息管理界面
void show_personal_info_screen(Database *db, const char *token);

// 缴费管理界面
void show_payment_management_screen(Database *db, const char *token);

// 业主信息查询界面
void show_owner_query_screen(Database *db, const char *token);

// 业主信息排序界面
void show_owner_sort_screen(Database *db, const char *token);

// 业主信息统计界面
void show_owner_statistics_screen(Database *db, const char *token);

// 业主系统维护界面
void show_owner_maintenance_screen(Database *db, const char *token);

#endif /* UI_OWNER_H */
