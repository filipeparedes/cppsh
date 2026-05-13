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
 * @version 0.4.0
 * @date 2026-05-07
 * 
 * @copyright Copyright (c) 2026
 * 
 */

#pragma once

#include "command.hpp"
#include "icommand_registry.hpp"
#include "executor.hpp"
#include <vector>

class Dispatcher : public ICommandRegistry {
    public:
        Dispatcher();

        /**
         * @brief Dispatches a Command
         * 
         * @param cmd -  the Command to dispatch
         * @return The status code
         */
        int dispatch(const cppsh::Command& cmd, ShellContext& context);

        /**
         * @brief Returns the list of registered command entries.
         *
         * @return A const reference to the vector of CommandEntry.
         */
        inline const std::vector<CommandEntry>& get_entries() const override { return entries; }

    private:
        Executor executor;
        std::vector<CommandEntry> entries;

};



