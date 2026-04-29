/**
 * @file utils.cpp
 * @brief Implementation for utils.hpp
 * 
 * @author Filipe Paredes (filipeparedes3@gmail.com)
 * 
 * @version 0.1
 * @date 2026-04-29
 * 
 * @copyright Copyright (c) 2026
 * 
*/

#include "utils.hpp"

#include <string.h>
#include <iostream>
#include <pwd.h>
#include <unistd.h>

namespace cppsh {

    std::string read_input() {
        std::string line;

        if (!std::getline(std::cin, line)) return "";

        return line;
    }

    std::string get_cwd() {
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

    std::string get_hostname() {
        char hostname[_POSIX_HOST_NAME_MAX];
        return gethostname(hostname, sizeof(hostname)) == 0 ? hostname : "localhost";
    }

    std::string get_username() {
        struct passwd* pw = getpwuid(getuid());
        return pw ? pw->pw_name : "user";
    }
}