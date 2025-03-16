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
#include <stdio.h>
#include <string.h>

#ifdef _WIN32
#include <conio.h>
#else
#include <termios.h>
#include <unistd.h>
#include <fcntl.h>

// Linux下所需的getch函数实现
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

// 读取密码（不回显）
void read_password(char *password, size_t size)
{
    if (!password || size <= 0)
        return;

    size_t i = 0;
    int ch;

    while (i < size - 1 && (ch = getch()) != '\n' && ch != '\r')
    {
        if (ch == 127 || ch == 8) // 退格键
        {
            if (i > 0)
            {
                i--;
                printf("\b \b"); // 退格，空格，再退格
            }
        }
        else
        {
            password[i++] = ch;
            printf("*"); // 显示星号
        }
    }

    password[i] = '\0';
    printf("\n");
}
