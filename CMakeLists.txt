cmake_minimum_required(VERSION 3.20)
project(cppsh)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

include_directories(src src/include src/commands/builtins/include lib include)

file(GLOB BUILTIN_FILES src/commands/builtins/*.cpp)
file(GLOB COMMAND_FILES src/commands/*.cpp)

add_executable(cppsh
    src/main.cpp
    src/shell.cpp
    src/dispatcher.cpp
    lib/utils.cpp
    lib/parser.cpp
    ${COMMAND_FILES}
    ${BUILTIN_FILES}
)

find_package(GTest REQUIRED)

add_executable(tests 
    tests/parser_test.cpp
    tests/dispatcher_test.cpp
    lib/parser.cpp
    lib/utils.cpp
    src/dispatcher.cpp
    ${BUILTIN_FILES}
    )
target_link_libraries(tests PRIVATE GTest::gtest_main)
target_include_directories(tests PRIVATE lib include src src/include src/commands/builtins/include)