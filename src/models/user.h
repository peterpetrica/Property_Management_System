#ifndef USER_H
#define USER_H

#include "db/database.h"
#include "auth/auth.h"

// Staff结构体定义
typedef struct
{
    char staff_id[37];
    char user_id[37];
    char name[100];
    char phone_number[20];
    char staff_type_id[37];
} Staff;

// Owner结构体定义
typedef struct
{
    char user_id[37];
    char username[100];
    char name[100];
    char phone_number[20];
    char email[100];
} Owner;

// Admin结构体定义
typedef struct
{
    char user_id[37];
    char username[100];
    char name[100];
} Admin;

// 用户类型枚举
typedef enum
{
    USER_ADMIN,
    USER_STAFF,
    USER_OWNER
} UserType;

// 服务人员相关函数声明
bool create_staff(Database *db, Staff *staff, const char *password);
bool update_staff(Database *db, const char *user_id, UserType user_type, Staff *staff);
bool get_staff_by_id(Database *db, const char *staff_id, Staff *staff);
int query_all_staff(Database *db, Staff *staff_list, int max_count);
int count_all_staff(Database *db);

// 业主相关函数声明
bool create_owner(Database *db, Owner *owner, const char *password);
bool update_owner(Database *db, const char *user_id, UserType user_type, Owner *owner);
bool get_owner_by_id(Database *db, const char *owner_id, Owner *owner);

// 管理员相关函数声明
bool create_admin(Database *db, Admin *admin, const char *password);
bool update_admin(Database *db, const char *user_id, UserType user_type, Admin *admin);
bool get_admin_by_id(Database *db, const char *admin_id, Admin *admin);

// 用户删除函数声明
bool delete_user(Database *db, const char *admin_id, UserType admin_type, const char *user_id, UserType user_type);

// 用户查询函数声明
bool query_username_by_user_id(Database *db, const char *user_id, char *username);
bool query_user_id_by_name(Database *db, const char *name, char *user_id);

// 业主排序和显示函数声明
int compare_id_asc(const void *a, const void *b);
void sort_owners(Database *db, int (*compare_func)(const void *, const void *));
void display_owners(Database *db);

#endif /* USER_H */