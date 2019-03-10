#include <grass.h>
#include <ctype.h>

static struct User **userlist;
static int numUsers;
static struct Command **cmdlist;
static int numCmds;
char port[7] = "31337";

// Helper function to run commands in unix.
void run_command(const char* command, int sock){
}


/*
 * Send a file to the client as its own thread
 *
 * fp: file descriptor of file to send
 * sock: socket that has already been created.
 */
void send_file(int fp, int sock) {
}

/*
 * Send a file to the server as its own thread
 *
 * fp: file descriptor of file to save to.
 * sock: socket that has already been created.
 * size: the size (in bytes) of the file to recv
 */
void recv_file(int fp, int sock, int size) {
}

// Server side REPL given a socket file descriptor
void *connection_handler(void* sockfd) {
}

/*
 * search all files in the current directory
 * and its subdirectory for the pattern
 *
 * pattern: an extended regular expressions.
 * Output: A line seperated list of matching files' addresses
 */
void search(char *pattern) {
    // TODO
}

// Parse the grass.conf file and fill in the global variables
void parse_grass() {
}

int main() {
    // TODO:
    // Parse the rass.conf file
    // Listen to the port and handle each connection

    int socket_fd = create_socket(server);
    if (socket_fd < 0) {
        fprintf(stderr, "Exit program because of error.\n");
    }

    struct sockaddr_in client_address;
    bzero(&client_address, sizeof(client_address));
    int client_struct_length = sizeof(client_address);
    int connection_fd = accept(socket_fd, (struct sockaddr *) &client_address, &client_struct_length);
    if (connection_fd < 0) {
        fprintf(stderr, "Accepting failed!\n");
    } else {
        printf("Client accepted...\n");
    }

    char *string_exit = "exit\n";
    while(true) {
        char buffer[16];
        bzero(buffer, sizeof(buffer));
        read(connection_fd, buffer, sizeof(buffer));
        printf("From client: %s\n", buffer);
        if (strncmp(buffer, string_exit, strlen(string_exit)) == 0) {
            write(connection_fd, string_exit, strlen(string_exit) + 1);
            printf("Exit...\n");
            break;
        }
    }

    fflush(stdout);
    close(socket_fd);

    exit(EXIT_SUCCESS);
}
