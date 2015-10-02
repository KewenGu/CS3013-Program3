#include <string.h>

#include <sys/socket.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <stdlib.h>
#include <unistd.h>

#define END_OF_PHOTO 0xAC

typedef struct frameData {

	char seqNum[2];
	char *payload; //somewhere between 1 and 130 bytes

	char errorDetect[2];
	char eop; //end-of-packet byte

} frame;

typedef struct frameACK {

	char seqNum[2];
	char errorDetect[2];

} frameACK;

int main(int argc, char** argv) {

	if(argc != 4) {

		printf("Usage: ./client <servermachine> <id> <num_photos>\n");
		exit(1);

	}

	//Pointer to socket structure that ends up filled in by gethostbyname
  	struct hostent *servHost;
  	struct sockaddr_in serverAddress;
  	unsigned short port = 5280;

  	servHost = gethostbyname(argv[1]);

  	//Reset the echoServAddr struct
  	memset(&serverAddress, 0, sizeof(serverAddress));
  	serverAddress.sin_family = AF_INET;

  	//Copy the address from the gethostbyname struct into echoServAddr
  	memcpy(&serverAddress.sin_addr.s_addr, servHost->h_addr_list[0], servHost->h_length);

  	//Convert the provided port to network byte order and assign to echoServAddr
  	serverAddress.sin_port = htons(port);

}