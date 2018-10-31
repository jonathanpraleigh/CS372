#include "ftserver.h"
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>


int main(int argc, char *argv[])
{
	char dpbuf[4096];

	//takes arg for port #. 
	if (argc != 2)
	{
		printf("Invalid parameters.\n");
		exit(0);
	}

	socketDescriptor = startup(argv[1]);

	for (;;)
	{
		listen(socketDescriptor, 5);

		addr_size = sizeof their_addr;
		new_fd = accept(socketDescriptor, (struct sockaddr *)&their_addr, &addr_size);

		recv(new_fd, dpbuf, 100, 0);

		int dsnumber = atoi(dpbuf);

		getnameinfo((struct sockaddr *)&their_addr, sizeof their_addr, clientaddr, sizeof clientaddr, hostaddr, sizeof hostaddr, 0 );

		int newds = createDataSocket(clientaddr, dsnumber);

		handleRequest(new_fd, newds, clientaddr);
	}

}

/**********************************************************************************
* Name:			startup
* Inputs:		server port
* Outputs:		Socket Descriptor
* Description:	Sets up server for listening.
**********************************************************************************/
int startup(char* sp)
{
	printf("starting up\n"); // DEBUGDEBUGDEBUGDEBUGDEBUGDEBUGDEBUGDEBUGDEBUGDEBUGDEBUGDEBUGDEBUGDEBUGDEBUGDEBUGDEBUGDEBUGDEBUGDEBUGDEBUGDEBUG
	int sock; 

	//from Beej's Guide to Network Programming, section 5.1 - 5.3

	//memset(&hints, 0, sizeof hints);
	hints.ai_family = AF_UNSPEC;     
	hints.ai_socktype = SOCK_STREAM; 
	hints.ai_flags = AI_PASSIVE;     

	getaddrinfo(NULL, sp, &hints, &servinfo);

	if ((status = getaddrinfo(NULL, sp, &hints, &servinfo)) != 0) 
	{
		fprintf(stderr, "getaddrinfo error: %s\n", gai_strerror(status));
		exit(1);
	}

	sock = socket(servinfo->ai_family, servinfo->ai_socktype, servinfo->ai_protocol);

	if ((status = bind(sock, servinfo->ai_addr, servinfo->ai_addrlen)) != 0)
	{
		fprintf(stderr, "Bind error\n");
		exit(1);
	}

	//address info no longer needed, so freeing memory here.
	freeaddrinfo(servinfo);

	printf("done starting up\n"); // DEBUGDEBUGDEBUGDEBUGDEBUGDEBUGDEBUGDEBUGDEBUGDEBUGDEBUGDEBUGDEBUGDEBUGDEBUGDEBUGDEBUGDEBUGDEBUGDEBUGDEBUGDEBUG
	return sock;
}

/**********************************************************************************
* Name:			handleRequest
* Inputs:		socket # and client's address (to be used to set up data socket).
* Outputs:		None.
* Description:	Takes connection and receives transmitted data. Based on data, calls
*				other functions.
**********************************************************************************/
void handleRequest(int s, int ds, char* clientAddress)
{
	char buf[4096];
	char filename[256];
	char portname[14];
	char invalidCommand[] = "ERROR: COMMAND NOT FOUND!\n";
	char invalidFile[] = "ERROR: FILE NOT FOUND!\n";
	int i;
	int j;
	//int ds;
	int portnumber;

	printf("handling request\n"); // DEBUGDEBUGDEBUGDEBUGDEBUGDEBUGDEBUGDEBUGDEBUGDEBUGDEBUGDEBUGDEBUGDEBUGDEBUGDEBUGDEBUGDEBUGDEBUGDEBUGDEBUGDEBUG
	
	memset(buf, '\0', sizeof buf);
	memset(filename, '\0', sizeof filename);

	printf("receiving message\n"); // DEBUGDEBUGDEBUGDEBUGDEBUGDEBUGDEBUGDEBUGDEBUGDEBUGDEBUGDEBUGDEBUGDEBUGDEBUGDEBUGDEBUGDEBUGDEBUGDEBUGDEBUGDEBUG

	recv(s, buf, 100, 0);

	printf("done receiving message\n"); // DEBUGDEBUGDEBUGDEBUGDEBUGDEBUGDEBUGDEBUGDEBUGDEBUGDEBUGDEBUGDEBUGDEBUGDEBUGDEBUGDEBUGDEBUGDEBUGDEBUGDEBUGDEBUG

	if (strncmp(buf, "-l", 2) == 0)
	{
		printf("got l request\n"); // DEBUGDEBUGDEBUGDEBUGDEBUGDEBUGDEBUGDEBUGDEBUGDEBUGDEBUGDEBUGDEBUGDEBUGDEBUGDEBUGDEBUGDEBUGDEBUGDEBUGDEBUGDEBUG
		for (i = 0; i < sizeof portname && buf[i + 3] != '\n'; i++)
		{
			portname[i] = buf[3 + i];
		}
		portnumber = atoi(portname);
//		ds = createDataSocket(clientAddress, portnumber);
		sendDirectory(ds);
		close(ds);
	}
	else if (strncmp(buf, "-g", 2) == 0)
	{
		printf("got g request\n"); // DEBUGDEBUGDEBUGDEBUGDEBUGDEBUGDEBUGDEBUGDEBUGDEBUGDEBUGDEBUGDEBUGDEBUGDEBUGDEBUGDEBUGDEBUGDEBUGDEBUGDEBUGDEBUG
		for (i = 0; i < sizeof filename && buf[i + 3] != ','; i++)
		{
			filename[i] = buf[3 + i];
		}

		if(validateFileName(filename) == -1)
		{
			sendError(s, invalidFile);
		}
		else
		{
			i++;
			for (j=0; j < sizeof portname && buf[i+3] != '\n'; j++)
			{
				portname[j] = buf[i+3];
				i++;
			}

			portnumber = atoi(portname);
//			ds = createDataSocket(clientAddress, portnumber);
			sendFile(s, ds, filename);
			close(ds);
		}
	}
	else
	{
		printf("command is %s", buf); // DEBUGDEBUGDEBUGDEBUGDEBUGDEBUGDEBUGDEBUGDEBUGDEBUGDEBUGDEBUGDEBUGDEBUGDEBUGDEBUGDEBUGDEBUGDEBUGDEBUGDEBUGDEBUG
		printf("invalid request\n"); // DEBUGDEBUGDEBUGDEBUGDEBUGDEBUGDEBUGDEBUGDEBUGDEBUGDEBUGDEBUGDEBUGDEBUGDEBUGDEBUGDEBUGDEBUGDEBUGDEBUGDEBUGDEBUG
		sendError(s, invalidCommand);
	}
}

/**********************************************************************************
* Name:			validateFileName
* Inputs:		string of file name.
* Outputs:		success code.
* Description:	validates provided file name.
**********************************************************************************/
int validateFileName(char* fn)
{
	DIR *d;
	struct dirent *dir;
	d = opendir(".");
	int validonzero = -1;

	if (d)
	{
		while ((dir = readdir(d)) != NULL)
		{
			if(strcmp(fn, dir->d_name) == 0)
			{
				validonzero = 0;
			}
		}
		closedir(d);
	}

	return validonzero;
}

/**********************************************************************************
 * Name:		sendError
 * Inputs:		socket #, error message string.
 * Outputs:		none.
 * Description:	Sends provided message to provided socket.
 **********************************************************************************/
int sendError(int s, char* m)
{
	printf("sending error\n"); // DEBUGDEBUGDEBUGDEBUGDEBUGDEBUGDEBUGDEBUGDEBUGDEBUGDEBUGDEBUGDEBUGDEBUGDEBUGDEBUGDEBUGDEBUGDEBUGDEBUGDEBUGDEBUG
	int mssgLength, bytesSent;

	mssgLength = strlen(m);
	
	send(s, m, mssgLength, 0);

	return 0;
}

/**********************************************************************************
* Name:			sendFile
* Inputs:		dataport s, filename string fn
* Outputs:		none.
* Description:	Sends file to provided data port.
**********************************************************************************/
int sendFile(int s, int ds, char * fn)
{
	char fileBuffer[4096];
	char fileLengthMssg[10];
	int fileLength;
	int n = 0;
	int totalSent = 0;
	int totalRead;
	int fd;
	struct stat fileSizeBuffer;

	fd = open(fn, O_RDONLY);

	fstat(fd, &fileSizeBuffer);

	fileLength = fileSizeBuffer.st_size;

	sprintf(fileLengthMssg, "%d", fileLength);

	send(s, fileLengthMssg, sizeof fileLengthMssg, 0); // sends single message with length of file.

	//The following is taken from Beej's Guide to Networking, Section 7.3
	// and from the following Stack Overflow page:
	// http://stackoverflow.com/questions/2014033/send-and-receive-a-file-in-socket-programming-in-linux-with-c-c-gcc-g

	while(1)
	{
		totalRead = read(fd, fileBuffer, sizeof fileBuffer - 1);
		if (totalRead == 0) break;

		void *bufferPointer = fileBuffer;

		while(totalRead > 0)
		{
			totalSent = send(ds, bufferPointer, sizeof fileBuffer, 0);
			totalRead -= totalSent;
			bufferPointer += totalSent;
		}
		
	}

	return 0;
}

/**********************************************************************************
* Name:			sendDirectory
* Inputs:		socket int s
* Outputs:		none.
* Description:	Sends directory to provided data port.
**********************************************************************************/
int sendDirectory(int s)
{
	printf("sending directory\n"); // DEBUGDEBUGDEBUGDEBUGDEBUGDEBUGDEBUGDEBUGDEBUGDEBUGDEBUGDEBUGDEBUGDEBUGDEBUGDEBUGDEBUGDEBUGDEBUGDEBUGDEBUGDEBUG
	DIR *d;
	struct dirent *dir;
	d = opendir(".");
	char fileList[4096];

	memset(fileList, '\0', 4095);

	if (d)
	{
		while ((dir = readdir(d)) != NULL)
		{
			strcat(fileList, dir->d_name);
			strcat(fileList, "\n");
		}
		closedir(d);

		send(s, fileList, sizeof fileList, 0);
	}

	return 0;
}

/**********************************************************************************
* Name:			createDataSocket
* Inputs:		address string, port number.
* Outputs:		returns socket integer.
* Description:	Creates data socket with provided address and port #.
**********************************************************************************/

int createDataSocket(char* addr, int portNumber)
{
	printf("creating data socket\n"); // DEBUGDEBUGDEBUGDEBUGDEBUGDEBUGDEBUGDEBUGDEBUGDEBUGDEBUGDEBUGDEBUGDEBUGDEBUGDEBUGDEBUGDEBUGDEBUGDEBUGDEBUGDEBUG
	struct addrinfo *x;
	struct addrinfo hints;
	int ds;
	char pn[10];

	memset(&hints, 0, sizeof hints);

	printf("sprintf\n"); // DEBUGDEBUGDEBUGDEBUGDEBUGDEBUGDEBUGDEBUGDEBUGDEBUGDEBUGDEBUGDEBUGDEBUGDEBUGDEBUGDEBUGDEBUGDEBUGDEBUGDEBUGDEBUG
	sprintf(pn, "%d", portNumber);
	printf("aaa%saaa\n", pn);

	//Setup for addressinfo struct from Beej's Guide to Network Programming
	//https://beej.us/guide/bgnet/

	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;

	printf("set up hints\n"); // DEBUGDEBUGDEBUGDEBUGDEBUGDEBUGDEBUGDEBUGDEBUGDEBUGDEBUGDEBUGDEBUGDEBUGDEBUGDEBUGDEBUGDEBUGDEBUGDEBUGDEBUGDEBUG
	//x is the addrinfo struct and is providing data to find the socket information 
	//and handle the connection.
	printf("getting address info\n"); // DEBUGDEBUGDEBUGDEBUGDEBUGDEBUGDEBUGDEBUGDEBUGDEBUGDEBUGDEBUGDEBUGDEBUGDEBUGDEBUGDEBUGDEBUGDEBUGDEBUGDEBUGDEBUG
	getaddrinfo("flip3.engr.oregonstate.edu", pn, &hints, &x);
	printf("getting new socket\n"); // DEBUGDEBUGDEBUGDEBUGDEBUGDEBUGDEBUGDEBUGDEBUGDEBUGDEBUGDEBUGDEBUGDEBUGDEBUGDEBUGDEBUGDEBUGDEBUGDEBUGDEBUGDEBUG
	ds = socket(x->ai_family, x->ai_socktype, x->ai_protocol); //s is sockfd
	printf("connecting\n"); // DEBUGDEBUGDEBUGDEBUGDEBUGDEBUGDEBUGDEBUGDEBUGDEBUGDEBUGDEBUGDEBUGDEBUGDEBUGDEBUGDEBUGDEBUGDEBUGDEBUGDEBUGDEBUG
	connect(ds, x->ai_addr, x->ai_addrlen);

	return ds;
}