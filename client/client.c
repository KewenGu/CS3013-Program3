#include <string.h>

#include <sys/socket.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <stdlib.h>
#include <unistd.h>

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

}