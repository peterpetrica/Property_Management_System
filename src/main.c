/*
 * 物业管理系统主程序入口
 *
 * 功能：
 * - 初始化用户界面
 * - 设置和检查数据存储目录
 * - 初始化数据库连接
 * - 创建并初始化数据库表结构
 * - 显示登录界面
 * - 登录成功后进入主界面
 * - 退出时清理资源
 *
 * 文件: main.c
 * 作用: 程序入口点和主控制流程
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "db/database.h"
#include "db/db_init.h"
#include "auth/auth.h"
#include "ui/ui_login.h"
#include "utils/file_ops.h"

#define DB_FILENAME "property_management.db"

int main()
{

    // 数据库保存的地方
    char data_dir[256];
    if (!get_data_dir(data_dir, sizeof(data_dir)))
    {
        fprintf(stderr, "无法获取数据目录\n");
        return 1;
    }

    // 如不存在则创建
    if (!file_exists(data_dir))
    {
        if (!create_directory(data_dir))
        {
            fprintf(stderr, "无法创建数据目录: %s\n", data_dir);
            return 1;
        }
    }

    // 数据库的路径
    char db_path[512];
    snprintf(db_path, sizeof(db_path), "%s/%s", data_dir, DB_FILENAME);

    // 初始化数据库
    Database db;
    if (db_init(&db, db_path) != SQLITE_OK)
    {
        fprintf(stderr, "无法初始化数据库\n");
        return 1;
    }

    LoginResult login_result = show_login_screen(&db);

    if (login_result.success)
    {
        // 主界面
        switch (login_result.user_type)
        {
        case USER_ADMIN:
            show_admin_main_screen(db, login_result.user_id, login_result.user_type);
            break;
        case USER_STAFF:
            show_staff_main_screen(db, login_result.user_id, login_result.user_type);
            break;
        case USER_OWNER:
            show_owner_main_screen(db, login_result.user_id, login_result.user_type);
            break;
        default:
            puts("未知的用户类型");
            break;
        }
    }
    else
    {
        printf("登录失败，程序退出\n");
    }

    // 清理资源
    db_close(&db);

    return 0;
}