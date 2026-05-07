# cppsh — Technical Manual

> Version: v0.1-alpha — covers the initial release of cppsh.

## Table of Contents

1. [Architecture Overview](#architecture-overview)
2. [Main Loop](#main-loop)
3. [Parser](#parser)
4. [Dispatcher](#dispatcher)
5. [Command Registry Interface](#command-registry-interface)
6. [Shell Context](#shell-context)
7. [Built-in Commands](#built-in-commands)
8. [Utility Library](#utility-library)
9. [Tests](#tests)

---

## Architecture Overview

cppsh follows a layered architecture with clear separation of responsibilities:

```
┌─────────────────────────────────────────────────┐
│               Shell::run()                      │
│    read input → parse → dispatch → execute      │
└────────────┬──────────────┬─────────────────────┘
             │              │
     ┌───────┴──────┐  ┌────┴──────────┐
     │ cppsh::Parser│  │  Dispatcher   │
     │              │  │               │
     │ tokenize()   │  │ dispatch()    │
     │ parse()      │  │ get_entries() │
     └──────────────┘  └──────┬────────┘
                              │
              ┌───────────────┼───────────────┐
              │               │               │
         builtin_cd    builtin_help    builtin_history
         builtin_exit
```

**Data flow:**

1. `Shell::run()` reads a line of input via `cppsh::read_input()`
2. The input is recorded in `ShellContext::history`
3. `cppsh::Parser::parse()` tokenizes the input into a `Command` struct
4. `Dispatcher::dispatch()` looks up the command in its registry and calls the matching handler
5. If no handler is found, an error is printed (executor planned for v0.2-alpha)

---

## Main Loop

**File:** `src/shell.cpp`

### Class: `Shell`

The `Shell` class owns the main REPL (Read-Eval-Print Loop) and holds all top-level state.

#### Members

| Member | Type | Description |
|---|---|---|
| `m_user` | `std::string` | Current logged-in username |
| `m_hostname` | `std::string` | Machine hostname |
| `context` | `ShellContext` | Shell runtime state |
| `parser` | `cppsh::Parser` | Command parser instance |
| `dsptchr` | `Dispatcher` | Command dispatcher instance |

#### `Shell()`

Initializes the shell. Resolves the current username and hostname via `cppsh::get_username()` and `cppsh::get_hostname()`. Initializes `context` with a reference to `dispatcher` via initializer list — required because `ShellContext` holds a reference to `ICommandRegistry`.

#### `run()`

Main loop. On each iteration:
- Prints the prompt via `print_prompt()`
- Reads input via `cppsh::read_input()`
- Exits on EOF (`Ctrl+D`)
- Skips blank lines
- Records the input in `context.history`
- Parses the input into a `Command` via `parser.parse()`
- Dispatches the command via `dispatcher.dispatch()`

#### `print_prompt()`

Prints the prompt in the format `user@hostname:~/path$ `. The current working directory is resolved via `get_cwd()`, which substitutes the `$HOME` prefix with `~` for a cleaner display.

#### `get_cwd()` *(in `lib/utils`)*

Calls `getcwd()` to obtain the current directory. If the path starts with the value of `$HOME`, replaces that prefix with `~`.

#### `read_input()` *(in `lib/utils`)*

Reads a full line from `stdin` using `std::getline`. Returns an empty string on EOF.

---

## Parser

**File:** `lib/parser.cpp`

### Class: `cppsh::Parser`

Responsible for turning a raw input string into a structured `Command`.

#### `parse(const std::string& input)`

Delegates to `tokenize()` and wraps the result in a `Command` struct.

#### `tokenize(const std::string& input)`

Uses a `std::stringstream` to split the input by whitespace into a vector of string tokens. Handles multiple consecutive spaces automatically — `ss >> token` skips whitespace between extractions.

**Example:**

```
Input:  "ls  -la  /tmp"
Output: ["ls", "-la", "/tmp"]
```

---

## Dispatcher

**File:** `src/dispatcher.cpp`

### Class: `Dispatcher`

Implements `ICommandRegistry`. Holds the command dispatch table and routes incoming `Command` objects to the correct handler.

#### `Dispatcher()`

Initializes the `entries` vector with all registered built-in commands:

```cpp
{"exit",    builtin_exit,    "Exit the shell"},
{"cd",      builtin_cd,      "Change directory"},
{"history", builtin_history, "List user's input history"},
{"help",    builtin_help,    "List all built-in commands"},
```

#### `dispatch(const cppsh::Command& cmd, ShellContext& context)`

Iterates `entries` and compares `cmd.args[0]` against each entry's `name`. On match, calls `entry.handler(cmd, context)` and returns its result. If no match is found, prints an error to `stderr` and returns `1`.

Returns `0` immediately if `cmd.args` is empty.

#### `get_entries()`

Returns a `const` reference to the internal `entries` vector. Implemented inline — used by `builtin_help` to list available commands.

---

## Command Registry Interface

**File:** `src/include/icommand_registry.hpp`

### Class: `ICommandRegistry`

Abstract interface that defines the contract for any command registry. Decouples `ShellContext` from the concrete `Dispatcher` implementation, following the Dependency Inversion principle.

#### `get_entries()`

Pure virtual method. Returns a `const` reference to the list of `CommandEntry` objects. Must be implemented by any class that inherits from `ICommandRegistry`.

#### `~ICommandRegistry()`

Virtual destructor — required to ensure correct destruction of derived classes when accessed through a base class pointer.

---

## Shell Context

**File:** `src/include/context.hpp`

### Struct: `ShellContext`

Holds the runtime state of the shell. Passed by reference to all command handlers, giving them access to shared state without global variables.

#### Fields

| Field | Type | Description |
|---|---|---|
| `history` | `std::vector<std::string>` | List of commands executed during the session |
| `registry` | `ICommandRegistry&` | Reference to the command registry (used by `help`) |

`ShellContext` is initialized in `Shell::Shell()` via initializer list, receiving a reference to the `Dispatcher` instance (which implements `ICommandRegistry`).

---

## Built-in Commands

**Directory:** `src/commands/builtins/`

All built-in commands share the same handler signature:

```cpp
int handler(const cppsh::Command& command, ShellContext& context);
```

Returns `0` on success, `1` on error — following Unix convention.

### `builtin_exit`

Calls `exit(0)`. Does not return.

### `builtin_cd`

Changes the current working directory using `chdir()`.

- With no arguments — changes to `$HOME`, falling back to `/` if `HOME` is not set
- With a path argument — changes to the specified path
- On failure — prints an error message and returns `1`

The path is passed directly to `chdir()` — the OS handles `.`, `..`, absolute and relative paths natively.

### `builtin_history`

Iterates `context.history` and prints each entry with a 1-based index. Uses a range-based for loop with `const std::string&` to avoid unnecessary copies.

### `builtin_help`

Iterates `context.registry.get_entries()` and prints each command's name and description with a 1-based index.

---

## Utility Library

**File:** `lib/utils.cpp`

All utilities live in the `cppsh` namespace.

### `get_username()`

Calls `getpwuid(getuid())` to retrieve the current user's username from the system password database. Falls back to `"user"` on failure.

### `get_hostname()`

Calls `gethostname()` to retrieve the machine hostname. Falls back to `"localhost"` on failure. Uses `_POSIX_HOST_NAME_MAX` for the buffer size.

### `get_cwd()`

Calls `getcwd()` to retrieve the current working directory. Substitutes the `$HOME` prefix with `~` if applicable. Returns `"?"` on failure.

### `read_input()`

Reads a line from `stdin` using `std::getline`. Returns an empty string on EOF.

---

## Tests

**Directory:** `tests/`

The test suite uses Google Test. Run with:

```bash
make test
```

### `parser_test.cpp`

| Test | Description |
|---|---|
| Simple command | `"ls"` → `args = ["ls"]` |
| Command with arguments | `"ls -la /home"` → `args = ["ls", "-la", "/home"]` |
| Multiple spaces | `"ls  -la"` → `args = ["ls", "-la"]` |
| Blank input | `"   "` → `args = []` |

### `dispatcher_test.cpp`

| Test | Description |
|---|---|
| `UnknownCommandReturnsError` | Unknown command returns `1` |
| `EmptyCommandReturnsZero` | Empty `args` returns `0` |
| `CdNoArgsReturnsZero` | `cd` with no arguments returns `0` |
| `CdValidPathReturnsZero` | `cd /tmp` returns `0` |
| `CdInvalidPathReturnsError` | Invalid path returns `1` |
| `HistoryEmptyReturnsZero` | `history` with empty history returns `0` |
| `HistoryWithEntriesReturnsZero` | `history` with entries returns `0` |
| `HelpReturnsZero` | `help` returns `0` |
| `GetEntriesNotEmpty` | Registry is not empty on initialization |
| `GetEntriesContainsBuiltins` | Registry contains `cd`, `exit`, `help`, `history` |