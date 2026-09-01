# cppsh — User Manual

> Version: v1.0

## Table of Contents

1. [Getting Started](#getting-started)
2. [The Prompt](#the-prompt)
3. [Running Commands](#running-commands)
4. [Built-in Commands](#built-in-commands)
5. [Environment Variables](#environment-variables)
6. [I/O Redirection](#io-redirection)
7. [Pipes](#pipes)
8. [Background Execution](#background-execution)
9. [Logical Operators](#logical-operators)
10. [Quotes and Escape Characters](#quotes-and-escape-characters)
11. [Tab Completion](#tab-completion)
12. [Script Execution](#script-execution)
13. [Error Messages](#error-messages)
14. [Keyboard Shortcuts](#keyboard-shortcuts)

---

## Getting Started

### Requirements

- Unix-based operating system (macOS or Linux) — cppsh uses Unix syscalls (`fork`, `execvp`, `dup2`, `pipe`, `sigaction`) and is not compatible with Windows
- C++23 or later
- CMake 3.28+
- Ninja
- LLVM clang++
- readline
- Google Test (for tests only)

### Installation

```bash
git clone https://github.com/filipeparedes/cppsh.git
cd cppsh
mkdir build && cd build
cmake .. -G Ninja
ninja
```

### Starting the shell

```bash
./build/cppsh
```

### Exiting the shell

Type `exit` or press `Ctrl+D`.

---

## The Prompt

The prompt displays your current context:

```
user@hostname:~/Directory$
```

| Part | Description |
|---|---|
| `user` | Current username |
| `hostname` | Machine hostname |
| `~/Directory` | Current directory (`~` replaces your home directory) |
| `$` | Prompt symbol |

---

## Running Commands

Any binary available in your `$PATH` can be run directly:

```bash
ls
ls -la
ls -la /tmp
grep "hello" file.txt
```

cppsh searches `$PATH` automatically — you do not need to specify the full path for standard system commands.

---

## Built-in Commands

Built-in commands are handled directly by cppsh, without spawning a child process.

### `cd` — Change Directory

```bash
cd              # go to $HOME
cd /tmp         # go to /tmp
cd ..           # go up one level
cd ~/Documents  # go to Documents in your home directory
```

If no argument is given, `cd` changes to `$HOME`. If `$HOME` is not set, it falls back to `/`.

---

### `exit` — Exit the Shell

```bash
exit
```

Exits cppsh. You can also press `Ctrl+D` to exit.

---

### `help` — List Built-in Commands

```bash
help
```

Prints a numbered list of all available built-in commands and their descriptions.

```
1 exit - Exit the shell
2 cd - Change directory
3 history - List user's input history
4 export - Create, update or list exported variables
5 unset - Delete an environment variable
6 source - Execute a script file
```

You can also use `-h` as a shorthand:

```bash
-h
```

---

### `history` — Command History

```bash
history
```

Prints a numbered list of all commands entered during the current session:

```
1  ls -la
2  cd /tmp
3  echo $HOME
```

---

### `export` — Export Variables

`export` promotes a shell variable to the OS environment, making it visible to child processes.

```bash
export FOO=bar          # define and export
export FOO              # export an already-defined variable
export                  # list all exported variables
```

**Example:**

```bash
export GREETING=hello
echo $GREETING          # hello
sh -c 'echo $GREETING'  # hello — child process can see it
```

---

### `unset` — Remove Variables

```bash
unset FOO
```

Removes `FOO` from both the shell state and the OS environment. Child processes will no longer see it.

---

### `source` — Run a Script in Current Context

```bash
source script.sh
```

Executes each line of `script.sh` in the current shell — changes to variables, directories, and exports persist after the script finishes.

This is different from running `./script.sh`, which spawns a child process and discards its state when done.

---

## Environment Variables

### Assigning Variables

```bash
NAME=User
PORT=8080
PATH_TO_PROJECT=/home/user/project
```

Variables defined this way are local to the shell — child processes cannot see them unless you `export` them.

### Exporting Variables

```bash
export NAME=User        # define and export in one step
export PORT             # export an existing variable
```

### Expanding Variables

Use `$VAR` to substitute a variable's value:

```bash
echo $NAME              # User
cd $PATH_TO_PROJECT     # changes to /home/user/project
ls $HOME                # lists your home directory
```

Inside double quotes, variables are expanded:

```bash
echo "Hello, $NAME!"    # Hello, User!
```

Inside single quotes, variables are **not** expanded:

```bash
echo 'Hello, $NAME!'    # Hello, $NAME!
```

### Last Exit Code — `$?`

`$?` expands to the exit code of the last executed command:

```bash
ls /tmp
echo $?     # 0 (success)

ls /nonexistent
echo $?     # 1 (error)
```

### Unset Variables

An undefined variable expands to nothing — the token is silently dropped:

```bash
echo $UNDEFINED_VAR     # prints an empty line
```

---

## I/O Redirection

### Output Redirection — `>`

Redirects the output of a command to a file, **overwriting** it:

```bash
ls > files.txt
echo "hello" > output.txt
```

If `files.txt` does not exist, it is created. If it does exist, it is overwritten.

### Append Redirection — `>>`

Redirects output to a file, **appending** to it:

```bash
echo "first line" > log.txt
echo "second line" >> log.txt
cat log.txt
# first line
# second line
```

### Input Redirection — `<`

Feeds a file as input to a command:

```bash
sort < names.txt
wc -l < file.txt
```

### Combining Input and Output

```bash
sort < input.txt > output.txt
```

### Redirection Before the Command

The position of the redirection operator does not matter:

```bash
< input.txt sort > output.txt
```

### Redirection with Built-ins

Redirection works with built-in commands too:

```bash
history > history.txt
help > help.txt
```

---

## Pipes

The `|` operator connects the output of one command to the input of the next:

```bash
ls | grep txt
ls -la | sort
cat file.txt | grep error | wc -l
```

### Chaining Multiple Pipes

You can chain as many commands as needed:

```bash
cat access.log | grep "404" | sort | uniq -c | sort -rn
```

### Pipes with I/O Redirection

Each command in a pipeline can have its own redirection:

```bash
sort < input.txt | grep hello > output.txt
```

Here `sort` reads from `input.txt`, its output flows into `grep`, and `grep`'s output is written to `output.txt`.

---

## Background Execution

Append `&` to run a command in the background, returning the prompt immediately:

```bash
sleep 10 &
```

cppsh prints the process ID (PID) of the background process:

```
[12345]: Background execution
```

You can use this PID to manage the process:

```bash
kill 12345      # terminate the background process
```

### Pipelines in Background

`&` applies to the whole pipeline:

```bash
cat large_file.txt | grep pattern | sort &
```

---

## Logical Operators

### `&&` — AND

The second command runs **only if** the first succeeds (exit code `0`):

```bash
mkdir /tmp/test && cd /tmp/test
```

If `mkdir` fails, `cd` is not executed.

### `||` — OR

The second command runs **only if** the first fails (non-zero exit code):

```bash
cd /tmp/test || mkdir /tmp/test
```

If `cd` fails (directory doesn't exist), `mkdir` runs.

### Chaining

You can chain multiple operators:

```bash
cd /tmp && ls && echo "done"
false || echo "first failed" || echo "second also failed"
```

Evaluation is left to right. Each operator applies to the result of the immediately preceding command.

---

## Quotes and Escape Characters

### Double Quotes `"`

Preserve spaces and allow variable expansion:

```bash
echo "hello world"          # one argument: hello world
echo "home is $HOME"        # expands $HOME
touch "my file.txt"         # file name with space
```

### Single Quotes `'`

Preserve everything literally — no expansion, no escape processing:

```bash
echo 'hello world'          # hello world
echo '$HOME'                # prints literally: $HOME
echo 'no \n escape here'    # prints literally: no \n escape here
```

### Backslash `\`

Escapes the next character, treating it as a literal:

```bash
echo hello\ world           # hello world (space escaped)
echo \$HOME                 # prints literally: $HOME
touch my\ file.txt          # file name with space
```

Backslash inside single quotes has no effect:

```bash
echo '\$HOME'               # \$HOME
```

### Unclosed Quotes

If you open a quote without closing it, everything until the end of the line is treated as part of the quoted string.

---

## Tab Completion

Press `Tab` to auto-complete:

| Context | Completes |
|---|---|
| Command name | Built-in commands and binaries in `$PATH` |
| `$` prefix | Environment variable names |
| Path | Files and directories |

**Examples:**

```bash
his<Tab>        → history
/tmp/<Tab>      → lists files in /tmp
$HO<Tab>        → $HOME
```

Press `Tab` twice to see all available completions when there are multiple matches.

---

## Script Execution

### Running a Script Directly

Pass a script file as an argument to cppsh:

```bash
./build/cppsh script.sh
```

cppsh processes each line of the script — empty lines and lines starting with `#` are ignored.

**Example script (`setup.sh`):**

```bash
# Set up environment
export PROJECT=/home/user/project
export DEBUG=1

cd $PROJECT
echo "Ready in $PROJECT"
```

```bash
./build/cppsh setup.sh
```

The exit code of the last command in the script is returned to the OS.

### `source` — Run in Current Context

```bash
source setup.sh
```

Unlike direct execution, `source` runs the script in your current shell — variables and directory changes persist after the script finishes.

### Comments

Lines starting with `#` are treated as comments and ignored:

```bash
# This is a comment
echo hello   # inline comments are not supported — this prints 'hello # inline comments are not supported'
```

> Note: only full-line comments (lines where `#` is the first non-whitespace character) are supported.

---

## Error Messages

### User Errors

Plain language messages for incorrect usage:

| Error | Example |
|---|---|
| Unknown command | `ls2: Unknown command` |
| Invalid path | `cd: /nonexistent: No such directory` |
| Invalid arguments | `cmd: 'arg' Invalid arguments` |
| Missing redirection target | `cppsh: missing redirection target after '>'` |
| Invalid identifier | `unset: \`123VAR\`: not a valid identifier` |

### System Errors

Hex error codes with OS context for internal failures:

```
error[0x0100]: os error: 12 (cannot allocate memory)
```

The hex code identifies the type of failure:

| Code | Meaning |
|---|---|
| `0x0100` | `fork()` failed |
| `0x0101` | `execvp()` failed |
| `0x0102` | Memory allocation failed (vector) |
| `0x0103` | Memory allocation failed (map) |

---

## Keyboard Shortcuts

| Shortcut | Action |
|---|---|
| `Ctrl+D` | Exit the shell (EOF) |
| `Ctrl+C` | Terminate the currently running command |
| `Ctrl+Z` | Suspend the currently running command |
| `↑` / `↓` | Navigate command history |
| `←` / `→` | Move cursor in current line |
| `Ctrl+A` | Move cursor to beginning of line |
| `Ctrl+E` | Move cursor to end of line |
| `Ctrl+L` | Clear the screen |
| `Tab` | Auto-complete command, variable or path |