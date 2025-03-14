#ifndef UI_ADMIN_H
#define UI_ADMIN_H

#include "db/database.h"
#include "auth/auth.h"

// 显示管理员主界面
void show_admin_main_screen(Database *db, const char *token);

// 信息管理界面
void show_info_management_screen(Database *db, const char *token);

// 服务分配界面
void show_service_assignment_screen(Database *db, const char *token);

// 信息查询界面
void show_info_query_screen(Database *db, const char *token);

// 信息排序界面
void show_info_sort_screen(Database *db, const char *token);

// 信息统计界面
void show_info_statistics_screen(Database *db, const char *token);

// 系统维护界面
void show_system_maintenance_screen(Database *db, const char *token);

#endif /* UI_ADMIN_H */
