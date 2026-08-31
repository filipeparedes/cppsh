# cppsh

A Unix shell implementation in C++23 supporting built-in commands, pipelines, I/O redirection, and external binary execution — built from scratch with raw system calls and C++23 modules.

> Developed as a personal project to deepen knowledge of systems programming, C++ and Unix internals.

---

## Features

- **Interactive prompt** — displays `user@hostname:~/path$` with home directory substitution
- **Command dispatcher** — table-driven dispatch system with case-insensitive matching
- **Built-in commands** — `cd`, `exit`, `help`, `history`, `export`, `unset`, `source`
- **External binary execution** — runs any binary in `$PATH` via `fork` + `execvp`
- **Pipelines** — connects commands via `|`, supporting chains of arbitrary length
- **I/O Redirection** — `>`, `>>`, `<` with per-command redirection in pipelines
- **Background execution** — runs commands in the background with `&`
- **Quotes and escape characters** — single/double quotes and `\` escape support
- **Environment variables** — assignment (`VAR=value`), expansion (`$VAR`, `"$VAR"`), `$?` for last exit code
- **Logical operators** — `&&` and `||` for conditional command execution
- **Tab completion** — completes built-ins, external binaries, `$VAR`, files and directories
- **Line editing** — readline integration with arrow key navigation and history
- **Script execution** — runs shell scripts via CLI (`./cppsh script.sh`) or `source`
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
- readline
- Google Test (for tests only)

```bash
brew install llvm cmake ninja readline googletest  # macOS
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

### Run a script

```bash
./build/cppsh script.sh
```

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
| `export` | `export [VAR=value]` | Export a variable to the environment |
| `unset` | `unset [VAR]` | Remove a variable from the environment |
| `source` | `source [file]` | Execute a script in the current shell context |

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
│   │   │   ├── history.cppm
│   │   │   ├── export.cppm
│   │   │   ├── unset.cppm
│   │   │   └── source.cppm
│   │   └── command_entry.cppm      # command_entry_t — dispatch table entry
│   ├── builtin_registry.cppm       # centralized builtin registry
│   ├── completion.cppm             # tab completion via readline
│   ├── dispatching.cppm            # dispatch() — logical pipeline evaluation
│   ├── execution.cppm              # exec(), exec_single(), exec_pl()
│   ├── scripting.cppm              # execute_file() — script execution
│   ├── shell_errors.cppm           # error_code_t, shell_error_t, print()
│   ├── shell_state.cppm            # singleton state — history, env, exit code
│   ├── shell.cppm                  # run(), init_builtins(), get_prompt()
│   ├── signal_handling.cppm        # handle_signal(), SIGINT, SIGTSTP
│   └── main.cpp                    # Entry point — routes interactive vs script mode
├── lib/
│   ├── command.cppm                # command_t struct
│   ├── env_entry.cppm              # env_entry_t struct
│   ├── pipeline.cppm               # pipeline_t struct
│   ├── parsing.cppm                # parse(), tokenize(), $VAR expansion
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
main(argc, argv)
  ├── execute_file(argv[1])          — script mode
  └── run()                          — interactive mode
        ├── init_builtins()
        ├── handle_signal()
        ├── setup_autocompletion()
        ├── read_input(prompt)       — readline-based
        ├── parse(input, env, $?)
        │     ├── tokenize()         — char-by-char, quotes, $VAR expansion
        │     ├── is_bg()            — detects &
        │     ├── split_logical()    — splits on &&, ||
        │     ├── split()            — splits on |
        │     └── redirect_io()      — detects <, >, >>
        │
        └── dispatch(log_pl)
              ├── evaluate_state()   — && / || state machine
              ├── try_execute_builtin()
              │     ├── handle_assignment()   — VAR=value
              │     ├── builtin_help()        — inline
              │     └── get_builtin()         — registry lookup
              └── exec(pl)
                    ├── exec_single()  — fork + execvp
                    └── exec_pl()      — N forks + N-1 pipes
```

---

## Error Handling

cppsh uses `std::expected` throughout — no exceptions. Functions that can fail return `std::expected<int, shell_error_t>`.

**User errors** — plain language messages:
```
cd: /naoexiste: No such directory
ls: Unknown command
unset: `123VAR`: not a valid identifier
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

### v0.3-beta
- ✅ I/O Redirection (>, >>, <)
- ✅ Pipes (|)
- ✅ Background execution (&)
- ✅ Quotes and escape characters
- ✅ Migrate to C++23 modules + procedural/DOD architecture (unplanned)

### v0.4-beta (current)
- ✅ Environment variables support
- ✅ Logical operators (&&, ||)
- ✅ Tab completion and line editing
- ✅ Script file execution

### v1.0 (WIP)
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
