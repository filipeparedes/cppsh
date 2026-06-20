module;
/**
 * @file cd.cppm
 * @brief Implementation of the cd builtin command.
 *
 * @author Filipe Paredes (filipeparedes3@gmail.com)
 *
 * @version 1.0.0
 * @date 2026-06-19
 *
 * @copyright Copyright (c) 2026
 *
 */

#include "errors/shell_error.hpp"

#include <unistd.h>
#include <iostream>
#include <string>

export module cppsh.builtin.cd;

import cppsh.command;
import cppsh.shell_state;

/**
 * @brief Changes directory.
 *
 * @param command The parsed command (args ignored).
 * @return Status code.
 */
export int builtin_cd(const command_t& command, shell_state_t& state) {
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