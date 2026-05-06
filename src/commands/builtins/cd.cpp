/**
 * @file cd.cpp
 * @brief Implementation of the cd builtin command.
 *
 * @author Filipe Paredes (filipeparedes3@gmail.com)
 *
 * @version 0.2.1
 * @date 2026-05-06
 *
 * @copyright Copyright (c) 2026
 *
 */

#include "cd.hpp"
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
        std::cout << "Couldn't find the directory '" << dir << "'"  << std::endl;
        return 1;
    }

   return 0;
}