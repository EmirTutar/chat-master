#include <pthread.h>
#include <string.h>
#include <unistd.h>
#include <mqueue.h>
#include <ctype.h>
#include <malloc.h>
#include "connectionhandler.h"
#include "clientthread.h"
#include "user.h"
#include "util.h"
#include "network.h"
#include "broadcastagent.h"

bool paused = false;
extern sem_t sem;

void *clientthread(void *arg)
{
    User *self = (User *)arg;

    debugPrint("Client thread started, for user sock %d", self->sock);

    Message msg;

    int code = 0;
    char sName[32] = "ServerGroup08";

    int status0 = networkReceive(self->sock, &msg);
    if (status0 == 0 && msg.header.type == 0) {
        int nameLength = nameBytesValidate(msg.body.lrq.name, msg.header.len - 5);
        if (nameLength != msg.header.len - 5) {
            errorPrint("invalid Username");
            code = 2;
        } else if (existUser(msg.body.lrq.name) == 1) {
            errorPrint("Username already taken");
            code = 1;
        } else if (msg.body.lrq.version != 0) {
            errorPrint("Version mismatch");
            code = 3;
        }
        lock();
        sendLoginResponse(self->sock, sName, code);
        unlock();
        if (code > 0) {
            lock("RemoveUser");
            removeUser(self);
            unlock("RemoveUser");
            goto exit;
        } else {
            lock();
            addName(msg.body.lrq.name, self);
            listForEachToAll(sendUserAdded, self->name);
            listForEachToOne((void (*)(int, char *)) sendUserAddedNew, self);
            unlock();
        }
    }
    memset(&msg.body, '\0', sizeof(msg.body));
//TODO: Receive messages and send them to all users, skip self
while (self->active){
    int status = networkReceive(self->sock, &msg);
    if(status == 0 && msg.header.type == 2) {
            //TODO: Adminfunktionen
            if (msg.body.c2s.text[0] == '/' && strcmp(self->name, "Admin") == 0) {
                if (strncmp(msg.body.c2s.text, "/kick", 5) == 0){
                    char kickname[31];
                    int i = 0;
                    while (msg.body.c2s.text[i+6] != 0) {
                        kickname[i] = msg.body.c2s.text[i+6]; //get Name of the User
                        i++;
                    }
                    if (kickname[0] == 0){
                        debugPrint("Name is empty");
                    } else if (existUser(kickname) == 1 && strcmp(kickname, "Admin") != 0){
                        lock();
                        listForEachURM((void (*)(int, char *, int)) sendUserRemoved, kickname, 1);
                        shutdown(getUser(kickname)->sock, SHUT_RDWR);
                        if (!kickUser(getUser(kickname))){
                            debugPrint("Error: Kickuser didnt work");
                        }
                        unlock();
                    } else if (strcmp(kickname, "Admin") == 0) {
                        lock();
                        sendServer2Client(self->sock, "", "Admin can't kick himself");
                        unlock();
                    } else {
                        lock();
                        sendServer2Client(self->sock, "", "Username doesnt exist");
                        unlock();
                    }
                } else if (strncmp(msg.body.c2s.text, "/pause", 6) == 0) {
                    debugPrint("Befehl: Pause");
                    if (!paused){
                        lock();
                        sendServer2Client(0, "", "This Server is taking a nap!\nServer is Paused.");
                        unlock();
                        sleep(1);
                        paused = true;
                        sem_wait(&sem);
                    } else {
                        lock();
                        sendServer2Client(self->sock, "", "The Server is already paused.");
                        unlock();
                    }
                } else if (strncmp(msg.body.c2s.text, "/resume", 7) == 0) {
                    debugPrint("Befehl: Resume");
                    if (paused){
                        lock();
                        sendServer2Client(0, "", "This Server has been awakened again!\nServer is Resumed.");
                        unlock();
                        sem_post(&sem);
                        paused = false;
                    } else {
                        lock();
                        sendServer2Client(self->sock, "", "The Server is already running.");
                        unlock();
                    }
                } else {
                    lock();
                    sendServer2Client(self->sock, "", "unknown command");
                    unlock();
                }
            } else if (msg.body.c2s.text[0] == '/' && strcmp(self->name, "Admin") != 0) {
                lock();
                sendServer2Client(self->sock, "", "You're not the Admin");
                unlock();
            } else {
                lock();
                if (!sendServer2Client(0, self->name, msg.body.c2s.text)){
                    sendServer2Client(self->sock, "", "MessageQueue overload");
                }
                unlock();
            }
        } else if (status == -2 && self->active) {
            lock();
            shutdown(self->sock, SHUT_RDWR);
            removeUser(self);
            listForEachURM((void (*)(int, char *, int)) sendUserRemoved, self->name, 0);
            unlock();
            pthread_detach(pthread_self());
            sleep(10000);
            goto exit;
        } else if (status == -1) {
            lock();
            removeUser(self);
            listForEachURM((void (*)(int, char *, int)) sendUserRemoved, self->name, 0);
            unlock();
            pthread_detach(pthread_self());
            sleep(10000);
            goto exit;
    }
        memset(&msg.body, '\0', sizeof(msg.body));
        if (self->active == 0){
            goto exit;
        }
}
    exit:
        infoPrint("Clientthread beendet.");
        return NULL;
}
