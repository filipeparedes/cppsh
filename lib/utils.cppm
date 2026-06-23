module;
/**
 * @file utils.cppm
 * @brief Implementation of some utilitary functions
 * 
 * @author Filipe Paredes (filipeparedes3@gmail.com)
 * 
 * @version 1.0.0
 * @date 2026-06-19
 * 
 * @copyright Copyright (c) 2026
 * 
*/

#include <string>
#include <iostream>
#include <pwd.h>
#include <unistd.h>
#include <vector>

export module cppsh.utils;

/**
 * @brief Reads a line of input from stdin.
 * 
 * @return The line entered by the user, or an empty string on EOF.
 */
export std::string read_input() {
    std::string line;

    if (!std::getline(std::cin, line)) return "";

    return line;
}

/**
 * @brief Compares two std::strings. Case-insensitive.
 * 
 * @param str1 - One of the std::strings to compare
 * @param str2 - The other std::string to compare
 * @return true if the strings are equal
 * @return false if the strings are not equal
 */
export bool iequals(const std::string& str1, const std::string& str2) {
    if (str1.size() != str2.size()) return false;

    return std::equal(str1.begin(), str1.end(), str2.begin(), [](char c1, char c2) {
        //Cast to unsigned char value is not negative, which could result in an undefined behaviour
        return std::tolower(static_cast<unsigned char >(c1)) == 
                std::tolower(static_cast<unsigned char>(c2));
    });
}

/**
 * @brief Convert a vector of std::strings into a vector of char*
 * 
 * @param v the vector of std::strings
 * @return std::vector<char*> the converted vector of char*
 */
export std::vector<char*> to_vchar(const std::vector<std::string>& v) {
    std::vector<char*> vchr;

    for(size_t i = 0; i < v.size(); i++) {
        vchr.push_back(const_cast<char*>(v[i].c_str()));
    }
    vchr.push_back(nullptr);

    return vchr;
}

/**
 * @brief Resolves the current working directory.
 * 
 * Replaces the home directory prefix with - if applicable.
 * 
 * @return A human-readable representation of the current directory.
 */
export std::string get_cwd() {
    char buffer[PATH_MAX];

    if (getcwd(buffer, sizeof(buffer)) == nullptr) {
        return "?";
    }

    std::string cwd(buffer);

    //Replace home directory prefix with ~
    const char* home = getenv("HOME");
    if (home) {
        std::string home_str(home);
        if (cwd.find(home_str) == 0) {
            cwd = "~" + cwd.substr(home_str.size());
        }
    }

    return cwd;
}

/**
 * @brief Get the current user's hostname
 * 
 * @return The hostname
 * OR
 * @return "localhost", in case of unsuccess retrieving the data
 */
export std::string get_hostname() {
    char hostname[_POSIX_HOST_NAME_MAX];
    return gethostname(hostname, sizeof(hostname)) == 0 ? hostname : "localhost";
}

/**
 * @brief Get the current username
 * 
 * @return The username
 * OR
 * @return "user", in case of unsuccess retrieving the data
 */
export std::string get_username() {
    struct passwd* pw = getpwuid(getuid());
    return pw ? pw->pw_name : "user";
}
