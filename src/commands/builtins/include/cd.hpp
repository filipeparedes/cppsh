/**
 * @file cd.hpp
 * @brief Declaration of the cd builtin command.
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

#include "command.hpp"
#include "commands/entry.hpp"

/**
 * @brief Changes directory.
 *
 * @param command The parsed command (args ignored).
 * @return Status code.
 */
int builtin_cd(const cppsh::Command& command, ShellContext& context);