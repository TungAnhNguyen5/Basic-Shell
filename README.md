# Basic Shell

## Overview
This project is a custom shell implementation that includes support for built-in commands, command history, and signal handling. The shell provides essential functionality while ensuring proper error handling and user-friendly behavior.

## Project Structure
The project follows a structured organization:
```
Basic-Shell/
│
│── gtest/
│   ├── shell_test.cpp
│  
│── include/
│   ├── msgs.h               
│   ├── shell.h
│         
│── src/                     
│── shell.c
│              
│── CMakeLists.txt                
│── shell                               
```

## Internal Commands
Internal commands are built into the shell and do not fork new processes. All commands are case-sensitive. Error messages follow the macros defined in `msgs.h`.

### 1. `exit`
- Exits the shell.
- If an argument is provided, the shell does not exit and displays:
  ```
  exit: too many arguments.
  ```

### 2. `pwd`
- Displays the current working directory using `getcwd()`.
- If `getcwd()` fails, displays:
  ```
  pwd: unable to get current directory.
  ```
- If an argument is provided, aborts with:
  ```
  pwd: too many arguments.
  ```

### 3. `cd`
- Changes the current working directory using `chdir()`.
- If `chdir()` fails, displays:
  ```
  cd: unable to change directory.
  ```
- If more than one argument is provided, aborts with:
  ```
  cd: too many arguments.
  ```
- Supports additional features:
  - `cd` with no arguments changes to the home directory.
  - `cd ~` changes to the home directory.
  - `cd ~/directory` changes to `directory` inside the home directory.
  - `cd -` switches back to the previous directory.

## History Feature
This shell maintains a history of the last 10 commands executed.
- Commands are numbered sequentially, starting at 0.
- If fewer than 10 commands exist, all are displayed.
- The list includes both internal and external commands.
- Output format:
  ```
  [command_number] [command]
  ```
- Background commands (`&`) are included with `&` in the history.

### 1. `history`
- Displays the 10 most recent commands in descending order.

### 2. `!n`
- Runs command number `n` from history.
- If `n` is invalid, displays:
  ```
  history: command invalid.
  ```

### 3. `!!`
- Runs the last executed command.
- If there is no history, displays:
  ```
  history: no command entered.
  ```
- The executed command is displayed before running.
- `!n` and `!!` are not added to history, but the command they execute is added.

## Help Command
### `help`
- Displays information about internal commands.
- If an argument is provided:
  - If the argument is an internal command, displays its description.
  - If the argument is not an internal command, displays:
    ```
    [command]: external command or application.
    ```
- If multiple arguments are provided, displays:
  ```
  help: too many arguments.
  ```
- If no arguments are provided, lists all internal commands and their descriptions.

## Signal Handling
- `CTRL-C` does not terminate the shell.
- A signal handler is implemented for `SIGINT`.
- The handler:
  - Displays help information.
  - Re-displays the command prompt.
- System calls interrupted by `SIGINT` are properly handled by checking `errno`.

## Error Handling
- All errors use the macros defined in `msgs.h`.
- System calls are checked for errors.
- Errors caused by `SIGINT` are handled separately from other errors.

## Usage
Compile and run the shell:
```sh
make
./myshell
```

Exit the shell with `exit`. Use `help` for available commands.
