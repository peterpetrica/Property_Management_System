/**
 * @file console.h
 * @brief 控制台交互相关的实用函数声明
 */
#ifndef CONSOLE_H
#define CONSOLE_H

#include <stddef.h>
#include "db/db_query.h"

// 读取密码（不回显）
void read_password(char *password, size_t size);

// 等待单个字符输入（无需按回车）
#ifndef _WIN32
int getch(void);
#endif

/**
 * @brief 暂停控制台执行，等待用户按任意键继续
 */
void pause_console();

/**
 * @brief 暂停控制台执行的另一种实现
 */
void wait_for_key(); // 修改函数名，避免与系统函数冲突

/**
 * @brief 打印查询结果
 *
 * @param result 查询结果指针
 */
void print_query_result(QueryResult *result);

#endif /* CONSOLE_H */
