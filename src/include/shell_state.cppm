module;
/**
 * @file context.hpp
 * @brief Defines the shell state struct, containing the shell's runtime state.
 *
 * shell_state_t is passed to command handlers to provide access to shared
 * states such as command history.
 *
 * @author Filipe Paredes (filipeparedes3@gmail.com)
 *
 * @version 1.0.0
 * @date 2026-06-19
 *
 * @copyright Copyright (c) 2026
 *
 */

#include <vector>
#include <string>
#include <span>

export module cppsh.shell_context;

/**
 * @brief Holds the runtime state of the shell.
 *
 * Passed by reference to command handlers that need to read
 * or modify shell state.
 */
export struct shell_state_t {
    std::vector<std::string> history;           // List of commands executed during the session
    //TODO: env variables
};