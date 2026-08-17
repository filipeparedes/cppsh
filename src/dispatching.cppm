module;
/**
 * @file dispatching.cppm
 * @brief Implementation for function related to command dispatching.
 * 
 * @author Filipe Paredes (filipeparedes3@gmail.com)
 * 
 * @version 2.0.0
 * @date 2026-08-17
 * 
 * @copyright Copyright (c) 2026
 * 
 */
#include <fcntl.h>
#include <unistd.h>
#include <vector>
#include <expected>
#include <string>

export module cppsh.dispatching;

import cppsh.shell_errors;
import cppsh.shell_state;
import cppsh.command_entry;
import cppsh.env_entry;
import cppsh.command;
import cppsh.pipeline;
import cppsh.execution;
import cppsh.utils;
import cppsh.builtin.cd;
import cppsh.builtin.exit;
import cppsh.builtin.history;
import cppsh.builtin.help;
import cppsh.builtin.export_cmd;
import cppsh.builtin.unset;

//List all built-in commands here
//except for help cmd (treated separately)
const std::vector<command_entry_t> entries = {
    {"exit",    "Exit the shell",                            "exit",                                     builtin_exit},
    {"cd",      "Change directory",                          "cd [dir]",                                 builtin_cd},
    {"history", "List user's input history",                 "history",                                  builtin_history},
    {"export",  "Create, update or list exported variables", "export [VAR]=[val], export [VAR], export", builtin_export},
    {"unset",   "Delete an environment variable",            "unset [VAR]",                              builtin_unset},
};

/**
 * @brief Dispatches a Pipeline of Commands
 * 
 * @param log_pl -  the list of pipeline to dispatch
 * @return The status code
 */
export std::expected<int, shell_error_t> dispatch(const std::vector<pipeline_t>& log_pl, shell_state_t& state) {
    if (log_pl.empty()) return 0;

    int current_exit_code = state.last_exit_code;
    bool run_next = true;

    for (const pipeline_t& pl : log_pl) {
        if (run_next) {
            command_t cmd;
            bool executed_builtin = false;

            //If there is only one entry, check built ins
            if (pl.cmds.size() == 1) {
                cmd = pl.cmds[0];

                //Handle assignment
                if (pl.cmds.size() == 1 && pl.cmds[0].type == command_type_t::assignment) {
                    const std::string& arg = pl.cmds[0].args[0];
                    size_t eq_pos = arg.find('=');
                    std::string key = arg.substr(0, eq_pos);
                    std::string value = arg.substr(eq_pos + 1);

                    state.env_variables[key] = env_entry_t{value, false};
                    
                    current_exit_code = 0;
                    executed_builtin = true;
                }
                //Handle help cmd separately
                else if (iequals(cmd.args[0], "help") || iequals(cmd.args[0], "-h")) {
                    std::expected<int, shell_error_t> help_res = builtin_help(cmd, entries);
                    if (!help_res) return help_res;

                    current_exit_code = help_res.value();
                    executed_builtin = true;
                }
                else {
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

                                close(fd);
                            }

                            //Execute built-in
                            std::expected<int, shell_error_t> result = entry.handler(cmd, state);

                            //restore IO direction back to normal
                            dup2(saved_stdout, STDOUT_FILENO);
                            dup2(saved_stdin, STDIN_FILENO);
                            close(saved_stdout);
                            close(saved_stdin);

                            if (!result) return result;

                            current_exit_code = result.value();
                            executed_builtin = true;
                            break;
                        }
                    }
                }
            }
        
            //Multiple entries or non-builtin -> straight to executor
            if (!executed_builtin) {
                std::expected<int, shell_error_t> res = exec(pl);
                if (!res) return res;

                //cmd not found
                if (res.value() == 127){
                    return std::unexpected(shell_error_t{error_code_t::COMMAND_NOT_FOUND, pl.cmds[0].args[0]});
                }
                current_exit_code = res.value();
            }
        }

        //evaluate if next cmd should run based on exit code of the previous
        if (pl.op == logical_op_t::AND){
            run_next = (current_exit_code == 0); //&& needs success
        } else if (pl.op == logical_op_t::OR) {
            run_next = (current_exit_code != 0); //|| Needs failure
        } else {
            run_next = true;
        }
    }

    return current_exit_code;
}
