/**
 * @file error_codes.hpp
 * @brief Contains definition of the error codes and their hexa value, in a ShellErrorCode enum class
 * 
 * @author Filipe Paredes (filipeparedes3@gmail.com) 
 * @version 0.2.0
 * 
 * @date 2026-05-15
 * 
 * @copyright Copyright (c) 2026
 * 
 */

 enum class ShellErrorCode : int {
    //User errors (0x0000 - 0x00FF)
    COMMAND_NOT_FOUND            = 0x0000,
    INVALID_PATH                 = 0x0001,
    INVALID_ARGS                 = 0x0002,
    MISSING_REDIRECTION_TARGET   = 0x0003,

    //System errors (0x0100 - 0xFFFF)
    FORK_FAILED        = 0x0100,
    EXECVP_FAILED      = 0x0101,
 };