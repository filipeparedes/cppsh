module;
/**
 * @file export.cppm
 * @brief Implementation of the export builtin command.
 *
 * @author Filipe Paredes (filipeparedes3@gmail.com)
 *
 * @version 1.2.0
 * @date 2026-08-31
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
import utils.str_utils;

/**
 * @brief Export built-in command
 *
 *  The export command can be used in 3 different ways:
 *      - export VAR=value => Creates (or updates) the var, and marks it as exported
 *      - export VAR => If the var already exists as local, marks it as exported
 *      - export => (no args) Lists all exported vars
 *
 * @param command The parsed command.
 *
 * @return Status code.
 */
export std::expected<int, shell_error_t> builtin_export(const command_t& command) {
    //No arguments => list all exported vars
    if (command.args.size()<2) {
        for (const auto& [key, entry] : get_env_variables()) {
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

        if (!str_utils::is_valid_identifier(key))
            return std::unexpected(shell_error_t{error_code_t::INVALID_IDENTIFIER, command.args[0], arg});

        if (has_value) {
            auto result = add_env_variable(key, env_entry_t{value, true});
            if (!result){
                //return error directly to the caller (dispatcher)
                return std::unexpected(result.error());
            }
        } else {
            //If already exists in state, mark as exported and sync to OS env
            const auto& envs = get_env_variables();
            auto it = envs.find(key);

            if (it != envs.end()){
                //copy existing entry, update flag and re-insert
                env_entry_t updated_entry = it->second;
                updated_entry.is_exported = true;

                auto result = add_env_variable(key, updated_entry);
                if (!result){
                    return std::unexpected(result.error());
                }
            }
        }
    }

    return 0;
}