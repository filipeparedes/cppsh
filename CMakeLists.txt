cmake_minimum_required(VERSION 3.28)

set(CMAKE_CXX_COMPILER /opt/homebrew/opt/llvm/bin/clang++)

project(cppsh)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED True)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_CXX_SCAN_FOR_MODULES ON)

# static lib with all modules
add_library(cppsh_lib STATIC)

target_sources(cppsh_lib
    PUBLIC
    FILE_SET CXX_MODULES FILES
        lib/env_entry.cppm
        lib/command.cppm
        lib/pipeline.cppm
        lib/parsing.cppm
        lib/utils.cppm
        src/commands/command_entry.cppm
        src/commands/builtins/cd.cppm
        src/commands/builtins/exit.cppm
        src/commands/builtins/help.cppm
        src/commands/builtins/history.cppm
        src/commands/builtins/export.cppm
        src/commands/builtins/unset.cppm
        src/dispatching.cppm
        src/execution.cppm
        src/shell_errors.cppm
        src/shell_state.cppm
        src/shell.cppm
        src/signal_handling.cppm
)

add_executable(cppsh src/main.cpp)
target_link_libraries(cppsh PRIVATE cppsh_lib) # use static lib 

#find_package(GTest REQUIRED)

#add_executable(tests
#    tests/parser_test.cpp
#    tests/dispatcher_test.cpp
#    tests/pipe_test.cpp
#)
#target_link_libraries(tests PRIVATE cppsh_lib GTest::gtest_main) # use static lib & gtest