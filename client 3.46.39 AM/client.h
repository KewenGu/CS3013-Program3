/* Kewen Gu & Preston Mueller, Program 3, CS3516 */

#define PORT_NUMBER 4074
#define TIME_OUT_USECS 300000

#define PACKET_SIZE 257
#define FRAME_SIZE 136
#define ACK_SIZE 5

#define FRAME_TYPE_DATA 0x01
#define FRAME_TYPE_ACK 0x02

#define END_OF_PHOTO_YES ((unsigned char)4)   // end of transmission
#define END_OF_PHOTO_NO ((unsigned char)3)    // end of text

#define END_OF_PACKET_YES ((unsigned char)4)  // end of transmission
#define END_OF_PACKET_NO ((unsigned char)3)   // end of text


int PhysicalEstablish(struct hostent* host, unsigned short port);
void ApplicationLayer(FILE *file, int sock);
void DatalinkLayer(unsigned char *packet, int packetSize, int sock);
int PhysicalLayer(unsigned char *frame, int frameSize, int sock);
unsigned char *ErrorHandling(unsigned char *frame, int len);
void DieWithError(char *errorMsg);

