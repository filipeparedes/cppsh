module;
/**
 * @file exit.cppm
 * @brief Implementation of the exit builtin command.
 *
 * @author Filipe Paredes (filipeparedes3@gmail.com)
 *
 * @version 1.2.0
 * @date 2026-06-20
 *
 * @copyright Copyright (c) 2026
 *
 */

#include <cstdlib>
#include <expected>
#include <print>

export module cppsh.builtin.exit;

import cppsh.command;
import cppsh.shell_state;
import cppsh.shell_errors;

/**
 * @brief Exits the shell.
 *
 * @param command The parsed command (args ignored).
 * @return Never returns — calls exit(0).
 */
export std::expected<int, shell_error_t> builtin_exit(const command_t& command, shell_state_t& state) {
    std::println("Exiting program...");
    exit(0);
}