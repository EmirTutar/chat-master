#include <pthread.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include "util.h"
#include "user.h"

static User *front = NULL;
static User *back = NULL;
static pthread_mutex_t userLock = PTHREAD_MUTEX_INITIALIZER;

//TODO: Implement the functions declared in user.h

int lock(){
    if (pthread_mutex_lock(&userLock) == 0){
        return 1;
    }
    errorPrint("Error: Mutex could't lock");
    return 0;
}
int unlock(){
    if (pthread_mutex_unlock(&userLock) == 0){
        return 1;
    }
    errorPrint("Error: Mutex couldn't unlock");
    return 0;
}

void printNames(){
      for (User *it = front; it != NULL; it = it->next) {
        infoPrint("%s", it->name);
    }
}

User *userAdd(pthread_t thread, int sock){
    User *newUser = malloc(sizeof (User));
    memset(newUser, '\0', sizeof(User));
    if (newUser == NULL) {
        errorPrint("No memory available");
        return NULL;
    }
    newUser->sock = sock;
    newUser->thread = thread;
    newUser->next = NULL;
    newUser->prev = NULL;
    if (front == NULL)
    {
        front = newUser; // list is empty
        back = newUser;
    }
    else
    {
        back->next = newUser;
        newUser->prev = back;
        back = newUser;
    }
    return newUser;
}

void listForEachToAll(void (*func)(int, char *), char* Name) {
    for (User *it = front; it != NULL; it = it->next) {
        if (it->active){
            func(it->sock, Name);
        }
    }
}

void listForEachToOne(void (*func)(int, char *), User *newUser) {
    for (User *it = front; it != NULL; it = it->next) {
        if (it != newUser && it->active){
            func(newUser->sock, it->name);
        }
    }
}

void listForEachURM(void (*func)(int, char *, int), char* Name, int code) {
    for (User *it = front; it != NULL; it = it->next) {
        if (strcmp(it->name, Name) != 0 && it->active){
            func(it->sock, Name, code);
        }
    }
}

int removeUser(User *deleteUser){
//    infoPrint("List before");
//    printNames();

    if (front == NULL){
        infoPrint("front is NULL");
        return -1;
    }
    User *next = deleteUser->next;
    User *prev = deleteUser->prev;
    if(prev != NULL){
        prev->next = next; // current element is at the start of the list
    }else{
        front = next;
    }
    if (next != NULL)
    {
        next->prev = prev; 
    }else{
        back = prev; // current element is at the end of the list
    }

    free(deleteUser);

    return 1;
}

int kickUser(User *kickUser){
    kickUser->active = 0;
    if (removeUser(kickUser) == 1){
        return 1;
    }
    return -1;
}

void addName(char name[31], User *user){
    strncpy(user->name, name, 31);
    user->name[31] = '\0';
    user->active = 1;
}

int existUser(char name[31]) {
    int value = -1;
    lock();
    for (User *it = front; it != NULL; it = it->next) {
        if (strcmp(it->name, name) == 0) {
            value = 1;
        }
    }
    unlock();
    return value;
}

User * getUser(char* name){
    for (User *it = front; it != NULL; it = it->next) {
        if (strcmp(it->name, name) == 0 && it->name != NULL){
            return it;
        }
    }
    return NULL;
}

User * getFront(){
    return front;
}
