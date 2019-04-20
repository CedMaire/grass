#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <pthread.h>
#include <stdbool.h>

static struct User **userlist;
static int numUsers;

struct User {
    const char* uname;
    const char* pass;

    bool isLoggedIn;
};

int fd[2];//File descriptor for creating a pipe
int fdb[2];


//Create User
void create_user(char* name, char* pass, int userID) {
	struct User *u;
	if ((u = calloc(1, sizeof(struct User))) == NULL) {
		fprintf(stderr, "allocation error\n");
	}
	const char* username;
	if ((username = calloc(16, sizeof(char))) == NULL) {
		fprintf(stderr, "allocation error\n");
	}
	strncpy(username, name, 16);

	const char* password;
	if ((password = calloc(16, sizeof(char))) == NULL) {
		fprintf(stderr, "allocation error\n");
	}
	strncpy(password, pass, 16);
	u->uname = username;
	u->pass = password;
	u->isLoggedIn = false;
	userlist[userID] = u;
}

void *child_process()
{
	int     result;
	char*    ch="admin";

	result = write (fd[1], &ch,7);
	if (result != 7){
		perror ("write");
		exit (2);
	}
	printf ("Child send: %s\n", ch);
	result = read (fdb[0],&ch,7);
	printf("Child receive: %s\n", ch);
}

int main()
{
	pthread_t       child;
	int             result;

	numUsers = 2;
	if ((userlist = calloc(2, sizeof(struct User*))) == NULL) {
		fprintf(stderr, "allocation error\n");
	}

	create_user("Benjamin", "1234", 0);
	create_user("admin", "root", 1);

	result = pipe (fd);
	if (result < 0){
		perror("pipe ");
		exit(1);
	}

	result = pipe (fdb);
	if (result < 0){
		perror("pipe ");
		exit(1);
	}

	pthread_create(&child,NULL,child_process,NULL);

	char*    ch = "AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA";

	result = read (fd[0],&ch,7);
	if (result != 7) {
		perror("read");
		exit(3);
	}

	printf ("Main: %s\n", ch);

	for(int i = 0; i < numUsers; i++) {
		if (strncmp((userlist[i])->uname, ch, 7) == 0) {
				printf("User ok\n");
				char*    ch2="OK";
				result = write (fdb[1], &ch2,7);
		}
	}


	pthread_join(child,NULL);
	return 0;
}
