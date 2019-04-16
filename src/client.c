#include <grass.h>
#include <netinet/in.h>

/*
 * Send a file to the server as its own thread
 *
 * fp: file descriptor of file to send
 * d_port: destination port
 */
void send_file(int fp, int d_port) {
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
    // TODO
}


/*
 * search all files in the current directory
 * and its subdirectory for the pattern
 *
 * pattern: an extended regular expressions.
 */
void search(char *pattern) {
    // TODO
}

int main(int argc, char **argv) {
    // TODO:
    // Make a short REPL to send commands to the server
    // Make sure to also handle the special cases of a get and put command

   int socket_fd = create_socket(client);

    while(true) {
        char buffer[25];
        //example of write to the server:
        //write(socket_fd, pswd, strlen(pswd) + 1);

    	break;
    }

    fflush(stdout);
    close(socket_fd);

    // Simple REPL.
    while (!feof(stdin) && !ferror(stdin)) {
        printf(SHELL_PROMPT);
        fflush(stdout);

        char input[MAX_INPUT_LENGTH + 1];
        memset(input, 0, MAX_INPUT_LENGTH + 1);

        if (fgets(input, MAX_INPUT_LENGTH + 1, stdin) == NULL || strlen(input) < 2) {
            fprintf(stderr, "ERROR SHELL: %s\n", SHELL_ERR_MESSAGES[ERR_INVALID_CMD]);
        } else {
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
            } else {
                // We execute the corresponding command or print an error if needed.
                int cmd_nb = -1;
                for (int i = 0; i < NB_CMD; ++i) {
                    if (strncmp(cmdAndArgs[0], shell_cmds[i].cname, strlen(shell_cmds[i].cname)) == 0) {
                        cmd_nb = i;

                        // We check if we have the correct number of args to call the method.
                        int feedback = check_args(cmd_nb, feedbackTok - 1);
                        if (feedback) {
                            fprintf(stderr, "ERROR SHELL: %s\n", SHELL_ERR_MESSAGES[feedback]);
                        } else {

                            const char* args[MAX_INPUT_LENGTH + 1];
                            memset(args, 0, MAX_INPUT_LENGTH + 1);
                            for (size_t j = 1; j < MAX_PARAM + 1; ++j) {
                                args[j - 1] = cmdAndArgs[j];
                            }

                            feedback = (shell_cmds[cmd_nb].fct)(args);
                            if (feedback) { // Can be FS error or SHELL error.
                                if (feedback < 0) {
                                    fprintf(stderr, "ERROR FS: %s\n", ERR_MESSAGES[feedback - ERR_FIRST]);
                                } else {
                                    fprintf(stderr, "ERROR SHELL: %s\n", SHELL_ERR_MESSAGES[feedback]);
                                }
                            }
                        }

                        i = NB_CMD + 1;
                    }
                }
                if (cmd_nb < 0) {
                    fprintf(stderr, "ERROR SHELL: %s\n", SHELL_ERR_MESSAGES[ERR_INVALID_CMD]);
                }
            }
        }
    }

    exit(EXIT_SUCCESS);
}
