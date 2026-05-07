/**
 * @file context.hpp
 * @brief Defines the ShellContext struct, containing the shell's runtime state.
 *
 * ShellContext is passed to command handlers to provide access to shared
 * shell state such as command history.
 *
 * @author Filipe Paredes (filipeparedes3@gmail.com)
 *
 * @version 0.2.0
 * @date 2026-05-06
 *
 * @copyright Copyright (c) 2026
 *
 */

#pragma once

#include <vector>
#include <string>
#include "icommand_registry.hpp"

/**
 * @brief Holds the runtime state of the shell.
 *
 * Passed by reference to command handlers that need to read
 * or modify shell state.
 */
struct ShellContext {
    std::vector<std::string> history;  // List of commands executed during the session
    ICommandRegistry& registry;

    ShellContext(ICommandRegistry& r) : registry(r) {}
};