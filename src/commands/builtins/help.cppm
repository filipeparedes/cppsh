module;
/**
 * @file help.cppm
 * @brief Implementation of the help builtin command.
 *
 * @author Filipe Paredes (filipeparedes3@gmail.com)
 *
 * @version 1.2.0
 * @date 2026-06-20
 *
 * @copyright Copyright (c) 2026
 *
 */
#include <iostream>
#include <span>
#include <expected>
#include <print>

export module cppsh.builtin.help;

import cppsh.command;
import cppsh.command_entry;
import cppsh.shell_errors;

/**
 * @brief Prints a brief description of every built-in command.
 *
 * @param command The parsed command (args ignored).
 * @return Status code.
 */
export std::expected<int, shell_error_t> builtin_help(const command_t& command, std::span<const command_entry_t> entries) {
    int i = 1;
    for (const command_entry_t entry : entries) {
        std::println("{} {} - {}", i++, entry.name, entry.description);
    }

    return 0;
}
