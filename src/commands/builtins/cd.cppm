module;
/**
 * @file cd.cppm
 * @brief Implementation of the cd builtin command.
 *
 * @author Filipe Paredes (filipeparedes3@gmail.com)
 *
 * @version 1.1.0
 * @date 2026-06-20
 *
 * @copyright Copyright (c) 2026
 *
 */

#include <unistd.h>
#include <iostream>
#include <string>
#include <expected>

export module cppsh.builtin.cd;

import cppsh.command;
import cppsh.shell_state;
import cppsh.shell_errors;

/**
 * @brief Changes directory.
 *
 * @param command The parsed command (args ignored).
 * @return Status code.
 */
export std::expected<int, shell_error_t> builtin_cd(const command_t& command, shell_state_t& state) {
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
        return std::unexpected(shell_error_t{error_code_t::INVALID_PATH, command.args[0], dir});
    }

   return 0;
}