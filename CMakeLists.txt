cmake_minimum_required(VERSION 3.28)
# LLVM clang++
set(CMAKE_CXX_COMPILER /opt/homebrew/opt/llvm/bin/clang++)


project(cppsh)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED True)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# enable modules
set(CMAKE_CXX_SCAN_FOR_MODULES ON)

include_directories(src src/include src/commands/builtins/include lib include)

file(GLOB BUILTIN_FILES src/commands/builtins/*.cpp)
file(GLOB COMMAND_FILES src/commands/*.cpp)

add_executable(cppsh
    src/main.cpp
    src/shell.cpp
    src/dispatcher.cpp
    src/executor.cpp
    src/signal_handling.cpp
    src/errors/shell_error.cpp
    lib/utils.cpp
    lib/parser.cpp
    ${COMMAND_FILES}
    ${BUILTIN_FILES}
)

target_sources(cppsh
    PRIVATE
        FILE_SET CXX_MODULES 
        FILES
            lib/command.cppm
            lib/pipeline.cppm
)