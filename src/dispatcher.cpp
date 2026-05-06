/**
 * @file dispatcher.cpp
 * @brief Implementation for dispatcher.hpp
 * 
 * @author Filipe Paredes (filipeparedes3@gmail.com)
 * 
 * @version 0.1
 * @date 2026-05-03
 * 
 * @copyright Copyright (c) 2026
 * 
 */

#include "dispatcher.hpp"
#include "commands/commands.hpp"
#include <iostream>

Dispatcher::Dispatcher() {
    entries = {
    {"exit", builtin_exit, "Exit the shell"},
    {"cd", builtin_cd, "Change directory"},
    };
}

int Dispatcher::dispatch(const cppsh::Command& cmd) {
    if (cmd.args.empty()) return 0;

    for (const cppsh::CommandEntry& entry : entries) {
        if (entry.name == cmd.args[0]) {
            return entry.handler(cmd);
        }
    }

    // TODO: executor
    std::cerr << "Unknown command: '" << cmd.args[0] << "'" << std::endl;
    return 1;
}