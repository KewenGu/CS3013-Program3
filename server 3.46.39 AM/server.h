/* Kewen Gu & Preston Mueller, Program 3, CS3516 */

#define PORT_NUMBER 4074
#define MAX_NUM_CLIENTS 10

#define PACKET_SIZE 257
#define FRAME_SIZE 136
#define ACK_SIZE 5

#define FRAME_TYPE_DATA 0x01
#define FRAME_TYPE_ACK 0x02

#define END_OF_PHOTO_YES ((unsigned char)4)   // end of transmission
#define END_OF_PHOTO_NO ((unsigned char)3)    // end of text

#define END_OF_PACKET_YES ((unsigned char)4)  // end of transmission
#define END_OF_PACKET_NO ((unsigned char)3)   // end of text


void ApplicationLayer(int clntSock, FILE *file);
int DatalinkLayer(unsigned char *packet, int clntSock);
unsigned char *ErrorHandling(unsigned char *frame, int len);
void DieWithError(char *errorMsg);
