#include <grass.h>

void do_nothing() {
    fprintf(stderr, "\nPlease quit by typing: exit\n");
    fprintf(stderr, ">>> ");
}

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

void repl_handler(int socket_fd) {
    char input[MAX_INPUT_LENGTH + 1];
    memset(input, 0, MAX_INPUT_LENGTH + 1);

    char server_feedback[MAX_INPUT_LENGTH];
    bzero(server_feedback, sizeof(server_feedback));

    while (!feof(stdin) && !ferror(stdin) && strncmp(input, "exit", 4) != 0) {
        memset(input, 0, MAX_INPUT_LENGTH + 1);
        bzero(server_feedback, sizeof(server_feedback));
        
        printf(SHELL_PROMPT);
        fflush(stdout);

        if (fgets(input, MAX_INPUT_LENGTH + 1, stdin) == NULL || strlen(input) < 2) {
            fprintf(stderr, "ERROR SHELL: %s\n", SHELL_ERR_MESSAGES[ERR_INVALID_CMD]);
        } else {
            write(socket_fd, input, strlen(input) + 1);

            read(socket_fd, server_feedback, sizeof(server_feedback));
            if (strlen(server_feedback) > 1) {
                printf("%s", server_feedback);
            }
        }
    }

    if (strncmp(input, "exit", 4) != 0) {
        char *string_exit = "exit\n";
        write(socket_fd, string_exit, strlen(string_exit) + 1);
        close(socket_fd);
    }
}

void auto_cmds_handler(int socket_fd, char * file_in, char * file_out) {
    char server_feedback[MAX_INPUT_LENGTH];
    bzero(server_feedback, sizeof(server_feedback));

    FILE * fp_in;
    char * current_line = NULL;
    size_t line_length = 0;
    size_t bytes_read;

    fp_in = fopen(file_in, "r");
    if (fp_in == NULL) {
        fprintf(stderr, "Error: input file must exist");
        char *string_exit = "exit\n";
        write(socket_fd, string_exit, strlen(string_exit) + 1);
        close(socket_fd);
        exit(EXIT_FAILURE);
    }

    FILE * fp_out;
    fp_out = fopen(file_out, "w+");
    if (fp_out == NULL) {
        fprintf(stderr, "Error: while opening output file");
        char *string_exit = "exit\n";
        write(socket_fd, string_exit, strlen(string_exit) + 1);
        close(socket_fd);
        fclose(fp_in);

        exit(EXIT_FAILURE);
    }

    while ((bytes_read = getline(&current_line, &line_length, fp_in)) != -1) {
        bzero(server_feedback, sizeof(server_feedback));
        if (line_length <= MAX_INPUT_LENGTH) {
            write(socket_fd, current_line, strlen(current_line) + 1);

            read(socket_fd, server_feedback, sizeof(server_feedback));
            if (strlen(server_feedback) > 1) {
                fprintf(fp_out, "%s", server_feedback);
            }
        } else {
            fprintf(stderr, "Error: input line too long");
        }
    }

    if (strncmp(current_line, "exit", 4) != 0) {
        char *string_exit = "exit\n";
        write(socket_fd, string_exit, strlen(string_exit) + 1);
        close(socket_fd);
    }

    fclose(fp_in);
    fclose(fp_out);
    if (current_line) {
        free(current_line);
    }
}

int main(int argc, char **argv) {
    UNUSED(argc);
    UNUSED(argv);

    signal(SIGINT, do_nothing);

    if (argc != 3 && argc != 5) {
        fprintf(stderr, "Incorrect number of arguments.\n");
        exit(EXIT_FAILURE);
    }

    int socket_fd = create_socket(client, argv[1], atoi(argv[2]));
    if (socket_fd < 0) {
        fprintf(stderr, "Error: Socket error.\n");
        exit(EXIT_FAILURE);
    }

    if (argc == 3) {
        repl_handler(socket_fd);
    } else {
        auto_cmds_handler(socket_fd, argv[3], argv[4]);
    }

    exit(EXIT_SUCCESS);
}
