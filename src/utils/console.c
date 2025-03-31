/*
 * 控制台工具函数集合
 *
 * 本文件提供了与控制台交互相关的工具函数，包括：
 * 1. 跨平台的getch实现（Windows/Linux）
 * 2. 密码安全输入功能（输入时显示星号，支持退格）
 *
 * 主要用于系统中需要处理控制台输入输出的场景，
 * 特别是需要隐藏密码输入的安全场景。
 */
#include "utils/console.h"
#include "db/db_query.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef _WIN32
#include <conio.h>
#else
#include <termios.h>
#include <unistd.h>
#include <fcntl.h>

/**
 * @brief 在Linux系统下获取单个字符输入，无回显
 *
 * @return int 返回读取的字符ASCII码
 */
int getch(void)
{
    struct termios oldattr, newattr;
    int ch;
    tcgetattr(STDIN_FILENO, &oldattr);
    newattr = oldattr;
    newattr.c_lflag &= ~(ICANON | ECHO);
    tcsetattr(STDIN_FILENO, TCSANOW, &newattr);
    ch = getchar();
    tcsetattr(STDIN_FILENO, TCSANOW, &oldattr);
    return ch;
}
#endif

/**
 * @brief 安全读取密码，输入时显示星号，支持退格键
 *
 * @param password 存储密码的字符数组
 * @param size 密码数组的最大容量
 */
void read_password(char *password, size_t size)
{
    if (!password || size <= 0)
        return;

    size_t i = 0;
    int ch;

    while (i < size - 1 && (ch = getch()) != '\n' && ch != '\r')
    {
        if (ch == 127 || ch == 8)
        {
            if (i > 0)
            {
                i--;
                printf("\b \b");
            }
        }
        else
        {
            password[i++] = ch;
            printf("*");
        }
    }

    password[i] = '\0';
    printf("\n");
}

/**
 * @brief 清屏功能
 */
void clear_screen(void)
{
#ifdef _WIN32
    system("cls");
#else
    system("clear");
#endif
}

/**
 * @brief 暂停控制台执行，等待用户按任意键继续
 */
void pause_console()
{
    printf("\n按任意键继续...");
    getchar();
}

/**
 * @brief 暂停控制台的另一种实现
 */
void wait_for_key() // 修改函数名，避免与系统函数冲突
{
    printf("\n按任意键继续...");
    fflush(stdin);
#ifdef _WIN32
    _getch();
#else
    getchar();
#endif
}

/**
 * @brief 打印查询结果
 *
 * @param result 查询结果指针
 */
void print_query_result(QueryResult *result)
{
    if (!result || result->row_count == 0)
    {
        printf("无查询结果。\n");
        return;
    }

    // 打印表头
    for (int i = 0; i < result->column_count; i++) // 修正成员名
    {
        printf("%-15s", result->column_names[i]); // 修正成员名
    }
    printf("\n");

    // 打印分隔线
    for (int i = 0; i < result->column_count; i++) // 修正成员名
    {
        printf("---------------");
    }
    printf("\n");

    // 打印数据行
    for (int i = 0; i < result->row_count; i++)
    {
        for (int j = 0; j < result->column_count; j++) // 修正成员名
        {
            printf("%-15s", result->rows[i].values[j] ? result->rows[i].values[j] : "NULL");
        }
        printf("\n");
    }

    // 打印总记录数
    printf("\n共 %d 条记录\n", result->row_count);
}
