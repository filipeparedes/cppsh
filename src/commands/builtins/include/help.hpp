/**
 * @file help.hpp
 * @brief Declaration of the help builtin command.
 *
 * @author Filipe Paredes (filipeparedes3@gmail.com)
 *
 * @version 0.1
 * @date 2026-05-07
 *
 * @copyright Copyright (c) 2026
 *
 */

#pragma once

#include "command.hpp"
#include "commands/entry.hpp"

/**
 * @brief Prints a brief description of every built-in command.
 *
 * @param command The parsed command (args ignored).
 * @return Status code.
 */
int builtin_help(const cppsh::Command& command, ShellContext& context);