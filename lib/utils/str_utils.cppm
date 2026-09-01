module;
/**
 * @file str_utils.cppm
 * @brief Implementation of some utilitary string functions
 * 
 * @author Filipe Paredes (filipeparedes3@gmail.com)
 * 
 * @version 0.0.1
 * @date 2026-08-31
 * 
 * @copyright Copyright (c) 2026
 * 
*/

#include <string>
#include <vector>
#include <algorithm>
#include <cctype>

export module utils.str_utils;

export namespace str_utils {

    /**
    * @brief Compares two std::strings. Case-insensitive.
    * 
    * @param str1 - One of the std::strings to compare
    * @param str2 - The other std::string to compare
    * @return true if the strings are equal
    * @return false if the strings are not equal
    */
    bool iequals(const std::string& str1, const std::string& str2) {
        if (str1.size() != str2.size()) return false;

        return std::equal(str1.begin(), str1.end(), str2.begin(), [](char c1, char c2) {
            return std::tolower(static_cast<unsigned char>(c1)) == 
                   std::tolower(static_cast<unsigned char>(c2));
        });
    }

    /**
    * @brief Convert a vector of std::strings into a vector of char*
    * 
    * @param v the vector of std::strings
    * @return std::vector<char*> the converted vector of char*
    */
    std::vector<char*> to_vchar(const std::vector<std::string>& v) {
        std::vector<char*> vchr;
        vchr.reserve(v.size() + 1); // Optimization: prevent reallocations

        for(const auto& str : v) {
            vchr.push_back(const_cast<char*>(str.c_str()));
        }
        vchr.push_back(nullptr);

        return vchr;
    }

    /**
    * @brief Validates whether a variable name is a valid POSIX identifier.
    *
    * @param name The identifier to check.
    * @return True if valid, false otherwise.
    */
    bool is_valid_identifier(const std::string& name) {
        if (name.empty() || (!std::isalpha(static_cast<unsigned char>(name[0])) && name[0] != '_'))
            return false;

        for (char c : name) {
            if (!std::isalnum(static_cast<unsigned char>(c)) && c != '_')
                return false;
        }
        return true;
    }

}