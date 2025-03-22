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
