/**
 * @file executor.hpp
 *
 * @brief Declaration for the command executor.
 * 
 * The executor receives a Command, searches for the command, 
 * and if the command is found, forks a new process and executes the command via execvp().
 * 
 * @author Filipe Paredes (filipeparedes3@gmail.com)
 * 
 * @version 0.1.0
 * @date 2026-05-13
 * 
 * @copyright Copyright (c) 2026
 * 
 */

#include "command.hpp"
#include "context.hpp"

class Executor {
    public:
        /**
         * @brief Searches for an external command, and if the command is found, fork a new process
         * and execute it via execvp().
         * 
         * @param cmd The command input
         * @return int Status code
         */
        int execute(const cppsh::Command& cmd);
};