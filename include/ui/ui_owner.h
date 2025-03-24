#ifndef UI_OWNER_H
#define UI_OWNER_H

#include "db/database.h"
#include "auth/auth.h"

/**
 * @brief 根据用户ID查询用户名
 *
 * @param db 数据库指针
 * @param user_id 用户ID
 * @param username 用于存储查询到的用户名的缓冲区
 * @return true 查询成功
 * @return false 查询失败
 */
bool query_username_by_user_id(Database *db, const char *user_id, char *username);

/**
 * @brief 查询用户的待缴费用
 *
 * @param db 数据库指针
 * @param user_id 用户ID
 */
void query_due_payments(Database *db, const char *user_id);

/**
 * @brief 更新逾期交易记录
 *
 * @param db 数据库指针
 * @return true 更新成功
 * @return false 更新失败
 */
bool update_overdue_transactions(Database *db);

/**
 * @brief 显示业主主界面
 *
 * @param db 数据库指针
 * @param user_id 用户ID
 * @param user_type 用户类型
 */
void show_owner_main_screen(Database *db, const char *user_id, UserType user_type);

/**
 * @brief 业主主界面功能入口
 *
 * @param db 数据库指针
 * @param user_id 用户ID
 * @param user_type 用户类型
 */
void main_screen_owner(Database *db, const char *user_id, UserType user_type);

/**
 * @brief 业主个人信息管理界面
 *
 * @param db 数据库指针
 * @param user_id 用户ID
 * @param user_type 用户类型
 */
void show_owner_personal_info_screen(Database *db, const char *user_id, UserType user_type);

/**
 * @brief 缴费管理界面
 *
 * @param db 数据库指针
 * @param user_id 用户ID
 * @param user_type 用户类型
 */
void show_payment_management_screen(Database *db, const char *user_id, UserType user_type);

/**
 * @brief 业主信息查询界面
 *
 * @param db 数据库指针
 * @param user_id 用户ID
 * @param user_type 用户类型
 */
void show_owner_query_screen(Database *db, const char *user_id, UserType user_type);

/**
 * @brief 业主信息排序界面
 *
 * @param db 数据库指针
 * @param user_id 用户ID
 * @param user_type 用户类型
 */
void show_owner_sort_screen(Database *db, const char *user_id, UserType user_type);

/**
 * @brief 业主信息统计界面
 *
 * @param db 数据库指针
 * @param user_id 用户ID
 * @param user_type 用户类型
 */
void show_owner_statistics_screen(Database *db, const char *user_id, UserType user_type);

/**
 * @brief 业主系统维护界面
 *
 * @param db 数据库指针
 * @param user_id 用户ID
 * @param user_type 用户类型
 */
void show_owner_maintenance_screen(Database *db, const char *user_id, UserType user_type);

/**
 * @brief 按业主ID升序比较函数
 *
 * @param a 第一个比较对象
 * @param b 第二个比较对象
 * @return int 比较结果：负数表示a<b，零表示a=b，正数表示a>b
 */
int compare_owner_id_asc(const void *a, const void *b);

/**
 * @brief 按业主ID降序比较函数
 *
 * @param a 第一个比较对象
 * @param b 第二个比较对象
 * @return int 比较结果：负数表示a>b，零表示a=b，正数表示a<b
 */
int compare_owner_id_desc(const void *a, const void *b);

/**
 * @brief 按业主姓名升序比较函数
 *
 * @param a 第一个比较对象
 * @param b 第二个比较对象
 * @return int 比较结果：负数表示a<b，零表示a=b，正数表示a>b
 */
int compare_owner_name_asc(const void *a, const void *b);

/**
 * @brief 按业主姓名降序比较函数
 *
 * @param a 第一个比较对象
 * @param b 第二个比较对象
 * @return int 比较结果：负数表示a>b，零表示a=b，正数表示a<b
 */
int compare_owner_name_desc(const void *a, const void *b);

#endif /* UI_OWNER_H */
