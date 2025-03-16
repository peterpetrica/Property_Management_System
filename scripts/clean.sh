#!/bin/bash

echo "===== 清理构建和数据文件 ====="

# 清除当前目录下的build文件夹
if [ -d "build" ]; then
    echo "正在删除 build 文件夹..."
    rm -rf build
    echo "build 文件夹已删除"
else
    echo "build 文件夹不存在"
fi

# 删除HOME下的数据库存放文件夹
PROP_DIR="${HOME}/.property_management"
if [ -d "$PROP_DIR" ]; then
    echo "正在删除 ${PROP_DIR} 文件夹..."
    rm -rf "$PROP_DIR"
    echo "${PROP_DIR} 文件夹已删除"
else
    echo "${PROP_DIR} 文件夹不存在"
fi

echo "清理完成"