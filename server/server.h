
#define WELLKNOWNPORT 5280
#define MAXPENDING 10

#define FRAME_PAYLOAD_SIZE 130
#define FRAMETYPE_DATA 0x01
#define FRAMETYPE_ACK 0x02

typedef struct frame {

	char frameType; //Now uses one of the two #defines above!
	char seqNum[2];
	//unsigned char *payload; //somewhere between 1 and 130 bytes
	char payload[FRAME_PAYLOAD_SIZE];

	char endOfPacket; // end-of-packet byte should be after the payload/packet
	char errorDetect[2];
	
} __attribute__((packed)) Frame;

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

