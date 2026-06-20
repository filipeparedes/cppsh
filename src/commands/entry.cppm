module;
/**
 * @file entry.cppm
 * @brief Defines the command entry struct used in the command dispatch table.
 *
 * Each entry maps a command name to a handler function and a description.
 * Handlers receive the parsed Command and the shell's runtime context.
 *
 * @author Filipe Paredes (filipeparedes3@gmail.com)
 *
 * @version 1.1.0
 * @date 2026-06.20
 *
 * @copyright Copyright (c) 2026
 *
 */

#include <string>
#include <expected>

export module cppsh.command_entry;

import cppsh.command;
import cppsh.shell_state;

export using command_handler_t =
    std::expected<int, shell_error_t>(*)(const command_t&, shell_state_t&);

/**
 * @brief Represents an entry in the command dispatch table.
 *
 * Maps a command name to its handler function and a human-readable description.
 * The handler receives the parsed command_t and the shell's runtime shell_context_t.
 */
export struct command_entry_t {
    std::string name;              // Command name (e.g. "exit", "cd")
    std::string description;       // Human-readable description for help
    std::string usage;

    command_handler_t handler;     // Handler function to call on match
};