# cppsh

A Unix shell implementation in C++23 supporting built-in commands, pipelines, I/O redirection, and external binary execution — built from scratch with raw system calls and C++23 modules.

> Developed as a personal project to deepen knowledge of systems programming, C++ and Unix internals.

---

## Features

- **Interactive prompt** — displays `user@hostname:~/path$` with home directory substitution
- **Command dispatcher** — table-driven dispatch system with case-insensitive matching
- **Built-in commands** — `cd`, `exit`, `help`, `history`
- **External binary execution** — runs any binary in `$PATH` via `fork` + `execvp`
- **Pipelines** — connects commands via `|`, supporting chains of arbitrary length
- **I/O Redirection** — `>`, `>>`, `<` with per-command redirection in pipelines
- **Background execution** — runs commands in the background with `&`
- **Quotes and escape characters** — single/double quotes and `\` escape support
- **Signal handling** — `Ctrl+C` terminates the running command, `Ctrl+Z` suspends it
- **Robust error handling** — `std::expected`-based error system with hex error codes for system errors
- **Command history** — tracks all executed commands during the session
- **Graceful EOF handling** — exits cleanly on `Ctrl+D`
- **Unit tested** — parser, dispatcher and pipeline covered with Google Test

---

## Requirements

- C++23 or later
- CMake 3.28+
- Ninja
- LLVM clang++
- Google Test (for tests only)

```bash
brew install llvm cmake ninja googletest  # macOS
```

---

## Building

```bash
git clone https://github.com/filipeparedes/cppsh.git
cd cppsh
mkdir build && cd build
cmake .. -G Ninja
ninja
```

### Run the shell

```bash
./build/cppsh
```

> Run directly rather than through `make` to ensure signal handling works correctly.

### Run tests

```bash
cd build && ninja && ./tests
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
│   ├── commands/
│   │   ├── builtins/
│   │   │   ├── cd.cppm
│   │   │   ├── exit.cppm
│   │   │   ├── help.cppm
│   │   │   └── history.cppm
│   │   └── command_entry.cppm      # command_entry_t — dispatch table entry
│   ├── dispatching.cppm            # dispatch() — routes Pipeline to handler
│   ├── execution.cppm              # exec(), exec_single(), exec_pl()
│   ├── shell_errors.cppm           # error_code_t, shell_error_t, print()
│   ├── shell_state.cppm            # shell_state_t — shell runtime state
│   ├── shell.cppm                  # run(), print_prompt()
│   ├── signal_handling.cppm        # handle_signal(), SIGINT, SIGTSTP
│   └── main.cpp                    # Entry point
├── lib/
│   ├── command.cppm                # command_t struct
│   ├── pipeline.cppm               # pipeline_t struct
│   ├── parsing.cppm                # parse(), tokenize(), redirect_io(), split()
│   └── utils.cppm                  # get_username(), get_hostname(), get_cwd(), iequals()
├── tests/
│   ├── parser_test.cpp
│   ├── dispatcher_test.cpp
│   └── pipe_test.cpp
├── CMakeLists.txt
└── README.md
```

---

## Architecture

```
run()
  ├── read_input()
  ├── parse()
  │     ├── tokenize()       — char-by-char, handles quotes and escape characters
  │     ├── is_bg()          — detects &, sets pipeline_t::bg
  │     ├── split()          — splits on |, builds vector<command_t>
  │     └── redirect_io()    — detects <, >, >>, sets command_t fields
  │
  └── dispatch()
        ├── help             — handled directly in dispatcher
        ├── builtin handler  — cd, exit, history
        │     └── I/O redirection via dup2 (save → redirect → restore)
        └── exec()
              ├── exec_single()   — fork + execvp + I/O redirection
              └── exec_pl()       — N forks + N-1 pipes + execvp
```

---

## Error Handling

cppsh uses `std::expected` throughout — no exceptions. Functions that can fail return `std::expected<int, shell_error_t>`.

**User errors** — plain language messages:
```
cd: /naoexiste: No such directory
ls: Unknown command
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

### v0.2-alpha
- ✅ Execute external binaries (fork + execvp)
- ✅ Signal handling (Ctrl+C, Ctrl+Z)
- ✅ Case-insensitive command dispatching
- ✅ Robust error handling system

### v0.3-beta (current)
- ✅ I/O Redirection (>, >>, <)
- ✅ Pipes (|)
- ✅ Background execution (&)
- ✅ Quotes and escape characters
- ✅ Migrate to C++23 modules + procedural/DOD architecture (unplanned)

### v0.4-beta (WIP)
- ✅ Environment variables support
- ✅ Logical operators (&&, ||)
- ✅ Tab completion and line editing
- Script file execution

### v1.0
- Total refactor & architecture review
- Subshells
- Tilde expansion (~)
- User Manual
  
### v1.1
- Command aliasing (alias & unalias)
- Built-in echo with variable expansion
- User configuration file
- Memory safety audit and leak detection
- CI/CD with GitHub Actions

---

## Documentation

For detailed technical information about the code architecture, design patterns, tests, etc. check the [Technical Manual](doc/technical_manual.md).

## Author

**Filipe Paredes** — [filipeparedes.dev](https://filipeparedes.dev) · [filipeparedes3@gmail.com](mailto:filipeparedes3@gmail.com)
