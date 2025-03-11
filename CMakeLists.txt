cmake_minimum_required(VERSION 3.10)

project(Property_Management_System)

find_package(unofficial-sqlite3 CONFIG REQUIRED)

add_executable(HelloWorld helloworld.c)

target_link_libraries(HelloWorld PRIVATE unofficial::sqlite3::sqlite3)