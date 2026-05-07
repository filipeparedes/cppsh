/**
 * @file history.hpp
 * @brief Declaration of the history builtin command.
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
 * @brief Prints the user's input history
 * 
 * @param command The parsed command
 * @param context The Shell's context 
 * @return int Status code
 */
int builtin_history(const cppsh::Command& command, ShellContext& context);