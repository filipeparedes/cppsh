module;
/**
 * @file sys_utils.cppm
 * @brief Implementation of some utilitary OS-related functions
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
#include <pwd.h>
#include <unistd.h>
#include <climits>
#include <cstdlib>

export module utils.sys_utils;

export namespace sys_utils {

    /**
    * @brief Resolves the current working directory.
    * 
    * Replaces the home directory prefix with - if applicable.
    * 
    * @return A human-readable representation of the current directory.
    */
    std::string get_cwd() {
        char buffer[PATH_MAX];
        if (getcwd(buffer, sizeof(buffer)) == nullptr)
            return "?";
    

        std::string cwd(buffer);

        //Replace home directory prefix with ~
        const char* home = std::getenv("HOME");
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
    * @return "localhost", in case of unsuccess retrieving the data
    */
    std::string get_hostname() {
        char hostname[_POSIX_HOST_NAME_MAX];
        return gethostname(hostname, sizeof(hostname)) == 0 ? hostname : "localhost";
    }

    /**
    * @brief Get the current username
    * 
    * @return The username
    * @return "user", in case of unsuccess retrieving the data
    */
    std::string get_username() {
        struct passwd* pw = getpwuid(getuid());
        return pw ? pw->pw_name : "user";
    }

}