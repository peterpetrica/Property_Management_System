#ifndef UI_LOGIN_H
#define UI_LOGIN_H

#include "db/database.h"
#include "auth/auth.h"

// 添加前向声明
void show_admin_main_screen(Database *db, const char *user_id, UserType user_type);
void show_staff_main_screen(Database *db, const char *user_id, UserType user_type);
void show_owner_main_screen(Database *db, const char *user_id, UserType user_type);

// 登录界面
LoginResult show_login_screen(Database *db);

// 注册界面
bool show_registration_screen(Database *db);

#endif /* UI_LOGIN_H */
