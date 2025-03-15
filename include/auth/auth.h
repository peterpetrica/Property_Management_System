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
    char token[64];
    int weight;
    UserType user_type;
    char user_id[32];
} LoginResult;

// 用户认证
LoginResult authenticate_user(Database *db, const char *username, const char *password);

// 验证token
bool validate_token(Database *db, const char *token, int *weight, UserType *user_type);

// 更改密码
bool change_password(Database *db, const char *user_id, UserType user_type, const char *old_password, const char *new_password);

// 重置密码 (仅管理员)
bool reset_password(Database *db, const char *admin_token, const char *user_id, UserType user_type);

// 注销token
bool invalidate_token(Database *db, const char *token);

#endif /* AUTH_H */
