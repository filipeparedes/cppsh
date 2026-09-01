module;
/**
 * @file shell.cppm
 * @brief Main shell functions
 * 
 * The Shell functions are responsible for the main execution loop,
 * prompt rendering, parsing and execution.
 * 
 * @author Filipe Paredes (filipeparedes3@gmail.com)
 * 
 * @version 1.9.1
 * @date 2026-08-31
 * 
 * @copyright Copyright (c) 2026
 * 
 */
#include <string>
#include <vector>
#include <pwd.h>
#include <unistd.h>
#include <limits.h>
#include <expected>
#include <print>
#include <optional>

export module cppsh.shell;

import cppsh.dispatching;
import cppsh.parsing;
import cppsh.signal_handling;
import cppsh.shell_errors;
import cppsh.shell_state;
import cppsh.pipeline;
import cppsh.completion;
import cppsh.command_entry;
import cppsh.builtin_registry;

import cppsh.builtin.exit;
import cppsh.builtin.cd;
import cppsh.builtin.history;
import cppsh.builtin.export_cmd;
import cppsh.builtin.unset;
import cppsh.builtin.source;

import utils.sys_utils;
import utils.io_utils;

/**
 * @brief Initializes the builtin registry with the entries
 */
export void init_builtins() {
    if (get_builtins().empty()){
        auto register_cmd = [](const command_entry_t& cmd) {
            if (auto res = add_builtin(cmd); !res) {
                print(res.error());
            }
        };

        register_cmd(command_entry_t{"exit", "Exit the shell", "exit", builtin_exit});
        register_cmd(command_entry_t{"cd", "Change directory", "cd [dir]", builtin_cd});
        register_cmd(command_entry_t{"history", "List user's input history","history", builtin_history});
        register_cmd(command_entry_t{"export", "Create, update or list exported variables", "export [VAR]=[val], export [VAR], export", builtin_export});
        register_cmd(command_entry_t{"unset", "Delete an environment variable", "unset [VAR]", builtin_unset});
        register_cmd(command_entry_t{"source", "Execute a script file", "source [file]", builtin_source});
    }
}

/**
 * @brief Builds and prints the shell prompt.
 * 
 * Displays the prompt in the format: user@hostname:~/path$
 * 
 */
std::string get_prompt(const std::string& user, const std::string& hostname) {
    return std::format("{}@{}:{}$ ", user, hostname, sys_utils::get_cwd());
}

/**
 * @brief Starts the main shell loop.
 * 
 * Continuously displays the prompt, reads a line of input,
 * and processes it until the user exits.
 *
 * @returns Unexpected: shell_error_t
 */
export std::expected<int, shell_error_t> run() {
    init_builtins();
    handle_signal();
    setup_autocompletion();

    const std::string user = sys_utils::get_username();
    const std::string hostname = sys_utils::get_hostname();

    while(true) {
        std::string prompt = get_prompt(user, hostname);
        std::optional<std::string> input_opt = io_utils::read_input(prompt);

        //EOF (CTRL+D) - exit gracefully
        if (!input_opt) {
            std::println("");
            break;
        }

        std::string input = std::move(input_opt.value());
        //Ignore blank lines
        if (input.empty() || input.find_first_not_of(" \t") == std::string::npos) continue;

        //add to history
        auto add = add_to_history(input);
        if (!add){
            set_exit_code(static_cast<int>(add.error().code));
            print(add.error());
        }

        //Parse input into Command-type obj
        auto par = parse(input, get_env_variables(), get_last_exit_code());
        if (!par) {
            set_exit_code(static_cast<int>(error_code_t::SYNTAX_ERROR));
            print(shell_error_t{error_code_t::SYNTAX_ERROR, "cppsh", "", par.error()});
            continue;
        }

        //Dispatch the command
        auto dis = dispatch(par.value());
        if (!dis) {
            set_exit_code(static_cast<int>(dis.error().code));
            print(dis.error());
        } else {
            set_exit_code(dis.value());
        }
    }

    return get_last_exit_code();
}