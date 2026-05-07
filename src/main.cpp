/**
 * @file main.cpp
 * @brief Entry point for cppsh.
 * 
 * Instantiates the Shell and starts the main loop.
 * 
 * @author Filipe Paredes (filipeparedes3@gmail.com)
 * @version 0.1
 * @date 2026-04-28
 * 
 * @copyright Copyright (c) 2026
 * 
 */

 #include "shell.hpp"

 int main() {
    Shell shell;
    shell.run();
    return 0;
 }