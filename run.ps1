$ErrorActionPreference = "Stop"

Write-Host "===== 配置项目 =====" -ForegroundColor Cyan
cmake --preset=default

Write-Host "===== 构建项目 =====" -ForegroundColor Cyan
cmake --build build

Write-Host "===== 运行测试 =====" -ForegroundColor Cyan
& "./build/tst/Factorial_test.exe"

Write-Host "===== 全部完成 =====" -ForegroundColor Green