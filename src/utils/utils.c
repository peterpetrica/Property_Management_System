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


void generate_uuid(char *out)
{
    static const char *chars = "0123456789abcdef";
    static bool seeded = false;

    if (!seeded)
    {
        srand((unsigned int)time(NULL));
        seeded = true;
    }

    // 生成UUID的各部分
    // 格式：8-4-4-4-12
    for (int i = 0; i < 36; i++)
    {
        if (i == 8 || i == 13 || i == 18 || i == 23)
        {
            out[i] = '-';
        }
        // 版本号 (UUID v4使用4)
        else if (i == 14)
        {
            out[i] = '4';
        }
        // 变体位 (UUID v4对应8, 9, A, B的任意一个)
        else if (i == 19)
        {
            out[i] = chars[8 + (rand() % 4)];
        }
        else
        {
            out[i] = chars[rand() % 16];
        }
    }

    // 添加结束符
    out[36] = '\0';
}


// 生成随机盐值
static char *generate_salt(size_t length)
{
    const char charset[] = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789./";
    const size_t charset_size = sizeof(charset) - 1;

    char *salt = (char *)malloc(length + 1);
    if (!salt)
        return NULL;

    // 使用OpenSSL的随机数生成器
    unsigned char *random_bytes = (unsigned char *)malloc(length);
    if (RAND_bytes(random_bytes, length) != 1)
    {
        free(salt);
        return NULL;
    }

    // 将随机字节映射到字符集
    for (size_t i = 0; i < length; i++)
    {
        salt[i] = charset[random_bytes[i] % charset_size];
    }
    salt[length] = '\0';

    return salt;
}

// 加密密码
char *hash_password(const char *password)
{
    if (!password)
        return NULL;

    // 生成盐值
    char *salt = generate_salt(16);
    if (!salt)
        return NULL;

    // 创建哈希上下文
    EVP_MD_CTX *mdctx = EVP_MD_CTX_new();
    if (!mdctx)
    {
        free(salt);
        return NULL;
    }

    // 初始化哈希上下文
    if (EVP_DigestInit_ex(mdctx, EVP_sha256(), NULL) != 1)
    {
        EVP_MD_CTX_free(mdctx);
        free(salt);
        return NULL;
    }

    // 更新哈希上下文 (添加盐值)
    if (EVP_DigestUpdate(mdctx, salt, strlen(salt)) != 1)
    {
        EVP_MD_CTX_free(mdctx);
        free(salt);
        return NULL;
    }

    // 更新哈希上下文 (添加密码)
    if (EVP_DigestUpdate(mdctx, password, strlen(password)) != 1)
    {
        EVP_MD_CTX_free(mdctx);
        free(salt);
        return NULL;
    }

    // 完成哈希计算
    unsigned char hash[EVP_MAX_MD_SIZE];
    unsigned int hash_len;
    if (EVP_DigestFinal_ex(mdctx, hash, &hash_len) != 1)
    {
        EVP_MD_CTX_free(mdctx);
        free(salt);
        return NULL;
    }

    // 释放哈希上下文
    EVP_MD_CTX_free(mdctx);

    // 分配结果缓冲区 (盐值 + $ + 哈希的十六进制表示)
    char *result = (char *)malloc(strlen(salt) + 1 + hash_len * 2 + 1);
    if (!result)
    {
        free(salt);
        return NULL;
    }

    // 复制盐值
    strcpy(result, salt);
    strcat(result, "$");

    // 添加哈希的十六进制表示
    char *p = result + strlen(salt) + 1;
    for (unsigned int i = 0; i < hash_len; i++)
    {
        sprintf(p + i * 2, "%02x", hash[i]);
    }

    free(salt);
    return result;
}

// 验证密码
bool verify_password(const char *password, const char *hash)
{
    if (!password || !hash)
        return false;

    // 查找分隔符
    const char *sep = strchr(hash, '$');
    if (!sep)
        return false;

    // 提取盐值
    size_t salt_len = sep - hash;
    char *salt = (char *)malloc(salt_len + 1);
    if (!salt)
        return false;
    strncpy(salt, hash, salt_len);
    salt[salt_len] = '\0';

    // 创建哈希上下文
    EVP_MD_CTX *mdctx = EVP_MD_CTX_new();
    if (!mdctx)
    {
        free(salt);
        return false;
    }

    // 初始化哈希上下文
    if (EVP_DigestInit_ex(mdctx, EVP_sha256(), NULL) != 1)
    {
        EVP_MD_CTX_free(mdctx);
        free(salt);
        return false;
    }

    // 更新哈希上下文 (添加盐值)
    if (EVP_DigestUpdate(mdctx, salt, strlen(salt)) != 1)
    {
        EVP_MD_CTX_free(mdctx);
        free(salt);
        return false;
    }

    // 更新哈希上下文 (添加密码)
    if (EVP_DigestUpdate(mdctx, password, strlen(password)) != 1)
    {
        EVP_MD_CTX_free(mdctx);
        free(salt);
        return false;
    }

    // 完成哈希计算
    unsigned char calc_hash[EVP_MAX_MD_SIZE];
    unsigned int hash_len;
    if (EVP_DigestFinal_ex(mdctx, calc_hash, &hash_len) != 1)
    {
        EVP_MD_CTX_free(mdctx);
        free(salt);
        return false;
    }

    // 释放哈希上下文
    EVP_MD_CTX_free(mdctx);

    // 分配缓冲区以保存计算出的哈希的十六进制表示
    char *calc_hash_hex = (char *)malloc(hash_len * 2 + 1);
    if (!calc_hash_hex)
    {
        free(salt);
        return false;
    }

    // 生成十六进制表示
    for (unsigned int i = 0; i < hash_len; i++)
    {
        sprintf(calc_hash_hex + i * 2, "%02x", calc_hash[i]);
    }

    // 比较哈希
    bool result = (strcmp(calc_hash_hex, sep + 1) == 0);

    // 清理
    free(salt);
    free(calc_hash_hex);

    return result;
}

// 格式化时间为字符串
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

// 从字符串解析时间
time_t parse_time(const char *time_str)
{
    if (!time_str)
        return (time_t)-1;

    struct tm tm_info = {0};

    // 尝试解析格式 YYYY-MM-DD HH:MM:SS
    if (sscanf(time_str, "%d-%d-%d %d:%d:%d",
               &tm_info.tm_year, &tm_info.tm_mon, &tm_info.tm_mday,
               &tm_info.tm_hour, &tm_info.tm_min, &tm_info.tm_sec) == 6)
    {
        tm_info.tm_year -= 1900; // 年份从1900年开始
        tm_info.tm_mon -= 1;     // 月份从0开始
        return mktime(&tm_info);
    }

    // 尝试解析格式 YYYY-MM-DD
    if (sscanf(time_str, "%d-%d-%d",
               &tm_info.tm_year, &tm_info.tm_mon, &tm_info.tm_mday) == 3)
    {
        tm_info.tm_year -= 1900;
        tm_info.tm_mon -= 1;
        return mktime(&tm_info);
    }

    return (time_t)-1; // 解析失败
}

// 安全字符串复制
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
