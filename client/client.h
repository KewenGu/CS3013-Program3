typedef struct frameData {

	char seqNum[2];
	void *payload; //somewhere between 1 and 130 bytes

	char errorDetect[2];
	char endOfPacket;

} Frame;

typedef struct frameACK {

	char seqNum[2];
	char errorDetect[2];

} FrameACK;

typedef struct packet {

	char data[256];
	char endOfPhoto;

} Packet;

int physical_Establish(struct hostent* host, unsigned short port);
