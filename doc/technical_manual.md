# cppsh — Technical Manual

> Version: v0.3-beta — covers the third release of cppsh.

## Table of Contents

1. [Architecture Overview](#architecture-overview)
2. [Main Loop](#main-loop)
3. [Parser](#parser)
4. [Dispatcher](#dispatcher)
5. [Execution](#execution)
6. [Signal Handling](#signal-handling)
7. [Error Handling](#error-handling)
8. [Shell State](#shell-state)
9. [Built-in Commands](#built-in-commands)
10. [Utility Library](#utility-library)
11. [Tests](#tests)

---

## Architecture Overview

cppsh follows a procedural, data-oriented architecture built on C++23 modules. All components are free functions operating on plain data structs — no classes, no inheritance, no virtual dispatch.

```
run()
  ├── read_input()
  ├── parse()                         — lib/parsing.cppm
  │     ├── tokenize()                — char-by-char lexer
  │     ├── is_bg()                   — detects &
  │     ├── split()                   — splits on |
  │     └── redirect_io()             — detects <, >, >>
  │
  └── dispatch()                      — src/dispatching.cppm
        ├── help                      — handled inline
        ├── builtin handler           — cd, exit, history
        │     └── dup2 for I/O        — save → redirect → restore
        └── exec()                    — src/execution.cppm
              ├── exec_single()       — one fork + execvp
              └── exec_pl()           — N forks + N-1 pipes
```

**Data flow:**

1. `run()` reads input via `read_input()`
2. Input is recorded in `shell_state_t::history`
3. `parse()` returns `std::expected<pipeline_t, std::string>`
4. `dispatch()` checks builtins, then falls through to `exec()`
5. `exec()` routes to `exec_single()` or `exec_pl()` based on pipeline size
6. Errors propagate via `std::expected<int, shell_error_t>` — no exceptions

---

## Main Loop

**Module:** `cppsh.shell` — `src/shell.cppm`

### `run()`

Main REPL loop. On each iteration:

- Prints the prompt via `print_prompt(user, hostname)`
- Reads input via `read_input()`
- Exits on EOF (`Ctrl+D`)
- Skips blank lines
- Records input in `state.history`
- Calls `parse(input)` — on error, calls `print(error)` and continues
- Calls `dispatch(pl, state)` — on error, calls `print(error)`

### `print_prompt(user, hostname)`

Prints the prompt in the format `user@hostname:~/path$ `. Resolves the current working directory via `get_cwd()`, substituting `$HOME` with `~`.

---

## Parser

**Module:** `cppsh.parsing` — `lib/parsing.cppm`

Returns `std::expected<pipeline_t, std::string>`. The pipeline is built through a sequence of transformation steps applied to the token vector.

### `tokenize(input)`

Reads the input character by character. Handles:

- **Whitespace** — token boundary when not inside quotes
- **Double quotes `"`** — preserves spaces, allows escape sequences inside
- **Single quotes `'`** — preserves everything literally, no escape processing
- **Backslash `\`** — escapes the next character (ignored inside single quotes)
- **Unclosed quotes** — treated as open until EOF

Uses a `Quote` enum (`None`, `Single`, `Double`) to track quoting state and an `is_escaped` flag for backslash handling.

**Example:**

```
Input:  "echo hello\ world"
Output: ["echo", "hello world"]

Input:  "echo 'no $expand'"
Output: ["echo", "no $expand"]
```

### `is_bg(tok_vec, pl)`

Checks if the last token is `&`. If so, sets `pl.bg = true` and removes the token. Must run before `split()`.

### `split(tok_vec, pl)`

Iterates the token vector looking for `|`. Each segment between pipes becomes a `command_t` with `args` set to that segment. The remaining tokens after the last pipe become the final command.

### `redirect_io(cmd)`

Iterates `cmd.args` looking for `<`, `>`, `>>`. On match:

- Sets the appropriate field on `cmd` (`input_file`, `output_file`, `append`)
- Erases the operator and filename from `cmd.args`
- Returns `std::unexpected(message)` if no filename follows the operator

### `parse(input)`

Orchestrates the pipeline:

```cpp
tokenize → is_bg → split → redirect_io (per command)
```

---

## Dispatcher

**Module:** `cppsh.dispatching` — `src/dispatching.cppm`

### `dispatch(pl, state)`

Returns `std::expected<int, shell_error_t>`.

For single-command pipelines:

1. Checks for `help` / `-h` — calls `builtin_help(cmd, entries)` directly
2. Iterates `entries` for a case-insensitive match on `cmd.args[0]`
3. On match, applies I/O redirection via `dup2`, calls the handler, then restores file descriptors
4. Falls through to `exec(pl)` if no builtin matches

For multi-command pipelines, goes directly to `exec(pl)`.

If `exec` returns `127`, returns `std::unexpected` with `error_code_t::COMMAND_NOT_FOUND`.

### `entries`

A `const std::vector<command_entry_t>` defined at module scope:

```cpp
const std::vector<command_entry_t> entries = {
    {"exit",    "Exit the shell",           "exit",        builtin_exit},
    {"cd",      "Change directory",         "cd [dir]",    builtin_cd},
    {"history", "List user's input history","history",     builtin_history},
};
```

`help` is not in the table — handled inline before the loop.

---

## Execution

**Module:** `cppsh.execution` — `src/execution.cppm`

### `exec(pl)`

Routes based on pipeline size:

- `pl.cmds.size() == 1` → `exec_single(pl)`
- `pl.cmds.size() > 1` → `exec_pl(pl)`

### `exec_single(pl)`

Executes a single external command:

1. `fork()` — creates a child process
2. On `fork` failure — returns `std::unexpected(shell_error_t{FORK_FAILED})`
3. Parent — if `pl.bg`, prints PID and returns immediately; otherwise `waitpid(WUNTRACED)`
4. Child:
   - `setpgrp()` — isolates the child in its own process group for signal handling
   - Resets `SIGINT` and `SIGTSTP` to `SIG_DFL`
   - Applies I/O redirection via `open()` + `dup2()`
   - Calls `execvp()` — searches `$PATH` automatically
   - On `execvp` failure — `exit(127)`

### `exec_pl(pl)`

Executes a pipeline of N commands with N-1 pipes:

1. Creates N-1 pipes via `pipe()`
2. Forks N children, one per command
3. Each child:
   - Connects `stdin` to the read end of the previous pipe (if not first)
   - Connects `stdout` to the write end of the next pipe (if not last)
   - Closes all pipe file descriptors after `dup2`
   - Applies per-command I/O redirection
   - Calls `execvp()`, exits with `127` on failure
4. Parent closes all pipe ends
5. Parent waits for all children — returns exit code of the last command

Closing all pipe ends in both parent and children is essential to prevent deadlocks — processes waiting for EOF that never arrives.

---

## Signal Handling

**Module:** `cppsh.signal_handling` — `src/signal_handling.cppm`

### `handle_signal()`

Registers handlers for `SIGINT` and `SIGTSTP` via `sigaction`. Called once at startup in `run()`.

### `handle_sigint(signum)`

Parent shell ignores `SIGINT`. The child process, having reset to `SIG_DFL` in `exec_single` or `exec_pl`, is terminated. The `\r\033[K` escape sequence clears the `^C` from the terminal line.

### `handle_sigtstp(signum)`

Parent shell ignores `SIGTSTP`. The child process, isolated via `setpgrp()`, is suspended. The `\r\033[K` escape sequence clears the `^Z` from the terminal line.

> Run the shell via `./build/cppsh` directly. Running through an intermediary process (e.g. `make run`) adds a process group layer that interferes with signal delivery.

---

## Error Handling

**Module:** `cppsh.shell_errors` — `src/shell_errors.cppm`

cppsh uses `std::expected` throughout — no exceptions. Every function that can fail returns `std::expected<int, shell_error_t>`.

### `error_code_t`

```cpp
enum class error_code_t : int {
    // User errors (0x0000 – 0x00FF)
    COMMAND_NOT_FOUND           = 0x0000,
    INVALID_PATH                = 0x0001,
    INVALID_ARGS                = 0x0002,
    MISSING_REDIRECTION_TARGET  = 0x0003,

    // System errors (0x0100 – 0xFFFF)
    FORK_FAILED     = 0x0100,
    EXECVP_FAILED   = 0x0101,
};
```

### `shell_error_t`

Plain data struct:

| Field | Type | Description |
|---|---|---|
| `code` | `error_code_t` | Error code |
| `cmd` | `std::string` | Command that caused the error |
| `arg` | `std::string` | Invalid argument |
| `usage` | `std::string` | Correct usage string |

### `is_system_error(error)`

Returns `true` if `static_cast<int>(error.code) >= 0x0100`.

### `print(error)`

Formats and prints the error to `stderr`:

**User error:**
```
cd: /naoexiste: No such directory
```

**System error:**
```
error[0x0100]: os error: 12 (cannot allocate memory)
```

Uses `{:04x}` format specifier for zero-padded 4-digit hex codes.

---

## Shell State

**Module:** `cppsh.shell_state` — `src/shell_state.cppm`

### `shell_state_t`

Plain data struct holding the shell's runtime state:

| Field | Type | Description |
|---|---|---|
| `history` | `std::vector<std::string>` | Commands executed during the session |

Passed by reference to builtin handlers that need access to session state.

---

## Built-in Commands

**Directory:** `src/commands/builtins/`

All builtins share the handler signature defined in `command_entry_t`:

```cpp
using command_handler_t = std::expected<int, shell_error_t>(*)(const command_t&, shell_state_t&);
```

### `builtin_exit`

Prints `"Exiting program..."` and calls `exit(0)`.

### `builtin_cd`

Changes the current working directory using `chdir()`.

- With no arguments — changes to `$HOME`, falling back to `/`
- With a path argument — changes to the specified path
- On failure — returns `std::unexpected(shell_error_t{INVALID_PATH, "cd", path})`

### `builtin_history`

Iterates `state.history` and prints each entry with a 1-based index using `std::println`.

### `builtin_help`

Receives `std::span<const command_entry_t>` — passed directly by the dispatcher. Prints each entry's name and description with a 1-based index. Not registered in the entries table — called inline in `dispatch()`.

---

## Utility Library

**Module:** `cppsh.utils` — `lib/utils.cppm`

### `get_username()`

Calls `getpwuid(getuid())` to retrieve the current username. Falls back to `"user"`.

### `get_hostname()`

Calls `gethostname()` to retrieve the machine hostname. Falls back to `"localhost"`.

### `get_cwd()`

Calls `getcwd()` to retrieve the current working directory. Substitutes `$HOME` with `~`. Returns `"?"` on failure.

### `read_input()`

Reads a line from `stdin` via `std::getline`. Returns an empty string on EOF.

### `iequals(a, b)`

Case-insensitive string comparison using `std::equal` with a lambda applying `std::tolower` via `static_cast<unsigned char>`. Short-circuits on size mismatch.

### `to_vchar(v)`

Converts `std::vector<std::string>` to `std::vector<char*>` terminated with `nullptr`, for passing to `execvp`. Uses `const_cast<char*>` since `execvp` expects `char**` but does not modify the strings.

---

## Tests

**Directory:** `tests/`

The test suite uses Google Test with C++23 module imports.

```bash
cd build && ninja && ./tests
```

### `parser_test.cpp`

| Test | Description |
|---|---|
| `SimpleCommand` | `"ls"` → 1 command, 1 arg |
| `CommandWithArguments` | `"ls -la /home"` → 3 args |
| `MultipleSpaces` | `"ls  -la"` → 2 args |
| `OnlySpaces` | `"   "` → empty pipeline |
| `InputRedirection` | `"sort < input.txt"` → `input_file = "input.txt"` |
| `OutputRedirection` | `"echo > output.txt"` → `output_file = "output.txt"`, `append = false` |
| `AppendRedirection` | `"echo >> output.txt"` → `append = true` |
| `IORedirection` | `"sort < input.txt > output.txt"` → both fields set |
| `InputRedirectionBeforeCommand` | `"< input.txt sort"` → correct |
| `BackgroundFlag` | `"sleep 10 &"` → `pl.bg = true` |
| `MissingInputFile` | `"sort <"` → `std::unexpected` |
| `MissingOutputFile` | `"echo >"` → `std::unexpected` |

### `pipe_test.cpp`

| Test | Description |
|---|---|
| `SimpleTwoStagePipe` | `"ls \| grep"` → 2 commands |
| `PipeWithArguments` | `"ls -la \| grep txt"` → args preserved per command |
| `MultiplePipesChain` | `"cat \| grep \| wc -l"` → 3 commands |
| `PipeWithExcessiveSpaces` | Multiple spaces handled correctly |
| `PipeMixedWithIORedirection` | Per-command I/O with pipes |

### `dispatcher_test.cpp`

| Test | Description |
|---|---|
| `EmptyCommandReturnsZero` | Empty pipeline returns `0` |
| `UnknownCommandReturnsError` | Returns `std::unexpected` with `COMMAND_NOT_FOUND` |
| `CdInvalidPathReturnsError` | Returns `std::unexpected` with `INVALID_PATH` |
| `CdValidPathReturnsZero` | `cd /tmp` returns `0` |
| `HistoryEmptyReturnsZero` | `history` with empty state returns `0` |
| `HistoryWithEntriesReturnsZero` | `history` with entries returns `0` |
| `HelpReturnsZero` | `help` returns `0` |
| `HelpCaseInsensitiveReturnsZero` | `heLP` returns `0` |