#include <grass.h>
#include <stdio.h>

void hijack_flow() {
    printf("Method hijack: Accepted\n");
    fflush(stdout);
}

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

