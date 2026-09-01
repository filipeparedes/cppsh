module;
/**
 * @file execution.cppm
 * @brief Implementation for functions related to execution
 * of external binaries.
 * 
 * @author Filipe Paredes (filipeparedes3@gmail.com)
 * 
 * @version 1.3.0
 * @date 2026-08-01
 * 
 * @copyright Copyright (c) 2026
 * 
 */
#include <unistd.h>
#include <csignal>
#include <fcntl.h>
#include <expected>
#include <sys/wait.h>
#include <vector>
#include <array>
#include <print>

export module cppsh.execution;

import cppsh.shell_errors;
import cppsh.pipeline;
import cppsh.shell_state;
import cppsh.command;
import utils.str_utils;

// ---- HELPER FUNCTIONS ----
/**
 * @brief Applies I/O redirections for the child process
 *      Exits the process if an error occurs to prevent zombie clones.
 * 
 * @param cmd - The command struct
 */
void apply_child_redirections(const command_t& cmd) {
    if (!cmd.input_file.empty()) {
        int file_desc = open(cmd.input_file.c_str(), O_RDONLY);
        if (file_desc == -1) {
            print(shell_error_t{error_code_t::OPEN_FAILED});
            exit(1);
        }
        dup2(file_desc, STDIN_FILENO);
        close(file_desc);
    }

    if (!cmd.output_file.empty()) {
        int flags = cmd.append ? O_WRONLY | O_CREAT | O_APPEND 
                               : O_WRONLY | O_CREAT | O_TRUNC;
        int file_desc = open(cmd.output_file.c_str(), flags, 0644);
        if (file_desc == -1) {
            print(shell_error_t{error_code_t::OPEN_FAILED});
            exit(1);
        }
        dup2(file_desc, STDOUT_FILENO);
        close(file_desc);
    }
}

/**
 * @brief Parses the raw waitpid status into a shell exit code.
 * 
 * @param status The raw status
 * @return int The parsed exit code
 */
int parse_wait_status(int status){
    if (WIFEXITED(status))
        return WEXITSTATUS(status);
    if (WIFSIGNALED(status) || WIFSTOPPED(status))
        return 128 + WTERMSIG(status);
    return 0;
}

/**
 * @brief Closes a specificed number of pipes.
 * 
 * @param pipes The pipes
 * @param count The count of pipes
 */
void close_pipes(const std::vector<std::array<int, 2>>& pipes, int count){
    for (int i=0; i<count; ++i){
        close(pipes[i][0]);
        close(pipes[i][1]);
    }
}

// ----- EXECUTION LOGIC ------

/**
 * @brief Executes a single command in a child process via fork + execvp.
 * 
 * Handles I/O redirection and background execution for a single command.
 * 
 * @param cmd The pipeline with the command to execute.
 * @return int Exit code of the child process, or 127 if the command was not found.
 */
std::expected<int, shell_error_t> exec_single(const pipeline_t& pl){
    command_t cmd = pl.cmds[0];

    pid_t c_pid = fork();
    if (c_pid == -1) 
        return std::unexpected(shell_error_t{error_code_t::FORK_FAILED});

    // ----- PARENT PROCESS -----
    if (c_pid > 0) {
        if (pl.bg) {
            std::println("[{}]: Background execution", c_pid);
            return 0;
        }

        int status;
        waitpid(c_pid, &status, WUNTRACED); // wait for child to end
        return parse_wait_status(status);
    }
    // ---- CHILD PROCESS -----
    else {
        setpgrp();
        signal(SIGINT, SIG_DFL); //Reset signal behaviour to default in child process
        signal(SIGTSTP, SIG_DFL);

        apply_child_redirections(cmd);

        std::vector<char*> argv = str_utils::to_vchar(cmd.args);
        //.data() converts std::vector<char*> into char**
        execvp(cmd.args[0].c_str(), argv.data());

        //If the process reaches here, execvp failed
        exit(127);
    }
}

/**
 * @brief Executes a pipeline of commands connected by pipes.
 * 
 * Creates N-1 pipes for N commands, forks a child process for each command,
 * and connects stdout of each process to stdin of the next via dup2.
 * 
 * @param pl The pipeline of commands to execute.
 * @return int Exit code of the last command in the pipeline.
 */
std::expected<int, shell_error_t> exec_pl(const pipeline_t& pl) {
    int n = pl.cmds.size();
    std::vector<std::array<int, 2>> pipes(n - 1);
    std::vector<pid_t> pids(n);

    for(int i=0; i<n-1; ++i){
        //create n-1 pipes
        if (pipe(pipes[i].data()) == -1){
            //cleanup opened pipes to avoid fd leaks
            close_pipes(pipes, i);
            return std::unexpected(shell_error_t{error_code_t::FORK_FAILED});
        }
    }

    //fork once for each command
    for (int i=0; i<n; ++i){
        pids[i] = fork();

        if (pids[i] == -1) {
            close_pipes(pipes, n-1);
            //wait for created children to prevent zombies
            for (int j=0; j<i; ++j)
                waitpid(pids[j], nullptr, 0);

            return std::unexpected(shell_error_t{error_code_t::FORK_FAILED});
        }
        //---- CHILD PROCESS ---
        if (pids[i] == 0) {
            //connect stdin to previous pipe (except 1st cmd)
            if (i>0) dup2(pipes[i-1][0], STDIN_FILENO);
            //connect stdout to next pipe (except last cmd)
            if (i<n-1) dup2(pipes[i][1], STDOUT_FILENO);

            close_pipes(pipes, n-1);
            apply_child_redirections(pl.cmds[i]);

            std::vector<char*> argv = str_utils::to_vchar(pl.cmds[i].args);
            execvp(pl.cmds[i].args[0].c_str(), argv.data());
            exit(127);
        }
    }

    // parent cleanup & wait
    close_pipes(pipes, n-1);

    int last_status = 0;
    for (int i=0; i<n; ++i) {
        int status;
        waitpid(pids[i], &status, WUNTRACED);
        if (i == n-1) last_status = parse_wait_status(status);
    }

    return last_status;
}

/**
 * @brief Handles the execution for an external command.
 * 
 * The executor receives a Pipeline of commands, searches for the commands, 
 * and if the command is found, forks a new process and executes the command via execvp().
 * 
 * @param pl The Pipeline of Commands input
 * @return int Status code
 */
export std::expected<int, shell_error_t> exec(const pipeline_t& pl) {
    if (pl.cmds.empty()) return 0;
    return pl.cmds.size() == 1 ? exec_single(pl) : exec_pl(pl);
}