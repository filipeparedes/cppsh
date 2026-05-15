/**
 * @file cd.cpp
 * @brief Implementation of the cd builtin command.
 *
 * @author Filipe Paredes (filipeparedes3@gmail.com)
 *
 * @version 0.3.0
 * @date 2026-05-06
 *
 * @copyright Copyright (c) 2026
 *
 */

#include "cd.hpp"
#include "errors/shell_error.hpp"

#include <unistd.h>
#include <iostream>
#include <string>

int builtin_cd(const cppsh::Command& command, ShellContext& context) {
    std::string dir;

    //No argument -> go to HOME, fallback to root
    if (command.args.size() < 2){
        const char* home = getenv("HOME");
        dir = home ? home : "/";
    }
    else {
        dir = command.args[1];
    }

    // Change directory
    if (chdir(dir.c_str()) == -1) {
        throw ShellError(ShellErrorCode::INVALID_PATH, command.args[0], dir);
    }

   return 0;
}