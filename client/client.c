#include <string.h>
#include <stdio.h>

#include <sys/socket.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <stdlib.h>
#include <unistd.h>

#include "client.h"

#define END_OF_PHOTO 0xAC

int main(int argc, char** argv) {

	if(argc != 4) {

		printf("Usage: ./client <servermachine> <id> <num_photos>\n");
		exit(1);

	}

	//Pointer to socket structure that ends up filled in by gethostbyname
  	struct hostent *servHost;
  	
  	unsigned short port = 5280;
  	servHost = gethostbyname(argv[1]);

  	int sock = physical_Establish(servHost, port);



}

int physical_Establish(struct hostent* host, unsigned short port) {

	struct sockaddr_in serverAddress;

	 //Reset the struct
  	memset(&serverAddress, 0, sizeof(serverAddress));
  	serverAddress.sin_family = AF_INET;

  	//Copy the address from the gethostbyname struct into struct
  	memcpy(&serverAddress.sin_addr.s_addr, host->h_addr_list[0], host->h_length);

  	//Convert the provided port to network byte order and assign to struct
  	serverAddress.sin_port = htons(port);

	int sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

	if(connect(sock, (struct sockaddr *) &serverAddress, sizeof(serverAddress)) < 0) {
    	printf("connect() failed\n");
    	exit(1);
  	}

  	return sock;

}