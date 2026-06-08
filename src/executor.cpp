/**
 * @file executor.cpp
 * @brief Implementation for executor.hpp
 * 
 * @author Filipe Paredes (filipeparedes3@gmail.com)
 * 
 * @version 0.4.1
 * @date 2026-06-05
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

int Executor::exec(const cppsh::Pipeline& pl) {
    if (pl.cmds.empty()) return 0;

    return pl.cmds.size() == 1 ? exec_single(pl) : exec_pl(pl);
}

int Executor::exec_single(const cppsh::Pipeline& pl){
    cppsh::Command cmd = pl.cmds[0];

    pid_t c_pid = fork();

    if (c_pid == -1) 
        throw ShellError(ShellErrorCode::FORK_FAILED);

    if (c_pid > 0) {
        //Parent process
        if (pl.bg) {
            std::cout << "[" << c_pid << "]: Background execution" << std::endl;
            return 0;
        }

        int status;
        waitpid(c_pid, &status, WUNTRACED); // wait for child to end

        if (WIFEXITED(status))
            return WEXITSTATUS(status);
        else if (WIFSIGNALED(status))
            return 0;
        else if (WIFSTOPPED(status))
            return 0;
        
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

            //redirect stdout to the output file
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

int Executor::exec_pl(const cppsh::Pipeline& pl) {
    int n = pl.cmds.size();
    std::vector<std::array<int, 2>> pipes(n - 1);
    std::vector<pid_t> pids(n);

    for(int i = 0; i<n-1; i++){
        //create n-1 pipes
        if (pipe(pipes[i].data()) == -1){
            //log error
            exit(1);
        }
    }

    //fork once for each command
    for (int i=0; i<n; i++){
        pids[i] = fork();

        if (pids[i] == -1) 
            throw ShellError(ShellErrorCode::FORK_FAILED);

        if (pids[i] == 0) {
            //Child process

            //connect stdin to previous pipe (except 1st cmd)
            if (i>0)
                dup2(pipes[i-1][0], STDIN_FILENO);
            
            //connect stdout to next pipe (except last cmd)
            if (i<n-1)
                dup2(pipes[i][1], STDOUT_FILENO);

            //close all pipe ends
            for (int j=0; j<n-1; j++){
                close(pipes[j][0]);
                close(pipes[j][1]);
            }

            // input redirection
            if (!pl.cmds[i].input_file.empty()) {
                int fd = open(pl.cmds[i].input_file.c_str(), O_RDONLY);
                if (fd == -1)
                    //log error
                    exit(1);

                dup2(fd, STDIN_FILENO);
                close(fd);
            }

            // output redirection
            if (!pl.cmds[i].output_file.empty()) {
                int flags = pl.cmds[i].append ? O_WRONLY | O_CREAT | O_APPEND
                                              : O_WRONLY | O_CREAT | O_TRUNC;
                int fd = open(pl.cmds[i].output_file.c_str(), flags, 0644);
                if(fd == -1)
                    //log error
                    exit(1);


                dup2(fd, STDOUT_FILENO);
                close(fd);
            }

            std::vector<char*> argv = cppsh::to_vchar(pl.cmds[i].args);
            execvp(pl.cmds[i].args[0].c_str(), argv.data());
            exit(127);
        }
    }

    // parent closes all pipe ends
    for (int i = 0; i < n-1; i++) {
        close(pipes[i][0]);
        close(pipes[i][1]);
    }

    // parent waits for all children
    int last_status = 0;
    for (int i = 0; i < n; i++) {
        int status;
        waitpid(pids[i], &status, 0);
        if (i == n-1 && WIFEXITED(status))
            last_status = WEXITSTATUS(status);
    }

    return last_status;
}