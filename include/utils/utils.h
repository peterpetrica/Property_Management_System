#ifndef UTILS_H
#define UTILS_H

#include <stdbool.h>
#include <time.h>

// 生成UUID
char *generate_uuid();

// 加密密码
char *hash_password(const char *password);

// 验证密码
bool verify_password(const char *password, const char *hash);

// 格式化时间为字符串
char *format_time(time_t time, char *buffer, int buffer_size);

// 从字符串解析时间
time_t parse_time(const char *time_str);

// 安全字符串复制
void safe_strcpy(char *dest, const char *src, size_t dest_size);

#endif /* UTILS_H */
