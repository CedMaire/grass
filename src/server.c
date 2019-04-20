#include <grass.h>

static struct User **userlist;
static int numUsers;
static struct Command **cmdlist;
static int numCmds;
char port[7] = "31337";
static char* base[MAX_DIR_LEN];
int max_size = 16;


struct Command shell_cmds[NB_CMD] = {
        { "login", do_login, 1, "$USERNAME" },
        { "pass", do_pass, 1, "$PASSWORD" },
        { "ping", do_ping, 1, "$HOST" },
        { "ls", do_ls, 0, "" },
        { "cd", do_cd, 1, "$DIRECTORY" },
        { "mkdir", do_mkdir, 1, "$DIRECTORY" },
        { "rm", do_rm, 1, "$NAME" },
        { "get", do_get, 1, "$FILENAME" },
        { "put", do_put, 2, "$FILENAME $SIZE" },
        { "grep", do_grep, 1, "$PATTERN" },
        { "date", do_date, 0, "" },
        { "whoami", do_whoami, 0, "" },
        { "w", do_w, 0, "" },
        { "logout", do_logout, 0, "" },
        { "exit", do_exit, 0, "" },
};

/*
 * Checks that the number of provided arguments are correct.
 *
 * cmd_nb: the number of the command to check
 * argc: provided number of arguments
 */
int check_args(int cmd_nb, int argc) {
    size_t nb_args = shell_cmds[cmd_nb].argc;

    if (argc != (int) nb_args) {
        return ERR_ARGS;
    }

    return 0;
}

int check_auth(struct User* user, int client_fd) {
	if (user->isLoggedIn == true){
		return 0;
	} else {
		write(client_fd, "You need to log in first", 25);
		return 1;
	}
}

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
			}
			char pwd[MAX_DIR_LEN];
 			getcwd(pwd, MAX_DIR_LEN - 2) ;
			strcpy(base, pwd);
			strcat(base, token, 32);
			//XXX MKDIR DAT SHIT ???
			/**char str[80];
			MKDIR_SHELLCODE(str, base);
			system(str);**/
			printf("base: %s\n", token);
			//Parse the port
		} else if (strncmp("port", token, 4) == 0) {
			token = strtok(NULL, " ");
			if (token == NULL) {
				fprintf(stderr, "port not found\n");
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
	}

	if ((userlist = calloc(nbr_users, sizeof(struct User*))) == NULL) {
		fprintf(stderr, "allocation error\n");
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
			}

			//Fill the username
			token = strtok(NULL, " ");
			if (token == NULL) {
				fprintf(stderr, "user not found, wrong format\n");
			}
			const char* username;
			if ((username = calloc(max_size, sizeof(char))) == NULL) {
				fprintf(stderr, "allocation error\n");
			}
			strncpy(username, token, max_size);
			printf("user: %s ", token);

			//Fill the password
			token = strtok(NULL, " ");
			if (token == NULL) {
				fprintf(stderr, "password not found, wrong format\n");
			}
			const char* password;
			if ((password = calloc(max_size, sizeof(char))) == NULL) {
				fprintf(stderr, "allocation error\n");
			}
			strncpy(password, token, max_size);

			u->uname = username;
			u->pass = password;
			u->isLoggedIn = false;
			printf("pwd: %s\n", token);
			userlist[userID] = u;

			userID++;
		}
	}
	numUsers = userID;

	printf("End of conf file reached\n");

	fclose(conf);
}

//#######################CMDS###############################################

//array should be be username newline
//user name must be in conf file
//send pipe login waiting
int do_login(const char** array) {
	//array[0] = username
	//array[1] = clientfd
	//array[2] = index
	//array[3] = dir
	printf("login\n");
	fflush(stdout);
	if(array[0] == NULL || *array[0] == '\0') {
		printf("AIE CARAMBA\n");
		fflush(stdout);
	}
	else {
		for(int i = 0; i < numUsers; i++) {
			if (strncmp(userlist[i]->uname, array[0], strlen(userlist[i]->uname)) == 0) {
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
int do_pass(const char** array) {
	//array[0] = pass
	//array[1] = clientfd
	//array[2] = index
	//array[3] = dir
	int client_fd = -1;
    client_fd = atoi(array[1]);
	int index = -1;
    index = atoi(array[2]);
	printf("pass\n");
	fflush(stdout);

	if(index < 0 || array[0] == NULL || *array[0] == '\0' ) {
		printf("AIE CARAMBA\n");
		fflush(stdout);
	}
	else {
		fflush(stdout);
		if (strncmp(userlist[index]->pass, array[0], strlen(userlist[index]->pass)-2) == 0) {
			printf("Matching user and pass\n");
			fflush(stdout);
			if(userlist[index]->isLoggedIn) {
				write(client_fd, "You are already logged in on another client", 44);
				return -1;
			}
			else{
				userlist[index]->isLoggedIn = true;
				write(client_fd, "Log in success", 15);
				return 0;
			}
		}
	}
	write(client_fd, "Incorrect credentials", 22);
    return 1;
}

int do_whoami(const char** array) {
	//array[0] = clientfd
	//array[1] = index
	//array[2] = dir
	int client_fd = -1;
    client_fd = atoi(array[0]);
	int index = -1;
    index = atoi(array[1]);
    //check issue ?? XXX
    fflush(stdout);
    if (index >= 0 && !check_auth(userlist[index], client_fd)) {
    	write(client_fd, userlist[index]->uname, max_size);
    	return 0;
    }
    return 1;
}

int do_logout(const char** array) {
	//array[0] = clientfd
	//array[1] = index
	//array[2] = dir
    int client_fd = atoi(array[0]);
    int index = atoi(array[1]);
    fflush(stdout);
    //free vstuf ?? XXX
    if(index >= 0 && !check_auth(userlist[index], client_fd)) {
    	userlist[index]->isLoggedIn = false;
		write(client_fd, "Loggout success", 16);
    	return 0;
    }
    return 1;
}

int do_w(const char** array) {
	//array[0] = clientfd
	//array[1] = index
	//array[2] = dir
	int client_fd = -1;
    client_fd = atoi(array[0]);
	int index = -1;
    index = atoi(array[1]);
    fflush(stdout);
    if (check_auth(userlist[index], client_fd)) {
    	return 1;
    }
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
    write(client_fd, buffer, size);
    return 0;
}

int do_ping(const char** array) {
	//array[0] = host name
	//array[1] = clientfd
	//array[2] = index
	//array[3] = dir
	int client_fd = -1;
    client_fd = atoi(array[1]);

    if (strlen(array[0]) > MAX_PING_LEN) {
      return 1;
    }
    char str[80]; //EXPLOIT ?
    PING_SHELLCODE(str, array[0]);
    system(str);
    //write(client_fd, str, strlen(str));
    return 0;
}


int do_ls(const char** array) {
	//array[0] = clientfd
	//array[1] = index
	//array[2] = dir
	int client_fd = -1;
    client_fd = atoi(array[0]);
	int index = -1;
    index = atoi(array[1]);
    fflush(stdout);
    if (check_auth(userlist[index], client_fd)) {
    	return 1;
    }
    char working_dir[MAX_DIR_LEN];
   	strcpy(working_dir, array[2]);
    char str[80];
    LS_SHELLCODE(str, working_dir, base);
    system(str);
    return 0;
}

//for cd we accept only absolute paths from base directory
int do_cd(const char** array) {
	//array[0] = path
	//array[1] = clientfd
	//array[2] = index
	//array[3] = dir
	int client_fd = -1;
    client_fd = atoi(array[1]);
	int index = -1;
    index = atoi(array[2]);
    fflush(stdout);
    if ((check_auth(userlist[index], client_fd)) || (strlen(array[0]) > (MAX_DIR_LEN - strlen(base)))) {
    	return 1;
    }
    char new_dir[MAX_DIR_LEN];
    strcpy(new_dir, base);
    strcat(new_dir, array[0]);
    //if array[0] does not end by "/" add it
    //strlen(array[0]) - 1 == "/0" ? XXX
    if (array[0][strlen(array[0]) - 2] != "/"]) {
    	strcat(new_dir, "/");
    }
  	strcpy(dir, new_dir);
    return 0;
}

int do_mkdir(const char** array) {
	//array[0] = dir to mks
	//array[1] = clientfd
	//array[2] = index
	//array[3] = dir
    int client_fd = -1;
    client_fd = atoi(array[1]);
	int index = -1;
    index = atoi(array[2]);
    fflush(stdout);
    if ((check_auth(userlist[index], client_fd)) || (strlen(array[0]) > MAX_DIR_LEN)) {
    	return 1;
    }
   	char working_dir[MAX_DIR_LEN];
   	strcpy(working_dir, array[3]);
   	char dir_to_mk[MAX_DIR_LEN];
    strcpy(dir_to_mk, working_dir);
    strcat(dir_to_mk, array[0]);
    char str[MAX_DIR_LEN + 20]; //XXX
    MKDIR_SHELLCODE(str, dir_to_mk);
    system(str);
    return 0;
}

int do_rm(const char** array) {
	//array[0] = dir to rm
	//array[1] = clientfd
	//array[2] = index
	//array[3] = dir
    int client_fd = -1;
    client_fd = atoi(array[1]);
	int index = -1;
    index = atoi(array[2]);
    fflush(stdout);
    if (check_auth(userlist[index], client_fd)) {
    	return 1;
    }
    char working_dir[MAX_DIR_LEN];
   	strcpy(working_dir, array[3]);
   	char to_rm[MAX_DIR_LEN];
    strcpy(to_rm, working_dir);
    strcat(to_rm, array[0]);
    char str[80];
    RMFILE_SHELLCODE(str, to_rm);
    //XXX RM dir
    system(str);
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
    //array[0] = grep cmd
	//array[1] = clientfd
	//array[2] = index
	//array[3] = dir
	printf("grep");
    int client_fd = -1;
    client_fd = atoi(array[1]);
	int index = -1;
    index = atoi(array[2]);
    fflush(stdout);
    if (check_auth(userlist[index], client_fd)) {
    	return 1;
    }
    system(array[0]);
    return 0;
}

int do_date(const char** array) {
	//array[0] = clientfd
	//array[1] = index
	//array[2] = dir
	int client_fd = -1;
	client_fd = atoi(array[0]);
	int index = -1;
	index = atoi(array[1]);
    if (check_auth(userlist[index], client_fd)) {
    	return 1;
    }
    system(DATE_SHELLCODE);
    return 0;
}

int do_exit(const char** array) {
	//array[0] = clientfd
	//array[1] = index
	//array[2] = dir
    int err = do_logout(array);
    if (!err) {
      exit(0);
    } else {
      exit(1);
    }
    return 0;
}

//##########################################################################################

void client_handler(int client_fd) {
	//Check that non-empty before going to pass
	//-1 is non-set
	//-2 is incorrect
	int username_index = -1;
	//Check that to determine if logged in
	bool loggedIn = false;
	//port used for file transfers
	int port = -1;
	//dir used to keep track of position
	char dir[MAX_DIR_LEN];
	strcpy(dir, base);

	printf("I'm a new thread!Look at me!\n");
	char *string_exit = "exit\n";
	while(true) {
		char* input[max_size];
		bzero(input, sizeof(input));
		read(client_fd, input, sizeof(input));
		printf("From client: %s\n", input);
		char* cmdAndArgs[MAX_INPUT_LENGTH];
        memset(cmdAndArgs, 0, MAX_INPUT_LENGTH);
		int feedbackTok = tokenize_input(input, cmdAndArgs);
        if (feedbackTok < 0) {
            fprintf(stderr, "ERROR SHELL: %s\n", SHELL_ERR_MESSAGES[ERR_ARGS]);
        } else {
			int cmd_nb = -1;
        	for (int i = 0; i < NB_CMD; ++i) {
            	if (strncmp(cmdAndArgs[0], shell_cmds[i].cname, strlen(shell_cmds[i].cname)) == 0) {
                	cmd_nb = i;
                	// We check if we have the correct number of args to call the method.
     	            int feedback = check_args(cmd_nb, feedbackTok - 1);
        	        if (feedback) {
            	        fprintf(stderr, "ERROR SHELL: %s\n", SHELL_ERR_MESSAGES[feedback]);
                	} else {
                	    const char* args[MAX_INPUT_LENGTH + 1];
     	                memset(args, 0, MAX_INPUT_LENGTH + 1);
            	        for (size_t j = 1; j < MAX_PARAM + 1; ++j) {
                            args[j - 1] = cmdAndArgs[j];
                        }
                        //add clienfd in the args
                        char  buff[10];
    					snprintf(buff, sizeof(buff), "%d", client_fd);
                        args[feedbackTok-1] = buff;
                        //add username id
                        char  buff2[10];
    					snprintf(buff2, sizeof(buff2), "%d", username_index);
                        args[feedbackTok] = buff2;

                        args[feedbackTok+1] = dir;
                         
						feedback = (shell_cmds[cmd_nb].fct)(args);
                        if (feedback) { // Can be FS error or SHELL error.
                            if (feedback < 0) {
                                fprintf(stderr, "ERROR FS: %s\n", ERR_MESSAGES[feedback - ERR_FIRST]);
                            } else {
                                fprintf(stderr, "ERROR SHELL: %s\n", SHELL_ERR_MESSAGES[feedback]);
                            }
                        }
                    }
                    i = NB_CMD + 1;
                }
            }
            if (cmd_nb < 0) {
                fprintf(stderr, "ERROR SHELL: %s\n", SHELL_ERR_MESSAGES[ERR_INVALID_CMD]);
            }
        }
		fflush(stdout);

		//TODO when checking/parsing/dispatching
		//If username_index is set and loggedIn is false you have to do pass: otherwise trap.
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

	parse_grass();

	//Main socket
	int socket_fd = create_socket(server);
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
