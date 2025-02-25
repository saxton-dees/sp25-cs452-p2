#include "lab.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>

char *get_prompt(const char *env) {
  const char *prompt = getenv(env);
  if (prompt == NULL) {
      prompt = "DeeShell>";
  }
  char *result = malloc(strlen(prompt) + 1);
  if (result != NULL) {
      strcpy(result, prompt);
  }
  return result;
}

int change_dir(char **dir) {
  (void)dir;
  return 0;
}

char **cmd_parse(char const *line) {
  int bufsize = 64, position = 0;
  char **tokens = malloc(bufsize * sizeof(char*));
  char *token;

  if (!tokens) {
      fprintf(stderr, "cmd_parse: allocation error\n");
      exit(EXIT_FAILURE);
  }

  token = strtok((char *)line, " \t\r\n\a");
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
  free(line);
}

char *trim_white(char *line) {
  char *end;

  // Trim leading space
  while (isspace((unsigned char)*line)) line++;

  if (*line == 0)  // All spaces?
      return line;

  // Trim trailing space
  end = line + strlen(line) - 1;
  while (end > line && isspace((unsigned char)*end)) end--;

  // Write new null terminator
  end[1] = '\0';

  return line;
}

bool do_builtin(struct shell *sh, char **argv) {
  (void)sh;
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
  return false;
}

void sh_init(struct shell *sh) {
  sh->prompt = get_prompt("SHELL_PROMPT");
}

void sh_destroy(struct shell *sh) {
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
