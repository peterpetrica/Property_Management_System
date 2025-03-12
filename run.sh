#!/bin/bash
set -e  # 遇错即退

echo "===== 配置项目 ====="
cmake --preset=default

echo "===== 构建项目 ====="
cmake --build build

echo "===== 运行测试 ====="
./build/tst/Factorial_test

echo "===== 全部完成 ====="