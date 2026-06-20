module;
/**
 * @file shell_error.cppm
 * @brief Implementation for error handling functions and data structures
 * 
 * @author Filipe Paredes (filipeparedes3@gmail.com) 
 * 
 * @version 1.0.0
 * @date 2026-06-19
 * 
 * @copyright Copyright (c) 2026
 * 
 */

#include <iostream>
#include <cerrno>
#include <iomanip>
#include <string>

export module cppsh.shell_errors;

/**
 * @enum error_code_t
 * @brief Identifies the type of error that occurred in the shell.
 *
 * Error codes are grouped by category:
 * - User errors (0x0000–0x00FF): caused by invalid input or usage
 * - System errors (0x0100–0xFFFF): caused by OS-level failures
 */
export enum class error_code_t : int {
    //User errors (0x0000 - 0x00FF)
    COMMAND_NOT_FOUND            = 0x0000,
    INVALID_PATH                 = 0x0001,
    INVALID_ARGS                 = 0x0002,
    MISSING_REDIRECTION_TARGET   = 0x0003,

    //System errors (0x0100 - 0xFFFF)
    FORK_FAILED        = 0x0100,
    EXECVP_FAILED      = 0x0101,
 };

/**
 * @struct shell_error_t
 * @brief Represents a shell error with its code and context.
 *
 * Returned via std::unexpected in functions that use std::expected.
 * Use print() to display the error to the user.
 */
export struct shell_error_t {
    error_code_t code;   //error Code
    std::string cmd;     //cmd that caused the error
    std::string arg;     //invalid arg passed to the cmd
    std::string usage;   //correct usage of cmd
};

/**
 * @brief Checks if the error code is a system error
 * 
 * @return true if the error is a system error
 * @return false if the error is a user error
 */
bool is_system_error(const shell_error_t& error){
    return static_cast<int>(error.code) >= 0x0100;
}

/**
 * @brief Prints the error message in the shell.
 */
export void print(const shell_error_t& error){
    if (is_system_error(error)){
        std::cerr << "error[0x" << std::hex << std::setw(4) << std::setfill('0') 
              << static_cast<int>(error.code) << "]: ";
        std::cerr << "os error: " << errno << " (" << strerror(errno) << ")" << std::endl;
        return;
    }

    //User error
    switch(error.code){
        case error_code_t::INVALID_PATH:
            std::cerr << error.cmd << ": " << error.arg << ": No such directory" << std::endl; 
            break;
        case error_code_t::COMMAND_NOT_FOUND:
            std::cerr << error.cmd << ": Unknown command" << std::endl;
            break;
        case error_code_t::INVALID_ARGS:
            std::cerr << error.cmd << ": '" << error.arg << "' Invalid arguments" << std::endl; 
            std::cerr << "Usage: " << error.usage << std::endl;   
            break;
        case error_code_t::MISSING_REDIRECTION_TARGET:
            std::cerr << error.cmd << ": " << error.usage << std::endl;
            break;
        default:
            std::cerr << "An unexpected error has occurred." << std::endl;
    }
}