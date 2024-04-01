#include <pthread.h>
#include <mqueue.h>
#include <sys/socket.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <semaphore.h>
#include <malloc.h>
#include "broadcastagent.h"
#include "util.h"
#include "network.h"
#include "user.h"

static mqd_t messageQueue;
const char mqName[11] = "/chatServer";
static pthread_t threadId;
static struct mq_attr attr;

sem_t sem;

static void *broadcastAgent(void *arg)
{
	//TODO: Implement thread function for the broadcast agent here!
    Message msg;
    while (1) {
        if (mq_receive(messageQueue,(char *)  &msg, sizeof(msg), NULL) == -1){
            errnoPrint("Error: Couldn't receive message from queue");
        }
        if (sem_wait(&sem) != 0) {
            errnoPrint("Error: sem_wait");
        }
        infoPrint("Msg: %s", msg.body.s2c.text);
        lock();
        for (User *it = getFront(); it != NULL; it = it->next) {
            if (it->active){
                networkSend(it->sock, &msg);
            }
        }
        unlock();
        sem_post(&sem);
    }
    return arg;
}

int broadcastAgentInit(void)
{
	//TODO: create message queue
    attr.mq_flags = 0;
    attr.mq_maxmsg = 10;
    attr.mq_msgsize = sizeof(Message);
    attr.mq_curmsgs = 0;
    messageQueue = mq_open(mqName, O_RDWR | O_CREAT | O_EXCL, 0660, &attr); //0660 unix-File-Berechtigung
    if (mq_unlink(mqName) == -1){
        errnoPrint("Error: Couldn't open MessageQueue");
    }
    //TODO: start thread
    pthread_create(&threadId, NULL, broadcastAgent, NULL);

    if(sem_init(&sem, 0, 1U) != 0){
        errnoPrint("Error: Couldn't init Semaphore");
    }
	return -1;
}

void broadcastAgentCleanup(void)
{
    //TODO: destroy message queue
    if (mq_close(messageQueue) == -1){
        errnoPrint("Error: Couldn't close MessageQueue");
    }
    if (mq_unlink(mqName) == -1){
        errnoPrint("Error: Couldn't unlink MessageQueue");
    }
    sem_destroy(&sem);
    //TODO: stop thread
    pthread_exit(NULL);
 }

int getMessageQueue(){
    return messageQueue;
}