typedef struct frameData {

	char seqNum[2];
	char *payload; //somewhere between 1 and 130 bytes

	char errorDetect[2];
	char eop; //end-of-packet byte

} frame;

typedef struct frameACK {

	char seqNum[2];
	char errorDetect[2];

} frameACK;

int physical_Establish(struct hostent* host, unsigned short port);
