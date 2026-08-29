module;
/**
 * @file context.hpp
 * @brief Defines the shell state struct and accessors, containing the shell's runtime state.
 *
 * The state is managed internally as a static instance and accessed/modified
 * through exposed module functions.
 *
 * @author Filipe Paredes (filipeparedes3@gmail.com)
 *
 * @version 2.0.0
 * @date 2026-08-27
 *
 * @copyright Copyright (c) 2026
 *
 */

#include <expected>
#include <vector>
#include <string>
#include <unordered_map>

import cppsh.env_entry;
import cppsh.shell_errors;

export module cppsh.shell_state;

/**
 * @brief Holds the runtime state of the shell.
 */
struct shell_state_t {
    std::vector<std::string> history;   // List of commands executed during the session
    std::unordered_map<std::string, env_entry_t> env_variables; // Map of the environment variables
    int last_exit_code = 0;
};

/**
 * @brief Retrieves the singleton instance of the internal shell state.
 *
 * Uses a function-local static variable to guarantee lazy initialization
 * (created only on the first call) and prevent Static Initialization Order Fiasco.
 *
 * @return Reference to the static shell state instance.
 */
shell_state_t& get_internal_state(){
    //created only once
    static shell_state_t instance;
    return instance;
}


/// ------------- HISTORY FUNCTIONS ---------------

/**
 * @brief Retrieves a read-only reference to the command history.
 * 
 * @return Read-only reference to the history vector.
 */
export const std::vector<std::string>& get_history(){
    return get_internal_state().history;
}

/**
 * @brief Appends a command string to the shell's history.
 * 
 * @param cmd The command string to add.
 * @returns 0 on success, or an error code if allocation fails.
 */
export std::expected<int, shell_error_t> add_to_history(const std::string& cmd){
    try {
        get_internal_state().history.push_back(cmd);
    } catch (const std::bad_alloc&) {
        return std::unexpected(shell_error_t{error_code_t::VECPUSH_FAILED});
    }

    return 0;
}

/**
 * @brief Clears the shell's command history.
 * Useful for testing and resetting state.
 */
export void clear_history(){
    get_internal_state().history.clear();
}


/// --------------- ENV VARIABLES FUNCTIONS ------------

/**
 * @brief Retrieves a read-only reference to the environment variables.
 * 
 * @return Read-only reference to the environment map.
 */
export const std::unordered_map<std::string, env_entry_t>& get_env_variables(){
    return get_internal_state().env_variables;
}

/**
 * @brief Inserts or updates an environment variable in the shell state.
 * 
 * @param key The name/key of the environment variable.
 * @param entry The environment entry data.
 * @returns 0 on success, or an error code if allocation fails.
 */
export std::expected<int, shell_error_t> add_env_variable(const std::string& key, const env_entry_t& entry){
    try {
        // add to internal shell state
        get_internal_state().env_variables.insert_or_assign(key, entry);

        if (entry.is_exported){
            // add to system process environment
            setenv(key.c_str(), entry.value.c_str(), 1);
        }

    } catch (const std::bad_alloc&) {
        return std::unexpected(shell_error_t{error_code_t::MAPINSRT_FAILED});
    }

    return 0;
}

/**
 * @brief Removes an environemnt variable in the shell state.
 * 
 * @param key The name/key of the environment variable.
 */
export void remove_env_variable(const std::string& key){
    auto& state = get_internal_state();

    // remove from internal shell state
    state.env_variables.erase(key);

    // remove from system process environment
    unsetenv(key.c_str());
}


/// ------------- EXIT CODE FUNCTIONS -------------

/**
 * @brief Sets the exit code for the most recently executed command.
 * 
 * @param code The integer exit code to store.
 */
export void set_exit_code(int code){
    get_internal_state().last_exit_code = code;
}

/**
 * @brief Retrieves the exit code of the last executed command.
 * 
 * @return int The last exit code.
 */
export int get_last_exit_code(){
    return get_internal_state().last_exit_code;
}