/*The Program:
Design and implement a simple chat system that works for one pair of users, i.e., create two programs: a chat server and a chat client.
The final version of your programs must accomplish the following tasks:
1. chatclient starts on host B, specifying host A’s hostname and port number on the command line.
2. chatclient on host B gets the user’s “handle” by initial query (a one-word name, up to 10 characters). chatclient will display this handle
as a prompt on host B, and will prepend it to all messages sent to host A. e.g., “ SteveO> Hi!!”
3. chatclient on host B sends an initial message to chatserve on host A : PORTNUM. This causes a connection to be established between Host A
and Host B. Host A and host B are now peers, and may alternate sending and receiving messages. Responses from host A should have host A’s
“handle” prepended.
4. Host B responds to Host A, or closes the connection with the command “\quit”

//starting stuff.

//asks for handle

//displays prompt

//on first message input from user, chatclient sends message to chatserve on host A. 
//stays connected as peers.

//sends message or upon typing "\quit" quits. */

#include "chatclient.hpp"
#include <iostream>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>

int main(int argc, char *argv[])
{
	userData user;

	if (argc != 3)
	{
		std::cout << "\nIncorrect number of arguments.\n";
		exit;
	}

	//argv[1] is host name
	//argv[2] is port number.

	handlePrompt(&user);
	sockfd = connectToHost(servinfo, argv[1], argv[2]);
	if (sockfd == -1) 
	{
		sockfd = connectToHost(servinfo, argv[1], argv[2]);
		if (sockfd == -1)
		{
			std::cout << "\nConnection failed. Terminating program.\n";
			exit;
		}
	}


	do
	{
		std::cout << user.username << "> ";
		std::getline(std::cin, input);
		send(sockfd, &input, input.length(), 0);
		response = getResponse();
		std::cout << "\n" << hostAName << "> " << response << "\n";
	} while (input != "\quit");

	closeConnection(sockfd);
	freeaddrinfo(servinfo);

}

int handlePrompt(userData x)
{
	std::cout << "Please enter your handle:";
	std::getline(std::cin, x.username);
	return 0;
}

//connectToHost starts connection and returns socket data. 

int connectToHost(addrinfo* x, char * arg1, char * arg2)
{
	int s;

	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_flags = AI_PASSIVE;

	getaddrinfo(arg1, arg2, &hints, &x);
	s = socket(x->ai_family, x->ai_socktype, x->ai_protocol); //s is sockfd
	connect(s, x->ai_addr, x->ai_addrlen);

	return s;
}

std::string getResponse()
{
	std::string response = NULL;
	bool responseCompleted = false;

	do
	{
		//wait for complete message to arrive
		//if complete message arrives, responseCompleted == true.
	} while (responseCompleted == false);
	return response;
}

int closeConnection(int s)
{
	close(s);
	return 0;
}
