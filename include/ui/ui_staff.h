#ifndef UI_STAFF_H
#define UI_STAFF_H

#include "db/database.h"
#include "auth/auth.h"

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

#endif /* UI_STAFF_H */
