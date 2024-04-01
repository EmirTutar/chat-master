#ifndef CHAT_PROTOCOL_H
#define CHAT_PROTOCOL_H

#include <stdint.h>

/* TODO: When implementing the fully-featured network protocol (including
 * login), replace this with message structures derived from the network
 * protocol (RFC) as found in the moodle course. */
enum { MSG_MAX = 1024 };

typedef struct __attribute__((packed)){
    uint8_t type;
    uint16_t len;
} Header;

typedef struct __attribute__((packed)){
    uint32_t magic;// = 0x0badf00d
    uint8_t version;
    char name[32];//>= 1 Byte; <=31 Byte
} LRQ;

typedef struct __attribute__((packed)){
    uint32_t magic;// = 3221340161
    uint8_t code;
    char sName[32];//>=1 Bytes, <=31 Bytes
} LRE;

typedef struct __attribute__((packed)){
    char text[512];//<= 512
} C2S;

typedef struct __attribute__((packed)){
    uint64_t timestamp;
    char originalSender[32];
    char text[512]; //<= 512 Bytes
} S2C;

typedef struct __attribute__((packed)){
    uint64_t timestamp;
    char name[32]; //>= 1 Byte; <= 31 Bytes
} UAD;

typedef struct __attribute__((packed)){
    uint64_t timestamp;
    uint8_t code;
    char name[32]; //>= 1 Byte; <= 31 Bytes
} URM;

typedef union Body {
    LRQ lrq;
    LRE lre;
    C2S c2s;
    S2C s2c;
    UAD uad;
    URM urm;
} Body;

typedef struct __attribute__((packed))
{
    Header header;
    Body body;
} Message;

int networkReceive(int fd, Message *buffer);
int networkSend(int fd, Message *buffer);

void sendLoginResponse(int fd, char sName[], int code);
void sendUserAdded(int fd, char name[]);
void sendUserAddedNew(int fd, char name[]);
int sendServer2Client(int fd, char name[], char text[]);
void sendUserRemoved(int fd, char name[], int code);

#endif