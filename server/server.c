#include <string.h>
#include <stdio.h>

#include <sys/socket.h>
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

int main(int argc, char *argv[])
{
	if (argc != 1)
	{
		printf("Usage: %s\n", argv[0]);
		exit(1);
	}

	int servSock, clntSock, bytesRcvd, TotalBytesRcvd;
	unsigned int clntLen;
	unsigned short port = WELLKNOWNPORT;
	struct sockaddr_in servAddr, clntAddr;
	char fileName[129];
	FILE *file;
	int clientID, numPhotos;
	unsigned short num;
	char *error_handling_result;
	char *recvBuf = malloc(1000 * sizeof(unsigned char));
	int i;
	Frame *frame = malloc(sizeof(Frame));
	Packet *packet = malloc(sizeof(Packet));
	Frame *ack = malloc(sizeof(Frame));
	Frame *window = malloc(2 * sizeof(Frame));
	int windowCount = 0;
	int endOfPhotoFlag = 0;
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

		/* Receive the client ID and the number of photos from the client */
		if (recv(clntSock, &clientID, sizeof(clientID), 0) < 0)
			DieWithError("recv() failed to get the clientID");
		printf("Client ID: %d\n", clientID);
		
		if(recv(clntSock, &numPhotos, sizeof(numPhotos), 0) < 0)
			DieWithError("recv() failed to get num photos");
		printf("Number of photos: %d\n", numPhotos);


		int currentFileIndex = 0;
		sprintf(fileName, "photo%d%d.jpg", clientID, i);
		printf("Opening file for writing\n");
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

				printf("Frame is correct!\n");

				//Constructing the ACK
				Frame ack;
				ack.frameType = FRAMETYPE_ACK;

	      		//Sending ACK
				if(send(clntSock, &ack, sizeof(Frame), 0) < 0)
	    			DieWithError("send() error");

	    		fwrite(incoming->payload, incoming->payloadLen, 1, file);
	    		continue;

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

    		for(int i = 0; i < currentPacket; i++) {
    				fwrite(packetArray[currentPacket].data, PACKET_SIZE, 1, file);
    		}
    		
    		fclose(file);


	}
		
	
	
		
		/*
		if (recv(clntSock, &numPhotos, sizeof(numPhotos), 0) < 0)
			DieWithError("recv() failed");
		printf("Number of Photos: %d\n", numPhotos);


		for (i = 0; i < numPhotos; i++)
		{
			sprintf(fileName, "photo%d%d.jpg", clientID, i);
			printf("Opening file for writing\n");
			if((file = fopen(fileName, "wb")) == NULL) 
		    {
		      fprintf(stderr, "%s couldn't be found!\n", fileName);
		      exit(1);
		    }

			while(!endOfPhotoFlag)
			{

		    //Receive frame data from client 
//				bytesRcvd = 0;
//				TotalBytesRcvd = 0;
				
				printf("Receving frame\n");
//				while (TotalBytesRcvd < frameLen)
//				{
					if ((bytesRcvd = recv(clntSock, recvBuf, frameLen, 0)) <= 0)
						DieWithError("recv() failed");
					TotalBytesRcvd += bytesRcvd;
					printf("bytesRcvd: %d\n", bytesRcvd);
//				}
				printf("Frame received\n");

				//Make one frame at a time
				make_Frame(&window[windowCount], recvBuf, 136);

				printf("Sequence number: %x\n", window[windowCount].seqNum[0] | window[windowCount].seqNum[1]);
				error_handling_result = error_Handling(window[windowCount], TotalBytesRcvd);

				printf("Original error detection bytes: %s\n", window[windowCount].errorDetect);
				printf("New error detection bytes Generated: %s\n", error_handling_result);

				//Checking the sequence number and the error detection bytes
				//if (num == seq_num && atoi(frame->errorDetect) == atoi(error_handling_result))
				//{
					printf("Frame is correct!\n");
					//Constructing the ACK
					ack->frameType = FRAMETYPE_ACK;
					memcpy(ack->seqNum, window[windowCount].seqNum, 2);
					memcpy(ack->errorDetect, window[windowCount].seqNum, 2);

					printf("ack->seqNum = %x %x\n", ack->seqNum[0], ack->seqNum[1]);
		      printf("ack->errorDetect = %x %x\n", ack->errorDetect[0], ack->errorDetect[1]);
		      
		      //Sending ACK
					if(send(clntSock, (unsigned char *)ack, sizeof(Frame), 0) < 0)
		    		DieWithError("send() error");

		    	printf("Sending ACK back successfully!\n");
		    	seq_num++;
		    //}

		    printf("Converting frames to packet\n");

		    // Convert frames to packet
		    if (window[windowCount].endOfPacket == END_OF_PACKET_YES)
		    	packetLen = make_Packet(packet, window, windowCount);

		    printf("Printing packet payload to file\n");

		  	fwrite(packet->data, 1, packetLen, file);

		    if (packet->endOfPhoto == END_OF_PHOTO_YES)
		    {
		    	endOfPhotoFlag = 1;
		    	break;
		    }
		  

		  	windowCount = !windowCount;
		  }

		  fclose(file);

  	}
  	*/

	
	

}


void DieWithError(char *errorMsg)
{
	perror(errorMsg);
	exit(1);
}




int make_Frame(Frame *frame, char *buffer, int bufSize)
{

	memcpy(frame, buffer, bufSize);
	return 1;
		
		/*
		frame->frameType = buffer[0];
		frame->seqNum[0] = buffer[1];
		frame->seqNum[1] = buffer[2];

		int i;
		//frame->payload = malloc(bufSize - 5);
		for(i = 3; i < bufSize - 3; i++)
			frame->payload[i - 3] = buffer[i];

		frame->endOfPacket = buffer[i];
		frame->errorDetect[0] = buffer[i + 1];
		frame->errorDetect[1] = buffer[i + 2];

		return 1;
		*/
}


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





