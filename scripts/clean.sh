#!/bin/bash

echo "===== 清理选项 ====="
echo "1. 清理构建文件夹"
echo "2. 清理数据库"
echo "3. 退出"
read -p "请选择要执行的操作 [1-3]: " choice

case $choice in
    1)
        # 清除当前目录下的build文件夹
        if [ -d "build" ]; then
            echo "正在删除 build 文件夹..."
            rm -rf build
            echo "build 文件夹已删除"
        else
            echo "build 文件夹不存在"
        fi
        ;;
    2)
        if [ -d "Database" ]; then
            echo "正在删除 Database 文件夹..."
            rm -rf Database
            echo "Database 文件夹已删除"
        else
            echo "Database 文件夹不存在"
        fi
        ;;
    3)
        echo "退出程序"
        exit 0
        ;;
    *)
        echo "无效的选择"
        exit 1
        ;;
esac

echo "清理完成"