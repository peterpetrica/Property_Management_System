#ifndef AUTH_H
#define AUTH_H

#include "db/database.h"
#include <stdbool.h>

// 用户类型枚举
typedef enum
{
    USER_ADMIN = 1, // 管理员
    USER_STAFF = 2, // 物业服务人员
    USER_OWNER = 3  // 业主
} UserType;

// 登录结果结构体
typedef struct
{
    bool success;
    int permission_level; // 与roles表的permission_level对应
    UserType user_type;
    char user_id[32]; // 与users表的user_id对应
} LoginResult;

// 用户认证
LoginResult authenticate_user(Database *db, const char *username, const char *password);

// 验证用户权限
bool validate_permission(Database *db, const char *user_id, UserType user_type, int min_permission_level);

// 更改密码
bool change_password(Database *db, const char *user_id, UserType user_type, const char *old_password, const char *new_password);

// 重置密码 (仅管理员)
bool reset_password(Database *db, const char *admin_id, UserType admin_type, const char *user_id, UserType user_type);

#endif /* AUTH_H */
