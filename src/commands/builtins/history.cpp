/**
 * @file history.cpp
 * @brief Implementation of the history builtin command.
 *
 * @author Filipe Paredes (filipeparedes3@gmail.com)
 *
 * @version 0.1.1
 * @date 2026-05-07
 *
 * @copyright Copyright (c) 2026
 *
 */

#include "history.hpp"
#include "context.hpp"
#include <iostream>

int builtin_history(const cppsh::Command& command, ShellContext& context) {
    int i = 1;
    for (const std::string& input : context.history) {
        std::cout << i++ << "  " << input << std::endl;
    }
    return 0;
}
