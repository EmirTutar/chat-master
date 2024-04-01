#include <errno.h>
#include <pthread.h>
#include <sys/socket.h>
#include <malloc.h>
#include "connectionhandler.h"
#include "util.h"
#include "clientthread.h"
#include "network.h"


static int createPassiveSocket(in_port_t port)
{
    int fd = -1;
    //TODO: socket()
        const int backlog = 4;
        fd = socket(AF_INET, SOCK_STREAM, 0); // IPv4, TCP, Auto protokoll
        if (fd == -1)
        {
            perror("Could not create socket");
            return fd;
        }
        const int on = 1;
        setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &on, sizeof (on));
        struct sockaddr_in addr;
        memset(&addr, 0, sizeof(addr));		    	//any fields not set below must be zero
        addr.sin_family = AF_INET;		        	//use IPv4
        addr.sin_addr.s_addr = htonl(INADDR_ANY);	//accept connections from all addresses and networks
        addr.sin_port = htons(port);			    //set the port to use
    //TODO: bind() to port
        if (bind(fd, (const struct sockaddr *)&addr, sizeof (addr)) == -1)
        {
            perror("Could not bind socket to address");
            close(fd);
            return -1;
        }
    //TODO: listen()
        if(listen(fd, backlog) == -1)
        {
            perror("Could not mark socket to be accepting connections");
            close(fd);
            return -1;
        }
    errno = ENOSYS;
    return fd;
}

int connectionHandler(in_port_t port)
{
    const int fd = createPassiveSocket(port);
    if(fd == -1)
    {
        errnoPrint("Unable to create server socket");
        return -1;
    }

    for(;;)
    {
        //TODO: accept() incoming connection
        const int active_sock = accept(fd, NULL, NULL);
            if(active_sock == -1)
            {
                perror("Could not accept client connection");
                continue;
            }
        //TODO: add connection to user list and start client thread
        pthread_t clientthreadThre;
        lock();
        User * newUser = userAdd(clientthreadThre, active_sock);
        pthread_create(&clientthreadThre, NULL, clientthread, newUser);
        unlock();
    }
    return 0;	//never reached
}