/**
 * @file shell_error.hpp
 * @brief Includes definition for ShellError class, which handles shell error I/O
 * 
 * @author Filipe Paredes (filipeparedes3@gmail.com)
 * @version 0.1
 * 
 * @date 2026-05-15
 * 
 * @copyright Copyright (c) 2026
 * 
 */

#include "error_codes.hpp"
#include <string>

class ShellError {
public:
    ShellError(ShellErrorCode code, std::string cmd = "", std::string arg = "", std::string usage = "");
    /**
     * @brief Prints the error message in the shell.
     */
    void print();

private:
    ShellErrorCode code;    //Error Code
    std::string cmd;        //Command that caused the error
    std::string arg;        //Invalid argument passed to the command
    std::string usage;      //Correct usage of the command

    /**
     * @brief Checks if the error code is a system error
     * 
     * @return true if the error is a system error
     * @return false if the error is a user error
     */
    bool is_system_error();
};