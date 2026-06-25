module;
/**
 * @file dispatching.cppm
 * @brief Implementation for dispatcher.hpp
 * 
 * @author Filipe Paredes (filipeparedes3@gmail.com)
 * 
 * @version 1.1.0
 * @date 2026-06-23
 * 
 * @copyright Copyright (c) 2026
 * 
 */
#include <fcntl.h>
#include <unistd.h>
#include <vector>
#include <expected>

export module cppsh.dispatching;

import cppsh.shell_errors;
import cppsh.shell_state;
import cppsh.command_entry;
import cppsh.command;
import cppsh.pipeline;
import cppsh.execution;
import cppsh.utils;
import cppsh.builtin.cd;
import cppsh.builtin.exit;
import cppsh.builtin.history;
import cppsh.builtin.help;

const std::vector<command_entry_t> entries = {
    {"exit",    "Exit the shell",           "exit",        builtin_exit},
    {"cd",      "Change directory",         "cd [dir]",    builtin_cd},
    {"history", "List user's input history","history",     builtin_history},
};

/**
 * @brief Dispatches a Pipeline of Commands
 * 
 * @param pl -  the Pipeline to dispatch
 * @return The status code
 */
export std::expected<int, shell_error_t> dispatch(const pipeline_t& pl, shell_state_t& state) {
    if (pl.cmds.empty()) return 0;
    command_t cmd;

    //If there is only one entry, check built ins
    if (pl.cmds.size() == 1) {
        cmd = pl.cmds[0];

        //Handle help cmd separately
        if (iequals(cmd.args[0], "help") || iequals(cmd.args[0], "-h")) {
            return builtin_help(cmd, entries);
        }

        for (const command_entry_t& entry : entries) {
            if (iequals(entry.name, cmd.args[0])){
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

                //TODO: background execution for built ins
                std::expected<int, shell_error_t> result = entry.handler(cmd, state);

                //restore IO direction back to normal
                dup2(saved_stdout, STDOUT_FILENO);
                dup2(saved_stdin, STDIN_FILENO);

                //close save files
                close(saved_stdout);
                close(saved_stdin);

                return result;
            }
        }
    }
    //Multiple entries -> straight to executor
    std::expected<int, shell_error_t> res = exec(pl);

    //error
    if (!res)
        return res;

    //returns value -> check for 127
    if (res.value() == 127)
        return std::unexpected(shell_error_t{error_code_t::COMMAND_NOT_FOUND, pl.cmds[0].args[0]});

    return res.value();
}

