#include <string.h>
#include <stdio.h>

#include <sys/socket.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <stdlib.h>
#include <unistd.h>

#include "server.h"

int main(int argc, char *argv[])
{
	if (argc != 1)
	{
		printf("Usage: %s\n", argv[0]);
		exit(1);
	}

	int servSock, clntSock;
	unsigned int clntLen;
	unsigned short port = WELLKNOWNPORT;
	struct sockaddr_in servAddr, clntAddr;

	/* Create the socket */
	if ((servSock = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0)
		DieWithError("socket() failed");
	/* Configure server address */
	memset(&servAddr, 0, sizeof(servAddr));
	servAddr.sin_family      = AF_INET;
	servAddr.sin_addr.s_addr = htonl(INADDR_ANY);
	servAddr.sin_port        = htons(port);
	/* Bind to the address */
	if (bind(servSock, (struct sockaddr*) &servAddr, sizeof(servAddr)))
		DieWithError("bind() failed");
	/* Listen to the socket */
	if (listen(servSock, MAXPENDING) < 0)
		DieWithError("listen() failed");

	while(1)
	{
		clntLen = sizeof(clntAddr);
		/* Accept the client */
		if ((clntSock = accept(servSock, (struct sockaddr *) &clntAddr, &clntLen)) < 0)
			DieWithError("accept() failed");
		printf("Handling client %s\n", inet_ntoa(clntAddr.sin_addr));


	}
}


void DieWithError(char *errorMsg)
{
	perror(errorMsg);
	exit(1);
}


