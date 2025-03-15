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
