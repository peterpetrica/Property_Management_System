#ifndef USER_H
#define USER_H

#include "db/database.h"
#include "auth/auth.h"
#include <stdbool.h>
#include <time.h>

// 用户信息
typedef struct
{
    char user_id[40];
    char username[64];
    char name[64];
    char phone_number[20];
    char email[64];
    char role_id[40];
    int status;
    time_t registration_date;
} User;

// 业主信息
typedef struct
{
    char user_id[40]; // 对应users表中的user_id
    char username[64];
    char name[64];
    char phone_number[20];
    char email[64];
    bool notification_required;
} Owner;

// 服务人员信息
typedef struct
{
    char staff_id[40]; // 对应staff表中的staff_id
    char user_id[40];  // 对应users表中的user_id
    char name[64];
    char phone_number[20];
    char staff_type_id[40]; // 对应staff_types表中的staff_type_id
    time_t hire_date;
    int status;
} Staff;

// 管理员信息
typedef struct
{
    char user_id[40]; // 对应users表中的user_id
    char username[64];
    char name[64];
} Admin;

// ==================== 用户态 ====================
// 创建业主账户
bool create_owner(Database *db, Owner *owner, const char *password);

// 更新业主信息
bool update_owner(Database *db, const char *user_id, UserType user_type, Owner *owner);

// 获取业主信息
bool get_owner_by_id(Database *db, const char *user_id, Owner *owner);

// 排序业主
void sort_owners(Database *db, int (*compare_func)(const void *, const void *));

// 显示业主列表
void display_owners(Database *db);

// ==================== 服务员 ====================
// 创建服务人员账户
bool create_staff(Database *db, Staff *staff, const char *password);

// 更新服务人员信息
bool update_staff(Database *db, const char *user_id, UserType user_type, Staff *staff);

// 获取服务人员信息
bool get_staff_by_id(Database *db, const char *staff_id, Staff *staff);

// 查询所有服务人员
int query_all_staff(Database *db, Staff *staff_list, int max_count);

// 统计服务人员数量
int count_all_staff(Database *db);

// ==================== 管理员 ====================
// 创建管理员账户
bool create_admin(Database *db, Admin *admin, const char *password);

// 更新管理员信息
bool update_admin(Database *db, const char *user_id, UserType user_type, Admin *admin);

// 获取管理员信息
bool get_admin_by_id(Database *db, const char *admin_id, Admin *admin);

// 删除用户 (仅管理员可用)
bool delete_user(Database *db, const char *admin_id, UserType admin_type, const char *user_id, UserType user_type);

// 通用查询函数
bool query_username_by_user_id(Database *db, const char *user_id, char *username);
bool query_user_id_by_name(Database *db, const char *name, char *user_id);
int compare_id_asc(const void *a, const void *b);

#endif /* USER_H */
