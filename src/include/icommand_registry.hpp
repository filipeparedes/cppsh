/**
 * @file icommand_registry.hpp
 * @brief Interface for command registry implementations.
 *
 * Defines the contract that any command registry must fulfill,
 * allowing ShellContext to depend on an abstraction rather than
 * a concrete implementation.
 *
 * @author Filipe Paredes (filipeparedes3@gmail.com)
 *
 * @version 0.1
 * @date 2026-05-07
 *
 * @copyright Copyright (c) 2026
 *
 */

#pragma once

#include <vector>
#include "commands/entry.hpp"

/**
 * @brief Interface for command registries.
 *
 * Any class that holds and exposes a list of commands must implement
 * this interface. Allows ShellContext to access command entries
 * without depending on a concrete Dispatcher implementation.
 */
class ICommandRegistry {
public:
    /**
     * @brief Returns the list of registered command entries.
     *
     * @return A const reference to the vector of CommandEntry.
     */
    virtual const std::vector<CommandEntry>& get_entries() const = 0;

    /**
     * @brief Virtual destructor.
     *
     * Required for correct destruction of derived classes through
     * a base class pointer.
     */
    virtual ~ICommandRegistry() = default;
};