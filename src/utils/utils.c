/*******************************************************************************
 * 工具函数库 - 实现文件
 *
 * 该文件提供了系统中使用的各种通用工具函数的实现：
 * - UUID生成：生成唯一标识符
 * - 密码处理：密码加密、盐值生成及密码验证
 * - 时间处理：时间格式化与解析
 * - 字符串处理：安全的字符串复制
 *
 * 这些功能为物业管理系统提供基础支持功能和安全保障。
 ******************************************************************************/

#include "utils/utils.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <openssl/sha.h>
#include <openssl/evp.h>
#include <openssl/rand.h>

#ifdef __linux__
#include <uuid/uuid.h>
#endif

/**
 * @brief 生成UUID字符串
 *
 * @param out 输出缓冲区，长度至少为37字节(36个字符+结束符)
 */
void generate_uuid(char *out)
{
    static const char *chars = "0123456789abcdef";
    static bool seeded = false;

    if (!seeded)
    {
        srand((unsigned int)time(NULL));
        seeded = true;
    }

    for (int i = 0; i < 36; i++)
    {
        if (i == 8 || i == 13 || i == 18 || i == 23)
        {
            out[i] = '-';
        }
        else if (i == 14)
        {
            out[i] = '4';
        }
        else if (i == 19)
        {
            out[i] = chars[8 + (rand() % 4)];
        }
        else
        {
            out[i] = chars[rand() % 16];
        }
    }

    out[36] = '\0';
}

/**
 * @brief 生成随机盐值
 *
 * @param length 盐值长度
 * @return char* 生成的盐值字符串，调用者负责释放内存
 */
static char *generate_salt(size_t length)
{
    const char charset[] = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789./";
    const size_t charset_size = sizeof(charset) - 1;

    char *salt = (char *)malloc(length + 1);
    if (!salt)
        return NULL;

    unsigned char *random_bytes = (unsigned char *)malloc(length);
    if (RAND_bytes(random_bytes, length) != 1)
    {
        free(salt);
        return NULL;
    }

    for (size_t i = 0; i < length; i++)
    {
        salt[i] = charset[random_bytes[i] % charset_size];
    }
    salt[length] = '\0';

    return salt;
}

/**
 * @brief 存储密码（当前为明文存储）
 *
 * @param password 原始密码
 * @param hashed_output 输出缓冲区
 * @param output_size 缓冲区大小
 * @return bool 操作是否成功
 */
bool hash_password(const char *password, char *hashed_output, size_t output_size)
{
    if (!password || !hashed_output || output_size <= 0)
    {
        return false;
    }

    strncpy(hashed_output, password, output_size - 1);
    hashed_output[output_size - 1] = '\0';

    return true;
}

/**
 * @brief 验证密码
 *
 * @param password 待验证的密码
 * @param hash 存储的密码（当前为明文）
 * @return bool 密码是否匹配
 */
bool verify_password(const char *password, const char *hash)
{
    if (!password || !hash)
        return false;

    return (strcmp(password, hash) == 0);
}

/**
 * @brief 格式化时间为字符串
 *
 * @param time_val 时间值
 * @param buffer 输出缓冲区
 * @param buffer_size 缓冲区大小
 * @return char* 格式化后的时间字符串，失败返回NULL
 */
char *format_time(time_t time_val, char *buffer, int buffer_size)
{
    if (!buffer || buffer_size <= 0)
        return NULL;

    struct tm *tm_info = localtime(&time_val);
    if (!tm_info)
    {
        buffer[0] = '\0';
        return buffer;
    }

    strftime(buffer, buffer_size, "%Y-%m-%d %H:%M:%S", tm_info);
    return buffer;
}

/**
 * @brief 从字符串解析时间
 *
 * @param time_str 时间字符串，格式为"YYYY-MM-DD HH:MM:SS"或"YYYY-MM-DD"
 * @return time_t 解析后的时间，失败返回-1
 */
time_t parse_time(const char *time_str)
{
    if (!time_str)
        return (time_t)-1;

    struct tm tm_info = {0};

    if (sscanf(time_str, "%d-%d-%d %d:%d:%d",
               &tm_info.tm_year, &tm_info.tm_mon, &tm_info.tm_mday,
               &tm_info.tm_hour, &tm_info.tm_min, &tm_info.tm_sec) == 6)
    {
        tm_info.tm_year -= 1900;
        tm_info.tm_mon -= 1;
        return mktime(&tm_info);
    }

    if (sscanf(time_str, "%d-%d-%d",
               &tm_info.tm_year, &tm_info.tm_mon, &tm_info.tm_mday) == 3)
    {
        tm_info.tm_year -= 1900;
        tm_info.tm_mon -= 1;
        return mktime(&tm_info);
    }

    return (time_t)-1;
}

/**
 * @brief 安全字符串复制
 *
 * @param dest 目标字符串
 * @param src 源字符串
 * @param dest_size 目标缓冲区大小
 */
void safe_strcpy(char *dest, const char *src, size_t dest_size)
{
    if (!dest || !src || dest_size <= 0)
        return;

    size_t i;
    for (i = 0; i < dest_size - 1 && src[i]; i++)
    {
        dest[i] = src[i];
    }
    dest[i] = '\0';
}

/**
 * @brief 去除字符串末尾的换行符
 *
 * @param str 要处理的字符串
 */
void trim_newline(char *str)
{
    if (!str)
        return;

    int len = strlen(str);
    if (len > 0 && str[len - 1] == '\n')
    {
        str[len - 1] = '\0';
    }
}

/**
 * @brief 清除输入缓冲区
 */
void clear_input_buffer(void) {
    int c;
    while ((c = getchar()) != '\n' && c != EOF);
}

/**
 * @brief 暂停程序执行，等待用户按任意键继续
 */
void pause(void) {
    printf("\n按任意键继续...");
    getchar();
}
