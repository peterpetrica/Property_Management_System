/**
 * ============================================================================
 * 文件名称：ui_main.c
 * 功能描述：物业管理系统的用户界面主要实现文件
 * ============================================================================
 */

#include "ui/ui_main.h"
#include "ui/ui_admin.h"
#include "ui/ui_staff.h"
#include "ui/ui_owner.h"
#include "utils/console.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef _WIN32
#include <conio.h>
#define CLEAR_COMMAND "cls"
#else
#include <unistd.h>
#define CLEAR_COMMAND "clear"
#endif

bool init_ui()
{
    // 简单实现，只是清屏
    system(CLEAR_COMMAND);
    return true;
}

void cleanup_ui()
{
    // 简单实现，不需要特别清理
}

// 清屏
void clear_screen()
{
    system(CLEAR_COMMAND);
}

// 显示标题
void show_title(const char *title)
{
    clear_screen();
    printf("\n===== %s =====\n\n", title);
}

// 主界面
void show_main_screen(Database *db, const char *user_id, UserType user_type)
{
    if (!db || !user_id)
    {
        show_error("系统错误：无效的用户会话");
        return;
    }

    // 根据用户类型显示不同的界面
    switch (user_type)
    {
    case USER_ADMIN:
        show_admin_main_screen(db, user_id, user_type);
        break;
    case USER_STAFF:
        show_staff_main_screen(db, user_id, user_type);
        break;
    case USER_OWNER:
        show_owner_main_screen(db, user_id, user_type);
        break;
    default:
        show_error("未知的用户类型");
        break;
    }
}

// 显示错误信息
void show_error(const char *message)
{
    printf("\n[错误] %s\n", message);
    printf("按任意键继续...");
    getch();
}

// 显示成功信息
void show_success(const char *message)
{
    printf("\n[成功] %s\n", message);
    printf("按任意键继续...");
    getch();
}

// 显示确认对话框
bool show_confirmation(const char *message)
{
    char choice;
    printf("\n%s [y/n]: ", message);
    choice = getch();
    printf("%c\n", choice);
    return (choice == 'y' || choice == 'Y');
}
