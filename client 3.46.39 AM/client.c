/* Kewen Gu & Preston Mueller, Program 3, CS3516 */

#include <string.h>
#include <stdio.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <stdlib.h>
#include <unistd.h>

#include "client.h"

// Keep track of a global sequence number
unsigned short global_seq_num = 0;
// Used to simulate artificial transmission errors
int error_generator = 0;

/*
Author: Preston Mueller
	Main program to execute the client
*/
int main(int argc, char** argv) 
{
	/* Check the correct number of inputs */
	if(argc != 4) 
	{
		printf("Usage: ./client <servermachine> <id> <num_photos>\n");
		exit(1);
	}

	int clientID = atoi(argv[2]);
	int numPhotos = atoi(argv[3]);
  char fileName[129];
  FILE *file;
  int i, j, k;

  /* Pointer to socket structure that ends up filled in by gethostbyname */
	struct hostent *servHost;
	unsigned short port = PORT_NUMBER;
	servHost = gethostbyname(argv[1]);
	int sock = PhysicalEstablish(servHost, port);

  if (send(sock, &clientID, sizeof(clientID), 0) != sizeof(clientID))
    DieWithError("send() failed");
  if (send(sock, &numPhotos, sizeof(numPhotos), 0) != sizeof(numPhotos))
    DieWithError("send() failed");

  /* Packet memory allocation. First make space, then we can do the math for the packets easier */
  for(i = 1; i <= numPhotos; i++) 
  {
  	/* Specify file names for reading */
    sprintf(fileName, "photo%d%d.jpg", clientID, i);
    
    if((file = fopen(fileName, "rb")) == NULL) 
    {
      fprintf(stderr, "%s couldn't be found!\n", fileName);
      close(sock);
      exit(1);
    }

    /* Go into the application layer */
    ApplicationLayer(file, sock);

    /* Close file after done reading */
    fclose(file);
  }
  /* Close the sock after the operations are done */
  printf("Done sending %d photos to the server\n", numPhotos);
  close(sock);
}



/*
Arthor: Preston Mueller
	This function establish connection between the client and the server,
	it returns the socket descriptor
*/
int PhysicalEstablish(struct hostent* host, unsigned short port)
{
//  printf("Connecting to host on port %d\n", port);

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

/*
Arthor: Preston Mueller
	This function is abstracted as the application layer.
	It reads data from the file and put it into packets, 
	and send each packet to the datalink layer.
*/
void ApplicationLayer(FILE *file, int sock)
{
  unsigned char packet[PACKET_SIZE];

  int bytesLoaded = 0;
  int pos = 0;
  int retval;

  
  while ((retval = fread(packet, 1, PACKET_SIZE - 1, file)) > 0)
  {	
		if (retval < PACKET_SIZE -  1)
			packet[retval] = END_OF_PHOTO_YES;
		else if (retval == PACKET_SIZE -  1)
			packet[retval] = END_OF_PHOTO_NO;

	  DatalinkLayer(packet, retval + 1, sock);
  } 
}

/*
Author: Kewen Gu
	This function is asbstracted as the datalink layer.
	It converts packet to frames, send each frame to the physical layer,
	and resend the frame when time out and incorrectly ACKed.
*/
void DatalinkLayer(unsigned char *packet, int packetSize, int sock)
{
  // First, initialize the frame
  int payloadLen = FRAME_SIZE - 6;
  int numFrames = (packetSize / payloadLen) + (packetSize % payloadLen > 0 ? 1 : 0);
  unsigned char frames[numFrames][FRAME_SIZE];

	unsigned char *errorCheck;
  int pos = 0;
  int i, j, frameSize, resendFlag;

  for (i = 0; i < numFrames; i++)
  {
  	frames[i][0] = FRAME_TYPE_DATA;
  	frames[i][1] = global_seq_num & 0xff00;
  	frames[i][2] = global_seq_num & 0x00ff;


  	for (j = 3; j < (FRAME_SIZE - 3) && pos < packetSize; j++)
  	{
  		frames[i][j] = packet[pos];
  		pos++;
  	}

  	if (pos == packetSize)
  		frames[i][j] = END_OF_PACKET_YES;
  	else
  		frames[i][j] = END_OF_PACKET_NO;

  	frameSize = j + 3;

  	errorCheck = ErrorHandling(frames[i], frameSize - 2);
  	frames[i][j + 1] = errorCheck[0];
  	frames[i][j + 2] = errorCheck[1];
//  	printf("Frame errorDetect: %02x %02x\n", frames[i][frameSize - 2], frames[i][frameSize - 1]);

		do {
			resendFlag = PhysicalLayer(frames[i], frameSize, sock);
		} while (resendFlag);


		if (global_seq_num == 255)
			global_seq_num = 0;
		else
    	global_seq_num++;
  }
}

/*
Author: Kewen Gu
	This function is abstracted as the physical layer.
	It receives a frame from the datalink layer, send the frame
	to the server, receive ACK from the server, and ask the datalink
	layer to resend the packet if time out or incorrectly ACKed.
*/
int PhysicalLayer(unsigned char *frame, int frameSize, int sock)
{
	unsigned char newFrame[FRAME_SIZE];
	unsigned char ack[ACK_SIZE];
	int resendFlag = 0;  // either time out or ACK failed
	int bytesRcvd = 0;
//	int totalBytesRcvd = 0;
	int retval;	
	struct timeval timer;

  fd_set fileDescriptorSet;
  FD_ZERO(&fileDescriptorSet);
  FD_SET(sock, &fileDescriptorSet);

  memcpy(newFrame, frame, FRAME_SIZE);

  if (error_generator % 6 == 5)
  		newFrame[3] = 0x00;
  error_generator++;

//  printf("Send frame to server: frame size = %d\n", frameSize);
  int n;
  if ((n = send(sock, newFrame, frameSize, 0)) != frameSize)
    DieWithError("send() error");

  timer.tv_sec = 0;
  timer.tv_usec = TIME_OUT_USECS;

  retval = select(sock + 1, &fileDescriptorSet, NULL, NULL, &timer);
  if (retval < 0)
    DieWithError("select() failed");
  
  else if (retval)
  {
  
    //There's data to receive
//    do
//		{
			if ((bytesRcvd = recv(sock, ack + bytesRcvd, ACK_SIZE, 0)) < 0)
				DieWithError("recv() failed");
//			totalBytesRcvd += bytesRcvd;
//		} while (bytesRcvd > 0);


//    printf("Internal sequence number = %02x\n", global_seq_num);
//    printf("ACK: %02x %02x %02x %02x %02x\n", ack[0], ack[1], ack[2], ack[3], ack[4]);

    if (ack[0] != FRAME_TYPE_ACK || ack[1] != frame[1] || ack[2] != frame[2] || ack[3] != ack[1] || ack[4] != ack[2])
    {
//    	printf("ACK failed\n");
    	resendFlag = 1;
    }
  }
  else
	{
//    printf("Time out!\n");
    resendFlag = 1;
  }

	return resendFlag;
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

