
#define WELLKNOWNPORT 5280

#define FRAME_PAYLOAD_SIZE 130

#define FRAMETYPE_DATA 0x01
#define FRAMETYPE_ACK 0x02

typedef struct frame {

	char frameType; //Now uses one of the two #defines above!
	char seqNum[2];
	char payload[FRAME_PAYLOAD_SIZE];

	char endOfPacket; // end-of-packet byte should be after the payload/packet
	char errorDetect[2];
	
} __attribute__((packed)) Frame;


typedef struct packet {

	char data[256];
	//char endOfPhoto;

} __attribute__((packed)) Packet;

void DieWithError(char *errorMsg);

int physical_Establish(struct hostent* host, unsigned short port);
void application_Layer(FILE *file, int sock);
void datalink_Layer(Packet *p, int packetSize, int sock);
void physical_Layer(Frame* buffer, int frameSize, int sock);
char* error_Handling(Frame t, int size);
