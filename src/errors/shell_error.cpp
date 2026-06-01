/**
 * @file shell_error.cpp
 * @brief Implementation for shell_error.hpp
 * 
 * @author Filipe Paredes (filipeparedes3@gmail.com) 
 * 
 * @version 0.1
 * @date 2026-05-15
 * 
 * @copyright Copyright (c) 2026
 * 
 */

#include "shell_error.hpp"
#include <iostream>
#include <cerrno>
#include <iomanip>

ShellError::ShellError(ShellErrorCode code, std::string cmd, std::string arg, std::string usage) 
    : code(code), cmd(cmd), arg(arg), usage(usage) {}

void ShellError::print(){
    if (is_system_error()){
        std::cerr << "error[0x" << std::hex << std::setw(4) << std::setfill('0') 
              << static_cast<int>(code) << "]: ";
        std::cerr << "os error: " << errno << " (" << strerror(errno) << ")" << std::endl;
        return;
    }

    //User error
    switch(code){
        case ShellErrorCode::INVALID_PATH:
            std::cerr << cmd << ": " << arg << ": No such directory" << std::endl; 
            break;
        case ShellErrorCode::COMMAND_NOT_FOUND:
            std::cerr << cmd << ": Unknown command" << std::endl;
            break;
        case ShellErrorCode::INVALID_ARGS:
            std::cerr << cmd << ": '" << arg << "' Invalid arguments" << std::endl; 
            std::cerr << "Usage: " << usage << std::endl;   
            break;
        case ShellErrorCode::MISSING_REDIRECTION_TARGET:
            std::cerr << cmd << ": " << usage << std::endl;
            break;
        default:
            std::cerr << "An unexpected error has occurred." << std::endl;
    }
}

bool ShellError::is_system_error(){
    return static_cast<int>(code) >= 0x0100;
}