#include <string.h>
#include <stdio.h>

#include <sys/socket.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <stdlib.h>
#include <unistd.h>

#include "client.h"

#define PACKET_SIZE sizeof(Packet)

#define END_OF_PHOTO_YES ((char)4)   // end of transmission
#define END_OF_PHOTO_NO ((char)3)    // end of text

#define END_OF_PACKET_YES ((char)4)  // end of transmission
#define END_OF_PACKET_NO ((char)3)   // end of text

//#define DATALINK_EXPECTATION_ACK 1
//#define DATALINK_EXPECTATION_

unsigned short seq_num = 0;

//Author: Kewen Gu
int main(int argc, char** argv) {

	if(argc != 4) {

		printf("Usage: ./client <servermachine> <id> <num_photos>\n");
		exit(1);

	}

	int clientID = atoi(argv[2]);
	int numPhotos = atoi(argv[3]);
  char fileName[129];
  FILE *file;
  int i, j, k;

	//Pointer to socket structure that ends up filled in by gethostbyname
	struct hostent *servHost;
	unsigned short port = WELLKNOWNPORT;
	servHost = gethostbyname(argv[1]);
	int sock = physical_Establish(servHost, port);

  //The server is expecting an indication of which client we are and how many photos
  if (send(sock, &clientID, sizeof(clientID), 0) < 0)
    DieWithError("send() failed");
  if (send(sock, &numPhotos, sizeof(numPhotos), 0) < 0)
    DieWithError("send() failed");

  //Packet memory allocation. First make space, then we can do the math for the packets easier
  for(i = 0; i < numPhotos; i++) 
  {
    sprintf(fileName, "photo%d%d.jpg", clientID, i);
    
    if((file = fopen(fileName, "rb")) == NULL) 
    {
      fprintf(stderr, "%s couldn't be found!\n", fileName);
      exit(1);
    }

    //Send the image to the application layer!
    printf("To application layer\n");
    application_Layer(file, sock);
    printf("Send %s to the server\n", fileName);

    fclose(file);
  }
}

//A function to exit with a failure code and an error
void DieWithError(char *errorMsg)
{
  perror(errorMsg);
  exit(1);
}


//Author: Preston Mueller
int physical_Establish(struct hostent* host, unsigned short port) {

  //Attempt to make a physical layer connection
  printf("Connecting to host on port %d\n", port);

  //Socket info and socket pointer
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

//Author: Preston Mueller
void application_Layer(FILE *file, int sock)
{
  //Check how large the file is
  fseek(file, 0, SEEK_END);
  int fileSize = ftell(file);
  fseek(file, 0, SEEK_SET);

  //Calculate how many packets this is gong to take
  int numPackets = (fileSize / PACKET_SIZE) + (fileSize % PACKET_SIZE > 0 ? 1 : 0);

  Packet *packets = malloc(numPackets * sizeof(Packet));

  //Load the file into a packet!
  int bytesLoaded = 0;
  int currentPacket = 0;
  int currentPosition = 0;
  int i;

  // Read from JPEG file one byte at a time
  while(bytesLoaded < fileSize) {
    fread(packets[currentPacket].data + currentPosition, 1, 1, file);

    currentPosition++;
    bytesLoaded++;

    // Change to a new packet after PACKET_SIZE bytes read
    if(currentPosition == PACKET_SIZE) {
      //packets[currentPacket].endOfPhoto = END_OF_PHOTO_NO; // Indicate not end-of-photo
      currentPosition = 0;
      currentPacket++;
    }

  }

  //In theory, we would indicate the end of photo byte here.
  //packets[numPackets - 1].endOfPhoto = END_OF_PHOTO_YES; // Indicate end-of-photo
    
  //Send the packets to the Data Link Layer!
  for (i = 0; i < numPackets; i++)
  {
    printf("To datalink layer\n");
    datalink_Layer(&packets[i], sizeof(packets[i]), sock);
  }

}

// Author: Preston Mueller
//    Put the payload into the frame
void datalink_Layer(Packet *p, int packetSize, int sock)
{

  // First, initialize the frame
  int numFrames = (packetSize / FRAME_PAYLOAD_SIZE) + (packetSize % FRAME_PAYLOAD_SIZE > 0 ? 1 : 0);
  Frame *frames = (Frame *)malloc(numFrames * sizeof(Frame));

  int currentFrame = 0;
  int currentPosition = 0;
  int bytesToFrame = 0;
  int bytesFramed = 0;
  int totalBytesFramed = 0;
  int i;

  //Above, we made frames. Here, we are filling them with packet payloads until we cannot anymore
  for(int i = 0; i < numFrames; i++) {

    if(i < numFrames-1) {
      //Frame has full payload
      memcpy(frames[i].payload, p->data + currentPosition, FRAME_PAYLOAD_SIZE);
      frames[i].endOfPacket = END_OF_PACKET_NO;

      //It's a data frame
      frames[i].frameType = FRAMETYPE_DATA;
      frames[i].seqNum[0] = seq_num & 0xff00;
      frames[i].seqNum[1] = seq_num & 0x00ff;

      currentPosition += FRAME_PAYLOAD_SIZE;
    }
    else if(i == (numFrames-1)) {
      //The frame does not have a full payload
      memcpy(frames[i].payload, p->data + currentPosition, packetSize % FRAME_PAYLOAD_SIZE);
      frames[i].endOfPacket = END_OF_PACKET_YES;

      //It's a data frame
      frames[i].frameType = FRAMETYPE_DATA;
      frames[i].seqNum[0] = seq_num & 0xff00;
      frames[i].seqNum[1] = seq_num & 0x00ff;

      currentPosition += (packetSize % FRAME_PAYLOAD_SIZE);
    }
    
    //Increment the global sequence number that we are sending frames with
    seq_num++;


  }

  //For each of the frames that we crearted, send it to the physical layer
  for(int i = 0; i < numFrames; i++) {
    printf("To physical layer with frame #: %x %x\n", frames[i].seqNum[0], frames[i].seqNum[1]);
    physical_Layer(&frames[i], sizeof(Frame), sock);
  }

  //Return back up to the application layer
  printf("Returning to application layer.\n");

}

//Author: Preston Mueller
void physical_Layer(Frame* buffer, int frameSize, int sock) 
{
  int timeOut = 1;
  int notACKed = 1;

  //Create a template ACK frame that we are going to fill with received data
  Frame *ack = malloc(sizeof(Frame));
  ack->frameType = FRAMETYPE_ACK;
  
  //Create a frame timer
  struct timeval timer;

  fd_set fileDescriptorSet;
  FD_ZERO(&fileDescriptorSet);
  FD_SET(sock, &fileDescriptorSet);

  //Indicate to the console how much we'll be sending
  printf("Physical send: length is %d\n", frameSize);

  while (timeOut)
    while (notACKed)
    {
      //Send the frame
      if (send(sock, buffer, frameSize, 0) < 0)
        DieWithError("send() error");
      
      timer.tv_sec = 3;
      timer.tv_usec = 0;

      printf("Physical layer: Starting timer\n");

      //Frame timer
      if (select(sock + 1, &fileDescriptorSet, NULL, NULL, &timer) < 0)
        DieWithError("select() failed");

      //If, here, we have 0s in the timer, that means the timer ran out of time
      if (timer.tv_sec == 0 && timer.tv_usec == 0) 
      {
        printf("Physical layer: Timed out!\n");
        timeOut = 1; // time out!
        break;
      }
      else
      {
        printf("Receiving ACK\n");
        timeOut = 0; // not timed out!
      
        //There's data to receive
        if (recv(sock, ack, sizeof(Frame), 0) < 0)
          DieWithError("recv() failed");

        printf("ACK received\n");

        //For our program we were unable to get the seqNum checking working,
        //  so we are assuming all ACKs are successful right now.

        if(1)
        //if (atoi(ack->seqNum) == seq_num && atoi(ack->errorDetect) == atoi(ack->seqNum))
        {
          notACKed = 0; // ACK successful!
          break;
        }
        else
        {
          printf("ACK failed!\n");
          notACKed = 1; // ACK failed!
        }
      }
    }

    printf("Sent frame successfully!\n");

}

/* 

Author: Kewen Gu

Function generates the error detection bytes
    how this work?
        suppose the frame in hex representation is "00 01 02 03 04 05 06 07... [2 error detection bytes]"
        then, error detection bytes = 00^02^04^06... + 01^03^05^07...  (^ is the operation of XOR, + is the operation of concatenation)
*/
char *error_Handling(Frame t, int size)
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
