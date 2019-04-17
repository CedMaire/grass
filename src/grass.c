#include <grass.h>
#include <stdio.h>

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

char need_id[3][MAX_STRING_SIZE] =
{ "pass",
  "logout",
  "whoami"
};

char need_fd[4][MAX_STRING_SIZE] =
{ "pass",
  "logout",
  "whoami",
  "w"
};

/*
 * Hijack the flow!
 *
 */
void hijack_flow() {
    printf("Method hijack: Accepted\n");
    fflush(stdout);
}

/*
 * Creates and returns a socket either for the client of the server.
 *
 * client_server: mode for the socket
 */
int create_socket(enum mode client_server) {
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
    server_address.sin_addr.s_addr = client_server == server ? htonl(INADDR_ANY) : inet_addr("127.0.0.1");
    server_address.sin_port = htons(6969);

    if (client_server == server) {
        if (bind(socket_fd, (struct sockaddr *) &server_address, sizeof(server_address)) != 0) {
            fprintf(stderr, "Socket binding failed!\n");
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
/**
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
