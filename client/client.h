
#define WELLKNOWNPORT 5280

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

int physical_Establish(struct hostent* host, unsigned short port);
void application_Layer(FILE *file, int sock);
void datalink_Layer(Packet *p, int packetSize, int sock);
void physical_Layer(Frame* buffer, int frameSize, int sock);
char* error_Handling(Frame t, int size);
