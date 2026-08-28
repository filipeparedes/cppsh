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
 * @version 0.0.3
 * @date 2026-08-28
 *
 * @copyright Copyright (c) 2026
 *
 */

#include <vector>
#include <string>
#include <optional>
#include <functional>

export module cppsh.builtin_registry;

import cppsh.command_entry; 
import cppsh.shell_errors;

import cppsh.builtin.exit;
import cppsh.builtin.cd;
import cppsh.builtin.history;
import cppsh.builtin.export_cmd;
import cppsh.builtin.unset;


/**
 * @brief List of all registered built-in commands
 *
 * Note: 'help' command is treated separately to avoid cirular dependencies 
 * when iterating over this list.
 * 
 */
const std::vector<command_entry_t> entries = {
    {"exit",    "Exit the shell",                            "exit",                                     builtin_exit},
    {"cd",      "Change directory",                          "cd [dir]",                                 builtin_cd},
    {"history", "List user's input history",                 "history",                                  builtin_history},
    {"export",  "Create, update or list exported variables", "export [VAR]=[val], export [VAR], export", builtin_export},
    {"unset",   "Delete an environment variable",            "unset [VAR]",                              builtin_unset},
};

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