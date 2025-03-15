#ifndef USER_H
#define USER_H

#include "db/database.h"
#include "auth/auth.h"
#include <stdbool.h>

// 业主信息
typedef struct
{
    char id[32];
    char name[64];
    char phone[20];
    char building_id[32];
    char apartment_id[32];
    bool notification_required;
    int weight;
} Owner;

// 服务人员信息
typedef struct
{
    char id[32];
    char name[64];
    char phone[20];
    char service_type[32]; // 管家、保安、清洁工等
    int weight;
} Staff;

// 管理员信息
typedef struct
{
    char id[32];
    char name[64];
    int weight;
} Admin;

// 创建业主账户
bool create_owner(Database *db, Owner *owner, const char *password);

// 更新业主信息
bool update_owner(Database *db, const char *token, Owner *owner);

// 获取业主信息
bool get_owner_by_id(Database *db, const char *owner_id, Owner *owner);

// 创建服务人员账户
bool create_staff(Database *db, Staff *staff, const char *password);

// 更新服务人员信息
bool update_staff(Database *db, const char *token, Staff *staff);

// 获取服务人员信息
bool get_staff_by_id(Database *db, const char *staff_id, Staff *staff);

// 创建管理员账户
bool create_admin(Database *db, Admin *admin, const char *password);

// 更新管理员信息
bool update_admin(Database *db, const char *token, Admin *admin);

// 获取管理员信息
bool get_admin_by_id(Database *db, const char *admin_id, Admin *admin);

// 删除用户 (仅管理员可用)
bool delete_user(Database *db, const char *admin_token, const char *user_id, UserType user_type);

#endif /* USER_H */
