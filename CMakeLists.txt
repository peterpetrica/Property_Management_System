cmake_minimum_required(VERSION 3.14)

# 为vcpkg设置平台
if(UNIX AND NOT APPLE)
  set(VCPKG_MANIFEST_FEATURES "linux")
endif()

project(Property_Management_System VERSION 1.0.0 LANGUAGES C)

# 设置C语言标准
set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)

# vcpkg集成
if(DEFINED ENV{VCPKG_ROOT} AND NOT DEFINED CMAKE_TOOLCHAIN_FILE)
  set(CMAKE_TOOLCHAIN_FILE "$ENV{VCPKG_ROOT}/scripts/buildsystems/vcpkg.cmake"
      CACHE STRING "")
endif()

# 查找依赖库
find_package(unofficial-sqlite3 CONFIG REQUIRED)
find_package(OpenSSL REQUIRED)

set(PLATFORM_LIBS "")
if(UNIX AND NOT APPLE)
  # 使用 vcpkg 的 libuuid
  find_path(LIBUUID_INCLUDE_DIR uuid/uuid.h)
  find_library(LIBUUID_LIBRARY uuid)
  include_directories(${LIBUUID_INCLUDE_DIR})
  set(PLATFORM_LIBS ${LIBUUID_LIBRARY})
endif()

set(SOURCE_FILES
    src/main.c
    src/db/database.c
    src/db/db_init.c
    src/db/db_query.c
    src/db/db_utils.c
    src/auth/auth.c
    src/ui/ui_login.c
    src/ui/ui_admin.c
    src/ui/ui_staff.c
    src/ui/ui_owner.c
    src/models/building.c
    src/models/apartment.c
    src/models/parking.c
    src/models/user.c
    src/models/service.c
    src/models/transaction.c
    src/utils/utils.c
    src/utils/file_ops.c
    src/utils/console.c
)

# 头文件位置
include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

# 编译出来的程序名称
add_executable(pms ${SOURCE_FILES})

# 链接库
target_link_libraries(pms PRIVATE 
    unofficial::sqlite3::sqlite3 
    OpenSSL::Crypto
    ${PLATFORM_LIBS}
)

# 添加测试
option(BUILD_TESTS "Build the tests" ON)
if(BUILD_TESTS)
    enable_testing()
    add_subdirectory(tests)
endif()

# 安装。。。就是放在这
install(TARGETS pms DESTINATION bin)