/**
 * @file executor.cpp
 * @brief Implementation for executor.hpp
 * 
 * @author Filipe Paredes (filipeparedes3@gmail.com)
 * 
 * @version 0.3.0
 * @date 2026-06-01
 * 
 * @copyright Copyright (c) 2026
 * 
 */

#include "include/executor.hpp"
#include "errors/shell_error.hpp"

#include "utils.hpp"
#include <unistd.h>
#include <iostream>
#include <csignal>
#include <fcntl.h>

int Executor::execute(const cppsh::Command& cmd) {
    if (cmd.args.empty()) return 0;

    pid_t c_pid = fork();

    if (c_pid == -1) 
        throw ShellError(ShellErrorCode::FORK_FAILED);
    else if (c_pid > 0) {
        //Parent process
        int status;
        waitpid(c_pid, &status, WUNTRACED); // wait for child to end

        if (WIFEXITED(status)) {
            return WEXITSTATUS(status);
        } else if (WIFSIGNALED(status)) {
            return 0;
        } else if (WIFSTOPPED(status)) {
            return 0;
        }
        return 0;
    }
    else {
        //Child process
        setpgrp();
        signal(SIGINT, SIG_DFL); //Reset signal behaviour to default in child process
        signal(SIGTSTP, SIG_DFL);

        std::vector<char*> argv = cppsh::to_vchar(cmd.args);

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

        //.data() converts std::vector<char*> into char**
        execvp(cmd.args[0].c_str(), argv.data());

        //If the process reaches here, execvp failed
        exit(127);
    }
}