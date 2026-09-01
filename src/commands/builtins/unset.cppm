module;
/**
 * @file unset.cppm
 * @brief Implementation of the unset builtin command.
 *
 * @author Filipe Paredes (filipeparedes3@gmail.com)
 *
 * @version 1.1.0
 * @date 2026-08-31
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
import utils.str_utils;

/**
 * @brief Unset built-in command.
 *
 * Removes shell and environment variables.
 * Supports: unset VAR1 VAR2 ...
 *
 * @param command The parsed command.
 * @return Status code.
 */
export std::expected<int, shell_error_t> builtin_unset(const command_t& command) {
    // unset with no arguments does nothing
    if (command.args.size() < 2) {
        return 0;
    }

    for (size_t idx = 1; idx < command.args.size(); ++idx) {
        const std::string& key = command.args[idx];

        if (!str_utils::is_valid_identifier(key))
            return std::unexpected(shell_error_t{error_code_t::INVALID_IDENTIFIER, command.args[0], key});

        remove_env_variable(key);
    }

    return 0;
}