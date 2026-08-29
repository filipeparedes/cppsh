module;
/**
 * @file scripting.cppm
 * @brief Implementation for script running related functions.
 * 
 * @author Filipe Paredes (filipeparedes3@gmail.com) 
 * 
 * @version 0.0.1
 * @date 2026-08-29
 * 
 * @copyright Copyright (c) 2026
 * 
 */

#include <expected>
#include <fstream>
#include <string>

import cppsh.parsing;
import cppsh.dispatching;
import cppsh.shell_errors;
import cppsh.shell_state;

export module cppsh.scripting;

/**
 * @brief Executes commands from a script file line by line.
 * 
 * @param filename Path to the script file.
 * @return Exit code of the last command, or an error.
 */
export std::expected<int, shell_error_t> execute_file(const std::string& filename){
    std::ifstream file(filename);

    if (!file.is_open())
        return std::unexpected(shell_error_t{error_code_t::INVALID_PATH});
    
    std::string line;
    int last_status = 0;

    while (std::getline(file, line)) {
        //Ignore empty lines & comments
        if (line.empty() || line.starts_with('#'))
            continue;
        
        auto pl_res = parse(line, get_env_variables(), get_last_exit_code());
        if (!pl_res.has_value())
            continue;

        auto res = dispatch(pl_res.value());
        if (res.has_value()){
            last_status = res.value();
            set_exit_code(last_status);
        }
    }

    return last_status;
}

