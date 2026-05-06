/**
 * @file exit.cpp
 * @brief Implementation of the exit builtin command.
 *
 * @author Filipe Paredes (filipeparedes3@gmail.com)
 *
 * @version 0.2.0
 * @date 2026-05-05
 *
 * @copyright Copyright (c) 2026
 *
 */

#include "exit.hpp"
#include <cstdlib>
#include <iostream>

int builtin_exit(const cppsh::Command& command, ShellContext& context) {
    std::cout << "Exiting program..." << std::endl;
    exit(0);
    return 0;
}