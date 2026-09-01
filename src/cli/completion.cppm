module;
/**
 * @file completion.cppm
 * @brief Tab completion module using readline.
 * 
 *  Provides auto-completion for built-in commands, external binaries in PATH, environemtn variables, and falls back to files/directories.
 *
 * @author Filipe Paredes (filipeparedes3@gmail.com)
 *
 * @version 0.1.0
 * @date 2026-09-01
 * 
 * @copyright Copyright (c) 2026
 * 
 */

#include <readline/readline.h>
#include <cstdlib>
#include <cstring>
#include <string>
#include <string_view>
#include <algorithm>
#include <vector>
#include <filesystem>

extern "C" {
    extern char **environ;
}

export module cppsh.completion;

import cppsh.builtin_registry;

/**
 * @brief Generates command matches (built-ins + PATH executables).
 * 
 * @param text The prefix word to match.
 * @param state 0 on the first call to initialize, non-zero on subsequent calls.
 * @return char* Dynamically allocated string of the next match, or nullptr if no matches remain. 
 *         Memory is freed by readline.
 */
char* command_generator(const char* text, int state) {
    static size_t match_index = 0;
    static std::vector<std::string> matches;

    // Initialize matches on the first call for a new word
    if (state == 0) {
        matches.clear();
        match_index = 0;
        std::string_view prefix(text);

        // Search in built-ins
        for (const auto& builtin : get_builtins()) {
            if (builtin.name.starts_with(prefix)) {
                matches.push_back(builtin.name);
            }
        }

        // Search in system PATH for external executables
        if (const char* path_env = std::getenv("PATH")) {
            std::string_view path_str(path_env);
            size_t start = 0;
            
            while (start < path_str.size()) {
                size_t end = path_str.find(':', start);
                std::string_view dir = path_str.substr(start, end - start);
                start = (end == std::string_view::npos) ? path_str.size() : end+1;
                
                if (dir.empty()) continue;

                std::error_code ec; // Prevent exceptions on invalid paths
                //is_directory implies exists()
                if (std::filesystem::is_directory(dir, ec)) {
                    for (const auto& entry : std::filesystem::directory_iterator(dir, ec)) {
                        std::string filename = entry.path().filename().string();
                        if (filename.starts_with(prefix)) {
                            matches.push_back(std::move(filename));
                        }
                    }
                }
            }
        }
        
        //remove duplicates (binaries present in /bin and /usr/bin)
        std::ranges::sort(matches);
        auto [first, last] = std::ranges::unique(matches);
        matches.erase(first, last);
    }

    // Return the next match (readline frees the memory)
    if (match_index < matches.size()) {
        return strdup(matches[match_index++].c_str());
    }
    return nullptr;
}

/**
 * @brief Generates environment variable matches.
 * 
 * @param text The prefix word to match (may or may not include the leading '$').
 * @param state 0 on the first call, non-zero on subsequent calls.
 * @return char* Dynamically allocated string of the next match, or nullptr if done.
 */
char* env_generator(const char* text, int state) {
    static size_t match_index = 0;
    static std::vector<std::string> matches;
    static bool includes_dollar = false;

    if (state == 0) {
        matches.clear();
        match_index = 0;
        
        includes_dollar = (text[0] == '$');
        std::string_view prefix(text);
        if (includes_dollar) {
            prefix.remove_prefix(1); // ignore '$'
        }

        // Iterate through all system environment variables
        for (char **env = environ; *env != nullptr; ++env) {
            std::string_view env_str(*env);
            size_t eq_pos = env_str.find('=');

            if (eq_pos != std::string_view::npos) {
                std::string_view var_name = env_str.substr(0, eq_pos);
                if (var_name.starts_with(prefix)) {
                    // Prepend '$' if the original input had it
                    if (includes_dollar)
                        matches.push_back("$" + std::string(var_name));
                    else 
                        matches.emplace_back(var_name);
                }
            }
        }

        std::ranges::sort(matches);
    }

    //return next match on the list
    if (match_index < matches.size()) {
        //strdup because readline takes ownership of this pointer and will free() it
        return strdup(matches[match_index++].c_str());
    }
    return nullptr;
}

/**
 * @brief Main readline completion router.
 * 
 * @param text The word being completed.
 * @param start Start index of the word in the readline buffer.
 * @param end End index of the word in the readline buffer.
 * @return char** Null-terminated array of matches, or nullptr to use default file completion.
 */
char** cppsh_completion(const char* text, int start, int end) {
    rl_attempted_completion_over = 0;

    // Environment variables
    bool is_var = (text[0] == '$') || (start > 0 && rl_line_buffer[start - 1] == '$');
    if (is_var) {
        return rl_completion_matches(text, env_generator);
    }

    // First word (Commands: built-ins or PATH)
    if (start == 0) {
        return rl_completion_matches(text, command_generator);
    }

    // Fallback to default readline behavior (files and directories)
    return nullptr;
}

/**
 * @brief Configures readline to use the custom completion logic.
 * 
 * Assigns the main router function to readline's callback pointer.
 * Should be called once during shell initialization.
 */
export void setup_autocompletion() {
    rl_attempted_completion_function = cppsh_completion;
}