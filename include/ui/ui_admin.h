/**
 * @file ui_admin.h
 * @brief 物业管理系统管理员界面功能声明
 *
 * 该头文件声明了物业管理系统中管理员界面相关的所有功能函数，
 * 包括管理员主界面、信息管理、服务分配、信息查询等各类功能。
 */
#ifndef UI_ADMIN_H
#define UI_ADMIN_H

#include "db/database.h"
#include "auth/auth.h"

/**
 * @brief 显示费用标准管理界面
 *
 * @param db 数据库连接指针
 * @param user_id 当前登录用户的ID
 * @param user_type 当前登录用户的类型
 */
void manage_fee_standards_screen(Database *db, const char *user_id, UserType user_type);

/**
 * @brief 显示信息管理界面
 *
 * @param db 数据库连接指针
 * @param user_id 当前登录用户的ID
 * @param user_type 当前登录用户的类型
 */
void show_info_management_screen(Database *db, const char *user_id, UserType user_type);

/**
 * @brief 显示服务分配界面
 *
 * @param db 数据库连接指针
 * @param user_id 当前登录用户的ID
 * @param user_type 当前登录用户的类型
 */
void show_service_assignment_screen(Database *db, const char *user_id, UserType user_type);

/**
 * @brief 显示信息查询界面
 *
 * @param db 数据库连接指针
 * @param user_id 当前登录用户的ID
 * @param user_type 当前登录用户的类型
 */
void show_info_query_screen(Database *db, const char *user_id, UserType user_type);

/**
 * @brief 显示信息排序界面
 *
 * @param db 数据库连接指针
 * @param user_id 当前登录用户的ID
 * @param user_type 当前登录用户的类型
 */
void show_info_sort_screen(Database *db, const char *user_id, UserType user_type);

/**
 * @brief 显示信息统计界面
 *
 * @param db 数据库连接指针
 * @param user_id 当前登录用户的ID
 * @param user_type 当前登录用户的类型
 */
void show_info_statistics_screen(Database *db, const char *user_id, UserType user_type);

/**
 * @brief 显示系统维护界面
 *
 * @param db 数据库连接指针
 * @param user_id 当前登录用户的ID
 * @param user_type 当前登录用户的类型
 */
void show_system_maintenance_screen(Database *db, const char *user_id, UserType user_type);

/**
 * @brief 显示房屋管理测试界面
 *
 * @param db 数据库连接指针
 * @param user_id 当前登录用户的ID
 * @param user_type 当前登录用户的类型
 */
void show_apartment_test_screen(Database *db, const char *user_id, UserType user_type);

/**
 * @brief 显示楼宇管理测试界面
 *
 * @param db 数据库连接指针
 * @param user_id 当前登录用户的ID
 * @param user_type 当前登录用户的类型
 */
void show_building_test_screen(Database *db, const char *user_id, UserType user_type);

/**
 * @brief 显示管理员主界面
 *
 * 展示所有管理员可用的功能选项并处理用户的选择
 *
 * @param db 数据库连接指针
 * @param user_id 当前登录用户的ID
 * @param user_type 当前登录用户的类型
 */
void show_admin_main_screen(Database *db, const char *user_id, UserType user_type);

// 以下函数声明是重复的，保持原样

void show_info_management_screen(Database *db, const char *user_id, UserType user_type);
void show_service_assignment_screen(Database *db, const char *user_id, UserType user_type);
void show_info_query_screen(Database *db, const char *user_id, UserType user_type);
void show_info_sort_screen(Database *db, const char *user_id, UserType user_type);
void show_info_statistics_screen(Database *db, const char *user_id, UserType user_type);
void show_system_maintenance_screen(Database *db, const char *user_id, UserType user_type);

#endif /* UI_ADMIN_H */
