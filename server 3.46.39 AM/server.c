/* Kewen Gu & Preston Mueller, Program 3, CS3516 */

#include <string.h>
#include <stdio.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <stdlib.h>
#include <unistd.h>

#include "server.h"

// Keep track of a global sequence number
unsigned short global_seq_num = 0;
// Used to simulate artificial transmission errors
int error_generator = 0;



/*
Author: Kewen Gu
	Main program for executing the concurrent server
*/
int main(int argc, char **argv)
{
	/* Check the correct number of inputs */
	if (argc != 1)
	{
		printf("Usage: %s\n", argv[0]);
		exit(1);
	}

	/* Variables declaration */
	int servSock, clntSock, clientID, numPhotos;
	unsigned int clntLen;
	unsigned short port = PORT_NUMBER;
	struct sockaddr_in servAddr, clntAddr;
	char fileName[129];
	FILE *file;
	int childpid;
	int i, j;


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
	if (listen(servSock, MAX_NUM_CLIENTS) < 0)
		DieWithError("listen() failed");


	while(1)
	{
		clntLen = sizeof(clntAddr);
		/* Accept the client */
		if ((clntSock = accept(servSock, (struct sockaddr *) &clntAddr, &clntLen)) < 0)
			DieWithError("accept() failed");
		
		/* Create a child process for every client accepted */
		if ((childpid = fork()) == 0)
		{
			close(servSock);

			printf("Handling client %s\n", inet_ntoa(clntAddr.sin_addr));

			/* Receive the client ID and the number of photos from the client */
			if (recv(clntSock, &clientID, sizeof(clientID), 0) < 0)
				DieWithError("recv() failed");
			if (recv(clntSock, &numPhotos, sizeof(numPhotos), 0) < 0)
				DieWithError("recv() failed");

			/* Receive data of each photo and write to file */
			for (i = 1; i <= numPhotos; i++)
			{
				/* Specify file names */
				sprintf(fileName, "photo%d%d.jpg", clientID, i);

				/* Open the file for writing */
				if((file = fopen(fileName, "wb")) == NULL) 
		    {
		      fprintf(stderr, "%s couldn't be found!\n", fileName);
		      exit(1);
		    }

		    /* Go into the application layer */
		    ApplicationLayer(clntSock, file);

		    /* Close file after done writing to it */
		    fclose(file);
		  }

		  printf("Received %d photos\n", numPhotos);
		  printf("Closing socket for client %s\n", inet_ntoa(clntAddr.sin_addr));
		  printf("==========================================\n");
		  exit(0);
		}
		close(clntSock);
	}
}


/*
Author: Kewen Gu
	This function is abstracted as the application layer.
	It receives packet from the datalink layer and write the 
	packet data to file until all the end-of-photo is detected
*/
void ApplicationLayer(int clntSock, FILE *file)
{
	unsigned char packet[PACKET_SIZE];
	int packetLen = 2 ;

	/* Receive packet from datalink layer and write packet data to file */
	do
	{
		if (packetLen > 1)
		{
			packetLen = DatalinkLayer(packet, clntSock);

			fwrite(packet, 1, packetLen - 1, file);
		}
	} while (packet[packetLen - 1] != END_OF_PHOTO_YES);
	/* The function ends after the end-of-photo indicator is read */
}


/*
Author: Preston Mueller
	This function is abstracted as the datalink layer + the physical layer.
	Since the physical layer merely send and receive from the client, 
	we just combine the datalink and the physical together.
	The function receives frames from the physical layer (client) and 
	check whether the frame is correct, send an ACK back to the client 
	and construct a packet out of the received frames.
*/
int DatalinkLayer(unsigned char *packet, int clntSock)
{
	unsigned char frame[FRAME_SIZE];
	unsigned char ack[ACK_SIZE];

	int bytesRcvd = 1;
//	int totalBytesRcvd;
	int endOfPhotoFlag = 0;
	int endOfPacketFlag = 0;
	unsigned char errorDetect[2];
	unsigned char *errorCheck;
	int seqNum;
	int lastSeqNum = -1;
	int packetLen = 0;
	int resendACK = 0;
	int i, times = 0;

	/* While there's data received */
	while (bytesRcvd)
	{
		bytesRcvd = 0;
//		totalBytesRcvd = 0;
//		do
//		{
			if ((bytesRcvd = recv(clntSock, frame + bytesRcvd, FRAME_SIZE, 0)) < 0)
				DieWithError("recv() failed");
//			totalBytesRcvd += bytesRcvd;
//		} while (bytesRcvd > 0);

		/* Check whether the frame has been received before (duplicate) */
		seqNum = (unsigned int)((frame[1] & 0xff00) | (frame[2] & 0x00ff));

		if (seqNum == lastSeqNum)
		{
//			printf("Duplicate frame received\n");
			global_seq_num--;
			resendACK = 1;
		}

		errorDetect[0] = frame[bytesRcvd - 2];
		errorDetect[1] = frame[bytesRcvd - 1];
//		printf("Original error detection bytes: %x %x\n", errorDetect[0], errorDetect[1]);
		
		errorCheck = ErrorHandling(frame, bytesRcvd - 2);
//		printf("New error detection bytes Generated: %x %x\n", errorCheck[0], errorCheck[1]);


		/* Checking the sequence number and the error detection bytes */
		if (errorCheck[0] == errorDetect[0] && errorCheck[1] == errorDetect[1] && frame[0] == FRAME_TYPE_DATA && seqNum == global_seq_num)
		{
			/* Constructing the ACK */
			ack[0] = FRAME_TYPE_ACK;
			ack[1] = frame[1];
			ack[2] = frame[2];
			ack[3] = ack[1];
			ack[4] = ack[2];

			/* Generate error on ACK */
			if (error_generator % 11 == 10)
				ack[3] = 0x00;

//			printf("ACK: %02x %02x %02x %02x %02x\n", ack[0], ack[1], ack[2], ack[3], ack[4]);
	    
	    /* Sending ACK to client */
			if(send(clntSock, ack, ACK_SIZE, 0) != ACK_SIZE)
	  		DieWithError("send() failed");

	  	/* Increment the squence number */
	  	lastSeqNum = global_seq_num;
	  	if (global_seq_num == 255)
				global_seq_num = 0;
			else
	    	global_seq_num++;

	    /* If not duplicate */
	  	if (!resendACK)
	  	{
				for (i = 3; i < bytesRcvd - 3; i++)
				{
					/* Convert frames to packet */
					packet[packetLen] = frame[i];
//					printf("packet[%d] = %02x\n", packetLen, packet[packetLen]);
					/* Return to application layer if end-of-packet reached */
					if (i == bytesRcvd - 4 && frame[i + 1] == END_OF_PACKET_YES)
						return packetLen + 1;
					else 
						packetLen++;
				}
	  	}
	  }
	  error_generator++;
	}
	return packetLen + 1;
}

/*
Author: Preston Mueller
	This function is used to generate the error detection bytes for the frames
*/
unsigned char *ErrorHandling(unsigned char *frame, int len)
{
  int i;
  unsigned char *result = malloc(2 * sizeof(unsigned char));

  for (i = 0; i < (len - 2); i += 2)
    result[0] = frame[i] ^ result[0];

  for (i = 1; i < (len - 2); i += 2)
    result[1] = frame[i] ^ result[1];

  return result;
}

/*
Author: Kewen Gu
	This function is used to exit from the program with the error message 
	when an error occurs
*/
void DieWithError(char *errorMsg)
{
	perror(errorMsg);
	exit(1);
}


