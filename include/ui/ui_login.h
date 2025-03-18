#ifndef UI_LOGIN_H
#define UI_LOGIN_H

#include "db/database.h"
#include "auth/auth.h"

// 登录界面
LoginResult show_login_screen(Database *db);

// 注册界面
bool show_registration_screen(Database *db);

#endif /* UI_LOGIN_H */
