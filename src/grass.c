#include <grass.h>

const char* const ERR_MESSAGES[] = {
        "", // no error
        "IO error",
        "filename already exists",
        "bad parameter",
};

const char* const SHELL_ERR_MESSAGES[] = {
        "", // no error
        "invalid command",
        "wrong number of arguments",
        "user not authentified", //CHECK_AUTH()
        "file/directory does not exist", //rm
        "directory already exists", //mkdir
        "cannot go there", //cd
};

/*
 * Hijack the flow!
 *
 */
void hijack_flow(void) {
    printf("Method hijack: Accepted\n");
    fflush(stdout);
}

/*
 * Creates and returns a socket either for the client of the server.
 *
 * client_server: mode for the socket
 */
int create_socket(enum mode client_server, char * addr_ip, int addr_port) {
    int socket_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (socket_fd == -1) {
        fprintf(stderr, "Socket creation failed!\n");
        return -1;
    } else {
        printf("Socket created...\n");
    }

    struct sockaddr_in server_address;
    bzero(&server_address, sizeof(server_address));

    server_address.sin_family = AF_INET;
    server_address.sin_addr.s_addr = client_server == server ? htonl(INADDR_ANY) : inet_addr(addr_ip);
    server_address.sin_port = htons(addr_port);

    if (client_server == server) {
        fcntl(socket_fd, F_SETFL, O_NONBLOCK);
        int sock_opt = 1;
        if (setsockopt(socket_fd, SOL_SOCKET, SO_REUSEADDR, (const char*) &sock_opt, sizeof(sock_opt)) < 0 ||
            setsockopt(socket_fd, SOL_SOCKET, SO_REUSEPORT, (const char*) &sock_opt, sizeof(sock_opt)) < 0) {
            fprintf(stderr, "Socket options {SO_REUSEADDR, SO_REUSEPORT} failed!\n");
            return -1;
        }

        int bind_error = -1;
        //printf("Bind_error: %d", bind_error);
        fflush(stdout);
        bind_error = bind(socket_fd, (struct sockaddr *) &server_address, sizeof(server_address));
        if (bind_error != 0) {
            fprintf(stderr, "Socket binding failed!\n");
            fprintf(stderr, "Error Number: %d", errno);
            return -1;
        } else {
            printf("Socket bound...\n");
        }

       if (listen(socket_fd, 2) != 0) {
            fprintf(stderr, "Listen failed!\n");
            return -1;
        } else {
            printf("Listening...\n");
        }
    } else if (client_server == client) {
        if (connect(socket_fd, (struct sockaddr *) &server_address, sizeof(server_address)) != 0) {
            fprintf(stderr, "Connection failed!\n");
            return -1;
        } else {
           printf("Connected...\n");
        }
    } else {
        return -1;
    }

    fflush(stdout);
    return socket_fd;
}

/*
 * Splits the input string into the command and all the arguments.
 *
 * input: the input to split
 * args: where to store each token after the split
 */
int tokenize_input(char* input, char** args) {
    REQUIRE_NON_NULL(input);
    REQUIRE_NON_NULL(args);

    char* inputCpy = input;
    int i = 0;

    while ((args[i] = strtok(inputCpy, SPACE)) != NULL) {
        if (i > MAX_PARAM + 1) {
            return -1;
        }

        i += 1;
        inputCpy = NULL;
    }

    return i;
}

/*
 * Parses the input of the user from the REPL and stores them splitted by command name and each argument.
 *
 * input: the input to parse
 * cmd_and_args: where to store the command and its argument after parsing
 */
int parse_shell_input(char* input, char** cmd_and_args) {
    REQUIRE_NON_NULL(input);
    REQUIRE_NON_NULL(cmd_and_args);

    // We replace the last "\n" with the EOL character.
    char* ending = strrchr(input, ENTER);
    if (ending != NULL) {
        *ending = STRING_END;
    } else {
        // We clear the input buffer of stdin. (In the case the input was longer than MAX_INPUT_LENGTH).
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
    memset(cmd_and_args, 0, MAX_INPUT_LENGTH);

    int feedbackTok = tokenize_input(input, cmd_and_args);
    if (feedbackTok < 0) {
        fprintf(stderr, "ERROR SHELL: %s\n", SHELL_ERR_MESSAGES[ERR_ARGS]);
        return ERR_ARGS;
    }

    return feedbackTok;
}

/**
int do_login(const char** array) {
    printf("login");
    return 0;
}

int do_pass(const char** array) {
    printf("pass");
    return 0;
}

int do_ping(const char** array) {
    printf("ping");
    //does not need authentication
    //returns unix output of ping $HOST -c 1
    if (strlen(array[0]) > MAX_PING_LEN) {
      return 1;
    }
    char str[80]; //EXPLOIT ?
    PING_SHELLCODE(str, array[0]);
    puts(str);
    return 0;
}

int do_ls(const char** array) {
    printf("ls");
    UNUSED(array);
    system(LS_SHELLCODE);
    //check authentication

    return 0;
}

int do_cd(const char** array) {
    printf("cd");
    //authentication
    //changing current rep
    //dir = array[0];
    return 0;
}

int do_mkdir(const char** array) {
    printf("mkdir");
    //authentication
    //check collision and permissions when create
    return 0;
}

int do_rm(const char** array) {
    printf("rm");
    //authentication
    //if exists, checks rights (recursive as works for dir too)
    //dir = array[0];
    return 0;
}

int do_get(const char** array) {
    printf("get");
    return 0;
}

int do_put(const char** array) {
    printf("put");
    return 0;
}

int do_grep(const char** array) {
    printf("grep");
    return 0;
}

int do_date(const char** array) {
    printf("date");
    UNUSED(array);
    check_auth();
    system(DATE_SHELLCODE);
    return 0;
}

int do_exit(const char** array) {
    printf("exit");
    UNUSED(array);
    int err = 0;
    if (!err) {
      exit(0);
    } else {
      exit(1);
    }
    return 0;
}
**/
