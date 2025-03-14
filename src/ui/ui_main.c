#include "ui/ui_main.h"
#include "ui/ui_admin.h"
#include "ui/ui_staff.h"
#include "ui/ui_owner.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef _WIN32
#include <conio.h>
#define CLEAR_COMMAND "cls"
#else
#include <termios.h>
#include <unistd.h>
#define CLEAR_COMMAND "clear"

// Linux平台下的getch实现
static int getch()
{
    struct termios oldt, newt;
    int ch;
    tcgetattr(STDIN_FILENO, &oldt);
    newt = oldt;
    newt.c_lflag &= ~(ICANON | ECHO);
    tcsetattr(STDIN_FILENO, TCSANOW, &newt);
    ch = getchar();
    tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
    return ch;
}
#endif

// 初始化UI
bool init_ui()
{
    // 初始化UI相关资源，目前无需特殊初始化
    return true;
}

// 清理UI资源
void cleanup_ui()
{
    // 清理UI相关资源，目前无需特殊清理
}

// 显示主界面
void show_main_screen(Database *db, const char *token, UserType user_type)
{
    switch (user_type)
    {
    case USER_ADMIN:
        show_admin_main_screen(db, token);
        break;
    case USER_STAFF:
        show_staff_main_screen(db, token);
        break;
    case USER_OWNER:
        show_owner_main_screen(db, token);
        break;
    default:
        show_error("未知的用户类型");
    }
}

// 显示错误信息
void show_error(const char *message)
{
    printf("\n错误: %s\n", message);
    printf("按任意键继续...");
    getch();
}

// 显示成功信息
void show_success(const char *message)
{
    printf("\n%s\n", message);
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

// 清屏
void clear_screen()
{
    system(CLEAR_COMMAND);
}

// 显示标题
void show_title(const char *title)
{
    clear_screen();
    printf("===== %s =====\n\n", title);
}
