#define _POSIX_SOURCE

#include <errno.h>
#include <limits.h>
#include <pwd.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#include "../include/msgs.h"
#include "../include/shell.h"

#define HISTORY_SIZE 10

char previous_dir[1024] = "";

void display_prompt() {
  char cwd[1024];
  if (getcwd(cwd, sizeof(cwd)) != NULL) {
    write(STDOUT_FILENO, cwd, strlen(cwd));
    write(STDOUT_FILENO, "$ ", 2);
  } else {
    write(STDERR_FILENO, FORMAT_MSG("shell", GETCWD_ERROR_MSG),
          strlen(FORMAT_MSG("shell", GETCWD_ERROR_MSG)));
  }
}

void read_command(char *buffer, size_t size) {
  ssize_t bytes_read = read(STDIN_FILENO, buffer, size);
  if (bytes_read == -1) {
    write(STDERR_FILENO, FORMAT_MSG("shell", READ_ERROR_MSG),
          strlen(FORMAT_MSG("shell", READ_ERROR_MSG)));
    return;
  }

  if (bytes_read > 0) {
    buffer[bytes_read - 1] = '\0';
  }
}

void execute_command_fg(char **args) {
  if (args == NULL || args[0] == NULL) {
    return;
  }

  pid_t pid = fork();

  if (pid == -1) {
    write(STDERR_FILENO, FORMAT_MSG("shell", FORK_ERROR_MSG),
          strlen(FORMAT_MSG("shell", FORK_ERROR_MSG)));
  } else if (pid == 0) {
    // Child process: Execute the command
    if (execvp(args[0], args) == -1) {
      write(STDERR_FILENO, FORMAT_MSG("shell", EXEC_ERROR_MSG),
            strlen(FORMAT_MSG("shell", EXEC_ERROR_MSG)));
      _exit(EXIT_FAILURE);
    }
  } else {
    // Parent process: Wait for the child process to finish
    int status;
    if (waitpid(pid, &status, 0) == -1) {
      write(STDERR_FILENO, FORMAT_MSG("shell", WAIT_ERROR_MSG),
            strlen(FORMAT_MSG("shell", WAIT_ERROR_MSG)));
    }
  }
}

void execute_command_bg(char **args) {
  if (args == NULL || args[0] == NULL) {
    return;
  }

  pid_t pid = fork();

  if (pid == -1) {
    write(STDERR_FILENO, FORMAT_MSG("shell", FORK_ERROR_MSG),
          strlen(FORMAT_MSG("shell", FORK_ERROR_MSG)));
  } else if (pid == 0) {
    // Child process: Execute the command
    if (execvp(args[0], args) == -1) {
      write(STDERR_FILENO, FORMAT_MSG("shell", EXEC_ERROR_MSG),
            strlen(FORMAT_MSG("shell", EXEC_ERROR_MSG)));
      _exit(EXIT_FAILURE);
    }
  } else {
    // Parent process: Background execution, do not wait for the child
    write(STDOUT_FILENO, "Process running in the background\n",
          strlen("Process running in the background\n"));
  }
}

void cleanup_zombies() {
  pid_t pid;
  int status;

  while ((pid = waitpid(-1, &status, WNOHANG)) > 0) {
    write(STDOUT_FILENO, "Cleaned up background process\n",
          strlen("Cleaned up background process\n"));
  }

  if (pid == -1 && errno != ECHILD) {
    write(STDERR_FILENO, FORMAT_MSG("shell", WAIT_ERROR_MSG),
          strlen(FORMAT_MSG("shell", WAIT_ERROR_MSG)));
  }
}

// task 1: internal commands
void change_directory(char **args) {
  char cwd[1024];
  char *home_dir = getenv("HOME");
  if (home_dir == NULL) {
    struct passwd *pw = getpwuid(getuid());
    home_dir = pw->pw_dir;
  }

  if (args[1] == NULL) {
    if (chdir(home_dir) == -1) {
      write(STDERR_FILENO, FORMAT_MSG("cd", CHDIR_ERROR_MSG),
            strlen(FORMAT_MSG("cd", CHDIR_ERROR_MSG)));
    }
  } else if (strcmp(args[1], "~") == 0) {
    if (chdir(home_dir) == -1) {
      write(STDERR_FILENO, FORMAT_MSG("cd", CHDIR_ERROR_MSG),
            strlen(FORMAT_MSG("cd", CHDIR_ERROR_MSG)));
    }
  } else if (strcmp(args[1], "-") == 0) {
    if (strlen(previous_dir) == 0) {
      write(STDERR_FILENO, "cd: no previous directory\n", 27);
    } else {
      if (chdir(previous_dir) == -1) {
        write(STDERR_FILENO, FORMAT_MSG("cd", CHDIR_ERROR_MSG),
              strlen(FORMAT_MSG("cd", CHDIR_ERROR_MSG)));
      }
    }
  } else if (args[2] != NULL) {
    write(STDERR_FILENO, FORMAT_MSG("cd", TMA_MSG),
          strlen(FORMAT_MSG("cd", TMA_MSG)));
  } else {
    if (getcwd(cwd, sizeof(cwd)) != NULL) {
      strncpy(previous_dir, cwd, sizeof(previous_dir) - 1);
      previous_dir[sizeof(previous_dir) - 1] = '\0';
    }
    if (chdir(args[1]) == -1) {
      write(STDERR_FILENO, FORMAT_MSG("cd", CHDIR_ERROR_MSG),
            strlen(FORMAT_MSG("cd", CHDIR_ERROR_MSG)));
    }
  }
}

void print_working_directory(char **args) {
  if (args[1] != NULL) {
    write(STDERR_FILENO, FORMAT_MSG("pwd", TMA_MSG),
          strlen(FORMAT_MSG("pwd", TMA_MSG)));
    return;
  }

  char cwd[1024];
  if (getcwd(cwd, sizeof(cwd)) != NULL) {
    write(STDOUT_FILENO, cwd, strlen(cwd));
    write(STDOUT_FILENO, "\n", 1);
  } else {
    write(STDERR_FILENO, FORMAT_MSG("pwd", GETCWD_ERROR_MSG),
          strlen(FORMAT_MSG("pwd", GETCWD_ERROR_MSG)));
  }
}

void display_help(char **args) {
  if (args[1] != NULL && args[2] != NULL) {
    write(STDERR_FILENO, FORMAT_MSG("help", TMA_MSG),
          strlen(FORMAT_MSG("help", TMA_MSG)));
    return;
  }

  if (args[1] == NULL) {
    write(STDOUT_FILENO, FORMAT_MSG("help", HELP_HELP_MSG),
          strlen(FORMAT_MSG("help", HELP_HELP_MSG)));
    write(STDOUT_FILENO, FORMAT_MSG("cd", CD_HELP_MSG),
          strlen(FORMAT_MSG("cd", CD_HELP_MSG)));
    write(STDOUT_FILENO, FORMAT_MSG("exit", EXIT_HELP_MSG),
          strlen(FORMAT_MSG("exit", EXIT_HELP_MSG)));
    write(STDOUT_FILENO, FORMAT_MSG("pwd", PWD_HELP_MSG),
          strlen(FORMAT_MSG("pwd", PWD_HELP_MSG)));
    write(STDOUT_FILENO, FORMAT_MSG("history", HISTORY_HELP_MSG),
          strlen(FORMAT_MSG("history", HISTORY_HELP_MSG)));
  } else {
    if (strcmp(args[1], "exit") == 0) {
      write(STDOUT_FILENO, FORMAT_MSG("exit", EXIT_HELP_MSG),
            strlen(FORMAT_MSG("exit", EXIT_HELP_MSG)));
    } else if (strcmp(args[1], "pwd") == 0) {
      write(STDOUT_FILENO, FORMAT_MSG("pwd", PWD_HELP_MSG),
            strlen(FORMAT_MSG("pwd", PWD_HELP_MSG)));
    } else if (strcmp(args[1], "cd") == 0) {
      write(STDOUT_FILENO, FORMAT_MSG("cd", CD_HELP_MSG),
            strlen(FORMAT_MSG("cd", CD_HELP_MSG)));
    } else if (strcmp(args[1], "history") == 0) {
      write(STDOUT_FILENO, FORMAT_MSG("history", HISTORY_HELP_MSG),
            strlen(FORMAT_MSG("history", HISTORY_HELP_MSG)));
    } else {
      write(STDOUT_FILENO, args[1], strlen(args[1]));
      write(STDOUT_FILENO, ": ", 2);
      write(STDOUT_FILENO, EXTERN_HELP_MSG, strlen(EXTERN_HELP_MSG));
      write(STDOUT_FILENO, "\n", 1);
    }
  }
}

void handle_exit(char **args) {
  if (args == NULL || args[0] == NULL) {
    exit(0);
  }

  if (args[1] != NULL) {
    write(STDERR_FILENO, FORMAT_MSG("exit", TMA_MSG),
          strlen(FORMAT_MSG("exit", TMA_MSG)));
  } else {
    exit(0);
  }
}

void sigint_handler(int sig) {
  write(STDOUT_FILENO, "\nCaught SIGINT\n", 15);
  char *args[] = {"help", NULL};
  display_help(args);
  display_prompt();
}

int handle_internal_commands(char **args) {
  if (args == NULL || args[0] == NULL) {
    return 0;
  }

  if (strcmp(args[0], "exit") == 0) {
    handle_exit(args);
    return 1;
  } else if (strcmp(args[0], "pwd") == 0) {
    print_working_directory(args);
    return 1;
  } else if (strcmp(args[0], "cd") == 0) {
    change_directory(args);
    return 1;
  } else if (strcmp(args[0], "help") == 0) {
    display_help(args);
    return 1;
  } else if (strcmp(args[0], "history") == 0) {
    display_history();
    return 1;
  }
  return 0;
}

// Task 2: history
char *command_history[HISTORY_SIZE];
int command_count = 0;

void itoa_safe(int num, char *str) {
  int i = 0;
  int is_negative = 0;

  if (num == 0) {
    str[i++] = '0';
    str[i] = '\0';
    return;
  }

  if (num < 0) {
    is_negative = 1;
    num = -num;
  }

  while (num != 0) {
    str[i++] = (num % 10) + '0';
    num /= 10;
  }

  if (is_negative) {
    str[i++] = '-';
  }

  str[i] = '\0';

  for (int j = 0, k = i - 1; j < k; j++, k--) {
    char temp = str[j];
    str[j] = str[k];
    str[k] = temp;
  }
}

char *my_strdup(const char *s) {
  size_t len = strlen(s) + 1;
  char *copy = malloc(len);

  if (copy) {
    memcpy(copy, s, len);
  }
  return copy;
}

void add_to_history(const char *command) {
  int index = command_count % HISTORY_SIZE;

  if (command_history[index]) {
    free(command_history[index]);
  }

  command_history[index] = my_strdup(command);
  command_count++;
}

void display_history() {
  char index_buffer[16];
  int num_commands =
      (command_count < HISTORY_SIZE) ? command_count : HISTORY_SIZE;

  for (int i = num_commands - 1; i >= 0; i--) {
    int index = i % HISTORY_SIZE;
    itoa_safe(i, index_buffer);

    write(STDOUT_FILENO, index_buffer, strlen(index_buffer));
    write(STDOUT_FILENO, "\t", 1);
    write(STDOUT_FILENO, command_history[index],
          strlen(command_history[index]));
    write(STDOUT_FILENO, "\n", 1);
  }
}

void execute_from_history(int command_number) {
  if (command_number < 0 || command_number >= command_count) {
    write(STDERR_FILENO, FORMAT_MSG("history", HISTORY_INVALID_MSG),
          strlen(FORMAT_MSG("history", HISTORY_INVALID_MSG)));
    return;
  }

  int index = command_number % HISTORY_SIZE;
  char *command = command_history[index];

  write(STDOUT_FILENO, command, strlen(command));
  write(STDOUT_FILENO, "\n", 1);

  add_to_history(command);
  char *args[64];
  char *saveptr;
  int i = 0;

  char *token = strtok_r(command, " ", &saveptr);
  while (token != NULL) {
    args[i++] = token;
    token = strtok_r(NULL, " ", &saveptr);
  }
  args[i] = NULL;

  execute_command_fg(args);
}

void execute_last_command() {
  if (command_count == 0) {
    write(STDERR_FILENO, FORMAT_MSG("history", HISTORY_NO_LAST_MSG),
          strlen(FORMAT_MSG("history", HISTORY_NO_LAST_MSG)));
    return;
  }
  execute_from_history(command_count - 1);
}

int parse_history_command(char *input) {
  if (strcmp(input, "!!") == 0) {
    execute_last_command();
    return 1;
  } else if (input[0] == '!' && input[1] != '\0') {
    int command_number = atoi(&input[1]);
    if (command_number == 0 && input[1] != '0') {
      write(STDERR_FILENO, FORMAT_MSG("history", HISTORY_INVALID_MSG),
            strlen(FORMAT_MSG("history", HISTORY_INVALID_MSG)));
      return 1;
    }
    execute_from_history(command_number);
    return 1;
  }
  return 0;
}

int main() {
  char buffer[1024];
  char *args[64];
  char *saveptr;

  signal(SIGINT, sigint_handler);

  while (1) {
    display_prompt();
    read_command(buffer, sizeof(buffer));
    if (buffer[0] == '\0') {
      continue;
    }

    if (parse_history_command(buffer)) {
      continue;
    }

    add_to_history(buffer);

    int i = 0;
    char *token = strtok_r(buffer, " ", &saveptr);
    while (token != NULL) {
      args[i++] = token;
      token = strtok_r(NULL, " ", &saveptr);
    }
    args[i] = NULL;

    if (!handle_internal_commands(args)) {
      int is_background = (i > 0 && strcmp(args[i - 1], "&") == 0);
      if (is_background) {
        args[--i] = NULL;
        execute_command_bg(args);
      } else {
        execute_command_fg(args);
      }

      cleanup_zombies();
    }
  }

  return 0;
}
