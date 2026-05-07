/**
 * @file help.cpp
 * @brief Implementation of the help builtin command.
 *
 * @author Filipe Paredes (filipeparedes3@gmail.com)
 *
 * @version 0.1.0
 * @date 2026-05-07
 *
 * @copyright Copyright (c) 2026
 *
 */

#include "help.hpp"
#include "context.hpp"
#include <iostream>

int builtin_help(const cppsh::Command& command, ShellContext& context) {
    int i = 1;
    for (const CommandEntry& entry : context.registry.get_entries()) {
        std::cout << i++ << " " << entry.name << " - " << entry.description << std::endl;
    }

    return 0;
}
