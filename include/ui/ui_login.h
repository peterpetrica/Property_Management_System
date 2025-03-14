#ifndef UI_LOGIN_H
#define UI_LOGIN_H

#include "db/database.h"
#include "auth/auth.h"

// 登录界面
LoginResult show_login_screen(Database *db);

// 注册界面
bool show_registration_screen(Database *db);

// 修改密码界面
bool show_change_password_screen(Database *db, const char *token, UserType user_type);

// 重置密码界面 (仅管理员)
bool show_reset_password_screen(Database *db, const char *token);

#endif /* UI_LOGIN_H */
