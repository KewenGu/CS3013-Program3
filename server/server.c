#include <string.h>
#include <stdio.h>

#include <sys/socket.h>
#include <sys/time.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <stdlib.h>
#include <unistd.h>

#include "server.h"

#define PACKET_SIZE sizeof(Packet)

#define END_OF_PHOTO_YES ((char)4)   // end of transmission
#define END_OF_PHOTO_NO ((char)3)    // end of text

#define END_OF_PACKET_YES ((char)4)  // end of transmission
#define END_OF_PACKET_NO ((char)3)   // end of text

unsigned short seq_num = 0;

//Author: Kewen Gu
int main(int argc, char *argv[])
{
	if (argc != 1)
	{
		printf("Usage: %s\n", argv[0]);
		exit(1);
	}

	//Socket variables
	int servSock, clntSock, bytesRcvd, TotalBytesRcvd;
	unsigned int clntLen;
	unsigned short port = WELLKNOWNPORT;
	struct sockaddr_in servAddr, clntAddr;

	//File processing variables
	char fileName[129];
	FILE *file;
	int clientID, numPhotos;
	unsigned short num;
	char *error_handling_result;

	//A buffer for recv()
	char *recvBuf = malloc(1000 * sizeof(unsigned char));
	int i;

	//Dynamically allocated structures so we have places to put data
	Frame *frame = malloc(sizeof(Frame));
	Packet *packet = malloc(sizeof(Packet));
	Frame *ack = malloc(sizeof(Frame));

	int frameLen = FRAME_PAYLOAD_SIZE + 6;
	int packetLen;

	/* Create the socket */
	if ((servSock = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0)
		DieWithError("socket() failed");

	/* Configure server address */
	memset(&servAddr, 0, sizeof(servAddr));
	servAddr.sin_family      = AF_INET;
	servAddr.sin_addr.s_addr = htonl(INADDR_ANY);
	servAddr.sin_port        = htons(port);

	//Bind to the address
	if (bind(servSock, (struct sockaddr*) &servAddr, sizeof(servAddr)))
		DieWithError("bind() failed");

	//Listen to the socket
	if (listen(servSock, MAXPENDING) < 0)
		DieWithError("listen() failed");

	struct timeval start_time;
	struct timeval end_time;



	while(1)
	{
		clntLen = sizeof(clntAddr);

		//Accept the client

		if ((clntSock = accept(servSock, (struct sockaddr *) &clntAddr, &clntLen)) < 0)
			DieWithError("accept() failed");
		printf("Handling client %s\n", inet_ntoa(clntAddr.sin_addr));

		gettimeofday(&start_time, NULL);

		/* Receive the client ID and the number of photos from the client */
		if (recv(clntSock, &clientID, sizeof(clientID), 0) < 0)
			DieWithError("recv() failed to get the clientID");
		printf("Data Link layer: Received client ID: %d\n", clientID);
		
		if(recv(clntSock, &numPhotos, sizeof(numPhotos), 0) < 0)
			DieWithError("recv() failed to get num photos");
		printf("Data Link layer: Received number of photos: %d\n", numPhotos);


		int currentFileIndex = 0;
		sprintf(fileName, "photo%d%d.jpg", clientID, i);
		printf("Application layer: Opening file for writing\n");
		if((file = fopen(fileName, "wb")) == NULL) 
	    {
	      fprintf(stderr, "%s couldn't be found!\n", fileName);
	      exit(1);
	    }

	    Packet* packetArray = malloc(1000 * sizeof(Packet));
	    int currentPacket = 0;
	    int posInCurrentPacket = 0;

	    int photosProcessed = 0;

		while(1) {
				Frame *incoming = malloc(sizeof(Frame));
				int bytesRcvd = recv(clntSock, incoming, sizeof(Frame), 0);
				
					if(bytesRcvd < 0) {
						gettimeofday(&end_time, NULL);

					    double timeDifference = (((end_time.tv_sec) * 1000000) + ((end_time.tv_usec))) - (((start_time.tv_sec) * 1000000) + ((start_time.tv_usec)));
					    printf("Client : transfer took %f seconds.\n", timeDifference/1000000);
					    exit(0);
					}

				printf("Data link layer: Received a frame.\n");

				//Constructing the ACK
				Frame ack;
				ack.frameType = FRAMETYPE_ACK;

				printf("Data link layer: ACKing frame.\n");

	      		//Sending ACK
				if(send(clntSock, &ack, sizeof(Frame), 0) < 0)
	    			DieWithError("send() error");

	    		fwrite(incoming->payload, FRAME_PAYLOAD_SIZE, 1, file);
	    		continue;

	    		//Fill packet with frame data
	    		int posInFrame = 0;
	    		while(posInFrame < FRAME_PAYLOAD_SIZE) {
	    			packetArray[currentPacket].data[posInCurrentPacket] = incoming->payload[posInFrame];

	    			posInCurrentPacket++;
	    			if(posInCurrentPacket == PACKET_SIZE || incoming->payload[posInFrame] == END_OF_PACKET_YES) {
	    				currentPacket++;
	    			}
	    			posInFrame++;
	    		}

    			
    			printf("Packets processed: %d\n", currentPacket);
    			
    			if(currentPacket == 43)break;
    		}

    		printf("Application layer: Writing image content to file\n");

    		//Write out all packets to the file system
    		int i;
    		for(i = 0; i < currentPacket; i++) {
    				fwrite(packetArray[currentPacket].data, PACKET_SIZE, 1, file);
    		}
    		
    		fclose(file);
 
	}

}

//Author: CS3516 course materials
void DieWithError(char *errorMsg)
{
	perror(errorMsg);
	exit(1);
}



//Author: Preston Mueller
int make_Frame(Frame *frame, char *buffer, int bufSize)
{

	memcpy(frame, buffer, bufSize);
	return 1;

}

//Author: Kewen Gu
int make_Packet(Packet *packet, Frame *frames, int index)
{
	int pos = 0;
	int i = index;
	int j;

	for (j = 0; j < sizeof(frames[i].payload); j++)
	{
		((unsigned char *)packet)[pos] = frames[i].payload[j];
		pos++;
	}

	if (frames[i].endOfPacket == END_OF_PACKET_YES) {}
		
	else if (frames[i].endOfPacket == END_OF_PACKET_NO)
	{
		i = !index;
		for (j = 0; j < sizeof(frames[i].payload); j++)
		{
			((unsigned char *)packet)[pos] = frames[i].payload[j];
			pos++;
		}
	}
	return pos;
}

//Author: Kewen Gu
char* error_Handling(Frame t, int size)
{
  int i;
  char *result = malloc(2 * sizeof(unsigned char));

  for (i = 0; i < (size - 2); i += 2) {

    result[0] = *(unsigned char *)&t ^ result[0];
  }

  for (i = 1; i < (size - 2); i += 2) {

    result[1] = *(unsigned char *)&t ^ result[1];
  }

  return result;
}
