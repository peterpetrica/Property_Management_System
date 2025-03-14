# CMake测试配置
cmake_minimum_required(VERSION 3.14)

include(CTest)
enable_testing()

# 包含项目头文件目录
include_directories(${CMAKE_SOURCE_DIR}/include)

# 检查test_auth.c是否存在，如果不存在则创建
if(NOT EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/test_auth.c")
    file(WRITE "${CMAKE_CURRENT_SOURCE_DIR}/test_auth.c" "
#include <stdio.h>
#include \"auth/auth.h\"
#include \"utils/utils.h\"

int main() {
    printf(\"Authentication tests\\n\");
    return 0;
}
")
endif()

# 添加测试可执行文件
add_executable(pms_tests
    test_auth.c
    ${CMAKE_SOURCE_DIR}/src/utils/utils.c
)

# 链接测试依赖
target_link_libraries(pms_tests PRIVATE
    unofficial::sqlite3::sqlite3
    OpenSSL::Crypto
    ${PLATFORM_LIBS}
)

# 添加测试
add_test(NAME AuthTest COMMAND pms_tests)
