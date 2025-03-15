#ifndef CONSOLE_H
#define CONSOLE_H

#include <stddef.h>

// 读取密码（不回显）
void read_password(char *password, size_t size);

// 等待单个字符输入（无需按回车）
#ifndef _WIN32
int getch(void);
#endif

#endif /* CONSOLE_H */
