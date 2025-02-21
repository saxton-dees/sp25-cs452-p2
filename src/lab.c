#include "lab.h"
#include <stdio.h>

char *get_prompt(const char *env) {
  (void)env;
  return NULL;
}

int change_dir(char **dir) {
  (void)dir;
  return 0;
}

char **cmd_parse(char const *line) {
  (void)line;
  return NULL;
}

void cmd_free(char **line) {
  (void)line;
}

char *trim_white(char *line) {
  (void)line;
  return NULL;
}

bool do_builtin(struct shell *sh, char **argv) {
  (void)sh;
  (void)argv;
  return false;
}

void sh_init(struct shell *sh) {
  (void)sh;
}

void sh_destroy(struct shell *sh) {
  (void)sh;
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
