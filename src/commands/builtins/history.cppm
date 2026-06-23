module;
/**
 * @file history.cppm
 * @brief Implementation of the history builtin command.
 *
 * @author Filipe Paredes (filipeparedes3@gmail.com)
 *
 * @version 1.2.0
 * @date 2026-06-20
 *
 * @copyright Copyright (c) 2026
 *
 */
#include <expected>
#include <print>

export module cppsh.builtin.history;

import cppsh.command;
import cppsh.shell_state;
import cppsh.shell_errors;

/**
 * @brief Prints the user's input history
 * 
 * @param command The parsed command
 * @param state The Shell's state 
 * @return int Status code
 */
export std::expected<int, shell_error_t> builtin_history(const command_t& command, shell_state_t& state) {
    int i = 1;
    for (const std::string& input : state.history) {
        std::println("{}: {}", i++, input);
    }
    return 0;
}
