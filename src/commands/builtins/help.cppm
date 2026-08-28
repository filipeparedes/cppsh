module;
/**
 * @file help.cppm
 * @brief Implementation of the help builtin command.
 *
 * @author Filipe Paredes (filipeparedes3@gmail.com)
 *
 * @version 2.0.0
 * @date 2026-08-28
 *
 * @copyright Copyright (c) 2026
 *
 */
#include <span>
#include <expected>
#include <print>

export module cppsh.builtin.help;

import cppsh.command;
import cppsh.command_entry;
import cppsh.shell_errors;
import cppsh.builtin_registry;

/**
 * @brief Prints a brief description of every built-in command.
 *
 * @param command The parsed command (args ignored).
 * @return Status code.
 */
export std::expected<int, shell_error_t> builtin_help(const command_t& command) {
    //Manually print help cmd (not on registry)
    std::println("1 help - Print this help message");

    int i = 2;
    for (const auto& builtin : get_builtins()) {
        std::println("{} {} - {}", ++i, builtin.name, builtin.description);
    }

    return 0;
}
