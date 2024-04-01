#ifndef BROADCASTAGENT_H
#define BROADCASTAGENT_H

#include <semaphore.h>
#include <stdbool.h>

int broadcastAgentInit(void);
void broadcastAgentCleanup(void);

int getMessageQueue();

#endif
