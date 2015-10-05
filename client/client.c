#include <string.h>
#include <stdio.h>

#include <sys/socket.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <stdlib.h>
#include <unistd.h>

#include "client.h"

#define PACKET_SIZE 256
#define FRAME_PAYLOAD_SIZE 130

#define END_OF_PHOTO_YES (char)4   // end of transmission
#define END_OF_PHOTO_NO (char)3    // end of text

#define END_OF_PACKET_YES (char)4  // end of transmission
#define END_OF_PACKET_NO (char)3   // end of text

int main(int argc, char** argv) {

	if(argc != 4) {

		printf("Usage: ./client <servermachine> <id> <num_photos>\n");
		exit(1);

	}

	int clientID = atoi(argv[2]);
	int num_photos = atoi(argv[3]);
  int i, j, k;

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
        totalBytesLoaded++;
        // Start filling a new packet after PACKET_SIZE bytes read
  			if(current_position == PACKET_SIZE) {
  				current_position = 0;
          packets[current_packet].endOfPhoto = END_OF_PHOTO_NO; // Indicate not end-of-photo
  				current_packet++;
  			}
  		}

      packets[current_packet].endOfPhoto = END_OF_PHOTO_YES; // Indicate end-of-photo
      current_packet++;

      


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

// Put the payload into the frame
void packet_to_frame(Packet *p, int seq_num)
{
  // First, initialize the frame
  int packetSize = sizeof(&p)
  int num_frames = (packetSize / FRAME_PAYLOAD_SIZE) + (packetSize % FRAME_PAYLOAD_SIZE > 0 ? 1 : 0);
  Frame *frames = (Frame *)malloc(num_frames * sizeof(Frame));
  int i, count = 0;
  int current_frame = 0;
  int bytesFramed = 0;
  int totalBytesFramed = 0;

  // Copy the packet into the frame payload
  while(totalBytesFramed < packetSize)
  {
    for(i = 0; i < FRAME_PAYLOAD_SIZE; i++)
    {
      frames[current_frame].payload[i] = (unsigned char *)(&p)[count];
      bytesFramed++;
      count++;
      // If reach the end-of-photo specifier
      if(frames[current_frame].payload[i] == END_OF_PHOTO_YES || frames[current_frame].payload[i] == END_OF_PHOTO_NO)
      {  
        frames[current_frame].endOfPacket = END_OF_PACKET_YES;
        break;
      }
    }
    frames[current_frame].seqNum = seq_num;
    frames[current_frame].endOfPacket = END_OF_PACKET_NO;
    frames[current_frame].errorDetect = error_handling(frames[current_frame].payload);
    current_frame++;
  }

  
  
}



