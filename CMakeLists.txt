cmake_minimum_required(VERSION 3.14)
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

# 处理 libuuid 依赖
if(UNIX AND NOT APPLE)
  find_package(PkgConfig REQUIRED)
  pkg_check_modules(UUID REQUIRED uuid)
  include_directories(${UUID_INCLUDE_DIRS})
endif()

# 创建一个变量来存储平台特定的链接库
set(PLATFORM_LIBS "")
if(UNIX AND NOT APPLE)
  set(PLATFORM_LIBS ${UUID_LIBRARIES})
endif()

# 创建一个变量来存储源文件
set(SOURCE_FILES
    src/main.c
    src/db/database.c
    src/db/db_init.c
    src/db/db_query.c
    src/auth/auth.c
    src/auth/tokens.c
    src/ui/ui_main.c
    src/ui/ui_login.c
    src/ui/ui_admin.c
    src/ui/ui_staff.c
    src/ui/ui_owner.c
    src/models/building.c
    src/models/apartment.c
    src/models/user.c
    src/utils/utils.c
    src/utils/file_ops.c
    src/utils/console.c
)

# 定义头文件路径
include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

# 定义可执行文件
add_executable(pms ${SOURCE_FILES})

# 链接库
target_link_libraries(pms PRIVATE 
    unofficial::sqlite3::sqlite3 
    OpenSSL::Crypto
    ${PLATFORM_LIBS}
)

# 添加测试
option(BUILD_TESTS "Build the tests" OFF)
if(BUILD_TESTS)
    enable_testing()
    add_subdirectory(tests)
endif()

# 安装规则
install(TARGETS pms DESTINATION bin)