#include <string.h>
#include <stdio.h>

#include <sys/socket.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <stdlib.h>
#include <unistd.h>

#include "client.h"

#define PACKET_SIZE 256

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

  	int total_size = 0;
  	int sizes[num_photos];

  	//Packet memory allocation. First make space, then we can do the math for the packets easier
  	for(int i = 0; i < num_photos; i++) {

  		char filename[255];
  		sprintf(filename, "photo%d%d.jpg", clientID, i);

  		FILE *file;
  		file = fopen(filename, "rb");
  		if(file == NULL) {
  			fprintf(stderr, "%s couldn't be found!\n", filename);
  			exit(1);
  		}

  		fseek(file, 0, SEEK_END);
  		total_size += ftell(file);
  		sizes[i] = ftell(file);

  		fclose(file);
  	}

  	int num_packets = (total_size / PACKET_SIZE) + (total_size % PACKET_SIZE > 0 ? 1 : 0);

  	//Take in the jpg data
  	Packet *packets = (Packet *)malloc(num_packets * sizeof(Packet));

  	int current_packet = 0;
  	int current_position = 0;
  	int totalBytesLoaded = 0;

  	for(int i = 0; i < num_photos; i++) {

  		char filename[255];
  		sprintf(filename, "photo%d%d.jpg", clientID, i);
      // Open the JPEG file for reading
  		FILE *file;
  		file = fopen(filename, "rb");
  		if(file == NULL) {
  			fprintf(stderr, "%s couldn't be found!\n", filename);
  			exit(1);
  		}

  		int bytesLoaded = sizes[i];

  		while(bytesLoaded > 0) {
        // Read from JPEG file one byte at a time
  			fread(packets[current_packet].data+current_position, 1, 1, file);

  			current_position++;
  			bytesLoaded--;
        // Start filling a new packet after PACKET_SIZE bytes read
  			if(current_position == PACKET_SIZE) {
  				current_position = 0;
  				current_packet++;
  			}
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



