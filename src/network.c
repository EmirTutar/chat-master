#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <time.h>
#include <mqueue.h>
#include "util.h"
#include "network.h"
#include "broadcastagent.h"

static struct timespec tm;

int networkReceive(int fd, Message *buffer) {
        int statusHeader = recv(fd, &(buffer->header), sizeof(buffer->header), MSG_WAITALL);
        if (statusHeader == -1) {
            //errnoPrint("Error in recv");
            return -1;
        } else if (statusHeader == 0) {
            return -2;
        }
        int statusBody;
        buffer->header.len = ntohs(buffer->header.len);
        if (buffer->header.len <= 0){
            errorPrint("Error: Header.len = 0");
            return -1;
        }
        statusBody = recv(fd, &(buffer->body), buffer->header.len, MSG_WAITALL);
        if (buffer->header.len != statusBody) {
            errnoPrint("Error: wrong Message size");
            return -1;
        }
        switch (buffer->header.type) {
            case 0:
                buffer->body.lrq.magic = ntohl(buffer->body.lrq.magic);
                if (buffer->body.lrq.magic != 0x0badf00d){
                    errnoPrint("Error: wrong Magic Number");
                    return -1;
                }
                break;
            case 2:
                infoPrint("Header.Type = 2 C2S received network.c");
                break;
            default:
                debugPrint("False HeaderType");
                break;
        }
    return 0;
}

int networkSend(int fd, Message *buffer)
{
	//TODO: Send complete message
    Message msgSend;
    msgSend.header = buffer->header;
    msgSend.body = buffer->body;

    uint16_t len;
    size_t res;

    switch (buffer->header.type) {
        case 1: //LoginResponse
            len =  sizeof(msgSend.header) +
                   sizeof(msgSend.body.lre.magic) +
                   sizeof(msgSend.body.lre.code) +
                   strlen(msgSend.body.lre.sName);
            msgSend.body.lre.magic = htonl(msgSend.body.lre.magic);
            msgSend.header.len = htons(len - sizeof(msgSend.header));
            res = send(fd, &msgSend, len, MSG_NOSIGNAL);
            if (res != len) {
                // errnoPrint("Error: sending LoginResponse failed");
            }
            break;
        case 3: // Server2Client
            len = sizeof(msgSend.header) +
                        sizeof(msgSend.body.s2c.timestamp) +
                        sizeof(msgSend.body.s2c.originalSender) +
                        strlen(msgSend.body.s2c.text);
            msgSend.body.s2c.timestamp = hton64u(msgSend.body.s2c.timestamp);
            msgSend.header.len = htons(len - sizeof(msgSend.header));
            res = send(fd, &msgSend, len, MSG_NOSIGNAL);
            if (res != len) {
                // errnoPrint("Error: sending Server2Client failed");
            }
            break;
        case 4: //UserAdd
            len =  sizeof(msgSend.header) +
                        sizeof(msgSend.body.uad.timestamp) +
                        strlen(msgSend.body.uad.name);
            msgSend.body.uad.timestamp = hton64u(msgSend.body.uad.timestamp);
            msgSend.header.len = htons(len - sizeof(msgSend.header));
            res = send(fd, &msgSend, len, MSG_NOSIGNAL);
            if (res != len) {
                // errnoPrint("Error: sending UserAdd failed");
            }
            break;
        case 5: //UserRemove
            len = sizeof(msgSend.header) +
                  sizeof(msgSend.body.urm.timestamp) +
                  sizeof (msgSend.body.urm.code) +
                  strlen(msgSend.body.urm.name);
            msgSend.body.urm.timestamp = hton64u(msgSend.body.urm.timestamp);
            msgSend.header.len = htons(len - sizeof(msgSend.header));
            res = send(fd, &msgSend, len, MSG_NOSIGNAL);
            if (res != len) {
                // errnoPrint("Error: sending UserRemoved failed");
            }
            break;
        default:
            infoPrint("Wrong Type-Number");
            break;
    }
    return -1;
}

void sendLoginResponse(int fd, char sName[], int code){
    Message loginRequest;
    loginRequest.body.lre.magic = 0xc001c001;
    loginRequest.body.lre.code = code;
    strncpy(loginRequest.body.lre.sName, sName, 31);
    loginRequest.body.lre.sName[31] = '\0';
    loginRequest.header.type = 1;
    networkSend(fd, &loginRequest);
}

void sendUserAdded(int fd, char name[]){
    Message userAdded;
    userAdded.header.type = 4;
    strncpy(userAdded.body.uad.name, name, 31);
    userAdded.body.uad.name[31] = '\0';
    time_t seconds;
    time(&seconds);
    userAdded.body.uad.timestamp = seconds;
    networkSend(fd, &userAdded);
}

void sendUserAddedNew(int fd, char name[]){
    Message userAdded;
    userAdded.header.type = 4;
    strncpy(userAdded.body.uad.name, name, 31);
    userAdded.body.uad.name[31] = '\0';
    userAdded.body.uad.timestamp = 0;
    networkSend(fd, &userAdded);
}

int sendServer2Client(int fd, char name[], char text[]){
    Message server2Client;
    server2Client.body.s2c.timestamp = time(NULL);
    strncpy(server2Client.body.s2c.originalSender, name, 31);
    server2Client.body.s2c.originalSender[31] = '\0';
    strncpy(server2Client.body.s2c.text, text, 511);
    server2Client.body.s2c.text[511] = '\0';
    server2Client.header.type = 3;
    tm.tv_sec = time(NULL) + 1;
    tm.tv_nsec = 0;
    if (fd == 0){
        if (mq_timedsend(getMessageQueue(), (char *)  &server2Client, sizeof(server2Client), 0, &tm) == -1){
            errnoPrint("Couldn't Send Message into the Queue");
            return 0;
        }
    } else {
        networkSend(fd, &server2Client);
    }
    return 1;
}

void sendUserRemoved(int fd, char name[], int code) {
    Message userRemoved;
    userRemoved.body.urm.timestamp = time(NULL);
    userRemoved.body.urm.code = code;
    strncpy(userRemoved.body.urm.name, name, 31);
    userRemoved.body.urm.name[31] = '\0';
    userRemoved.header.type = 5;
    networkSend(fd, &userRemoved);
}