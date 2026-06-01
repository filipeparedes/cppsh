/**
 * @file dispatcher.cpp
 * @brief Implementation for dispatcher.hpp
 * 
 * @author Filipe Paredes (filipeparedes3@gmail.com)
 * 
 * @version 0.6.0
 * @date 2026-05-03
 * 
 * @copyright Copyright (c) 2026
 * 
 */

#include "dispatcher.hpp"
#include "errors/shell_error.hpp"
#include "commands/commands.hpp"
#include "commands/entry.hpp"
#include "utils.hpp"

#include <iostream>
#include <fcntl.h>
#include <unistd.h>

Dispatcher::Dispatcher() {
    entries = {
    {"exit", builtin_exit, "Exit the shell"},
    {"cd", builtin_cd, "Change directory"},
    {"history", builtin_history, "List user's input history"},
    {"help", builtin_help, "List all built-in commands and their brief descriptions"}
    };
}

int Dispatcher::dispatch(const cppsh::Command& cmd, ShellContext& context) {
    if (cmd.args.empty()) return 0;

    for (const CommandEntry& entry : entries) {
        if (cppsh::iequals(entry.name, cmd.args[0])){
            //save default IO direction
            int saved_stdout = dup(STDOUT_FILENO);
            int saved_stdin = dup(STDIN_FILENO);

            //Input redirection
            if (!cmd.input_file.empty()) {
                //get file descriptor for the input file
                int fd = open(cmd.input_file.c_str(), O_RDONLY);

                //redirect stdin to the input file
                dup2(fd, STDIN_FILENO);

                //close the file
                close(fd);
            }

            //Output redirection
            if (!cmd.output_file.empty()) {
                //define if it overwrites (truncates) or appends
                int flags = cmd.append ? O_WRONLY | O_CREAT | O_APPEND 
                                    : O_WRONLY | O_CREAT | O_TRUNC;
                
                //get file descriptor for the output file
                int fd = open(cmd.output_file.c_str(), flags, 0644);

                //redirect stdout to the input file
                dup2(fd, STDOUT_FILENO);

                //close the file
                close(fd);
            }

            int result = entry.handler(cmd, context);

            //restore IO direction back to normal
            dup2(saved_stdout, STDOUT_FILENO);
            dup2(saved_stdin, STDIN_FILENO);

            //close save files
            close(saved_stdout);
            close(saved_stdin);

            return result;
        }
    }

    int res = executor.execute(cmd);
    if (res == 127)
        throw ShellError(ShellErrorCode::COMMAND_NOT_FOUND, cmd.args[0]);

    return res;
}

