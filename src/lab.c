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

/**
 * @brief Changes the current working directory.
 *
 * This function changes the current working directory. If no argument is
 * provided (dir[1] is NULL), it changes to the user's home directory.
 * If an argument is provided, it changes to the specified directory.
 *
 * @param dir An array of strings representing the command and its arguments.
 * dir[0] is expected to be "cd", and dir[1] is the target directory.
 * @return 0 on success, -1 on failure.
 */
int change_dir(char **dir) {
    const char *target_dir;

    // Check if a target directory is provided.
    if (dir[1] == NULL) {
        // No argument provided, change to home directory.
        target_dir = getenv("HOME");

        // If HOME environment variable is not set, use the user's home directory from passwd.
        if (target_dir == NULL) {
            struct passwd *pw = getpwuid(getuid());
            if (pw == NULL) {
                // Handle error getting user information.
                perror("getpwuid");
                return -1;
            }
            target_dir = pw->pw_dir;
        }
    } else {
        // Change to the specified directory.
        target_dir = dir[1];
    }

    // Attempt to change the directory.
    if (chdir(target_dir) != 0) {
        // Handle error changing directory.
        perror("cd"); // Print error message to stderr.
        return -1;
    }

    // Directory change successful.
    return 0;
}


/**
 * @brief Parses a command line string into an array of tokens.
 *
 * This function takes a command line string as input and splits it into
 * individual tokens based on whitespace delimiters (spaces, tabs,
 * carriage returns, newlines, and form feeds). It dynamically allocates
 * memory for the tokens and returns an array of strings, where each string
 * represents a token. The array is null-terminated.
 *
 * @param line The command line string to parse.
 * @return An array of strings representing the parsed tokens, or NULL on error.
 * The caller is responsible for freeing the allocated memory.
 */
char **cmd_parse(char const *line) {
    int bufsize = 64; // Initial buffer size for tokens.
    int position = 0;  // Current position in the tokens array.
    char **tokens = malloc(bufsize * sizeof(char *)); // Allocate initial token array.
    char *line_copy, *token;

    // Check if initial memory allocation for tokens was successful.
    if (tokens == NULL) {
        fprintf(stderr, "cmd_parse: allocation error\n");
        exit(EXIT_FAILURE); // Exit with an error code.
    }

    // Create a copy of the input string to avoid modifying the original.
    line_copy = strdup(line);
    if (line_copy == NULL) {
        fprintf(stderr, "cmd_parse: allocation error\n");
        exit(EXIT_FAILURE); // Exit with an error code.
    }

    // Tokenize the string using strtok.
    token = strtok(line_copy, " \t\r\n\a"); // Delimiters: space, tab, CR, LF, FF.
    while (token != NULL) {
        tokens[position] = token; // Store the token in the tokens array.
        position++;

        // Check if the tokens array needs to be resized.
        if (position >= bufsize) {
            bufsize += 64; // Increase buffer size.
            tokens = realloc(tokens, bufsize * sizeof(char *)); // Reallocate memory.
            if (tokens == NULL) {
                fprintf(stderr, "cmd_parse: allocation error\n");
                exit(EXIT_FAILURE); // Exit with an error code.
            }
        }

        token = strtok(NULL, " \t\r\n\a"); // Get the next token.
    }

    tokens[position] = NULL; // Null-terminate the tokens array.
    return tokens; // Return the array of tokens.
}


/**
 * @brief Frees the memory allocated for a command line argument array.
 *
 * This function frees the memory allocated for an array of strings (char **)
 * representing command line arguments. It first frees the memory allocated
 * for the first string (line[0]), which is assumed to be a duplicated string,
 * and then frees the memory allocated for the array itself.
 *
 * @param line The array of strings to free.
 * It's safe to pass NULL to this function.
 */
void cmd_free(char **line) {
    // Check if the input array is NULL. If so, there's nothing to free.
    if (line == NULL) {
        return; // Exit the function.
    }

    // Free the memory allocated for the first string in the array (line[0]).
    // This is typically a duplicated string created by strdup or similar.
    if (line[0] != NULL) { //Added check so that we do not try to free a NULL pointer.
        free(line[0]);
    }

    // Free the memory allocated for the array of strings itself.
    free(line);
}

/**
 * @brief Trims leading and trailing whitespace from a string.
 *
 * This function takes a string as input and removes leading and trailing
 * whitespace characters (spaces, tabs, etc.). It returns a new dynamically
 * allocated string containing the trimmed result.
 *
 * @param line The string to trim.
 * @return A dynamically allocated string containing the trimmed result,
 * or NULL if the input string is NULL or memory allocation fails.
 * The caller is responsible for freeing the allocated memory.
 */
char *trim_white(char *line) {
    // Check if the input string is NULL.
    if (line == NULL) {
        return NULL; // Return NULL if the input is NULL.
    }

    // Find the first non-whitespace character.
    char *start = line;
    while (isspace((unsigned char)*start)) {
        start++;
    }

    // If the string is all whitespace, return an empty string.
    if (*start == '\0') {
        return strdup(""); // Return a dynamically allocated empty string.
    }

    // Find the end of the string.
    char *end = line + strlen(line) - 1;

    // Move back to the last non-whitespace character.
    while (end > start && isspace((unsigned char)*end)) {
        end--;
    }

    // Null-terminate the trimmed string.
    *(end + 1) = '\0';

    // Return a dynamically allocated copy of the trimmed string.
    return strdup(start);
}

/**
 * @brief Executes built-in shell commands.
 *
 * This function checks if the given command is a built-in shell command
 * and executes it if it is. Currently supported built-in commands are:
 * - exit: Exits the shell.
 * - cd: Changes the current directory.
 * - history: Prints the command history.
 *
 * @param sh A pointer to the shell structure.
 * @param argv An array of strings containing the command and its arguments.
 * @return true if the command was a built-in command and was executed,
 * false otherwise.
 */
bool do_builtin(struct shell *sh, char **argv) {
    if (argv == NULL || *argv == NULL) { //Safety check for NULL
        return false;
    }
    // Check for the "exit" command.
    if (strcmp(*argv, "exit") == 0) { 
        printf("Goodbye!\n");
        cmd_free(argv);    // Free the command arguments.
        sh_destroy(sh);   // Clean up the shell.
        exit(0);          // Exit the shell with a success code.
    }

    // Check for the "cd" command.
    if (strcmp(*argv, "cd") == 0) { 
        change_dir(argv); // Change the directory.
        return true;       // Indicate that a built-in command was executed.
    }

    // Check for the "history" command.
    if (strcmp(*argv, "history") == 0) { 
        HIST_ENTRY **hist_list = history_list(); // Get the history list.
        if (hist_list) {
            // Iterate through the history list and print each entry.
            for (int i = 0; hist_list[i]; i++) {
                printf("%d: %s\n", i + history_base, hist_list[i]->line);
            }
        }
        return true; // Indicate that a built-in command was executed.
    }

    // If none of the built-in commands were found, return false.
    return false;
}


/**
 * @brief Initializes the shell.
 *
 * This function performs the necessary initialization steps for the shell,
 * including:
 * - Determining if the shell is running interactively.
 * - Putting the shell in its own process group.
 * - Ignoring various signals.
 * - Setting the shell's prompt.
 *
 * @param sh A pointer to the shell structure to be initialized.
 */
void sh_init(struct shell *sh) {
    // Determine if the shell is running interactively.
    sh->shell_terminal = STDIN_FILENO;
    sh->shell_is_interactive = isatty(sh->shell_terminal);

    // If the shell is interactive, put it in its own process group and
    // ensure it's in the foreground.
    if (sh->shell_is_interactive) {
        while (tcgetpgrp(sh->shell_terminal) != (sh->shell_pgid = getpgrp())) {
            kill(-sh->shell_pgid, SIGTTIN);
        }

        // Ignore common signals that could interrupt the shell.
        signal(SIGINT, SIG_IGN);  // Ignore interrupt signals (Ctrl+C).
        signal(SIGQUIT, SIG_IGN); // Ignore quit signals (Ctrl+\).
        signal(SIGTSTP, SIG_IGN); // Ignore stop signals (Ctrl+Z).
        signal(SIGTTIN, SIG_IGN); // Ignore background input signals.
        signal(SIGTTOU, SIG_IGN); // Ignore background output signals.

        // Put the shell in its own process group to isolate it from
        // other processes.
        sh->shell_pgid = getpid();
        if (setpgid(sh->shell_pgid, sh->shell_pgid) < 0) {
            perror("Couldn't put the shell in its own process group");
            exit(1);
        }

        // Get control of the terminal so the shell can receive input
        // and manage output.
        tcsetpgrp(sh->shell_terminal, sh->shell_pgid);

        // Save the current terminal attributes so they can be restored
        // later.
        tcgetattr(sh->shell_terminal, &sh->shell_tmodes);
    }

    // Set the shell's prompt using the MY_PROMPT environment variable
    // or a default prompt if it's not set.
    sh->prompt = get_prompt("MY_PROMPT");
}

/**
 * @brief Cleans up and frees resources used by the shell.
 *
 * This function performs cleanup tasks for the shell, such as
 * freeing dynamically allocated memory. Currently, it frees the
 * memory allocated for the shell prompt.
 *
 * @param sh A pointer to the shell structure to be cleaned up.
 */
void sh_destroy(struct shell *sh) {
    // Free the memory allocated for the shell prompt if it's not NULL.
    if (sh->prompt != NULL) {
        free(sh->prompt);
        sh->prompt = NULL; // Set the pointer to NULL to prevent accidental use after freeing.
    }
    // TODO: further cleanup tasks here
}

/**
 * @brief Parses command-line arguments.
 *
 * This function parses the command-line arguments passed to the program.
 * It currently supports the following option:
 * -v: Print the version number and exit.
 *
 * If an invalid option is encountered, it prints a usage message to stderr
 * and exits with an error code.
 *
 * @param argc The number of command-line arguments.
 * @param argv An array of strings containing the command-line arguments.
 */
void parse_args(int argc, char **argv) {
    int opt;

    // Use getopt to parse command-line options.
    while ((opt = getopt(argc, argv, "v")) != -1) {
        switch (opt) {
            case 'v':
                // Print the version number and exit with a success code.
                printf("Version: %d.%d\n", lab_VERSION_MAJOR, lab_VERSION_MINOR);
                exit(0);
                break; // This break is technically unnecessary due to the exit() call.

            default:  // '?' indicates an invalid option.
                // Print a usage message to stderr and exit with an error code.
                fprintf(stderr, "Usage: %s [-v]\n", *argv); // Corrected: *argv instead of argv
                exit(1);
        }
    }
}
