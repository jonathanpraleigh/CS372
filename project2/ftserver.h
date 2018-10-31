#ifndef FTSERVER_H
#define FTSERVER_H

#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>

//Globals.
int status;
int socketDescriptor;
int new_fd;

char clientaddr[16];
char hostaddr[16];

socklen_t addr_size;

struct addrinfo hints;
struct addrinfo *servinfo;
struct sockaddr_storage their_addr;

//Functions
int startup(char*);
void handleRequest(int, int, char*);
int createDataSocket(char*, int);
int sendDirectory(int);
int sendFile(int, int, char*);
int validateFileName(char*);
int sendError(int, char*);


#endif
