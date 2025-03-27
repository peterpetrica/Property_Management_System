#ifndef UTILS_H
#define UTILS_H

#include <stdbool.h>
#include <time.h>
#include <stdlib.h>
char *generate_transaction_id(void);
char *get_current_date(void);
// 生成UUID
void generate_uuid(char *out);

// 修改 hash_password 函数声明
bool hash_password(const char *password, char *hashed_output, size_t output_size);

// 验证密码
bool verify_password(const char *password, const char *hash);

// 格式化时间为字符串
char *format_time(time_t time, char *buffer, int buffer_size);

// 从字符串解析时间
time_t parse_time(const char *time_str);

// 安全字符串复制
void safe_strcpy(char *dest, const char *src, size_t dest_size);

/**
 * @brief 去除字符串末尾的换行符
 *
 * @param str 要处理的字符串
 */
void trim_newline(char *str);

#endif /* UTILS_H */
