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

    int socket_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (socket_fd == -1) {
        fprintf(stderr, "Socket creation failed!\n");
        exit(EXIT_FAILURE);
    } else {
        printf("Socket created...\n");
    }

    struct sockaddr_in server_address;
    bzero(&server_address, sizeof(server_address));

    server_address.sin_family = AF_INET;
    server_address.sin_addr.s_addr = inet_addr("127.0.0.1");
    server_address.sin_port = htons(6969);

    if (connect(socket_fd, (struct sockaddr*) &server_address, sizeof(server_address)) != 0) {
        fprintf(stderr, "Connection failed!\n");
        exit(EXIT_FAILURE);
    } else {
        printf("Connected...\n");
    }

    char *string_exit = "exit\n";
    while(true) {
        char buffer[16];
        bzero(buffer, sizeof(buffer));
        write(socket_fd, string_exit, strlen(string_exit) + 1);
        read(socket_fd, buffer, sizeof(buffer));
        printf("From server: %s\n", buffer);
        if (strncmp(buffer, string_exit, strlen(string_exit)) == 0) {
            printf("Exit...\n");
            break;
        }
    }

    fflush(stdout);

    close(socket_fd);
    exit(EXIT_SUCCESS);
}
