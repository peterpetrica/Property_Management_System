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

// 复制文件
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

// 删除文件
bool delete_file(const char *path)
{
    return (remove(path) == 0);
}

// 检查文件是否存在
bool file_exists(const char *path)
{
    struct stat buffer;
    return (stat(path, &buffer) == 0);
}

// 创建目录
bool create_directory(const char *path)
{
    return (mkdir(path, 0755) == 0 || errno == EEXIST);
}

// 获取程序的数据目录路径
char *get_data_dir(char *buffer, size_t size)
{
    if (!buffer || size <= 0)
    {
        return NULL;
    }

#ifdef _WIN32
    // Windows
    const char *appdata = getenv("APPDATA");
    if (!appdata)
    {
        return NULL;
    }

    snprintf(buffer, size, "%s\\PropertyManagementSystem", appdata);
#else
    // Linux
    const char *home = getenv("HOME");
    if (!home)
    {
        return NULL;
    }

    snprintf(buffer, size, "%s/.property_management", home);
#endif

    // 确保目录存在
    if (!file_exists(buffer))
    {
        if (!create_directory(buffer))
        {
            return NULL;
        }
    }

    return buffer;
}