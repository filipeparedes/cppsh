module;
/**
 * @file unset.cppm
 * @brief Implementation of the unset builtin command.
 *
 * @author Filipe Paredes (filipeparedes3@gmail.com)
 *
 * @version 0.0.1
 * @date 2026-08-16
 *
 * @copyright Copyright (c) 2026
 *
 */

#include <cstdlib>
#include <string>
#include <expected>
#include <cctype>

export module cppsh.builtin.unset;

import cppsh.command;
import cppsh.shell_state;
import cppsh.shell_errors;
import cppsh.utils;

/**
 * @brief Unset built-in command.
 *
 * Removes shell and environment variables.
 * Supports: unset VAR1 VAR2 ...
 *
 * @param command The parsed command.
 * @param state Shell state containing environment variables.
 * @return Status code.
 */
export std::expected<int, shell_error_t> builtin_unset(const command_t& command, shell_state_t& state) {
    // unset with no arguments does nothing
    if (command.args.size() < 2) {
        return 0;
    }

    for (size_t idx = 1; idx < command.args.size(); ++idx) {
        const std::string& key = command.args[idx];

        if (!is_valid_identifier(key))
            return std::unexpected(shell_error_t{error_code_t::INVALID_IDENTIFIER, command.args[0], key});

        // remove from internal shell state
        state.env_variables.erase(key);

        // remove from system process environment
        unsetenv(key.c_str());
    }

    return 0;
}