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

// 生成UUID
char *generate_uuid()
{
    char *uuid_str = (char *)malloc(37); // 36个字符加上空终止符
    if (!uuid_str)
        return NULL;

#ifdef __linux__
    uuid_t uuid;
    uuid_generate(uuid);
    uuid_unparse_lower(uuid, uuid_str);
#else
    // 在非Linux系统上，使用OpenSSL的随机数生成器模拟UUID
    unsigned char random_bytes[16];
    if (RAND_bytes(random_bytes, sizeof(random_bytes)) != 1)
    {
        free(uuid_str);
        return NULL;
    }

    // 设置UUID版本 (版本4 - 随机)
    random_bytes[6] = (random_bytes[6] & 0x0F) | 0x40;
    random_bytes[8] = (random_bytes[8] & 0x3F) | 0x80;

    sprintf(uuid_str,
            "%02x%02x%02x%02x-%02x%02x-%02x%02x-%02x%02x-%02x%02x%02x%02x%02x%02x",
            random_bytes[0], random_bytes[1], random_bytes[2], random_bytes[3],
            random_bytes[4], random_bytes[5], random_bytes[6], random_bytes[7],
            random_bytes[8], random_bytes[9], random_bytes[10], random_bytes[11],
            random_bytes[12], random_bytes[13], random_bytes[14], random_bytes[15]);
#endif

    return uuid_str;
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
    unsigned char random_bytes[length];
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
