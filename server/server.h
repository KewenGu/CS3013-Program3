
#define WELLKNOWNPORT 5280
#define MAXPENDING 10

typedef struct frameData {

	char seqNum[2];
	unsigned char *payload; //somewhere between 1 and 130 bytes
	char endOfPacket; // end-of-packet byte should be after the payload/packet
	char errorDetect[2];
	
} Frame;

typedef struct frameACK {

	char seqNum[2];
	char errorDetect[2];

} FrameACK;


typedef struct packet {

	char data[256];
	char endOfPhoto;

} Packet;


void DieWithError(char *errorMsg);

Frame make_Frame(char *buffer, int bufSize);

char* error_Handling(Frame t, int size);

