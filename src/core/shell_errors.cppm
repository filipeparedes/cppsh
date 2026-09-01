module;
/**
 * @file shell_error.cppm
 * @brief Implementation for error handling functions and data structures
 * 
 * @author Filipe Paredes (filipeparedes3@gmail.com) 
 * 
 * @version 1.4.0
 * @date 2026-08-27
 * 
 * @copyright Copyright (c) 2026
 * 
 */

#include <cerrno>
#include <format>
#include <string>
#include <print>
#include <cstring>
#include <string_view>

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
    INVALID_IDENTIFIER           = 0x0004,

    //System errors (0x0100 - 0xFFFF)
    FORK_FAILED                  = 0x0100,
    EXECVP_FAILED                = 0x0101,
    VECPUSH_FAILED               = 0x0102,
    MAPINSRT_FAILED              = 0x0103,
    OPEN_FAILED                  = 0x0104,
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
 * @brief Maps error codes to template string dynamically.
 * {0} maps to error.cmd
 * {1} maps to error.arg
 * {2} maps to error.usage
 *
 * @param code - The error code
 */
constexpr std::string_view get_error_template(error_code_t code) {
    using enum error_code_t;

    switch (code) {
        case INVALID_PATH:               return "{0}: {1}: No such directory";
        case COMMAND_NOT_FOUND:          return "{0}: Unknown command";
        case INVALID_ARGS:               return "{0}: '{1}' Invalid arguments\nUsage: {2}";
        case MISSING_REDIRECTION_TARGET: return "{0}: {2}";
        case INVALID_IDENTIFIER:         return "{0}: `{1}`: not a valid identifier";
        default:                         return "An unexpected error has occurred.";
    }
}

/**
 * @brief Prints the error message in the shell.
 */
export void print(const shell_error_t& error){
    if (is_system_error(error)){
        std::println(stderr, "error[0x{:04x}]: os error: {} ({})",
                        static_cast<int>(error.code), errno, std::strerror(errno));
        return;
    }

    std::string_view fmt = get_error_template(error.code);
    std::string msg = std::vformat(fmt, std::make_format_args(error.cmd, error.arg, error.usage));

    std::println(stderr, "{}", msg);
}