/**
 * file_ops.c
 * 文件操作工具函数实现
 *
 * 本文件提供了一系列文件操作函数，包括：
 * - 复制文件
 * - 删除文件
 * - 检查文件是否存在
 * - 创建目录
 * - 获取程序的数据目录路径
 *
 * 这些函数旨在简化程序中的文件操作，并提供跨平台(Windows/Linux)支持
 */
#include "utils/file_ops.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <errno.h>

#ifdef _WIN32
#include <direct.h>
#define mkdir(path, mode) _mkdir(path)
#endif

/**
 * @brief 复制文件
 *
 * @param src 源文件路径
 * @param dest 目标文件路径
 * @return bool 操作成功返回true，失败返回false
 */
bool copy_file(const char *src, const char *dest)
{
    FILE *source = fopen(src, "rb");
    if (!source)
    {
        return false;
    }

    FILE *destination = fopen(dest, "wb");
    if (!destination)
    {
        fclose(source);
        return false;
    }

    char buffer[4096];
    size_t bytes;
    while ((bytes = fread(buffer, 1, sizeof(buffer), source)) > 0)
    {
        fwrite(buffer, 1, bytes, destination);
    }

    fclose(source);
    fclose(destination);
    return true;
}

/**
 * @brief 删除文件
 *
 * @param path 要删除的文件路径
 * @return bool 操作成功返回true，失败返回false
 */
bool delete_file(const char *path)
{
    return (remove(path) == 0);
}

/**
 * @brief 检查文件是否存在
 *
 * @param path 要检查的文件路径
 * @return bool 文件存在返回true，不存在返回false
 */
bool file_exists(const char *path)
{
    struct stat buffer;
    return (stat(path, &buffer) == 0);
}

/**
 * @brief 创建目录
 *
 * @param path 要创建的目录路径
 * @return bool 创建成功或目录已存在返回true，失败返回false
 */
bool create_directory(const char *path)
{
    return (mkdir(path, 0755) == 0 || errno == EEXIST);
}

/**
 * @brief 获取程序的数据目录路径
 *
 * @param buffer 用于存储路径的缓冲区
 * @param size 缓冲区大小
 * @return char* 成功返回缓冲区指针，失败返回NULL
 */
char *get_data_dir(char *buffer, size_t size)
{
    if (!buffer || size <= 0)
    {
        return NULL;
    }

#ifdef _WIN32
    const char *appdata = getenv("APPDATA");
    if (!appdata)
    {
        return NULL;
    }

    snprintf(buffer, size, "%s\\PropertyManagementSystem", appdata);
#else
#include <unistd.h>
    char cwd[1024];
    const char *home = getcwd(cwd, sizeof(cwd));
    if (!home)
    {
        return NULL;
    }

    snprintf(buffer, size, "%s/Database", home);
#endif

    if (!file_exists(buffer))
    {
        if (!create_directory(buffer))
        {
            return NULL;
        }
    }

    return buffer;
}