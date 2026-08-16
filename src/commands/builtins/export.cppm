module;
/**
 * @file export.cppm
 * @brief Implementation of the export builtin command.
 *
 * @author Filipe Paredes (filipeparedes3@gmail.com)
 *
 * @version 0.1.0
 * @date 2026-08-16
 *
 * @copyright Copyright (c) 2026
 *
 */

#include <cstdlib>
#include <string>
#include <print>
#include <expected>
#include <cctype>

export module cppsh.builtin.export_cmd;

import cppsh.command;
import cppsh.shell_state;
import cppsh.shell_errors;
import cppsh.env_entry;
import cppsh.utils;

/**
 * @brief Export built-in command
 *
 *  The export command can be used in 3 different ways:
 *      - export VAR=value => Creates (or updates) the var, and marks it as exported
 *      - export VAR => If the var already exists as local, marks it as exported
 *      - export => (no args) Lists all exported vars
 *
 * @param command The parsed command.
 * @param shell_state_t& The shell state
 *
 * @return Status code.
 */
export std::expected<int, shell_error_t> builtin_export(const command_t& command, shell_state_t& state) {
    //No arguments => list all exported vars
    if (command.args.size()<2) {
        for (const auto& [key, entry] : state.env_variables) {
            if (entry.is_exported) {
                std::println("{}=\"{}\"", key, entry.value);
            }
        }
        return 0;
    }
    
    //Process each argument (supports export VAR=val VAR2 VAR3=val3)
    for (size_t idx=1; idx<command.args.size(); ++idx){
        const std::string& arg = command.args[idx];
        size_t eq_pos = arg.find('=');

        std::string key;
        std::string value;
        bool has_value = (eq_pos != std::string::npos);

        if (has_value) {
            key = arg.substr(0, eq_pos);
            value = arg.substr(eq_pos+1);
        } else {
            key = arg;
        }

        if (!is_valid_identifier(key))
            return std::unexpected(shell_error_t{error_code_t::INVALID_IDENTIFIER, command.args[0], arg});

        if (has_value) {
            state.env_variables[key] = env_entry_t{value, true};
            setenv(key.c_str(), value.c_str(), 1);
        } else {
            //If already exists in state, mark as exported and sync to OS env
            auto it = state.env_variables.find(key);
            if (it != state.env_variables.end()){
                it->second.is_exported = true;
                setenv(key.c_str(), it->second.value.c_str(), 1);
            }
        }
    }

    return 0;
}