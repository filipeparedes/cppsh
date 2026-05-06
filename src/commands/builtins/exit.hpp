/**
 * @file exit.hpp
 * @brief Declaration of the exit builtin command.
 *
 * @author Filipe Paredes (filipeparedes3@gmail.com)
 *
 * @version 0.1
 * @date 2026-05-05
 *
 * @copyright Copyright (c) 2026
 *
 */

#pragma once

#include "command.hpp"

/**
 * @brief Exits the shell.
 *
 * @param command The parsed command (args ignored).
 * @return Never returns — calls exit(0).
 */
int builtin_exit(const cppsh::Command& command);