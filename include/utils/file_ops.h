#ifndef FILE_OPS_H
#define FILE_OPS_H

#include <stdbool.h>
#include <stddef.h>

// 复制文件
bool copy_file(const char *src, const char *dest);

// 删除文件
bool delete_file(const char *path);

// 检查文件是否存在
bool file_exists(const char *path);

// 创建目录
bool create_directory(const char *path);

// 获取程序的数据目录路径
char *get_data_dir(char *buffer, size_t size);

#endif /* FILE_OPS_H */
