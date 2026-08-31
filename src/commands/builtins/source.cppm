module;
/**
 * @file source.cppm
 * @brief Implementation of the source builtin command.
 *
 * @author Filipe Paredes (filipeparedes3@gmail.com)
 *
 * @version 0.0.1
 * @date 2026-08-29
 *
 * @copyright Copyright (c) 2026
 *
 */

#include <vector>
#include <string>
#include <expected>

export module cppsh.builtin.source;

import cppsh.scripting;
import cppsh.shell_errors;
import cppsh.command;

/**
 * @brief Executes commands from a file in the current shell context.
 *
 * @param cmd The parsed command
 * @returns int Status code
 */
export std::expected<int, shell_error_t> builtin_source(const command_t& cmd) {
    if (cmd.args.size() < 2) {
        // Missing filename argument
        return std::unexpected(shell_error_t{error_code_t::INVALID_ARGS}); 
    }

    // Execute the script in the current environment
    return execute_file(cmd.args[1]);
}