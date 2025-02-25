#include "lab.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <pwd.h>
#include <readline/readline.h>
#include <readline/history.h>
#include <signal.h>

/**
 * @brief Retrieves a shell prompt string from an environment variable.
 *
 * This function attempts to retrieve a prompt string from the environment
 * variable specified by 'env'. If the environment variable is not set or
 * is NULL, it returns a default prompt "DeeShell>". The returned string
 * is dynamically allocated and must be freed by the caller.
 *
 * @param env The name of the environment variable to retrieve the prompt from.
 * @return A dynamically allocated string containing the prompt, or NULL on error.
 */
char *get_prompt(const char *env) {
    // Attempt to retrieve the prompt from the environment variable.
    const char *prompt = getenv(env);

    // If the environment variable is not set, use the default prompt.
    if (prompt == NULL) {
        prompt = "DeeShell>";
    }

    // Allocate memory for the prompt string, including the null terminator.
    char *result = malloc(strlen(prompt) + 1);

    // Check if memory allocation was successful.
    if (result == NULL) {
        // Handle memory allocation failure (e.g., log an error, return NULL).
        return NULL; // Return NULL to indicate error.
    }

    // Copy the prompt string into the allocated memory.
    strcpy(result, prompt);

    // Return the dynamically allocated prompt string.
    return result;
}

int change_dir(char **dir) {
  if (dir[1] == NULL) {
      // No argument provided, change to home directory
      const char *home = getenv("HOME");
      if (home == NULL) {
        struct passwd *pw = getpwuid(getuid());
        home = pw->pw_dir;
      }
      if (chdir(home) != 0) {
        perror("cd");
        return -1;
      }
  } else {
      // Change to the specified directory
      if (chdir(dir[1]) != 0) {
        perror("cd");
        return -1;
      }
  }
  return 0;
}
char **cmd_parse(char const *line) {
  int bufsize = 64, position = 0;
  char **tokens = malloc(bufsize * sizeof(char*));
  char *line_copy, *token;

  if (!tokens) {
    fprintf(stderr, "cmd_parse: allocation error\n");
    exit(EXIT_FAILURE);
  }

  line_copy = strdup(line);
  if (!line_copy) {
    fprintf(stderr, "cmd_parse: allocation error\n");
    exit(EXIT_FAILURE);
  }

  token = strtok(line_copy, " \t\r\n\a");
  while (token != NULL) {
      tokens[position] = token;
      position++;

      if (position >= bufsize) {
        bufsize += 64;
        tokens = realloc(tokens, bufsize * sizeof(char*));
        if (!tokens) {
          fprintf(stderr, "cmd_parse: allocation error\n");
          exit(EXIT_FAILURE);
        }
      }

      token = strtok(NULL, " \t\r\n\a");
  }
  tokens[position] = NULL;
  return tokens;
}

void cmd_free(char **line) {
  if (line == NULL) {
    return;
  }
  free(line[0]); // Free the duplicated line
  free(line);
}

char *trim_white(char *line) {
  if (line == NULL) {
    return NULL;
  }

  // Find the first non-whitespace character
  while (isspace((unsigned char)*line)) {
    line++;
  }

  // If the line is all whitespace, return an empty string
  if (*line == '\0') {
    return strdup("");
  }

  // Find the end of the string
  char *end = line + strlen(line) - 1;

  // Move back to the last non-whitespace character
  while (end > line && isspace((unsigned char)*end)) {
    end--;
  }

  // Null-terminate the trimmed string
  *(end + 1) = '\0';

  return strdup(line);
}

bool do_builtin(struct shell *sh, char **argv) {
  if (strcmp(argv[0], "exit") == 0) {
    printf("Goodbye!\n");
    cmd_free(argv);
    sh_destroy(sh);
    exit(0);
  }
  if (strcmp(argv[0], "cd") == 0) {
    change_dir(argv);
    return true;
  }
  if (strcmp(argv[0], "history") == 0) {
    HIST_ENTRY **hist_list = history_list();
    if (hist_list) {
      for (int i = 0; hist_list[i]; i++) {
        printf("%d: %s\n", i + history_base, hist_list[i]->line);
      }
    }
    return true;
  }
  return false;
}

void sh_init(struct shell *sh) {
  //See if running interactively
  sh->shell_terminal = STDIN_FILENO;
  sh->shell_is_interactive = isatty(sh->shell_terminal);

  //loop until on foreground
  if(sh->shell_is_interactive) {
    while(tcgetpgrp(sh->shell_terminal) != (sh->shell_pgid = getpgrp()))
      kill(- sh->shell_pgid, SIGTTIN);
  }

  //ignore signals
  signal(SIGINT, SIG_IGN);
  signal(SIGQUIT, SIG_IGN);
  signal(SIGTSTP, SIG_IGN);
  signal(SIGTTIN, SIG_IGN);
  signal(SIGTTOU, SIG_IGN);

  //put ourselves in our own process group
  sh->shell_pgid = getpid();
  if(setpgid(sh->shell_pgid, sh->shell_pgid) < 0) {
    perror("Couldn't put the shell in its own process group");
    exit(1);
  }

  //get control of the terminal
  tcsetpgrp(sh->shell_terminal, sh->shell_pgid);
  tcgetattr(sh->shell_terminal, &sh->shell_tmodes);

  //set prompt
  sh->prompt = get_prompt("MY_PROMPT");
}

void sh_destroy(struct shell *sh) {
  //free the prompt
  if (sh->prompt) {
    free(sh->prompt);
  }
}

void parse_args(int argc, char **argv) {
  int opt;
  
  while ((opt = getopt(argc, argv, "v")) != -1) {
    switch (opt) {
      case 'v':
        printf("Version: %d.%d\n", lab_VERSION_MAJOR, lab_VERSION_MINOR);
        exit(0);
        break;
      default: /* '?' */
        fprintf(stderr, "Usage: %s [-v]\n", argv[0]);
        exit(1);
    }
  }
}
