cmake_minimum_required(VERSION 3.22)

if(UNIX AND NOT APPLE)
  set(VCPKG_MANIFEST_FEATURES "linux")
endif()

project(Property_Management_System)

find_package(unofficial-sqlite3 CONFIG REQUIRED)

set(SOURCES src/main.c src/factorial.c)
set(HEADERS src/factorial.h)

add_executable(Factorial ${SOURCES} ${HEADERS})

set_property(TARGET Factorial PROPERTY C_STANDARD 11)
set_property(TARGET Factorial PROPERTY C_STANDARD_REQUIRED ON)

target_link_libraries(Factorial PRIVATE unofficial::sqlite3::sqlite3)

enable_testing() 

add_subdirectory (tst)