#include <string.h>
#include <stdio.h>

#include <sys/socket.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <stdlib.h>
#include <unistd.h>

#include "client.h"

#define END_OF_PHOTO_YES 0xAC
#define END_OF_PHOTO_NO 0xAD

#define END_OF_PACKET_YES 0xAE
#define END_OF_PACKET_NO 0xAF

int main(int argc, char** argv) {

	if(argc != 4) {

		printf("Usage: ./client <servermachine> <id> <num_photos>\n");
		exit(1);

	}

	int clientID = atoi(argv[2]);
	int num_photos = atoi(argv[3]);

	//Pointer to socket structure that ends up filled in by gethostbyname
  	struct hostent *servHost;
  	
  	unsigned short port = WELLKNOWNPORT;
  	servHost = gethostbyname(argv[1]);

  	//int sock = physical_Establish(servHost, port);

  	//Packet formation
  	for(int i = 0; i < num_photos; i++) {

  		char filename[255];
  		sprintf(filename, "photo%d%d.jpg", clientID, i);


  		FILE *file;
  		file = fopen(filename, "r");
  		if(file == NULL) {
  			fprintf(stderr, "%s couldn't be found!\n", filename);
  			exit(1);
  		}


  	}



}


void DieWithError(char *errorMsg)
{
  perror(errorMsg);
  exit(1);
}


int physical_Establish(struct hostent* host, unsigned short port) {

	struct sockaddr_in serverAddress;
  int sock;

	  //Reset the struct
  	memset(&serverAddress, 0, sizeof(serverAddress));
  	serverAddress.sin_family = AF_INET;

  	//Copy the address from the gethostbyname struct into struct
  	memcpy(&serverAddress.sin_addr.s_addr, host->h_addr_list[0], host->h_length);

  	//Convert the provided port to network byte order and assign to struct
  	serverAddress.sin_port = htons(port);

	if ((sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0)
    DieWithError("sock() failed");

	if(connect(sock, (struct sockaddr *) &serverAddress, sizeof(serverAddress)) < 0) 
    DieWithError("connect() failed");

  	return sock;

}



