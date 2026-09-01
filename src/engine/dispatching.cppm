module;
/**
 * @file dispatching.cppm
 * @brief Implementation for function related to command dispatching.
 * 
 * @author Filipe Paredes (filipeparedes3@gmail.com)
 * 
 * @version 3.3.1
 * @date 2026-09-01
 * 
 * @copyright Copyright (c) 2026
 * 
 */
#include <fcntl.h>
#include <unistd.h>
#include <vector>
#include <expected>
#include <string>
#include <optional>

export module cppsh.dispatching;

import cppsh.shell_errors;
import cppsh.shell_state;
import cppsh.command_entry;
import cppsh.builtin_registry;
import cppsh.env_entry;
import cppsh.command;
import cppsh.pipeline;
import cppsh.execution;
import cppsh.builtin.help;

import utils.str_utils;

/// ------- HELPER FUNCTIONS -------

/**
 * @brief Evaluates if the next pipeline should run
 * based on logical op and current exit code
 * 
 * @param op The logical operator
 * @param current_exit_code The current exit code
 * @return true or false
 */
bool evaluate_state(logical_op_t op, int current_exit_code){
    if (op == logical_op_t::AND) return (current_exit_code == 0);
    if (op == logical_op_t::OR) return (current_exit_code != 0);
    return true;
}

/**
 * @brief Handles variable assignment
 * 
 * @param cmd The assignment command
 */
void handle_assignment(const command_t& cmd){
    const std::string& arg = cmd.args[0];
    size_t eq_pos = arg.find('=');

    //prevent crash if '=' is missing
    if (eq_pos == std::string::npos) {
        set_exit_code(1);
        return;
    }

    std::string key = arg.substr(0, eq_pos);
    std::string value = arg.substr(eq_pos + 1);

    auto result = add_env_variable(key, env_entry_t{value, false});

    if (!result){
        print(result.error());
        set_exit_code(1);
    } else {
        set_exit_code(0);
    }
}

/**
 * @brief Checks if a command is the help command
 * 
 * @param cmd The command
 * @return true or false
 */
bool is_help_cmd(const command_t& cmd){
    return str_utils::iequals(cmd.args[0], "help") || str_utils::iequals(cmd.args[0], "-h");
}

/**
 * @brief Set the up input redirection for a built-in command
 * 
 * @param input_file The input file
 */
std::expected <void, shell_error_t> setup_input_redirection(const std::string& input_file){
    if (input_file.empty()) return {};

    //get file descriptor for the input file
    int file_desc = open(input_file.c_str(), O_RDONLY);
    if (file_desc == -1)
        return std::unexpected(shell_error_t{error_code_t::OPEN_FAILED, "cppsh", input_file});

    //redirect stdin to the input file
    dup2(file_desc, STDIN_FILENO);
    close(file_desc);

    return {};
}

/**
 * @brief Set the up output redirection for a built-in command
 * 
 * @param output_file The output file
 * @param append The append flag
 */
std::expected<void, shell_error_t> setup_output_redirection(const std::string& output_file, bool append){
    if (output_file.empty()) return {};

    //define if it overwrites (truncates) or appends
    int flags = append ? O_WRONLY | O_CREAT | O_APPEND 
                        : O_WRONLY | O_CREAT | O_TRUNC;
    
    int file_desc = open(output_file.c_str(), flags, 0644);
    if (file_desc == -1)
        return std::unexpected(shell_error_t{error_code_t::OPEN_FAILED, "cppsh", output_file});

    //redirect stdout to the input file
    dup2(file_desc, STDOUT_FILENO);
    close(file_desc);

    return {};
}

/// -------- MAIN DISPATCH LOGIC ---------

/**
 * @brief Attempts to find the respective build-in command and execute it
 * 
 * @param cmd The command
 * @param executed_builtin [out] Execution flag
 *
 * @returns Exit code on success
 * @returns Shell Error on error
 */
std::expected<int, shell_error_t> try_execute_builtin(const command_t& cmd, bool& executed_builtin){
    executed_builtin = false;

    if (cmd.type == command_type_t::assignment){
        handle_assignment(cmd);
        executed_builtin = true;
        return 0;
    }

    bool is_help = is_help_cmd(cmd);
    auto matched_builtin = is_help ? std::nullopt : get_builtin(cmd.args[0]);

    if (is_help || matched_builtin.has_value()) {
        executed_builtin = true;

        //save original FDs
        int saved_stdout = dup(STDOUT_FILENO);
        int saved_stdin = dup(STDIN_FILENO);

        //apply redirections
        auto in_res = setup_input_redirection(cmd.input_file);
        auto out_res = setup_output_redirection(cmd.output_file, cmd.append);

        std::expected<int, shell_error_t> result = 0;

        if (!in_res) {
            result = std::unexpected(in_res.error());
        } else if (!out_res) {
            result = std::unexpected(out_res.error());
        } else {
            //execute command if redirections succeeded
            if (is_help)
                result = builtin_help(cmd);
            else 
                result = matched_builtin.value().get().handler(cmd);
        }

        //restore original FDs safely
        if (saved_stdout != -1) {
            dup2(saved_stdout, STDOUT_FILENO);
            close(saved_stdout);
        }
        if (saved_stdin != -1) {
            dup2(saved_stdin, STDIN_FILENO);
            close(saved_stdin);
        }

        return result;
    }

    //not built-in, dispatcher continues to fork/exec
    return 0;
}

/**
 * @brief Dispatches a Pipeline of Commands
 * 
 * @param log_pl -  the list of pipeline to dispatch
 * @return The status code
 */
export std::expected<int, shell_error_t> dispatch(const std::vector<pipeline_t>& log_pl) {
    if (log_pl.empty()) return 0;

    int current_exit_code = get_last_exit_code();
    bool run_next = true;

    for (const pipeline_t& pl : log_pl) {
        if (run_next) {
            bool executed_builtin = false;

            //Only one entry -> check built ins
            if (pl.cmds.size() == 1) {
                std::expected<int, shell_error_t> builtin_res = try_execute_builtin(pl.cmds[0], executed_builtin);

                if (executed_builtin){
                    if (!builtin_res){
                        print(builtin_res.error());
                        current_exit_code = static_cast<int>(builtin_res.error().code);
                    } else {
                        current_exit_code = builtin_res.value();
                    }
                }
            }
        
            //Multiple entries or non-builtin -> straight to executor
            if (!executed_builtin) {
                std::expected<int, shell_error_t> res = exec(pl);
                if (!res) return res;

                //cmd not found
                if (res.value() == 127)
                    return std::unexpected(shell_error_t{error_code_t::COMMAND_NOT_FOUND, pl.cmds[0].args[0]});
                
                current_exit_code = res.value();
            }
        }

        run_next = evaluate_state(pl.op, current_exit_code);
    }

    return current_exit_code;
}
