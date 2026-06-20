module;
/**
 * @file exit.cppm
 * @brief Implementation of the exit builtin command.
 *
 * @author Filipe Paredes (filipeparedes3@gmail.com)
 *
 * @version 1.0.0
 * @date 2026-06-19
 *
 * @copyright Copyright (c) 2026
 *
 */

#include <cstdlib>
#include <iostream>

export module cppsh.builtin.exit;

import cppsh.command;
import cppsh.shell_state;

/**
 * @brief Exits the shell.
 *
 * @param command The parsed command (args ignored).
 * @return Never returns — calls exit(0).
 */
export int builtin_exit(const command_t& command, shell_state_t& state) {
    std::cout << "Exiting program... \n";
    exit(0);
    return 0;
}