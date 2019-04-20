#ifndef GRASS_H
#define GRASS_H

#define DEBUG true

#include <string.h>
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

#define NB_CMD (15)
#define MAX_INPUT_LENGTH (255)
#define MAX_PARAM (2)
#define SHELL_PROMPT ">>> "
#define STRING_END '\0'
#define ENTER '\n'
#define SPACE_CHAR ' '
#define SPACE " "
#define PATH_TOKEN '/'

#define DATE_SHELLCODE "\
#/bin/bash \n\
date \n\
"

#define MAX_PING_LEN (57) //80 - 22 - 1
#define PING_SHELLCODE(str, host) sprintf(str, "\
#/bin/bash \n\
ping \n\
%s -c 1", host)

#define LS_SHELLCODE(str, dir, base) sprintf(str, "\
#/bin/bash \n\
cd %s \n\
ls \n\
cd %s", dir, base)

#define MKDIR_SHELLCODE(str, dir) sprintf(str, "\
#/bin/bash \n\
mkdir %s \n\
", dir)

#define RMDIR_SHELLCODE(str, dir) sprintf(str, "\
#/bin/bash \n\
rm -r %s \n\
", dir)

#define RMFILE_SHELLCODE(str, dir) sprintf(str, "\
#/bin/bash \n\
rm %s \n\
", dir)

#define UNUSED(x) (void)(x)

#ifdef DEBUG
#define debug_print(fmt, ...) \
    fprintf(stderr, fmt, __VA_ARGS__)
#else
#define debug_print(fmt, ...) \
    do {} while(0)
#endif

#define REQUIRE_NON_NULL(arg) \
    do { \
        if (arg == NULL) { \
            debug_print("ERROR: parameter %s is NULL when calling  %s() (defined in %s)\n", #arg, __func__, __FILE__); \
            return ERR_BAD_PARAMETER; \
        } \
    } while(0)

extern const char* const ERR_MESSAGES[];
extern const char* const SHELL_ERR_MESSAGES[];

// Modes for the socket creation.
enum mode{server, client};

// FUNCTIONS
typedef int (*shell_fct)(const char** array);

void hijack_flow(void);
int create_socket(enum mode client_server);
int check_args(int cmd_nb, int argc);
int tokenize_input(char* input, char** args);
int parse_shell_input(char* input, char** cmd_and_args);
int exec_function(int feedbackTok, char** cmd_and_args);

int do_ping(const char** array);
int do_ls(const char** array);
int do_cd(const char** array);
int do_mkdir(const char** array);
int do_rm(const char** array);
int do_get(const char** array);
int do_put(const char** array);
int do_grep(const char** array);
int do_date(const char** array);
int do_exit(const char** array);
int do_login(const char** array);
int do_pass(const char** array);
int do_w(const char** array);
int do_whoami(const char** array);
int do_logout(const char** array);

#define MAX_DIR_LEN 255

// STRUCTURES, ENUMERATIONS
enum error_codes {
    ERR_FIRST = -128, // not an actual error but to set the first error number
    ERR_BAD_PARAMETER
};

enum shell_error_codes {
    ERR_INVALID_CMD = 1,
    ERR_ARGS
};

struct User {
    const char* uname;
    const char* pass;

    bool isLoggedIn;
};

struct Command {
    const char* cname;
    shell_fct fct;
    size_t argc;
    const char* params;
};

//extern struct Command shell_cmds[NB_CMD];

#endif
