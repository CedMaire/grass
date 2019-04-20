#include <grass.h>

/*
 * Send a file to the server as its own thread
 *
 * fp: file descriptor of file to send
 * d_port: destination port
 */
void send_file(int fp, int d_port) {
    UNUSED(fp);
    UNUSED(d_port);
    // TODO
}

/*
 * Recv a file from the server as its own thread
 *
 * fp: file descriptor of file to save to.
 * d_port: destination port
 * size: the size (in bytes) of the file to recv
 */
void recv_file(int fp, int d_port, int size) {
    UNUSED(fp);
    UNUSED(d_port);
    UNUSED(size);
    // TODO
}


/*
 * search all files in the current directory
 * and its subdirectory for the pattern
 *
 * pattern: an extended regular expressions.
 */
void search(char *pattern) {
    UNUSED(pattern);
    // TODO
}

int main(int argc, char **argv) {
    UNUSED(argc);
    UNUSED(argv);
    // TODO:
    // Make a short REPL to send commands to the server
    // Make sure to also handle the special cases of a get and put command

    int socket_fd = create_socket(client);

    //close(socket_fd);

    // Simple REPL.
    while (!feof(stdin) && !ferror(stdin)) {
        printf(SHELL_PROMPT);
        fflush(stdout);

        char input[MAX_INPUT_LENGTH + 1];
        memset(input, 0, MAX_INPUT_LENGTH + 1);

        if (fgets(input, MAX_INPUT_LENGTH + 1, stdin) == NULL || strlen(input) < 2) {
            fprintf(stderr, "ERROR SHELL: %s\n", SHELL_ERR_MESSAGES[ERR_INVALID_CMD]);
        } else {
            //printf("Got input: %s", input);
            //printf("Got length: %lu", strlen(input));
            // We replace the last "\n" with the EOL character.
            char* ending = strrchr(input, ENTER);
            if (ending != NULL) {
                *ending = STRING_END;
            } else {
                // We clear the input buffer of stdin. (In the case the input was longer than MAX_INPUT_LENGTH).
                // TODO: check if acctually have EOL char.
                int useless = getchar();
                while (useless != ENTER && useless != EOF) {
                    useless = getchar();
                }
            }

            // We replace the last "/" from a path string if needed.
            ending = strrchr(input, PATH_TOKEN);
            if (ending != NULL && *(ending + 1) == STRING_END && *(ending - 1) != SPACE_CHAR) {
                *ending = STRING_END;
            }
            ending = NULL;

            // We split the input into the command and the provided arguments.
            char* cmdAndArgs[MAX_INPUT_LENGTH];
            memset(cmdAndArgs, 0, MAX_INPUT_LENGTH);

            int feedbackTok = tokenize_input(input, cmdAndArgs);
            if (feedbackTok < 0) {
                fprintf(stderr, "ERROR SHELL: %s\n", SHELL_ERR_MESSAGES[ERR_ARGS]);
            }
            write(socket_fd, input, strlen(input) + 1);
            //read output
        }
    }

    exit(EXIT_SUCCESS);
}
