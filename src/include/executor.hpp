/**
 * @file executor.hpp
 *
 * @brief Declaration for the command executor.
 * 
 * The executor receives a Pipeline of commands, searches for the commands, 
 * and if the command is found, forks a new process and executes the command via execvp().
 * 
 * @author Filipe Paredes (filipeparedes3@gmail.com)
 * 
 * @version 0.3.0
 * @date 2026-06-05
 * 
 * @copyright Copyright (c) 2026
 * 
 */

#include "pipeline.hpp"
#include "context.hpp"

class Executor {
    public:
        /**
         * @brief Handles the execution for an external command.
         * 
         * @param pl The Pipeline of Commands input
         * @return int Status code
         */
        int exec(const cppsh::Pipeline& pl);

    private:
        /**
         * @brief Executes a single command in a child process via fork + execvp.
         * 
         * Handles I/O redirection and background execution for a single command.
         * 
         * @param cmd The pipeline with the command to execute.
         * @return int Exit code of the child process, or 127 if the command was not found.
         */
        int exec_single(const cppsh::Pipeline& pl);

        /**
         * @brief Executes a pipeline of commands connected by pipes.
         * 
         * Creates N-1 pipes for N commands, forks a child process for each command,
         * and connects stdout of each process to stdin of the next via dup2.
         * 
         * @param pl The pipeline of commands to execute.
         * @return int Exit code of the last command in the pipeline.
         */
        int exec_pl(const cppsh::Pipeline& pl);
};