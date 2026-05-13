/**
 * @file utils.hpp
 * @brief Defines some useful functions related to user data and input.
 * 
 * @author Filipe Paredes (filipeparedes3@gmail.com)
 * 
 * @version 0.1
 * @date 2026-04-29
 * 
 * @copyright Copyright (c) 2026
 * 
*/

#pragma once

#include <string>

namespace cppsh {
    /**
     * @brief Reads a line of input from stdin.
     * 
     * @return The line entered by the user, or an empty string on EOF.
     */
    std::string read_input();

    /**
     * @brief Compares two std::strings. Case-insensitive.
     * 
     * @param str1 - One of the std::strings to compare
     * @param str2 - The other std::string to compare
     * @return true if the strings are equal
     * @return false if the strings are not equal
     */
    bool iequals(const std::string& str1, const std::string& str2);
    
    /**
     * @brief Resolves the current working directory.
     * 
     * Replaces the home directory prefix with - if applicable.
     * 
     * @return A human-readable representation of the current directory.
     */
    std::string get_cwd();

    /**
     * @brief Get the current username
     * 
     * @return The username
     * OR
     * @return "user", in case of unsuccess retrieving the data
     */
    std::string get_username();

    /**
     * @brief Get the current user's hostname
     * 
     * @return The hostname
     * OR
     * @return "localhost", in case of unsuccess retrieving the data
     */
    std::string get_hostname();
}
