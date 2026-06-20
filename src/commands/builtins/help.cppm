module;
/**
 * @file help.cppm
 * @brief Implementation of the help builtin command.
 *
 * @author Filipe Paredes (filipeparedes3@gmail.com)
 *
 * @version 1.0.0
 * @date 2026-06-19
 *
 * @copyright Copyright (c) 2026
 *
 */
#include <iostream>
#include <span>

export module cppsh.builtin.help;

import cppsh.command;
import cppsh.command_entry;
/**
 * @brief Prints a brief description of every built-in command.
 *
 * @param command The parsed command (args ignored).
 * @return Status code.
 */
export int builtin_help(const command_t& command, std::span<const command_entry_t> entries) {
    int i = 1;
    for (const command_entry_t entry : entries) {
        std::cout << i++ << " " << entry.name << " - " << entry.description << "\n";
    }

    return 0;
}
