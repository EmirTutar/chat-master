#ifndef USER_H
#define USER_H

#include <pthread.h>
#include <stdbool.h>

typedef struct User
{
	struct User *prev; //altes element (l)
    struct User *next; //neues element (r)
    pthread_t thread;	//thread ID of the client thread
    int sock;		//socket for client
    int active;
    char name[];
} User;

int lock();
int unlock();

//TODO: Add prototypes for functions that fulfill the following tasks:
// * Add a new user to the list and start client thread
User *userAdd(pthread_t thread, int sock);
// * Iterate over the complete list (to send messages to all users)
void listForEach(void (*func)(User *));
void listForEachToAll(void (*func)(int, char *), char* Name);
void listForEachToOne(void (*func)(int, char *), User *newUser);
void listForEachURM(void (*func)(int, char *, int), char* Name, int code);

// * Remove a user from the list
//CAUTION: You will need proper locking!
int removeUser(User *deleteUser);
int kickUser(User *kickUser);
void addName(char name[31], User *user);
int existUser(char name[31]);
User * getUser(char* name);
User * getFront();
void free_all();
#endif
