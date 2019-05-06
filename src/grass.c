#include <grass.h>

/*
 * Hijack the flow!
 *
 */
void hijack_flow(void) {
    printf("Method hijack: Accepted\n");
    fflush(stdout);
}

char * trim_string(char * string) {
    while (isspace(*string)) {
        string += 1;
    }

    char* end_str = string + strlen(string) - 1;
    while (end_str >= string && isspace(*end_str)) {
        *end_str = '\0';
        end_str -= 1;
    }

    return string;
}

/*
 * Creates and returns a socket either for the client of the server.
 *
 * client_server: mode for the socket
 */

int create_socket(enum mode client_server, char * address_ip, int address_port) {
    int socket_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (socket_fd == -1) {
        fprintf(stderr, "Socket creation failed!\n");
        return -1;
    }

    struct sockaddr_in server_address;
    bzero(&server_address, sizeof(server_address));

    server_address.sin_family = AF_INET;
    server_address.sin_addr.s_addr = client_server == server ? htonl(INADDR_ANY) : inet_addr(address_ip);
    server_address.sin_port = htons((uint16_t) address_port);

    if (client_server == server) {
        int sock_opt = 1;
        if (setsockopt(socket_fd, SOL_SOCKET, SO_REUSEADDR, (const char*) &sock_opt, sizeof(sock_opt)) < 0 ||
            setsockopt(socket_fd, SOL_SOCKET, SO_REUSEPORT, (const char*) &sock_opt, sizeof(sock_opt)) < 0) {
            fprintf(stderr, "Socket options {SO_REUSEADDR, SO_REUSEPORT} failed!\n");
            return -1;
        }

        int bind_error = bind(socket_fd, (struct sockaddr *) &server_address, sizeof(server_address));
        if (bind_error != 0) {
            fprintf(stderr, "Socket binding failed!\n");
            fprintf(stderr, "Error Number: %d\n", errno);
            return -1;
        }

        if (listen(socket_fd, 2) != 0) {
            fprintf(stderr, "Listen failed!\n");
            return -1;
        }
    } else if (client_server == client) {
        if (connect(socket_fd, (struct sockaddr *) &server_address, sizeof(server_address)) != 0) {
            fprintf(stderr, "Connection failed! Tried port %d\n", address_port);
            fprintf(stderr, "Error Number: %d\n", errno);
            return -1;
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
int tokenize(char * input, char ** args) {
    char * input_copy = input;

    int i = 0;
    while ((args[i] = strtok(input_copy, SPACE)) != NULL) {
        i += 1;
        input_copy = NULL;
    }

    return i;
}
