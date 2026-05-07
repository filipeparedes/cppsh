/**
 * @file entry.hpp
 * @brief Defines the CommandEntry struct used in the command dispatch table.
 *
 * Each entry maps a command name to a handler function and a description.
 * Handlers receive the parsed Command and the shell's runtime context.
 *
 * @author Filipe Paredes (filipeparedes3@gmail.com)
 *
 * @version 0.2.0
 * @date 2026-05-07
 *
 * @copyright Copyright (c) 2026
 *
 */

#pragma once

#include <string>
#include <functional>
#include "command.hpp"

class ShellContext; //forward declaration

/**
 * @brief Represents an entry in the command dispatch table.
 *
 * Maps a command name to its handler function and a human-readable description.
 * The handler receives the parsed Command and the shell's runtime ShellContext.
 */
struct CommandEntry {
    std::string name;                                               // Command name (e.g. "exit", "cd")
    std::function<int(const cppsh::Command&, ShellContext&)> handler;     // Handler function to call on match
    std::string description;                                        // Human-readable description for help
};