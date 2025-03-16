# 测试文件编译用的子cmake
cmake_minimum_required(VERSION 3.14)

include(CTest)
enable_testing()

include_directories(${CMAKE_SOURCE_DIR}/include)

add_executable(pms_tests
    test_auth.c
    ${CMAKE_SOURCE_DIR}/src/utils/utils.c
)

target_link_libraries(pms_tests PRIVATE
    unofficial::sqlite3::sqlite3
    OpenSSL::Crypto
    ${PLATFORM_LIBS}
)

add_test(NAME AuthTest COMMAND pms_tests)
