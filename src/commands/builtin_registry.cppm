module;
/**
 * @file builtin_registry.cppm
 * @brief Registry module for shell built-in commands.
 *
 *  Centralizes the list of built-in commands to avoid circular dependencies between the
 *  command dispatcher, the help systems, and individual commands.
 *
 * @author Filipe Paredes (filipeparedes3@gmail.com)
 *
 * @version 1.0.0
 * @date 2026-08-30
 *
 * @copyright Copyright (c) 2026
 *
 */

#include <vector>
#include <string>
#include <optional>
#include <functional>
#include <expected>

export module cppsh.builtin_registry;

import cppsh.command_entry; 
import cppsh.shell_errors;


/**
 * @brief List of all registered built-in commands
 *
 * Note: 'help' command is treated separately to avoid cirular dependencies 
 * when iterating over this list.
 * 
 */
std::vector<command_entry_t> entries;

/**
 * @brief Retrieves the complete list of built-in commands.
 * 
 * @return Read-only reference to the vector of command entries.
 */
export const std::vector<command_entry_t>& get_builtins(){
    return entries;
}

/**
 * @brief Searches for a built-in command by its name.
 * 
 * @param name The name of the built-in command to find.
 * @return std::optional wrapping a reference to the command entry
 */
export std::optional<std::reference_wrapper<const command_entry_t>> get_builtin(const std::string& name){
    for (const auto& entry : entries){
        if (entry.name == name){
            return std::cref(entry); //Const REFerence wrapper
        }
    }
    return std::nullopt;
}

/**
 * @brief 
 * 
 * @param builtin 
 * @return export 
 */
export std::expected<int, shell_error_t> add_builtin(const command_entry_t& builtin){
    try {
        entries.push_back(builtin);
    } catch (const std::bad_alloc&) {
        return std::unexpected(shell_error_t{error_code_t::VECPUSH_FAILED});
    }

    return 0;
}