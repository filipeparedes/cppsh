module;
/**
 * @file history.cppm
 * @brief Implementation of the history builtin command.
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

export module cppsh.builtin.history;

import cppsh.command;
import cppsh.shell_state;

/**
 * @brief Prints the user's input history
 * 
 * @param command The parsed command
 * @param state The Shell's state 
 * @return int Status code
 */
export int builtin_history(const command_t& command, shell_state_t& state) {
    int i = 1;
    for (const std::string& input : state.history) {
        std::cout << i++ << "  " << input << "\n";
    }
    return 0;
}
