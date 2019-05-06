#include <grass.h>

static struct User **user_list;
static int num_users;
static char * address_ip = "127.0.0.1";
static char port[7] = "";
static char base[PATH_MAX];

static int next_port_offset = 1;

static volatile bool keep_running = true;

int do_login(const char** array);
int do_pass(const char** array);
int do_ping(const char** array);
int do_ls(const char** array);
int do_cd(const char** array);
int do_mkdir(const char** array);
int do_rm(const char** array);
int do_get(const char** array);
int do_put(const char** array);
int do_grep(const char** array);
int do_date(const char** array);
int do_whoami(const char** array);
int do_w(const char** array);
int do_logout(const char** array);
int do_exit(const char** array);

/*
*This structure define the name of the valids commands, the corresponding 
*function and the number of expected argument
*/
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

void stop_running() {
    keep_running = false;
}

/*
*Free the users in user_list
*
*number_users: the number of users in user_list
*/
void free_users(int number_users) {
    for (int i = 0; i < number_users; ++i) {
        free(user_list[i]->username);
        free(user_list[i]->password);
        free(user_list[i]->curr_dir);
        free(user_list[i]);
    }
}

/*
 * Checks that the number of provided arguments is correct.
 *
 * cmd_nb: the number of the command to check
 * argc: provided number of arguments
 */
int check_args(int cmd_nb, int argc) {
    if (argc != (int) shell_cmds[cmd_nb].argc) {
        return -1;
    }
    return 0;
}

/*
*This function checks if the user is logged in
*
*
*username_index: the index of the user in user_list
*client_fd: the channel of the client
*
*return:
*0 all good
*1 need to login
*2 pass was expected, username_index should be set to 1
*/
int check_auth(int username_index, int client_fd) {
    if (username_index < 0) {
        char * str_err = "Error: login first.\n";
        fprintf(stderr, "%s", str_err);
        write(client_fd, str_err, strlen(str_err));
        return -1;
    } else if (user_list[username_index]->is_logged_in == true){
        return 0;
    } else {
        char * str_err = "Error: login first.\n";
        fprintf(stderr, "%s", str_err);
        write(client_fd, str_err, strlen(str_err));
        return -1;
    }
}

/*
* Helper function to run commands in unix.
*
*command: the command we wish to run
*client_fd: the channel of the client
*
*return 0 if ok, -1 otherwise
*/
int run_command(const char* command, int client_fd) {
    FILE * fp_output = popen(command, "r");
    if (fp_output == NULL) {
        char * err_str = "Error: failed to run command.\n";
        fprintf(stderr, "%s", err_str);
        write(client_fd, err_str, strlen(err_str) + 1);
        return -1;
    }

    char cmd_output[50 * MAX_INPUT_LENGTH];
    bzero(cmd_output, 50 * MAX_INPUT_LENGTH);
    char cmd_output_line[MAX_INPUT_LENGTH];
    bzero(cmd_output_line, MAX_INPUT_LENGTH);
    while (fgets(cmd_output_line, sizeof(cmd_output_line), fp_output) != NULL) {
        strncat(cmd_output, cmd_output_line, strlen(cmd_output_line));
        bzero(cmd_output_line, MAX_INPUT_LENGTH);
    }
    write(client_fd, cmd_output, strlen(cmd_output) + 1);

    pclose(fp_output);

    return 0;
}

/*
* Parse the grass.conf file and fill in the global variables
*/
void parse_grass() {
    FILE* conf;
    if ((conf = fopen("grass.conf", "r")) == NULL) {
        fprintf(stderr, "Error: grass.conf not found.\n");
        exit(EXIT_FAILURE);
    }

    char * current_line = NULL;
    size_t line_length = 0;
    size_t nbr_users = 0;
    while (getline(&current_line, &line_length, conf) != -1) {
        char* token = strtok(current_line, " ");
        if (strncmp("#", token, 1) == 0) {
            continue;
            //Parse the base
        } else if (strncmp("base", token, 4) == 0) {
            token = strtok(NULL, " ");
            if (token == NULL) {
                fprintf(stderr, "Error: base not found.\n");
                fclose(conf);
                free(current_line);
                exit(EXIT_FAILURE);
            }
            getcwd(base, PATH_MAX - 2);
            char * trimmed_path = trim_string(token);
            if (strncmp(trimmed_path, ".", 1) != 0) {
                strcat(base, "/");
                strcat(base, trimmed_path);
            }
            *(base + strlen(base) + 1) = '\0';
            *(base + strlen(base)) = '/';

            //Parse the port
        } else if (strncmp("port", token, 4) == 0) {
            token = strtok(NULL, " ");
            if (token == NULL) {
                fprintf(stderr, "Error: port not found.\n");
                fclose(conf);
                free(current_line);
                exit(EXIT_FAILURE);
            }
            char * trimmed_port = trim_string(token);
            strncpy(port, trimmed_port, strlen(trimmed_port) + 1);
            //Count the number of user
        } else if (strncmp("user", token, 4) == 0) {
            nbr_users++;
        }
    }

    if ((user_list = calloc(nbr_users, sizeof(struct User*))) == NULL) {
        fprintf(stderr, "Error: allocation error.\n");
        fclose(conf);
        free(current_line);
        exit(EXIT_FAILURE);
    }

    //Set the offset to beginning.
    rewind(conf);
    //Fill the userlist
    int userID = 0;
    while (getline(&current_line, &line_length, conf) != -1) {
        char* token = strtok(current_line, " ");

        if (strncmp("user", token, 4) == 0) {
            //allocate a User
            struct User *u;
            if ((u = calloc(1, sizeof(struct User))) == NULL) {
                fprintf(stderr, "Error: allocation error.\n");
                free_users(userID);
                free(user_list);
                fclose(conf);
                free(current_line);
                exit(EXIT_FAILURE);
            }

            //Fill the username
            token = strtok(NULL, " ");
            if (token == NULL) {
                fprintf(stderr, "Error: user not found, wrong format.\n");
                free_users(userID);
                free(u);
                free(user_list);
                fclose(conf);
                free(current_line);
                exit(EXIT_FAILURE);
            }
            char* username;
            if ((username = calloc(MAX_CREDENTIAL_LEN, sizeof(char))) == NULL) {
                fprintf(stderr, "Error: allocation error.\n");
                free_users(userID);
                free(username);
                free(u);
                free(user_list);
                fclose(conf);
                free(current_line);
                exit(EXIT_FAILURE);
            }
            char * trimmed_username = trim_string(token);
            strncpy(username, trimmed_username, strlen(trimmed_username) + 1);

            // Fill base path for user
            char* base_user_path;
            if ((base_user_path = calloc(MAX_INPUT_LENGTH, sizeof(char))) == NULL) {
                fprintf(stderr, "Error: allocation error.\n");
                free_users(userID);
                free(base_user_path);
                free(username);
                free(u);
                free(user_list);
                fclose(conf);
                free(current_line);
                exit(EXIT_FAILURE);
            }
            bzero(base_user_path, MAX_INPUT_LENGTH);
            strcpy(base_user_path, base);
            strcat(base_user_path, username);
            strcat(base_user_path, "/");

            char mkdir_path_user[MAX_INPUT_LENGTH];
            bzero(mkdir_path_user, MAX_INPUT_LENGTH);
            MKDIR_SHELLCODE_PARSE(mkdir_path_user, base_user_path);
            system(mkdir_path_user);

            //Fill the password
            token = strtok(NULL, " ");
            if (token == NULL) {
                fprintf(stderr, "Error: password not found, wrong format.\n");
                free_users(userID);
                free(base_user_path);
                free(username);
                free(u);
                free(user_list);
                fclose(conf);
                free(current_line);
                exit(EXIT_FAILURE);
            }
            char* password;
            if ((password = calloc(MAX_CREDENTIAL_LEN, sizeof(char))) == NULL) {
                fprintf(stderr, "Error: allocation error.\n");
                free_users(userID);
                free(password);
                free(base_user_path);
                free(username);
                free(u);
                free(user_list);
                fclose(conf);
                free(current_line);
                exit(EXIT_FAILURE);
            }
            char * trimmed_pass = trim_string(token);
            strncpy(password, trimmed_pass, strlen(trimmed_pass));

            u->username = username;
            u->password = password;
            u->curr_dir = base_user_path;
            u->is_logged_in = false;
            user_list[userID] = u;

            userID++;
        }
    }
    num_users = userID;

    free(current_line);
    fclose(conf);
}

// ####################### CMDS ###############################################
// ####################### #### ###############################################
// ####################### #### ###############################################
// ####################### #### ###############################################

/*
*Log the user, need do_pass to finish the login
*
*array[0] = username
*array[1] = clientfd
*array[2] = index
*
*return the index of the user in the user_list or -1 in case of error
*/
int do_login(const char** array) {
    const char * username = array[0];
    int client_fd = (int) strtol(array[1], NULL, 10);
    int index = (int) strtol(array[2], NULL, 10);

    if (index > 0 && user_list[index]->is_logged_in) {
        char * str_err = "Error: already logged in.\n";
        fprintf(stderr, "%s", str_err);
        write(client_fd, str_err, strlen(str_err));
        return -1;
    }

    if(!(username == NULL || *username == '\0')) {
        for(int i = 0; i < num_users; i++) {
            if (strlen(user_list[i]->username) == strlen(username) && strncmp(user_list[i]->username, username, strlen(user_list[i]->username)) == 0) {
                write(client_fd, "\n", 1);
                return i;
            }
        }
    }

    char * err_string = "Error: unknown user...\n";
    char buffer[256];
    strncpy(buffer, array[0], 256);
    printf(buffer);
    fprintf(stderr, "%s", err_string);
    write(client_fd, err_string, strlen(err_string) + 1);
    return -1;
}


/*
*Complete the login if the do_login function is correctly exectuted before and the password is right
*
*array[0] = pass
*array[1] = clientfd
*array[2] = index
*array[3] = dir
*
*return 0 if all ok, -1 otherwise
*/
int do_pass(const char** array) {
    const char * received_password = array[0];
    int client_fd = (int) strtol(array[1], NULL, 10);
    int index = (int) strtol(array[2], NULL, 10);

    if (index < 0) {
        char * err_str = "Error: tell me your username first.\n";
        fprintf(stderr, "%s", err_str);
        write(client_fd, err_str, strlen(err_str));
        return -1;
    }

    if (index > 0 && user_list[index]->is_logged_in) {
        char * str_err = "Error: already logged in.\n";
        fprintf(stderr, "%s", str_err);
        write(client_fd, str_err, strlen(str_err));
        return -1;
    }

    const char * real_password = user_list[index]->password;

    if(!(index < 0 || received_password == NULL || *received_password == '\0')) {
        if (strlen(real_password) == strlen(received_password) && strncmp(real_password, received_password, strlen(real_password)) == 0) {
            if (user_list[index]->is_logged_in) {
                char * err_str = "Error: already logged in with another account.\n";
                fprintf(stderr, "%s", err_str);
                write(client_fd, err_str, strlen(err_str));
                return -1;
            } else {
                user_list[index]->is_logged_in = true;

                char base_user_path[MAX_INPUT_LENGTH];
                bzero(base_user_path, MAX_INPUT_LENGTH);
                strcpy(base_user_path, base);
                strcat(base_user_path, user_list[index]->username);
                strcat(base_user_path, "/");

                strcpy((char *) user_list[index]->curr_dir, base_user_path);

                write(client_fd, "\n", 1);
                return 0;
            }
        }
    }

    char * err_str = "Error: incorrect credentials.\n";
    fprintf(stderr, "%s", err_str);
    write(client_fd, err_str, strlen(err_str));

    return -1;
}

/*
*Write who the client is
*
*array[0] = clientfd
*array[1] = index
*array[2] = dir
*
*return 0 if all good, err otherwise
*/
int do_whoami(const char** array) {
    int client_fd = (int) strtol(array[0], NULL, 10);
    int index = (int) strtol(array[1], NULL, 10);

    int err = check_auth(index, client_fd);
    if (err) {
        return err;
    }

    char to_send[strlen(user_list[index]->username) + 2];
    strncpy(to_send, user_list[index]->username, strlen(user_list[index]->username));
    *(to_send + strlen(user_list[index]->username)) = '\n';
    *(to_send + strlen(user_list[index]->username) + 1) = '\0';
    write(client_fd, to_send, strlen(to_send));

    return 0;
}

/*
*Log the user out
*
*array[0] = clientfd
*array[1] = index
*array[2] = dir
*
*return 0 if all ok, err otherwise
*/

int do_logout(const char** array) {
    int client_fd = (int) strtol(array[0], NULL, 10);
    int index = (int) strtol(array[1], NULL, 10);

    int err = check_auth(index, client_fd);
    if (err) {
        return err;
    }

    user_list[index]->is_logged_in = false;
    write(client_fd, "\n", 1);

    return 0;
}

/*
*Write the number of connected users
*
*array[0] = clientfd
*array[1] = index
*array[2] = dir
*
*return 0 if all good, err otherwise
*/
int do_w(const char** array) {
    int client_fd = (int) strtol(array[0], NULL, 10);
    int index = (int) strtol(array[1], NULL, 10);

    int err = check_auth(index, client_fd);
    if (err) {
        return err;
    }

    size_t size = 0;
    for(int i = 0; i < num_users; i++) {
        if(user_list[i]->is_logged_in) {
            size += strlen(user_list[i]->username) + 1;
        }
    }
    char buffer[size];
    bzero(buffer, size);

    for(int i = 0; i < num_users; i++) {
        if(user_list[i]->is_logged_in) {
            strcat(buffer, user_list[i]->username);
            strcat(buffer, " ");
        }
    }

    *(buffer + size - 1) = '\n';
    write(client_fd, buffer, size);

    return 0;
}

/*
*Ping the defined host
*
*array[0] = host name
*array[1] = clientfd
*array[2] = index
*array[3] = dir
*
*return 0 if all good, an error otherwise
*/
int do_ping(const char** array) {
    const char * host_name = array[0];
    int client_fd = (int) strtol(array[1], NULL, 10);

    if (strlen(array[0]) > MAX_HOST_LEN) {
        char * err_str = "Error: host name too long, must be < 80.\n";
        fprintf(stderr, "%s", err_str);
        write(client_fd, err_str, strlen(err_str));

	char error[8];
	strcpy(error, array[0]);

        return -1;
    }

    char str[2 * MAX_HOST_LEN];
    PING_SHELLCODE(str, host_name);

    int err_feedback = run_command(str, client_fd);
    if (err_feedback < 0) {
        return err_feedback;
    }

    return 0;
}

/*
*Write the result of the ls command
*
*array[0] = clientfd
*array[1] = index
*array[2] = dir
*
*return 0 if all good, err otherwise
*/
int do_ls(const char** array) {
    int client_fd = (int) strtol(array[0], NULL, 10);
    int index = (int) strtol(array[1], NULL, 10);

    int err = check_auth(index, client_fd);
    if (err) {
        return err;
    }

    char str[PATH_MAX + 100];
    LS_SHELLCODE(str, user_list[index]->curr_dir, base);

    int err_feedback = run_command(str, client_fd);
    if (err_feedback < 0) {
        return err_feedback;
    }

    return 0;
}

/*
*Execute the cd command with the submitted path
*
*array[0] = path
*array[1] = clientfd
*array[2] = index
*array[3] = dir
*
*return 0 if all good, -1 or err otherwise
*/
int do_cd(const char** array) {
    const char * path = array[0];
    int client_fd = (int) strtol(array[1], NULL, 10);
    int index = (int) strtol(array[2], NULL, 10);

    int err = check_auth(index, client_fd);
    if (err) {
        return err;
    }

    char new_dir[PATH_MAX];
    bzero(new_dir, PATH_MAX);
    strcpy(new_dir, user_list[index]->curr_dir);
    strcat(new_dir, "/");
    strcat(new_dir, path);

    char * reduced_dir = realpath(new_dir, NULL);
    if (reduced_dir != NULL) {
        if (strlen(reduced_dir) < strlen(base)) {
            char * err_str = "Error: trying to go out of container.\n";
            fprintf(stderr, "%s", err_str);
            write(client_fd, err_str, strlen(err_str) + 1);

            free(reduced_dir);
            return -1;
        } else if (strlen(reduced_dir) - strlen(base) > 128) {
            char * str_err = "Error: path too long.\n";
            fprintf(stderr, "%s", str_err);
            write(client_fd, str_err, strlen(str_err));

            free(reduced_dir);
            return -1;
        } else {
            strcpy((char *) user_list[index]->curr_dir, reduced_dir);
            write(client_fd, "\n", 1);

            free(reduced_dir);
            return 0;
        }
    } else {
        char * err_str = "Error: while resolving directory path.\n";
        fprintf(stderr, "%s", err_str);
        write(client_fd, err_str, strlen(err_str) + 1);

        return -1;
    }
}

/*
*Execute the mkdir command with the given name
*
*array[0] = dir to mks
*array[1] = clientfd
*array[2] = index
*array[3] = dir
*
*return 0 if all good, -1 or err otherwise
*/
int do_mkdir(const char** array) {
    const char * dir = array[0];
    int client_fd = (int) strtol(array[1], NULL, 10);
    int index = (int) strtol(array[2], NULL, 10);

    int err = check_auth(index, client_fd);
    if (err) {
        return err;
    }

    char new_dir_copy[strlen(dir) + 1];
    bzero(new_dir_copy, strlen(dir));
    strcpy(new_dir_copy, dir);

    while (*(new_dir_copy + strlen(new_dir_copy) - 1) == '/') {
        *(new_dir_copy + strlen(new_dir_copy) - 1) = '\0';
    }

    bool contains_slash = (strchr(new_dir_copy, '/') != NULL);
    if (!contains_slash) {
        char new_dir[PATH_MAX];
        bzero(new_dir, PATH_MAX);
        strcpy(new_dir, user_list[index]->curr_dir);
        strcat(new_dir, "/");
        strcat(new_dir, new_dir_copy);

        if (strlen(new_dir) - strlen(base) > 128) {
            char * str_err = "Error: new directory path too long.\n";
            fprintf(stderr, "%s", str_err);
            write(client_fd, str_err, strlen(str_err));
	    
	    char error[8];
	    strcpy(error, array[0]);

            return -1;
        } else if (access(new_dir, F_OK | R_OK) == 0) {
            char * err_str = "Error: folder already exists.\n";
            fprintf(stderr, "%s", err_str);
            write(client_fd, err_str, strlen(err_str) + 1);

            return -1;
        } else {
            char str[PATH_MAX + 100];
            MKDIR_SHELLCODE(str, new_dir);

            int err_feedback = run_command(str, client_fd);
            if (err_feedback < 0) {
                return err_feedback;
            }

            return 0;
        }
    } else {
        char * err_str = "Error: only allowed to mkdir in current working directory.\n";
        fprintf(stderr, "%s", err_str);
        write(client_fd, err_str, strlen(err_str) + 1);

        return -1;
    }
}

/*
*Execute the rm command with the given dir
*
*array[0] = dir to rm
*array[1] = clientfd
*array[2] = index
*array[3] = dir
*
*return 0 if all good, -1 or err otherwise
*/
int do_rm(const char** array) {
    const char * dir = array[0];
    int client_fd = (int) strtol(array[1], NULL, 10);
    int index = (int) strtol(array[2], NULL, 10);

    int err = check_auth(index, client_fd);
    if (err) {
        return err;
    }

    char new_dir_copy[strlen(dir) + 1];
    bzero(new_dir_copy, strlen(dir));
    strcpy(new_dir_copy, dir);

    while (*(new_dir_copy + strlen(new_dir_copy) - 1) == '/') {
        *(new_dir_copy + strlen(new_dir_copy) - 1) = '\0';
    }
    bool contains_slash = (strchr(new_dir_copy, '/') != NULL);

    if (!contains_slash) {
        char new_dir[PATH_MAX];
        bzero(new_dir, PATH_MAX);
        strcpy(new_dir, user_list[index]->curr_dir);
        strcat(new_dir, "/");
        strcat(new_dir, new_dir_copy);
        char * reduced_dir = realpath(new_dir, NULL);

        if (reduced_dir == NULL) {
            char * err_str = "Error: folder or file does not exists.\n";
            fprintf(stderr, "%s", err_str);
            write(client_fd, err_str, strlen(err_str) + 1);

            return -1;
        } else if (strlen(reduced_dir) - strlen(base) > 128) {
            char * str_err = "Error: path too long.\n";
            fprintf(stderr, "%s", str_err);
            write(client_fd, str_err, strlen(str_err));

            free(reduced_dir);
            return -1;
        } else {
            char str[PATH_MAX + 100];
            RMFILE_SHELLCODE(str, reduced_dir);

            int err_feedback = run_command(str, client_fd);
            if (err_feedback < 0) {
                free(reduced_dir);
                return err_feedback;
            }

            free(reduced_dir);
            return 0;
        }
    } else {
        char * err_str = "Error: only allowed to remove inside current working directory.\n";
        fprintf(stderr, "%s", err_str);
        write(client_fd, err_str, strlen(err_str) + 1);

        return -1;
    }
}

/*
*This is the handler function for get. It permits parallelization
*
*arg: a pointer to a Get_Args struct (contain the port, file and size of the file)
*/
static void * getee_handler(void *arg) {
    struct Get_Args * get_args = arg;

    int socket_fd = create_socket(server, address_ip, get_args->port);
    if (socket_fd < 0) {
        fprintf(stderr, "Error: terminating get because of socket error.\n");
        fclose(get_args->file);
        pthread_detach(pthread_self());
        pthread_exit(0);
    }

    struct sockaddr_in client_address;
    bzero(&client_address, sizeof(client_address));
    unsigned int client_struct_length = sizeof(client_address);
    int connection_fd = accept(socket_fd, (struct sockaddr *) &client_address, &client_struct_length);
    if (connection_fd >= 0) {
        char packet[get_args->size];
        bzero(packet, get_args->size);
        size_t size_r = fread(packet, sizeof(char), (get_args->size - 1), get_args->file);
        if (ferror(get_args->file) != 0 || size_r != get_args->size - 1) {
            char * read_err = "Error: reading file.\n";
            fprintf(stderr, "%s", read_err);
            write(connection_fd, read_err, strlen(read_err));
        }

        write(connection_fd, packet, get_args->size);
    }

    fclose(get_args->file);
    free(get_args);
    close(connection_fd);
    close(socket_fd);

    pthread_detach(pthread_self());
    pthread_exit(0);
}

/*
*Execute the get command
*
*array[0] = filename
*array[1] = client_fd
*array[2] = index
*
*return 0 if all good, -1 or err otherwise
*/
int do_get(const char** array) {
    const char * filename = array[0];
    int client_fd = (int) strtol(array[1], NULL, 10);
    int index = (int) strtol(array[2], NULL, 10);

    int err = check_auth(index, client_fd);
    if (err) {
        return err;
    }

    char file_path[MAX_INPUT_LENGTH];
    bzero(file_path, MAX_INPUT_LENGTH);
    strcpy(file_path, user_list[index]->curr_dir);
    strcat(file_path, "/");
    strcat(file_path, filename);

    FILE* file;
    if ((file = fopen(file_path, "r")) == NULL) {
        char * str_err = "Error: file not found.\n";
        fprintf(stderr, "%s", str_err);
        write(client_fd, str_err, strlen(str_err));
        return -1;
    }

    //Calculate size
    int size = 0;
    fseek(file, 0, SEEK_END);
    size = (int) ftell(file);
    if (size < 0) {
        char * str_err = "Error: get error.\n";
        fprintf(stderr, "%s", str_err);
        write(client_fd, str_err, strlen(str_err));
        return -1;
    }

    //Reset the pointer to the beginning of the file
    if (fseek(file, 0, SEEK_SET) != 0) {
        char * str_err = "Error: get error.\n";
        fprintf(stderr, "%s", str_err);
        write(client_fd, str_err, strlen(str_err));
        return -1;
    }

    int port_transfer = (int) (strtol(port, NULL, 10) + next_port_offset);
    next_port_offset += 5;

    struct Get_Args * get_args = (struct Get_Args *) calloc(1, sizeof(struct Get_Args));
    get_args->port = port_transfer;
    get_args->file = file;
    get_args->size = (size_t) (size + 1);

    pthread_t get_child;
    pthread_create(&get_child, NULL, &getee_handler, get_args);

    char response[MAX_INPUT_LENGTH];
    bzero(response, MAX_INPUT_LENGTH);
    sprintf(response, "get port: %d size: %d\n", port_transfer, size);

    write(client_fd, response, strlen(response) + 1);

    return 0;
}

/*
*This is the handler function for put. It permits parallelization
*
*arg: a pointer to a Get_Args struct (contain the port, file name and size of the file)
*/
static void * putee_handler(void *arg) {
    struct Get_Args * get_args = arg;

    int socket_fd = create_socket(server, address_ip, get_args->port);
    if (socket_fd < 0) {
        fprintf(stderr, "Error: terminating put because of socket error.\n");
        pthread_detach(pthread_self());
        pthread_exit(0);
    }

    struct sockaddr_in client_address;
    bzero(&client_address, sizeof(client_address));
    unsigned int client_struct_length = sizeof(client_address);
    int connection_fd = accept(socket_fd, (struct sockaddr *) &client_address, &client_struct_length);
    if (connection_fd >= 0) {
        char client_feedback[get_args->size];
        bzero(client_feedback, get_args->size);
        read(connection_fd, client_feedback, get_args->size);

        FILE* fp;
        fp = fopen(get_args->file_name, "w+");
        if (fp == NULL || strcmp(client_feedback, "\n") == 0) {
            fprintf(stderr, "Error: putting file, check the name and/or the size.\n");
        } else {
            fwrite(client_feedback, sizeof(char), get_args->size, fp);
            fclose(fp);
        }
    }

    free(get_args);
    close(connection_fd);
    close(socket_fd);

    pthread_detach(pthread_self());
    pthread_exit(0);
}

/*
*Execute the put command
*
*array[0] = filename
*array[1] = file size
*array[2] = client_fd
*array[3] = index
*
*return 0 if all good, err otherwise
*/
int do_put(const char** array) {
    const char * filename = array[0];
    int file_size = (int) strtol(array[1], NULL, 10);
    int client_fd = (int) strtol(array[2], NULL, 10);
    int index = (int) strtol(array[3], NULL, 10);

    int err = check_auth(index, client_fd);
    if (err) {
        return err;
    }

    char file_path[MAX_INPUT_LENGTH];
    bzero(file_path, MAX_INPUT_LENGTH);
    strcpy(file_path, user_list[index]->curr_dir);
    strcat(file_path, "/");
    strcat(file_path, filename);

    int port_transfer = (int) (strtol(port, NULL, 10) + next_port_offset);
    next_port_offset += 5;

    struct Get_Args * put_args = (struct Get_Args *) calloc(1, sizeof(struct Get_Args));
    put_args->port = port_transfer;
    strcpy(put_args->file_name, file_path);
    put_args->size = (size_t) file_size;

    pthread_t put_child;
    pthread_create(&put_child, NULL, &putee_handler, put_args);

    char response[MAX_INPUT_LENGTH];
    bzero(response, MAX_INPUT_LENGTH);
    sprintf(response, "put port: %d\n", port_transfer);

    write(client_fd, response, strlen(response) + 1);

    return 0;
}

/*
*Execute grep and write its output to the client
*
*array[0] = grep cmd
*array[1] = clientfd
*array[2] = index
*array[3] = dir
*
*return 0 if all good, err otherwise
*/
int do_grep(const char** array) {
    const char * regex = array[0];
    int client_fd = (int) strtol(array[1], NULL, 10);
    int index = (int) strtol(array[2], NULL, 10);

    int err = check_auth(index, client_fd);
    if (err) {
        return err;
    }

    char str[PATH_MAX + 100];
    GREP_SHELLCODE(str, user_list[index]->curr_dir, regex, base);

    int err_feedback = run_command(str, client_fd);
    if (err_feedback < 0) {
        return err_feedback;
    }

    return 0;
}


/*
*Write the result of the date command to the client
*
*array[0] = clientfd
*array[1] = index
*array[2] = dir
*
*return 0 if all good, an error otherwise
*/
int do_date(const char** array) {
    int client_fd = (int) strtol(array[0], NULL, 10);
    int index = (int) strtol(array[1], NULL, 10);

    int err = check_auth(index, client_fd);
    if (err) {
        return err;
    }

    int err_feedback = run_command(DATE_SHELLCODE, client_fd);
    if (err_feedback < 0) {
        return err_feedback;
    }

    return 0;
}

/*
*Close the session of the client and logit out
*
*array[0] = clientfd
*array[1] = index
*array[2] = dir
*
*return 0
*/
int do_exit(const char** array) {

    int index = (int) strtol(array[1], NULL, 10);

    if (index >= 0 && user_list[index]->is_logged_in == true) {
        do_logout(array);
    }

    return 0;
}

// ##########################################################################################
// ##########################################################################################
// ##########################################################################################

/*
*A function that handle a client. Normally spawned in a new thread by the main
*
**arg: the connection informations of the client
*/
static void * client_handler(void *arg) {
    struct Client_Connection * cl_co = arg;
    int client_fd = cl_co->client_fd;

    int username_index = -1;
    char input[MAX_INPUT_LENGTH + 1];
    bool looping = true;
    while(looping) {
        bzero(input, MAX_INPUT_LENGTH + 1);
        read(client_fd, input, MAX_INPUT_LENGTH + 1);

        // Empty socket buffer.
        int socket_options = fcntl(client_fd, F_GETFL);
        fcntl(client_fd, F_SETFL, (socket_options | O_NONBLOCK));
        char empty_socket[MAX_INPUT_LENGTH + 1];
        int read_bytes = read(client_fd, empty_socket, MAX_INPUT_LENGTH + 1);
        while (read_bytes > 0) {
            read_bytes = read(client_fd, empty_socket, MAX_INPUT_LENGTH + 1);
        }
        fcntl(client_fd, F_SETFL, socket_options);

        looping = (strncmp(input, "exit\n", 5) != 0);

        char* ending = strrchr(input, ENTER);
        if (ending != NULL) {
            *ending = STRING_END;
        }
        ending = strrchr(input, PATH_TOKEN);
        if (ending != NULL && *(ending + 1) == STRING_END && *(ending - 1) != SPACE_CHAR) {
            *ending = STRING_END;
        }

        char* cmdAndArgs[MAX_INPUT_LENGTH];
        bzero(cmdAndArgs, MAX_INPUT_LENGTH);
        int feedbackTok = tokenize(input, cmdAndArgs);
        if (cmdAndArgs[0] == NULL) {
            char * str_err = "Error: incorrect format.\n";
            fprintf(stderr, "%s", str_err);
            write(client_fd, str_err, strlen(str_err));
        } else {
            int cmd_nb = -1;

            bool user_logged_out = (username_index < 0);
            bool user_logged_in = (username_index >= 0 && user_list[username_index]->is_logged_in);
            bool waiting_for_pass = (!user_logged_in && !user_logged_out);

            for (int i = 0; i < NB_CMD; ++i) {
                if (strlen(shell_cmds[i].cname) == strlen(cmdAndArgs[0]) && strncmp(cmdAndArgs[0], shell_cmds[i].cname, strlen(shell_cmds[i].cname)) == 0) {
                    cmd_nb = i;
                    // We check if we have the correct number of args to call the method.
                    int feedback = check_args(cmd_nb, feedbackTok - 1);
                    if (feedback) {
                        char * err_str = "Error: wrong number of arguments.\n";
                        fprintf(stderr, "%s", err_str);
                        write(client_fd, err_str, strlen(err_str));
                    } else {
                        const char* args[MAX_INPUT_LENGTH + 1];
                        bzero(args, MAX_INPUT_LENGTH + 1);
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

                        feedback = (shell_cmds[cmd_nb].fct)(args);
                        //found user so update user_id
                        if (feedback >= 0 && strcmp(shell_cmds[cmd_nb].cname, "login") == 0) {
                            username_index = feedback;
                        }
                        //successful logout user_id back to -1
                        if (feedback >= 0 && strcmp(shell_cmds[cmd_nb].cname, "logout") == 0) {
                            username_index = -1;
                        }
                        //pass was expected but other cmd was ran 
                        if (waiting_for_pass && strcmp(shell_cmds[cmd_nb].cname, "pass") != 0) {
                            username_index = -1;
                        }
                    }
                    i = NB_CMD + 1;
                }
            }
        

            if (cmd_nb < 0) {
            	char * err_str = "Error: invalid command.\n";
                fprintf(stderr, "%s", err_str);
                write(client_fd, err_str, strlen(err_str));
            }
        }
        fflush(stdout);
    }

    close(client_fd);
    free(cl_co);
    pthread_detach(pthread_self());
    pthread_exit(0);
}

int main() {
    printf("SERVER RUNNING\n");
    signal(SIGINT, (__sighandler_t) stop_running);
    parse_grass();

    //Main socket
    int socket_fd = create_socket(server, address_ip, (int) strtol(port, NULL, 10));
    if (socket_fd < 0) {
        fprintf(stderr, "Exit program because of socket error.\n");
        exit(EXIT_FAILURE);
    }
    fcntl(socket_fd, F_SETFL, O_NONBLOCK);

    //Wait for clients
    while(keep_running){
        //Create socket for a connecting client
        struct sockaddr_in client_address;
        bzero(&client_address, sizeof(client_address));
        unsigned int client_struct_length = sizeof(client_address);
        int connection_fd = accept(socket_fd, (struct sockaddr *) &client_address, &client_struct_length);
        if (connection_fd >= 0) {
            struct Client_Connection * cl_co;
            cl_co = calloc(1, sizeof(struct Client_Connection));
            cl_co->client_fd = connection_fd;

            //Create new thread for the client
            pthread_t child;

            int err = pthread_create(&child, NULL, &client_handler, cl_co);
            if(err != 0) {
                fprintf(stderr, "Error: failed to open a new thread.\n");
                close(connection_fd);
            }
        }
    }

    free_users(num_users);
    free(user_list);

    fflush(stdout);
    close(socket_fd);

    exit(EXIT_SUCCESS);
}
