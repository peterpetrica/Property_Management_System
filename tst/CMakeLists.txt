cmake_minimum_required (VERSION 3.10)

####################
# Setup GoogleTest #
####################
configure_file(CMakeLists.txt.ini googletest-download/CMakeLists.txt)
execute_process(COMMAND ${CMAKE_COMMAND} -G "${CMAKE_GENERATOR}" .
  RESULT_VARIABLE result
  WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/googletest-download )
if(result)   
  message(FATAL_ERROR "CMake step for googletest failed: ${result}")
endif()
execute_process(COMMAND ${CMAKE_COMMAND} --build .
  RESULT_VARIABLE result
  WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/googletest-download )
if(result)
  message(FATAL_ERROR "Build step for googletest failed: ${result}")
endif()
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
add_subdirectory(${CMAKE_CURRENT_BINARY_DIR}/googletest-src
                 ${CMAKE_CURRENT_BINARY_DIR}/googletest-build
                 EXCLUDE_FROM_ALL)


############################
# Compile the test program #
############################
project (Property_Management_System_test)

set(SOURCES test.cxx ../src/factorial.c) 
set(HEADERS ../src/factorial.h)

add_executable(Factorial_test ${SOURCES} ${HEADERS})

set_property(TARGET Factorial_test PROPERTY CXX_STANDARD 11)
set_property(TARGET Factorial_test PROPERTY CXX_STANDARD_REQUIRED ON)
set_property(TARGET Factorial_test PROPERTY C_STANDARD 11)  
set_property(TARGET Factorial_test PROPERTY C_STANDARD_REQUIRED ON)

include_directories(../src)

target_link_libraries(Factorial_test gtest_main)

enable_testing()

add_test(NAME Factorial_test COMMAND Factorial_test)