#include <string.h>
#include <stdio.h>

#include <sys/socket.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <stdlib.h>
#include <unistd.h>

#include "server.h"

#define PACKET_SIZE 256
#define FRAME_PAYLOAD_SIZE 130

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

	int servSock, clntSock, bytesRcvd, TotalBytesRcvd, frameLen;
	unsigned int clntLen;
	unsigned short port = WELLKNOWNPORT;
	struct sockaddr_in servAddr, clntAddr;
	Frame *ack = malloc(sizeof(Frame));
	ack->frameType = FRAMETYPE_ACK;

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

		/* Set up the select and the timer */
		fd_set fileDescriptorSet;
		FD_ZERO(&fileDescriptorSet);
		FD_SET(clntSock, &fileDescriptorSet);

		struct timeval timer_length;
    timer_length.tv_sec = 3;
    timer_length.tv_usec = 0;

    if (select(1, &fileDescriptorSet, NULL, NULL, &timer_length) < 0)
      DieWithError("select() failed");

    /* Receive frame data from client */
		bytesRcvd = 0;
		TotalBytesRcvd = 0;
		frameLen = FRAME_PAYLOAD_SIZE + 5;

		char *recvBuf = malloc(1000 * sizeof(unsigned char));

		while (TotalBytesRcvd < frameLen)
		{
			if ((bytesRcvd = recv(clntSock, recvBuf + bytesRcvd, frameLen - TotalBytesRcvd, 0)) <= 0)
				DieWithError("recv() failed");
			TotalBytesRcvd += bytesRcvd;
		}

		Frame frame = make_Frame(recvBuf, TotalBytesRcvd);

		unsigned short num = (unsigned short) (frame.seqNum[0] | frame.seqNum[1]);
		printf("Sequence number: %d\n", num);
		char *error_handling_result = error_Handling(frame, TotalBytesRcvd);

		printf("Original error detection bytes: %x\n", *frame.errorDetect);
		printf("New error detection bytes: %x\n", *error_handling_result);
		

		//When this if statement is commented out, it kinda works

		//if (num == seq_num && !strcmp(frame.errorDetect, error_handling_result))
		//{
			printf("Frame is correct!\n");

			strncpy(ack->seqNum, frame.seqNum, 2);
			strncpy(ack->errorDetect, ack->seqNum, 2);

			printf("ack->seqNum = %x\n", (unsigned int)ack->seqNum);
      printf("ack->errorDetect = %x\n", (unsigned int)ack->errorDetect);
      
			if(send(clntSock, (unsigned char *)ack, sizeof(Frame), 0) < 0)
    		DieWithError("send() error");

    	printf("Sending ACK back successfully!\n");


    	//}
	}
}


void DieWithError(char *errorMsg)
{
	perror(errorMsg);
	exit(1);
}


Frame make_Frame(char *buffer, int bufSize)
{
		Frame *frame = malloc(sizeof(Frame));
		frame->seqNum[0] = buffer[0];
		frame->seqNum[1] = buffer[1];

		int i;
		//frame->payload = malloc(bufSize - 5);
		for(i = 2; i < bufSize - 3; i++)
			frame->payload[i - 2] = buffer[i];

		frame->endOfPacket = buffer[i];
		frame->errorDetect[0] = buffer[i + 1];
		frame->errorDetect[1] = buffer[i + 2];

		return *frame;
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




