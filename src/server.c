#include <grass.h>
#include <ctype.h>
#include <stdbool.h>

static struct User **userlist;
static int numUsers;
static struct Command **cmdlist;
static int numCmds;
char port[7] = "31337";
static char* base[32];
int max_size = 16;

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
int parse_grass() {
	FILE* conf;
	if ((conf = fopen("grass.conf", "r")) == NULL) {
		fprintf(stderr, "grass.conf not found\n");
		fflush(stdout);
		return 0;
	} else {
		printf("File loaded\n");
	}

	char* line[32];
	size_t nbr_users = 0;
	size_t offset_of_users = 0;
	while (fgets(line, 32, conf) != NULL) {
		char* token = strtok(line, " ");
		if (strncmp("#", token, 1) == 0) {
			continue;
			//Parse the base
		}  else if (strncmp("base", token, 4) == 0) {
			token = strtok(NULL, " ");
			if (token == NULL) {
				fprintf(stderr, "base not found\n");
				fflush(stdout);
				return 0;
			}
			strncpy(base, token, 32);
			printf("base: %s\n", token);
			//Parse the port
		} else if (strncmp("port", token, 4) == 0) {
			token = strtok(NULL, " ");
			if (token == NULL) {
				fprintf(stderr, "port not found\n");
				fflush(stdout);
				return 0;
			}
			strncpy(port, token, 7);
			printf("port: %s\n", token);
			//Count the number of user
		} else if (strncmp("user", token, 4) == 0) {
			if (nbr_users == 0) {
				offset_of_users = ftell(conf);
			}
			nbr_users++;
		}
	}
	//Set the offset just before the users
	if (fseek(conf, offset_of_users - 32, SEEK_SET) != 0) {
		fprintf(stderr, "error when counting the users\n");
		fflush(stdout);
		return 0;
	}

	if ((userlist = calloc(nbr_users, sizeof(struct User*))) == NULL) {
		fprintf(stderr, "allocation error\n");
		fflush(stdout);
		return 0;
	}

	//Fill the userlist
	int userID = 0;
	while (fgets(line, 32, conf) != NULL) {
		char* token = strtok(line, " ");

		if (strncmp("user", token, 4) == 0) {
			//allocate a User
			struct User *u;
			if ((u = calloc(1, sizeof(struct User))) == NULL) {
				fprintf(stderr, "allocation error\n");
				fflush(stdout);
				return 0;
			}

			//Fill the username
			token = strtok(NULL, " ");
			if (token == NULL) {
				fprintf(stderr, "user not found, wrong format\n");
				fflush(stdout);
				return 0;
			}
			const char* username;
			if ((username = calloc(max_size, sizeof(char))) == NULL) {
				fprintf(stderr, "allocation error\n");
				fflush(stdout);
				return 0;
			}
			strncpy(username, token, max_size);
			printf("user: %s ", token);

			//Fill the password
			token = strtok(NULL, " ");
			if (token == NULL) {
				fprintf(stderr, "password not found, wrong format\n");
				fflush(stdout);
				return 0;
			}
			const char* password;
			if ((password = calloc(max_size, sizeof(char))) == NULL) {
				fprintf(stderr, "allocation error\n");
				fflush(stdout);
				return 0;
			}
			strncpy(password, token, max_size);

			u->uname = username;
			u->pass = password;
			u->isLoggedIn = false;
			printf("pwd: %s\n", token);
			fflush(stdout);
			userlist[userID] = u;

			userID++;
		}
	}
	numUsers = userID;

	printf("End of conf file reached\n");
	fflush(stdout);

	fclose(conf);

	return 1;
}

//array should be be username newline
//user name must be in conf file
//send pipe login waiting
const int do_login_server(const char** array) {
	printf("login\n");
	fflush(stdout);
	if(array == NULL) {
		printf("AIE CARAMBA\n");
		fflush(stdout);
	}
	else {
		for(int i = 0; i < numUsers; i++) {
			if (strncmp(userlist[i]->uname, array, strlen(userlist[i]->uname)) == 0) {
				printf("Known User\n");
				fflush(stdout);
				return i;
			}
		}
	}
	return -2;
}

//directly follow login command
//password followed by newline
//if matches, user authentified
//password = array[0];
//send to pipe password and check that exists one waiting password
int do_pass_server(const char** array, int index, int client_fd) {
	printf("pass\n");
	fflush(stdout);

	if(index < 0 || array == NULL) {
		printf("AIE CARAMBA\n");
		fflush(stdout);
	}
	else {
		fflush(stdout);
		if (strncmp(userlist[index]->pass, array, strlen(userlist[index]->pass)-2) == 0) {
			printf("Matching user and pass\n");
			fflush(stdout);
			if(userlist[index]->isLoggedIn) {
				write(client_fd, "You are already logged in on another client", 44);
				return -1;
			}
			else{
				userlist[index]->isLoggedIn = true;
				write(client_fd, "Log in success", 15);
				return 1;
			}
		}
	}
	write(client_fd, "Incorrect credentials", 22);
    return 0;
}

int do_whoami_server(const char** array, int index, int client_fd) {
    printf("whoami\n");
    fflush(stdout);
    UNUSED(array);
    if(index >= 0 && userlist[index]->isLoggedIn) {
    	write(client_fd, userlist[index]->uname, max_size);
    	return 1;
    }
    //check auth
    //currently logged user
    return 0;
}

int do_logout_server(const char** array, int index, int client_fd) {
    printf("logout\n");
    fflush(stdout);
    UNUSED(array);
    //check authentication
    //logout
    if(index >= 0 && userlist[index]->isLoggedIn) {
    	userlist[index]->isLoggedIn = false;
		write(client_fd, "Loggout success", 16);
    	return 1;
    }
    return 0;
}

int do_w_server(const char** array, int clientfd) {
    printf("w\n");
    fflush(stdout);
    UNUSED(array);
    //successful auth
    //list of each logged in user on a single line space separated
    int size = 0;
    for(int i = 0; i < numUsers; i++) {
    	if(userlist[i]->isLoggedIn) {
    		size += strlen(userlist[i]->uname + 1);
    	}
    }
    char* buffer[size];

    bool first = true;
    for(int i = 0; i < numUsers; i++) {
    	if(userlist[i]->isLoggedIn) {
    		if(first) {
        		strcpy(buffer, userlist[i]->uname);
        		first = false;
    		} else {
    			strcat(buffer, " ");
    			strcat(buffer, userlist[i]->uname);
    		}
    	}
    }
    printf("%s", buffer);
    fflush(stdout);
    write(clientfd, buffer, size);
    return 0;
}

int do_get_server(const char** array, int client_fd, pthread_t thread, int port) {
	printf("get\n");
	fflush(stdout);

	if(port >= 0) {
		pthread_cancel(thread);
	}

	//Load file
	if(array == NULL) {
		printf("AIE CARAMBA");
		fflush(stdout);
	} else {
		FILE* file;
		if ((file = fopen(*array, "r")) == NULL) {
			fprintf(stderr, "file not found\n");
			fflush(stdout);
			return 0;
		} else {
			printf("File loaded\n");
			fflush(stdout);
		}
		//Calculate size
		int size=0;
		fseek (file,0,SEEK_END);
		size= ftell (file);
		if (size!=-1) {
			printf("Size: %d\n" ,size);
			fflush(stdout);

		} else {
			printf("Not able to calculate size\n");
			fflush(stdout);
			return 0;
		}

		//Reset the pointer to the beginning of the file
		if (fseek(file, 0, SEEK_SET) != 0) {
			fprintf(stderr, "enable to reset the offset\n");
			fflush(stdout);
			return 0;
		}

		//DO WE SEND PACKET SIZE OR PACKET SIZE + 1 (SIZE + \0)
		//Send the size and the port to the client
		//TODO

		//Load the file in the buffer (hope the file isn't too large otherwise it could be slow)
		char* packet[size + 1];
		int size_r = fread(packet, sizeof(char), size, file);
		printf("%s\n",packet);
		if (ferror(file) != 0 || size_r != size) {
			fputs("Error reading file\n", stderr);
		}else{
			packet[size] = '\0';
		}
		fclose(file);


		//Write the file to the new port
		int socket_fd = create_socket(server, port);
		//DO WE SEND PACKET SIZE OR PACKET SIZE + 1 (SIZE + \0)
		write(socket_fd, packet, size);
		close(socket_fd);

	}
	return 0;
}

void client_handler(int client_fd) {
	//Check that non-empty before going to pass
	//-1 is non-set
	//-2 is incorrect
	int username_index = -1;
	//Check that to determine if logged in
	bool loggedIn = false;

	//Variable for get/put
	pthread_t thread;
	int port;

	printf("I'm a new thread!Look at me!\n");
	char *string_exit = "exit\n";
	while(true) {
		char* buffer[max_size];
		bzero(buffer, sizeof(buffer));
		read(client_fd, buffer, sizeof(buffer));
		printf("From client: %s\n", buffer);
		fflush(stdout);

		//TODO when checking/parsing/dispatching
		//If username_index is set and loggedIn is false you have to do pass: otherwise trap.
		//Check that line end by \n
		//Check if loggedIn
		//Check if the client is already loggedIn what do we do if pass or login?!?

		/* FUNCTIONS WHO NEED A SPECIAL HANDLING OF THEIR RETURN
		int answer = do_logout_server("", username_index, client_fd);
		if(answer == 1) {
			loggedIn = false;
			username_index = -1;
		}

		//What to do if the function is do_pass
		int result = do_pass_server(&buffer, username_index, client_fd);
		if(result == 1) {
			loggedIn = true;
		}else {
			username_index = -1;
		}
		break;
		*/
	}
	close(client_fd);
}

int main() {
	char* t = "coucou.txt";
	do_get_server(&t,0, 0,0);
	if (parse_grass() == 0) {
		exit(EXIT_SUCCESS);;
	}

	//Main socket
	int socket_fd = create_socket(server, 1337);
	if (socket_fd < 0) {
		fprintf(stderr, "Exit program because of error.\n");
	}

	//Wait for clients
	while(1){
		//Create socket for a connecting client
		struct sockaddr_in client_address;
		bzero(&client_address, sizeof(client_address));
		int client_struct_length = sizeof(client_address);
		int connection_fd = accept(socket_fd, (struct sockaddr *) &client_address, &client_struct_length);
		if (connection_fd < 0) {
			fprintf(stderr, "Accepting failed!\n");
		} else {
			printf("Client accepted...\n");
		}

		//Create new thread for the client
		pthread_t child;

		int err = pthread_create(&child,NULL,client_handler,connection_fd);
		if(err != 0) {
			fprintf(stderr, "Fail to open a new thread!\n");
		}
	}

	fflush(stdout);
	close(socket_fd);

	exit(EXIT_SUCCESS);
}
