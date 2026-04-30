cmake_minimum_required(VERSION 3.20)
project(cppsh)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

include_directories(src lib include)

add_executable(cppsh 
    src/main.cpp
    src/shell.cpp
    lib/utils.cpp
    lib/parser.cpp
    )

find_package(GTest REQUIRED)

add_executable(tests 
    tests/parser_test.cpp 
    lib/parser.cpp
    )
target_link_libraries(tests PRIVATE GTest::gtest_main)
target_include_directories(tests PRIVATE lib)