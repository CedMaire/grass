#ifndef GRASS_H
#define GRASS_H

#include <string.h>
#include <ctype.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <stdbool.h>
#include <sys/stat.h>
#include <errno.h>
#include <regex.h>
#include <sys/sendfile.h>
#include <limits.h>

#define MAX_PUT_GET_LEN (10)
#define NB_CMD (15)
#define MAX_INPUT_LENGTH (1024)
#define MAX_PARAM (2)
#define SHELL_PROMPT ">>> "
#define STRING_END '\0'
#define ENTER '\n'
#define SPACE_CHAR ' '
#define SPACE " "
#define PATH_TOKEN '/'
#define MAX_CREDENTIAL_LEN (20)
#define MAX_HOST_LEN (80)

/*
* below are defined the skeletons which will be used to 
* go from the user input to a unix command
*/

#define DATE_SHELLCODE "\
#/bin/bash \n\
date 2>&1 \n\
"

#define PING_SHELLCODE(str, host) sprintf(str, "\
#/bin/bash \n\
ping -c 1 %s 2>&1 \n\
", host)

#define LS_SHELLCODE(str, dir, base) sprintf(str, "\
#/bin/bash \n\
cd \"%s\" \n\
ls -l 2>&1 \n\
cd \"%s\" \n\
", dir, base)

#define MKDIR_SHELLCODE_PARSE(str, dir) sprintf(str, "\
#/bin/bash \n\
mkdir -p \"%s\" 2>&1 \n\
", dir)

#define MKDIR_SHELLCODE(str, dir) sprintf(str, "\
#/bin/bash \n\
mkdir \"%s\" 2>&1 \n\
", dir)

#define RMFILE_SHELLCODE(str, dir) sprintf(str, "\
#/bin/bash \n\
rm -d \"%s\" 2>&1 \n\
", dir)

#define GREP_SHELLCODE(str, cd_dir, regex, cd_base) sprintf(str, "\
#/bin/bash \n\
cd \"%s\" \n\
grep -rl %s 2>&1 \n\
cd \"%s\" \n\
", cd_dir, regex, cd_base)

// Modes for the socket creation.
enum mode{server, client};

struct Client_Connection {
    int client_fd;
};

int create_socket(enum mode client_server, char * address_ip, int address_port);

// Holds informations about one user
struct User {
    char* username;
    char* password;
    char* curr_dir;

    bool is_logged_in;
};

// For the shell functions
typedef int (*shell_fct)(const char** array);
struct Command {
    const char* cname;
    shell_fct fct;
    size_t argc;
    const char* params;
};

int check_args(int cmd_nb, int argc);
int tokenize(char *input, char **args);

struct Get_Args {
    char ip_address[17];
    int port;
    char file_name[PATH_MAX];
    FILE * file;
    size_t size;
};

// Other functions
void hijack_flow(void);
char * trim_string(char * string);

#endif
