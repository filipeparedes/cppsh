# cppsh — Technical Manual

> Version: v0.2-alpha — covers the second release of cppsh.

## Table of Contents

1. [Architecture Overview](#architecture-overview)
2. [Main Loop](#main-loop)
3. [Parser](#parser)
4. [Dispatcher](#dispatcher)
5. [Executor](#executor)
6. [Signal Handling](#signal-handling)
7. [Error Handling](#error-handling)
8. [Command Registry Interface](#command-registry-interface)
9. [Shell Context](#shell-context)
10. [Built-in Commands](#built-in-commands)
11. [Utility Library](#utility-library)
12. [Tests](#tests)

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
              ┌───────────────┴───────────────┐
              │                               │
         builtin_cd                        Executor
         builtin_help                    fork + execvp            
         builtin_exit         
         builtin_history              
                        
```

**Data flow:**

1. `Shell::run()` reads a line of input via `cppsh::read_input()`
2. The input is recorded in `ShellContext::history`
3. `cppsh::Parser::parse()` tokenizes the input into a `Command` struct
4. `Dispatcher::dispatch()` looks up the command in its registry and calls the matching handler
5. If no builtin matches, the `Executor` forks a child process and runs the command via `execvp`
6. If the command is not found, a `ShellError` is thrown and caught in `Shell::run()`

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

Initializes the shell. Resolves the current username and hostname via `cppsh::get_username()` and `cppsh::get_hostname()`. Initializes `context` with a reference to `dispatcher` via initializer list. Registers signal handlers via `handle_signal()`.

#### `run()`

Main loop. On each iteration:
- Prints the prompt via `print_prompt()`
- Reads input via `cppsh::read_input()`
- Exits on EOF (`Ctrl+D`)
- Skips blank lines
- Records the input in `context.history`
- Parses the input into a `Command` via `parser.parse()`
- Dispatches the command via `dispatcher.dispatch()` inside a `try/catch` block
- Catches `ShellError` and calls `e.print()`

#### `print_prompt()`

Prints the prompt in the format `user@hostname:~/path$ `. The current working directory is resolved via `get_cwd()`, which substitutes the `$HOME` prefix with `~` for a cleaner display.

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

Iterates `entries` and compares `cmd.args[0]` against each entry's `name` using `cppsh::iequals` for case-insensitive matching. On match, calls `entry.handler(cmd, context)` and returns its result.

If no builtin matches, delegates to `executor.execute(cmd)`. If the executor returns `127` (command not found in `$PATH`), throws `ShellError(ShellErrorCode::COMMAND_NOT_FOUND)`.

Returns `0` immediately if `cmd.args` is empty.

#### `get_entries()`

Returns a `const` reference to the internal `entries` vector. Used by `builtin_help` to list available commands.

---

## Executor

**File:** `src/executor.cpp`

### Class: `Executor`

Responsible for running external binaries — any command not matched by the dispatcher as a builtin.

#### `execute(const cppsh::Command& cmd)`

1. Calls `fork()` to create a child process
2. On `fork` failure, throws `ShellError(ShellErrorCode::FORK_FAILED)`
3. In the child process:
   - Calls `setpgrp()` to place the child in its own process group, isolating signal delivery
   - Resets `SIGINT` and `SIGTSTP` to `SIG_DFL` so the child responds normally to signals
   - Converts `cmd.args` to `char**` via `cppsh::to_vchar()`
   - Calls `execvp()` — the `p` variant searches `$PATH` automatically
   - If `execvp` returns, it failed — calls `exit(127)` (Unix convention for command not found)
4. In the parent process:
   - Waits for the child via `waitpid(c_pid, &status, WUNTRACED)`
   - `WUNTRACED` allows the parent to detect when the child is suspended (`Ctrl+Z`)
   - Returns the appropriate exit code based on how the child terminated:
     - `WIFEXITED` — returns `WEXITSTATUS(status)`
     - `WIFSIGNALED` — returns `0` (killed by signal, not a shell error)
     - `WIFSTOPPED` — returns `0` (suspended by `Ctrl+Z`)

---

## Signal Handling

**File:** `src/signal_handling.cpp`

### `handle_signal()`

Registers signal handlers for `SIGINT` and `SIGTSTP` using `sigaction`. Called once in `Shell::Shell()`.

### `handle_sigint(int signum)`

Called when `Ctrl+C` is pressed. The parent shell ignores `SIGINT` via `signal(SIGINT, SIG_IGN)`. The child process, having reset to `SIG_DFL` before `execvp`, is terminated normally.

### `handle_sigtstp(int signum)`

Called when `Ctrl+Z` is pressed. The parent shell ignores `SIGTSTP` via `signal(SIGTSTP, SIG_IGN)`. The child process, isolated in its own process group via `setpgrp()`, is suspended normally.

> **Note:** Run the shell directly via `./build/cppsh` rather than `make run`. Running through `make` adds an extra process layer that interferes with process group isolation and signal delivery.

---

## Error Handling

**Files:** `src/errors/error_codes.hpp`, `src/errors/shell_error.hpp`, `src/errors/shell_error.cpp`

### Error Code Ranges

| Range | Category |
|---|---|
| `0x0000` – `0x00FF` | User errors |
| `0x0100` – `0xFFFF` | System errors |

### `ShellErrorCode`

```cpp
enum class ShellErrorCode : int {
    // User errors
    COMMAND_NOT_FOUND = 0x0001,
    INVALID_PATH      = 0x0002,
    INVALID_ARGS      = 0x0003,

    // System errors
    FORK_FAILED       = 0x0100,
    EXECVP_FAILED     = 0x0101,
};
```

### Class: `ShellError`

Wraps a `ShellErrorCode` with optional context — the command name, the invalid argument, and the correct usage string. Thrown at the point of failure and caught in `Shell::run()`.

#### Members (private)

| Member | Type | Description |
|---|---|---|
| `code` | `ShellErrorCode` | Error code |
| `cmd` | `std::string` | Command that caused the error |
| `arg` | `std::string` | Invalid argument passed to the command |
| `usage` | `std::string` | Correct usage of the command |

#### `print()`

Determines the output format based on `is_system_error()`:

**User error:**
```
cppsh: 'cd': invalid path '/naoexiste'
```

**System error:**
```
cppsh: error[0x0100]: 
  --> os error: 12 (cannot allocate memory)
```

#### `is_system_error()` (private)

Returns `true` if `static_cast<int>(code) >= 0x0100`.

---

## Command Registry Interface

**File:** `src/include/icommand_registry.hpp`

### Class: `ICommandRegistry`

Abstract interface that defines the contract for any command registry. Decouples `ShellContext` from the concrete `Dispatcher` implementation, following the Dependency Inversion principle.

#### `get_entries()`

Pure virtual method. Returns a `const` reference to the list of `CommandEntry` objects.

#### `~ICommandRegistry()`

Virtual destructor — required to ensure correct destruction of derived classes when accessed through a base class pointer.

---

## Shell Context

**File:** `src/include/context.hpp`

### Struct: `ShellContext`

Holds the runtime state of the shell. Passed by reference to all command handlers.

#### Fields

| Field | Type | Description |
|---|---|---|
| `history` | `std::vector<std::string>` | List of commands executed during the session |
| `registry` | `ICommandRegistry&` | Reference to the command registry (used by `help`) |

---

## Built-in Commands

**Directory:** `src/commands/builtins/`

All built-in commands share the same handler signature:

```cpp
int handler(const cppsh::Command& command, ShellContext& context);
```

Returns `0` on success, throws `ShellError` on error.

### `builtin_exit`

Calls `exit(0)`. Does not return.

### `builtin_cd`

Changes the current working directory using `chdir()`.

- With no arguments — changes to `$HOME`, falling back to `/` if `HOME` is not set
- With a path argument — changes to the specified path
- On failure — throws `ShellError(ShellErrorCode::INVALID_PATH, "cd", path)`

### `builtin_history`

Iterates `context.history` and prints each entry with a 1-based index.

### `builtin_help`

Iterates `context.registry.get_entries()` and prints each command's name and description with a 1-based index.

---

## Utility Library

**File:** `lib/utils.cpp`

All utilities live in the `cppsh` namespace.

### `get_username()`

Calls `getpwuid(getuid())` to retrieve the current user's username. Falls back to `"user"` on failure.

### `get_hostname()`

Calls `gethostname()` to retrieve the machine hostname. Falls back to `"localhost"` on failure.

### `get_cwd()`

Calls `getcwd()` to retrieve the current working directory. Substitutes the `$HOME` prefix with `~`. Returns `"?"` on failure.

### `read_input()`

Reads a line from `stdin` using `std::getline`. Returns an empty string on EOF.

### `iequals(const std::string& a, const std::string& b)`

Case-insensitive string comparison. Uses `std::equal` with a lambda that applies `std::tolower` (via `static_cast<unsigned char>`) to each character pair. Short-circuits on size mismatch.

### `to_vchar(const std::vector<std::string>& v)`

Converts a `std::vector<std::string>` to a `std::vector<char*>` terminated with `nullptr`, suitable for passing to `execvp`. Uses `const_cast<char*>` since `execvp` expects `char**` but does not modify the strings.

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
| `UnknownCommandThrows` | Unknown command throws `ShellError` |
| `EmptyCommandReturnsZero` | Empty `args` returns `0` |
| `CdNoArgsReturnsZero` | `cd` with no arguments returns `0` |
| `CdValidPathReturnsZero` | `cd /tmp` returns `0` |
| `CdInvalidPathThrows` | Invalid path throws `ShellError` |
| `HistoryEmptyReturnsZero` | `history` with empty history returns `0` |
| `HistoryWithEntriesReturnsZero` | `history` with entries returns `0` |
| `HelpReturnsZero` | `help` returns `0` |
| `HelpIReturnsZero` | `heLP` returns `0` (case-insensitive) |
| `GetEntriesNotEmpty` | Registry is not empty on initialization |
| `GetEntriesContainsBuiltins` | Registry contains `cd`, `exit`, `help`, `history` |

### `error_test.cpp`

| Test | Description |
|---|---|
| `InvalidPathThrows` | `INVALID_PATH` throws `ShellError` |
| `CommandNotFoundThrows` | `COMMAND_NOT_FOUND` throws `ShellError` |
| `ForkFailedThrows` | `FORK_FAILED` throws `ShellError` |
| `NoThrowOnSuccess` | Constructing `ShellError` does not throw |