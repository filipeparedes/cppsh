/**
 * @file utils.cpp
 * @brief Implementation for utils.hpp
 * 
 * @author Filipe Paredes (filipeparedes3@gmail.com)
 * 
 * @version 0.2.0
 * @date 2026-05-13
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

    bool iequals(const std::string& str1, const std::string& str2) {
        if (str1.size() != str2.size()) return false;

        return std::equal(str1.begin(), str1.end(), str2.begin(), [](char c1, char c2) {
            //Cast to unsigned char value is not negative, which could result in an undefined behaviour
            return std::tolower(static_cast<unsigned char >(c1)) == 
                   std::tolower(static_cast<unsigned char>(c2));
        });
    }

    std::vector<char*> to_vchar(const std::vector<std::string>& v) {
        std::vector<char*> vchr;

        for(size_t i = 0; i < v.size(); i++) {
            vchr.push_back(const_cast<char*>(v[i].c_str()));
        }
        vchr.push_back(nullptr);

        return vchr;
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