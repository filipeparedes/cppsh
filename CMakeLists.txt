cmake_minimum_required(VERSION 3.20)
project(cppsh)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

include_directories(src lib include)

add_executable(cppsh 
    src/main.cpp
    src/shell.cpp
    lib/utils.cpp
    lib/parser.cpp
    )