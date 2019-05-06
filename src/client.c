#include <grass.h>

FILE * client_stderr = NULL;

static int threads_open = 0;

static regex_t * get_regex;
static regex_t * put_regex;

/*
* do_nothing is called when the client tries to exit the shell by force
* and it just prints a warning message 
*/
void do_nothing() {
    fprintf(stderr, "\nPlease quit by typing: exit or pressing CTRL-D!\n");
    if (client_stderr == stderr) {
        printf(SHELL_PROMPT);
    }
    fflush(stdout);
}

/*
*geter_handler is the function that ensures the correct 
*behavior of the get function on the client side
*
*arg: pointer on a Get_Args struct which contains infos about this transfer
*/
static void * geter_handler(void *arg) {
    struct Get_Args * get_args = arg;

    int tries = 0;
    int socket_fd;
    do {
        sleep(1);
        socket_fd = create_socket(client, get_args->ip_address, get_args->port);
        tries += 1;
    } while (socket_fd < 0 && tries < 3);
    if (socket_fd < 0) {
        fprintf(client_stderr, "Error: socket connection failed for get.\n");
        if (client_stderr == stderr) {
            printf(SHELL_PROMPT);
        }
        fflush(stdout);
        threads_open -= 1;
        pthread_detach(pthread_self());
        pthread_exit(0);
    }

    char server_feedback[get_args->size];
    bzero(server_feedback, get_args->size);
    read(socket_fd, server_feedback, get_args->size);

    FILE* fp = fopen(get_args->file_name, "w+");
    if (fp == NULL) {
        fprintf(client_stderr, "Error: file could not be open.\n");
    } else {
        fwrite(server_feedback, sizeof(char), get_args->size, fp);
        fclose(fp);
    }

    free(get_args);
    close(socket_fd);

    fflush(stdout);
    threads_open -= 1;
    pthread_detach(pthread_self());
    pthread_exit(0);
}

/*
*puter_handler is the function that ensures the correct 
*behavior of the put function on the client side
*
*arg: pointer on a Get_Args struct which contains infos about this transfer
*/
static void * puter_handler(void *arg) {
    struct Get_Args * get_args = arg;

    int tries = 0;
    int socket_fd;
    do {
        sleep(1);
        socket_fd = create_socket(client, get_args->ip_address, get_args->port);
        tries += 1;
    } while (socket_fd < 0 && tries < 3);
    if (socket_fd < 0) {
        fprintf(client_stderr, "Error: socket connection refused for put.\n");
        if (client_stderr == stderr) {
            printf(SHELL_PROMPT);
        }
        fflush(stdout);
        threads_open -= 1;
        fclose(get_args->file);
        pthread_detach(pthread_self());
        pthread_exit(0);
    }

    char packet[get_args->size];
    bzero(packet, get_args->size);

    long int real_size=0;
    fseek(get_args->file, 0,SEEK_END);
    real_size=ftell(get_args->file);
    fseek(get_args->file, 0, SEEK_SET);

    size_t size_r = fread(packet, sizeof(char), (get_args->size - 1), get_args->file);
    
    if ((ferror(get_args->file) != 0) || (real_size != get_args->size - 1) || size_r != get_args->size - 1) {
        char * read_err = "\nError: putting file, check the name and/or the size.\n>>> ";
        fprintf(client_stderr, "%s", read_err);
        write(socket_fd, "\n", 1);
    } else {
        write(socket_fd, packet, get_args->size);
    }

    close(socket_fd);

    fflush(stdout);
    threads_open -= 1;
    fclose(get_args->file);
    free(get_args);
    pthread_detach(pthread_self());
    pthread_exit(0);
}

/*
*handle_server_feedback reads the feedback of the server from the socket and behaves
*accordingly whever it is a put/get function or another one
*
*output_fp: where to write the server's feedback
*socket_fd: socket connected to the server
*ip_addr: address of the server
*input: client input 
*/
void handle_server_feedback(FILE* output_fp, int socket_fd, char * ip_addr, char * input) {
    char server_feedback[MAX_INPUT_LENGTH];
    bzero(server_feedback, sizeof(server_feedback));

    read(socket_fd, server_feedback, sizeof(server_feedback));
    if (strlen(server_feedback) > 1) {
        if (regexec(get_regex, server_feedback, 0, NULL, 0) == 0) {
            char * splitted[MAX_PUT_GET_LEN];
            bzero(splitted, MAX_PUT_GET_LEN);
            tokenize(server_feedback, splitted);

            struct Get_Args * get_args = (struct Get_Args *) calloc(1, sizeof(struct Get_Args));
            strcpy(get_args->ip_address, ip_addr);
            get_args->port = (int) strtol(splitted[2], NULL, 10);

            char * start_filename = strchr(input, ' ');
            start_filename += 1;
            strncpy(get_args->file_name, start_filename, strlen(start_filename) - 1);

            get_args->size = (size_t) strtol(splitted[4], NULL, 10);

            pthread_t get_child;
            threads_open += 1;
            pthread_create(&get_child, NULL, &geter_handler, get_args);
        } else if (regexec(put_regex, server_feedback, 0, NULL, 0) == 0) {
            char * splitted[MAX_PUT_GET_LEN];
            bzero(splitted, MAX_PUT_GET_LEN);
            tokenize(server_feedback, splitted);

            struct Get_Args * get_args = (struct Get_Args *) calloc(1, sizeof(struct Get_Args));
            strcpy(get_args->ip_address, ip_addr);
            get_args->port = (int) strtol(splitted[2], NULL, 10);

            *(input + strlen(input) - 1) = '\0';

            char * start_filename = strchr(input, ' ');
            start_filename += 1;
            char * end_filename = strchr(start_filename, ' ');
            *end_filename = '\0';
            strncpy(get_args->file_name, start_filename, strlen(start_filename));

            end_filename += 1;
            get_args->size = (size_t) (strtol(end_filename, NULL, 10) + 1);
            get_args->file = fopen(get_args->file_name, "r");

            pthread_t put_child;
            threads_open += 1;
            pthread_create(&put_child, NULL, (void *(*)(void *)) puter_handler, get_args);
        } else {
            fprintf(output_fp, "%s", server_feedback);
        }
    }
}

/*
*shell_handler will loop and get a line by line input from the client and send it to the server
*
*socket_fd: socket used to connect to the server
*ip_address: address of the server
*/
void shell_handler(int socket_fd, char *ip_address) {
    char input[MAX_INPUT_LENGTH + 1];
    bzero(input, MAX_INPUT_LENGTH + 1);

    client_stderr = stderr;

    while (!feof(stdin) && !ferror(stdin) && strncmp(input, "exit", 4) != 0) {
        printf(SHELL_PROMPT);
        fflush(stdout);

        if (fgets(input, MAX_INPUT_LENGTH + 1, stdin) != NULL) {
            char* ending = strrchr(input, ENTER);
            if (ending != NULL) {
                if (strlen(input) > strlen("put ") && strncmp("put ", input, strlen("put ")) == 0) {
                    char filename[PATH_MAX];
                    bzero(filename, PATH_MAX);
                    char * space_fn = strchr(input, ' ');
                    space_fn += 1;

                    char * second_space = strchr(space_fn, ' ');

                    strncpy(filename, space_fn, second_space - space_fn);

                    if (access(filename, F_OK | R_OK) != 0) {
                        fprintf(stderr, "Error: file does not exist or not readable\n");
                        continue;
                    }
                }

                write(socket_fd, input, strlen(input) + 1);
                handle_server_feedback(stdout, socket_fd, ip_address, input);
            } else {
                int useless = getchar();
                while (useless != ENTER && useless != EOF) {
                    useless = getchar();
                }

                fprintf(stderr, "%s", "Error: input too long.\n");
            }
        }
    }

    if (strncmp(input, "exit", 4) != 0) {
        char *string_exit = "exit\n";
        write(socket_fd, string_exit, strlen(string_exit) + 1);
        close(socket_fd);
    }
}

/*
*auto_commands_handler reads the input from a file and not from the client.
*It then sends them to the server
*
*socket_fd: socket used to connect to the server
*ip_address: address of th server
*file_in: file to read the commands to execute from
*file_out: file to write the server outputs to
*/
void auto_commands_handler(int socket_fd, char *ip_address, char *file_in, char *file_out) {
    FILE * fp_in = fopen(file_in, "r");
    if (fp_in == NULL) {
        fprintf(stderr, "Error: input file must exist.\n");
        char *string_exit = "exit\n";
        write(socket_fd, string_exit, strlen(string_exit) + 1);
        close(socket_fd);
        regfree(get_regex);
        regfree(put_regex);
        free(get_regex);
        free(put_regex);
        exit(EXIT_FAILURE);
    }

    FILE * fp_out = fopen(file_out, "w+");
    if (fp_out == NULL) {
        fprintf(stderr, "Error: can't open output file.\n");
        char *string_exit = "exit\n";
        write(socket_fd, string_exit, strlen(string_exit) + 1);
        close(socket_fd);
        fclose(fp_in);
        regfree(get_regex);
        regfree(put_regex);
        free(get_regex);
        free(put_regex);
        exit(EXIT_FAILURE);
    }

    client_stderr = fp_out;

    char * current_line = NULL;
    size_t line_length = 0;
    while (getline(&current_line, &line_length, fp_in) != -1) {
        if (line_length <= MAX_INPUT_LENGTH) {
            write(socket_fd, current_line, strlen(current_line) + 1);
            handle_server_feedback(fp_out, socket_fd, ip_address, current_line);
        } else {
            fprintf(stderr, "Error: input line too long.\n");
        }
    }

    if (strncmp(current_line, "exit", 4) != 0) {
        char *string_exit = "exit\n";
        write(socket_fd, string_exit, strlen(string_exit) + 1);
        close(socket_fd);
    }

    fclose(fp_in);
    fclose(fp_out);
    free(current_line);
}

int main(int argc, char **argv) {
    signal(SIGINT, do_nothing);

    get_regex = calloc(1, sizeof(regex_t));
    if (get_regex == NULL) {
        fprintf(stderr, "Could not alloc regex.\n");
        exit(EXIT_FAILURE);
    }
    put_regex = calloc(1, sizeof(regex_t));
    if (put_regex == NULL) {
        fprintf(stderr, "Could not alloc regex.\n");
        regfree(get_regex);
        free(get_regex);
        exit(EXIT_FAILURE);
    }

    int err_regex = regcomp(get_regex, "get port: ", 0);
    if (err_regex) {
        fprintf(stderr, "Could not compile regex.\n");
        regfree(get_regex);
        regfree(put_regex);
        free(get_regex);
        free(put_regex);
        exit(EXIT_FAILURE);
    }
    err_regex = regcomp(put_regex, "put port: ", 0);
    if (err_regex) {
        fprintf(stderr, "Could not compile regex.\n");
        regfree(get_regex);
        regfree(put_regex);
        free(get_regex);
        free(put_regex);
        exit(EXIT_FAILURE);
    }

    if (argc != 3 && argc != 5) {
        fprintf(stderr, "Incorrect number of arguments.\n");
        regfree(get_regex);
        regfree(put_regex);
        free(get_regex);
        free(put_regex);
        exit(EXIT_FAILURE);
    }

    int socket_fd = create_socket(client, argv[1], (int) strtol(argv[2], NULL, 10));
    if (socket_fd < 0) {
        fprintf(stderr, "Error: Socket error.\n");
        regfree(get_regex);
        regfree(put_regex);
        free(get_regex);
        free(put_regex);
        exit(EXIT_FAILURE);
    }

    if (argc == 3) {
        shell_handler(socket_fd, argv[1]);
    } else {
        auto_commands_handler(socket_fd, argv[1], argv[3], argv[4]);
    }

    while (threads_open > 0) {
        fprintf(stderr, "Waiting for threads to finish...\n");
        sleep(1);
    }

    regfree(get_regex);
    regfree(put_regex);
    free(get_regex);
    free(put_regex);

    exit(EXIT_SUCCESS);
}
