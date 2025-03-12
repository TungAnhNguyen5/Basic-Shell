#ifndef SHELL_N
#define SHELL_N

#include <stddef.h>

void display_prompt();

void read_command(char *buffer, size_t size);

void execute_command_fg(char **args);

void execute_command_bg(char **args);

void cleanup_zombies();

void handle_exit(char **args);

void print_working_directory(char **args);

void display_help(char **args);

void sigint_handler(int sig);

int handle_internal_commands(char **args);

void add_to_history(const char *command);

void display_history();

void execute_from_history(int command_number);

void execute_last_command();

int parse_history_command(char *input);

#endif
