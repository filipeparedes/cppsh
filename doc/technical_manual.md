# cppsh — Technical Manual

> Version: v0.4-beta - covers the fourth release of cppsh.

## Table of Contents

1. [Architecture Overview](#architecture-overview)
2. [Main Loop](#main-loop)
3. [Parser](#parser)
4. [Dispatcher](#dispatcher)
5. [Execution](#execution)
6. [Signal Handling](#signal-handling)
7. [Error Handling](#error-handling)
8. [Shell State](#shell-state)
9. [Builtin Registry](#builtin-registry)
10. [Built-in Commands](#built-in-commands)
11. [Tab Completion](#tab-completion)
12. [Script Execution](#script-execution)
13. [Utility Library](#utility-library)
14. [Tests](#tests)

---

## Architecture Overview

cppsh follows a procedural, data-oriented architecture built on C++23 modules. All components are free functions operating on plain data structs without any classes, inheritance, or virtual dispatch.

```
main(argc, argv)
  ├── execute_file(argv[1])           — script mode
  └── run()                           — interactive mode
        ├── init_builtins()
        ├── handle_signal()
        ├── setup_autocompletion()
        ├── read_input(prompt)        — readline-based
        ├── parse(input, env, exit_code)
        │     ├── tokenize()          — char-by-char, quotes, $VAR expansion
        │     ├── is_bg()             — detects &
        │     ├── split_logical()     — splits on &&, ||
        │     ├── split()             — splits on |
        │     └── redirect_io()       — detects <, >, >>
        │
        └── dispatch(log_pl)
              ├── evaluate_state()    — && / || state machine
              ├── try_execute_builtin()
              │     ├── handle_assignment()   — VAR=value
              │     ├── builtin_help()        — inline
              │     └── get_builtin()         — registry lookup
              │           └── handler(cmd)
              └── exec(pl)
                    ├── exec_single() — one fork + execvp
                    └── exec_pl()     — N forks + N-1 pipes
```

**Data flow:**

1. `main` routes to `run()` or `execute_file()` based on CLI arguments
2. `run()` reads input via `read_input()` (readline)
3. Input is recorded in shell state via `add_to_history()`
4. `parse()` returns `std::expected<std::vector<pipeline_t>, std::string>`
5. `dispatch()` iterates logical pipelines, evaluating `&&`/`||` state machine
6. Per pipeline, `try_execute_builtin()` checks registry; falls through to `exec()` if no match
7. Errors propagate via `std::expected<int, shell_error_t>` - no exceptions
8. Exit code stored via `set_exit_code()` after every command

---

## Main Loop

**Module:** `cppsh.shell` — `src/shell.cppm`

### `init_builtins()`

Registers all built-in commands into the builtin registry on first call (guarded by `get_builtins().empty()`). Called once at startup. Registered commands: `exit`, `cd`, `history`, `export`, `unset`, `source`. 

`help` is not registered: handled inline in the dispatcher.

### `get_prompt(user, hostname)`

Returns a formatted prompt string `user@hostname:~/path$ ` using `std::format`. Resolves the current working directory via `get_cwd()`.

### `run()`

Main REPL loop. On each iteration:

- Builds prompt via `get_prompt()`
- Reads input via `read_input(prompt)` — returns `std::optional<std::string>`; `std::nullopt` on EOF (`Ctrl+D`)
- Skips blank lines
- Records input via `add_to_history()`
- Calls `parse(input, get_env_variables(), get_last_exit_code())`
- On parse error, calls `print(error)` and continues
- Calls `dispatch(par.value())`
- On dispatch error or success, updates exit code via `set_exit_code()`

### `read_input(prompt)`

Uses `readline()` for input - provides line editing, arrow key navigation, and history. On non-empty input, calls `add_history()` to register with readline's internal history. Frees readline-allocated memory and returns `std::optional<std::string>`.

---

## Parser

**Module:** `cppsh.parsing` - `lib/parsing.cppm`

Returns `std::expected<std::vector<pipeline_t>, std::string>`. The pipeline list is built through a sequence of transformation steps.

`parse(input, env_variables, last_exit_code)` — takes the current environment map and last exit code for variable expansion.

### `tokenize(input, env_variables, last_exit_code)`

Reads the input character by character. Handles:

- **Whitespace** - token boundary when not inside quotes
- **Double quotes `"`** - preserves spaces, performs `$VAR` expansion inside
- **Single quotes `'`** - preserves everything literally, no expansion
- **Backslash `\`** - escapes the next character (ignored inside single quotes)
- **`$VAR`** - expands to the variable's value from `env_variables` or `getenv()`, inside double quotes or unquoted; `$?` expands to `last_exit_code`
- **Unclosed quotes** — treated as open until EOF

**Example:**

```
name=Filipe
echo "$name world"  →  ["echo", "Filipe world"]
echo '$name'        →  ["echo", "$name"]
echo $?             →  ["echo", "0"]
```

### `is_bg(tok_vec, pl)`

Checks if the last token is `&`. If so, sets `pl.bg = true` and removes the token.

### `split_logical(tok_vec)`

Splits the token vector on `&&` and `||` operators, producing a list of `pipeline_t` with their associated `logical_op_t` (`AND`, `OR`, or `NONE` for the first pipeline).

### `split(tok_vec, pl)`

Splits on `|`, building `command_t` entries within a single `pipeline_t`.

### `redirect_io(cmd)`

Iterates `cmd.args` looking for `<`, `>`, `>>`. On match, sets the appropriate field and erases the operator and filename from `cmd.args`. Returns `std::unexpected(message)` if no filename follows.

### `is_assignment(cmd)`

Detects `KEY=VALUE` pattern in `cmd.args[0]`. Validates the key against POSIX rules — must start with a letter or underscore, contain only alphanumeric characters or underscores. Sets `cmd.type = command_type_t::assignment` on match.

---

## Dispatcher

**Module:** `cppsh.dispatching` - `src/dispatching.cppm`

### `dispatch(log_pl)`

Receives `std::vector<pipeline_t>` -> the logical pipeline list from `parse()`. Iterates over each pipeline, applying the `&&`/`||` state machine:

```
for each pipeline_t pl:
    if run_next:
        try_execute_builtin() or exec(pl)
        update current_exit_code
    run_next = evaluate_state(pl.op, current_exit_code)
```

Returns the exit code of the last executed pipeline.

### `evaluate_state(op, exit_code)`

- `logical_op_t::AND` - returns `true` if `exit_code == 0`
- `logical_op_t::OR` - returns `true` if `exit_code != 0`
- `logical_op_t::NONE` - always `true`

### `try_execute_builtin(cmd, executed_builtin)`

1. If `cmd.type == assignment` → calls `handle_assignment()`, sets `executed_builtin = true`
2. If `cmd.args[0]` is `help` or `-h` → calls `builtin_help(cmd)` inline
3. Otherwise → calls `get_builtin(cmd.args[0])` from the registry
4. On match: saves file descriptors, applies I/O redirection via `setup_input_redirection()` / `setup_output_redirection()`, calls the handler, restores file descriptors
5. If no match: `executed_builtin = false`, dispatcher falls through to `exec(pl)`

### `handle_assignment(cmd)`

Extracts `KEY` and `VALUE` from `cmd.args[0]`. Calls `add_env_variable(key, {value, false})`. Does not export to the OS environment - `is_exported` is `false` until `export` is called.

### `setup_input_redirection(input_file)` / `setup_output_redirection(output_file, append)`

Helper functions that open the target file and call `dup2()` to redirect `STDIN_FILENO` or `STDOUT_FILENO`. Called inside `try_execute_builtin()` before the handler, with file descriptors saved and restored afterwards.

---

## Execution

**Module:** `cppsh.execution` - `src/execution.cppm`

### `exec(pl)`

Routes based on pipeline size:

- `pl.cmds.size() == 1` → `exec_single(pl)`
- `pl.cmds.size() > 1` → `exec_pl(pl)`

### `exec_single(pl)`

1. `fork()` - on failure returns `std::unexpected(FORK_FAILED)`
2. Parent - if `pl.bg`, prints PID and returns; otherwise `waitpid(WUNTRACED)`
3. Child:
   - `setpgrp()` - isolates child in own process group
   - Resets `SIGINT` and `SIGTSTP` to `SIG_DFL`
   - Applies I/O redirection via `open()` + `dup2()`
   - `execvp()` - searches `$PATH`
   - On failure - `exit(127)`

### `exec_pl(pl)`

1. Creates N-1 pipes via `pipe()`
2. Forks N children
3. Each child: connects stdin/stdout to pipe ends via `dup2()`, closes all pipe fds, applies per-command I/O redirection, calls `execvp()`
4. Parent closes all pipe ends, waits for all children
5. Returns exit code of the last command

---

## Signal Handling

**Module:** `cppsh.signal_handling` — `src/signal_handling.cppm`

### `handle_signal()`

Registers handlers for `SIGINT` and `SIGTSTP` via `sigaction`. Called once at startup.

### `handle_sigint(signum)` / `handle_sigtstp(signum)`

Parent shell ignores the signal. Child process (reset to `SIG_DFL`) terminates or suspends normally. `\r\033[K` clears the `^C`/`^Z` from the terminal line.

> Run via `./build/cppsh` directly — intermediary processes interfere with signal delivery.

---

## Error Handling

**Module:** `cppsh.shell_errors` - `src/shell_errors.cppm`

### `error_code_t`

```cpp
enum class error_code_t : int {
    // User errors (0x0000 – 0x00FF)
    COMMAND_NOT_FOUND           = 0x0000,
    INVALID_PATH                = 0x0001,
    INVALID_ARGS                = 0x0002,
    MISSING_REDIRECTION_TARGET  = 0x0003,
    INVALID_IDENTIFIER          = 0x0004,

    // System errors (0x0100 – 0xFFFF)
    FORK_FAILED                 = 0x0100,
    EXECVP_FAILED               = 0x0101,
    VECPUSH_FAILED              = 0x0102,
    MAPINSRT_FAILED             = 0x0103,
};
```

### `shell_error_t`

| Field | Type | Description |
|---|---|---|
| `code` | `error_code_t` | Error code |
| `cmd` | `std::string` | Command that caused the error |
| `arg` | `std::string` | Invalid argument |
| `usage` | `std::string` | Correct usage string |

### `get_error_template(code)`

Maps each user error code to a `constexpr std::string_view` format template. Placeholders: `{0}` → `cmd`, `{1}` → `arg`, `{2}` → `usage`.

### `print(error)`

For system errors, prints `error[0x{:04x}]: os error: N (message)` to `stderr`. For user errors, formats the template via `std::vformat()` and prints to `stderr`.

---

## Shell State

**Module:** `cppsh.shell_state` - `src/shell_state.cppm`

The shell state is managed as a function-local static singleton accessed via `get_internal_state()`. This prevents the Static Initialization Order Fiasco and avoids passing state by reference through every function call.

### `shell_state_t` (internal)

| Field | Type | Description |
|---|---|---|
| `history` | `std::vector<std::string>` | Commands executed during the session |
| `env_variables` | `std::unordered_map<std::string, env_entry_t>` | Shell environment variables |
| `last_exit_code` | `int` | Exit code of the last executed command |

### Exported accessor functions

| Function | Description |
|---|---|
| `get_history()` | Read-only reference to history |
| `add_to_history(cmd)` | Appends to history, returns `std::expected` |
| `clear_history()` | Clears history |
| `get_env_variables()` | Read-only reference to env map |
| `add_env_variable(key, entry)` | Inserts/updates variable; calls `setenv()` if `is_exported` |
| `remove_env_variable(key)` | Removes from map and calls `unsetenv()` |
| `get_last_exit_code()` | Returns last exit code |
| `set_exit_code(code)` | Stores exit code |

### `env_entry_t`

| Field | Type | Description |
|---|---|---|
| `value` | `std::string` | Variable value |
| `is_exported` | `bool` | Whether the variable is exported to child processes |

---

## Builtin Registry

**Module:** `cppsh.builtin_registry` — `src/builtin_registry.cppm`

Centralizes the list of built-in commands to decouple the dispatcher, help system, and individual commands. Resolves the circular dependency that would arise if the dispatcher imported each builtin directly.

### `get_builtins()`

Returns a read-only reference to the registered entries vector.

### `get_builtin(name)`

Searches for a builtin by name. Returns `std::optional<std::reference_wrapper<const command_entry_t>>` - a const reference wrapper to avoid copying entries.

### `add_builtin(entry)`

Appends a `command_entry_t` to the registry. Returns `std::expected<int, shell_error_t>` - fails with `VECPUSH_FAILED` on allocation failure.

Builtins are registered during `init_builtins()` in `shell.cppm`, not at module initialization - avoiding static initialization order issues.

---

## Built-in Commands

**Directory:** `src/commands/builtins/`

All builtins share the handler signature:

```cpp
using command_handler_t = std::expected<int, shell_error_t>(*)(const command_t&);
```

Note: handlers no longer receive `shell_state_t&` - state is accessed via the singleton.

### `builtin_exit`

Prints `"Exiting program..."` and calls `exit(0)`.

### `builtin_cd`

Changes directory via `chdir()`. No args → `$HOME`. On failure → `std::unexpected(INVALID_PATH)`.

### `builtin_history`

Calls `get_history()` and prints each entry with a 1-based index.

### `builtin_help`

Calls `get_builtins()` and prints each entry's name and description. Not in the registry - called inline in the dispatcher.

### `builtin_export`

- `export VAR=value` - sets variable and marks `is_exported = true`, calls `setenv()`
- `export VAR` - promotes an existing shell variable to exported
- `export` (no args) - lists all exported variables

### `builtin_unset`

Removes a variable via `remove_env_variable()`, which calls `unsetenv()`.

### `builtin_source`

Reads a script file line by line and dispatches each line through the full parse → dispatch pipeline within the current shell context. Allows scripts to mutate shell state (cd, export, unset).

---

## Tab Completion

**Module:** `cppsh.completion` - `src/completion.cppm`

### `setup_autocompletion()`

Registers a readline completion function via `rl_completion_entry_function` or `rl_attempted_completion_function`. Called once at startup.

Completion behaviour:

- **Built-in commands** - completed from `get_builtins()`
- **External binaries** - searched in all directories listed in `$PATH`
- **Environment variables** - triggered when input starts with `$`; completed from `get_env_variables()`
- **Files and directories** - fallback for paths and arguments

---

## Script Execution

**Module:** `cppsh.scripting` — `src/scripting.cppm`

### `execute_file(path)`

Opens a script file and reads it line by line. For each line:

- Skips empty lines and comments (`#`)
- Calls `parse()` then `dispatch()` - same pipeline as interactive mode
- Updates exit code via `set_exit_code()`

Returns the exit code of the last executed command, propagated to the OS via `main()`.

Scripts run in the current shell context - `source` and direct execution share the same state.

---

## Utility Library

**Module:** `cppsh.utils` — `lib/utils.cppm`

### `get_username()`

Calls `getpwuid(getuid())`. Falls back to `"user"`.

### `get_hostname()`

Calls `gethostname()`. Falls back to `"localhost"`.

### `get_cwd()`

Calls `getcwd()`. Substitutes `$HOME` with `~`. Returns `"?"` on failure.

### `read_input(prompt)`

Wraps `readline()`. Returns `std::optional<std::string>` — `std::nullopt` on EOF.

### `iequals(a, b)`

Case-insensitive comparison via `std::equal` + `std::tolower`.

### `to_vchar(v)`

Converts `std::vector<std::string>` to `std::vector<char*>` terminated with `nullptr`.

---

## Tests

**Directory:** `tests/`

```bash
cd build && ninja && ./tests
```

### `parser_test.cpp`

| Test | Description |
|---|---|
| `SimpleCommand` | `"ls"` → 1 command, 1 arg |
| `CommandWithArguments` | `"ls -la /home"` → 3 args |
| `MultipleSpaces` | `"ls  -la"` → 2 args |
| `OnlySpaces` | `"   "` → empty |
| `InputRedirection` | `"sort < input.txt"` → `input_file` set |
| `OutputRedirection` | `"echo 'teste' > output.txt"` → `output_file` set, `append = false` |
| `AppendRedirection` | `"echo 'teste' >> output.txt"` → `append = true` |
| `IORedirection` | `"sort < input.txt > output.txt"` → both fields set |
| `InputRedirectionBeforeCommand` | `"< input.txt sort"` → correct order |
| `BackgroundFlag` | `"sleep 10 &"` → `pl.bg = true` |
| `MissingInputFile` | `"sort <"` → `std::unexpected` |
| `MissingOutputFile` | `"echo >"` → `std::unexpected` |
| `SimpleVarExpansion` | `"echo $USER"` → expanded value |
| `VarExpansionInDoubleQuotes` | `"echo \"user: $USER\""` → expanded inside quotes |
| `VarExpansionInSingleQuotes` | `"echo '$USER'"` → literal, no expansion |
| `VarConcatenation` | `"cat prefix_$FILE.txt"` → concatenated with literal |
| `UnsetVarExpandsToEmpty` | `"echo $NON_EXISTENT"` → token dropped |
| `MultipleVarsInOneToken` | `"cd $DIR/$FILE"` → both vars expanded |
| `IsolatedDollarSign` | `"echo $"` → `$` treated as literal |

### `pipe_test.cpp`

| Test | Description |
|---|---|
| `SimpleTwoStagePipe` | `"ls \| grep"` → 2 commands |
| `PipeWithArguments` | `"ls -la \| grep txt"` → args preserved per command |
| `MultiplePipesChain` | `"cat \| grep \| wc -l"` → 3 commands |
| `PipeWithExcessiveSpaces` | Multiple spaces handled correctly |
| `PipeMixedWithIORedirection` | Per-command I/O in pipeline |

### `dispatcher_test.cpp`

| Test | Description |
|---|---|
| `EmptyCommandReturnsZero` | Empty logical pipeline returns `0` |
| `UnknownCommandReturnsError` | Returns `std::unexpected` with `COMMAND_NOT_FOUND` |
| `CdInvalidPathReturnsErrorCode` | Returns `0` with error code as value - error handled by builtin |
| `CdValidPathReturnsZero` | `cd /tmp` returns `0` |
| `HistoryEmptyReturnsZero` | `history` with cleared state returns `0` |
| `HistoryWithEntriesReturnsZero` | `history` with entries returns `0` |
| `LogicalAndSkipsOnFailure` | Failed `cd` with `AND` - second command skipped |
| `LogicalOrRunsOnFailure` | Failed `cd` with `OR` - second command runs |
| `ExportSetsEnvironmentVariable` | `export VAR=value` - value visible via `getenv()` |
| `UnsetRemovesEnvironmentVariable` | `unset VAR` - variable removed from OS environment |