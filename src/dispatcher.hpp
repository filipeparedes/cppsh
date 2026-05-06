/**
 * @file dispatcher.hpp
 *
 * @brief Declaration for the command dispatcher.
 * 
 * The dispatcher receives a Command, finds respective CommandEntry, 
 * and calls the function in the found entry.
 * 
 * @author Filipe Paredes (filipeparedes3@gmail.com)
 * 
 * @version 0.1
 * @date 2026-05-03
 * 
 * @copyright Copyright (c) 2026
 * 
 */

#pragma once

#include "command.hpp"
#include <vector>

class Dispatcher {
    public:
        Dispatcher();

        /**
         * @brief Dispatches a Command
         * 
         * @param cmd -  the Command to dispatch
         * @return The status code
         */
        int dispatch(const cppsh::Command& cmd);

    private:
        std::vector<cppsh::CommandEntry> entries;
};



