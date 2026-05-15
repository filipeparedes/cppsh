# cppsh

A Unix shell implementation in C++ supporting built-in commands, command dispatching and an extensible command registry — built from scratch with raw system calls.

> Developed as a personal project to deepen knowledge of systems programming, C++ and Unix internals.

---

## Features

- **Interactive prompt** — displays `user@hostname:~/path$` with home directory substitution
- **Command dispatcher** — extensible, table-driven dispatch system with case-insensitive matching
- **Built-in commands** — `cd`, `exit`, `help`, `history`
- **External binary execution** — runs any binary in `$PATH` via `fork` + `execvp`
- **Signal handling** — `Ctrl+C` terminates the running command, `Ctrl+Z` suspends it
- **Robust error handling** — structured error system with hex error codes for system errors
- **Command history** — tracks all executed commands during the session
- **Graceful EOF handling** — exits cleanly on `Ctrl+D`
- **Unit tested** — parser, dispatcher and error handling covered with Google Test

---

## Requirements

- C++17 or later
- CMake 3.20+
- Google Test (for tests only)

```bash
brew install cmake googletest  # macOS
```

---

## Building

```bash
git clone https://github.com/filipeparedes/cppsh.git
cd cppsh
make
```

### Run the shell

```bash
make
./build/cppsh
```

> Run directly instead of `make run` to ensure signal handling works correctly.

### Run tests

```bash
make test
```

### Clean build

```bash
make clean
```

---

## Built-in Commands

| Command | Usage | Description |
|---|---|---|
| `cd` | `cd [dir]` | Change directory. Defaults to `$HOME` if no argument given |
| `exit` | `exit` | Exit the shell |
| `help` | `help` | List all available built-in commands |
| `history` | `history` | List all commands executed in the current session |

---

## Project Structure

```
cppsh/
├── src/
│   ├── include/                    # Internal shared headers
│   │   ├── context.hpp             # ShellContext — shell runtime state
│   │   ├── dispatcher.hpp          # Command dispatcher
│   │   ├── executor.hpp            # External binary executor
│   │   ├── shell.hpp               # Main shell loop
│   │   └── icommand_registry.hpp   # ICommandRegistry interface
│   ├── commands/
│   │   ├── builtins/               # Built-in command implementations
│   │   │   ├── include/            # Built-in headers
│   │   │   ├── cd.cpp
│   │   │   ├── exit.cpp
│   │   │   ├── help.cpp
│   │   │   └── history.cpp
│   │   ├── entry.hpp               # CommandEntry — dispatch table entry
│   │   └── commands.hpp            # Centralized builtin includes
│   ├── errors/
│   │   ├── error_codes.hpp         # ShellErrorCode — hex error codes
│   │   ├── shell_error.hpp         # ShellError — structured error class
│   │   └── shell_error.cpp
│   ├── dispatcher.cpp
│   ├── executor.cpp
│   ├── signal_handling.cpp         # SIGINT and SIGTSTP handlers
│   ├── shell.cpp
│   └── main.cpp                    # Entry point
├── include/                        # Public headers
│   ├── parser.hpp                  # Input parser
│   ├── utils.hpp                   # Utility functions
│   └── command.hpp                 # Command struct
├── lib/                            # Public implementations
│   ├── parser.cpp
│   └── utils.cpp
├── tests/
│   ├── parser_test.cpp
│   ├── dispatcher_test.cpp
│   └── error_test.cpp
├── docs/
├── CMakeLists.txt
├── Makefile
└── README.md
```

---

## Architecture

cppsh follows a clean separation of concerns:

```
main.cpp
  └── Shell::run()
        ├── cppsh::read_input()       # reads user input
        ├── cppsh::Parser::parse()    # tokenizes input into Command
        └── Dispatcher::dispatch()    # routes Command to handler
              ├── builtin handler     # cd, exit, help, history
              └── Executor            # fork + execvp for external binaries
                    └── ShellError    # structured error reporting
```

The `Dispatcher` implements `ICommandRegistry`, allowing `ShellContext` to access the command registry without depending on a concrete implementation — following the Dependency Inversion principle.

---

## Error Handling

cppsh distinguishes between two categories of errors:

**User errors** — plain language messages:
```
cd: /naoexiste: no such directory
```

**System errors** — hex error codes with OS context:
```
error[0x0100]: os error: 12 (cannot allocate memory)
```

Error codes follow a structured range:

| Range | Category |
|---|---|
| `0x0000` – `0x00FF` | User errors |
| `0x0100` – `0xFFFF` | System errors |

---

## Roadmap

### v0.1-alpha
- ✅ Built-in commands: cd, exit, help, history
- ✅ Command dispatcher
- ✅ Command history
- ✅ Interactive prompt

### v0.2-alpha (current)
- ✅ Execute external binaries (fork + execvp)
- ✅ Signal handling (Ctrl+C, Ctrl+Z)
- ✅ Case-insensitive command dispatching
- ✅ Robust error handling system

### v0.3-beta (WIP)
- I/O Redirection (>, >>, <)
- Pipes (|)
- Background execution (&)
- Quotes and escape characters

### v0.4-beta
- Environment variables support
- Built-in echo with variable expansion
- Logical operators (&&, ||)
- Script file execution

### v1.0
- Command aliasing (alias & unalias)
- Tab completion and line editing
- User configuration file
- Memory safety audit and leak detection
- CI/CD with GitHub Actions

---

## Documentation

For detailed technical information about the code architecture, design patterns, tests, etc. check the [Technical Manual](doc/technical_manual.md).

## Author

**Filipe Paredes** — [filipeparedes3@gmail.com](mailto:filipeparedes3@gmail.com)