#include <grass.h>
#include <ctype.h>

static struct User **userlist;
static int numUsers;
static struct Command **cmdlist;
static int numCmds;
char port[7] = "31337";
static char* base[32];

// Helper function to run commands in unix.
void run_command(const char* command, int sock) {
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
	FILE* conf;
	if ((conf = fopen("grass.conf", "r")) == NULL) {
		fprintf(stderr, "grass.conf not found\n");
	} else {
		printf("File loaded\n");
	}

	char* line[32];
	size_t comment_lines = 0;
	size_t nbr_users = 0;
	size_t offset_of_users = 0;
	while (fgets(line, 32, conf) != NULL) {
		char* token = strtok(line, " ");
		if (strncmp("#", token, 1) == 0) {
			comment_lines++;
		//Parse the base
		} else if (comment_lines == 3 && strncmp("base", token, 4) == 0) {
			token = strtok(NULL, " ");
			if (token == NULL) {
				fprintf(stderr, "base not found\n");
			}
			strncpy(base, token, 32);
			printf("base: %s\n", token);
		//Parse the port
		} else if (comment_lines == 4 && strncmp("port", token, 4) == 0) {
			token = strtok(NULL, " ");
			if (token == NULL) {
				fprintf(stderr, "port not found\n");
			}
			strncpy(port, token, 7);
			printf("port: %s\n", token);
		//Count the number of user
		} else if (comment_lines == 5 && strncmp("user", token, 4) == 0) {
			if (nbr_users == 0) {
				offset_of_users = ftell(conf);
			}
			nbr_users++;
		}
	}
	//Set the offset just before the users
	if (fseek(conf, offset_of_users - 32, SEEK_SET) != 0) {
		fprintf(stderr, "error when counting the users\n");
	}

	if (userlist = calloc(nbr_users, sizeof(struct User*)) == NULL) {
		fprintf(stderr, "allocation error\n");
	}
	//Fill the userlist
	while (fgets(line, 32, conf) != NULL) {
		char* token = strtok(line, " ");
		if (User u = calloc(1, sizeof(struct User)) == NULL) {
			fprintf(stderr, "allocation error\n");
		}
		if (comment_lines == 5 && strncmp("user", token, 4) == 0) {
			token = strtok(NULL, " ");
			if (token == NULL) {
				fprintf(stderr, "user not found, wrong format\n");
			}
			printf("user: %s ", token);

			token = strtok(NULL, " ");
			if (token == NULL) {
				fprintf(stderr, "password not found, wrong format\n");
			}
			printf("pwd: %s\n", token);
		}
	}

	printf("End of conf file reached\n");
}

int main() {

	parse_grass();
	// TODO:
	// Parse the grass.conf file
	// Listen to the port and handle each connection

	int socket_fd = create_socket(server);
	if (socket_fd < 0) {
		fprintf(stderr, "Exit program because of error.\n");
	}

	struct sockaddr_in client_address;
	bzero(&client_address, sizeof(client_address));
	int client_struct_length = sizeof(client_address);
	int connection_fd = accept(socket_fd, (struct sockaddr *) &client_address,
			&client_struct_length);
	if (connection_fd < 0) {
		fprintf(stderr, "Accepting failed!\n");
	} else {
		printf("Client accepted...\n");
	}

	char *string_exit = "exit\n";
	while (true) {
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
