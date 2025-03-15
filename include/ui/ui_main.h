#ifndef UI_MAIN_H
#define UI_MAIN_H

#include "db/database.h"
#include "auth/auth.h"

// 初始化UI
bool init_ui();

// 清理UI资源
void cleanup_ui();

// 显示主界面
void show_main_screen(Database *db, const char *token, UserType user_type);

// 显示错误信息
void show_error(const char *message);

// 显示成功信息
void show_success(const char *message);

// 显示确认对话框
bool show_confirmation(const char *message);

#endif /* UI_MAIN_H */
