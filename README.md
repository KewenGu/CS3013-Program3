Kewen Gu & Preston Mueller
Program 3
CS 3516
Oct 13, 2015

## Introduction:
'''
	In this program assignement, we implemented a photo client and a concurrent photo server. Both client and server are constructed as three layer, the application/network layer, the datalink layer and the physical layer. 
	For the client, the application layer reads data from the photo and store the data into packets with maximum size of 256 bytes, then it sends each packet to the datalink layer. The datalink layer receives packet from the application layer, converts packet to frames, sends each frame to the physical layer. The the physical layer send the frame received to the server, wait until receive an ACK from the server. If the physical layer does not receive the ACK in the specified period, the time out bit will be set, and if the ACK received is not correct, the not ACKed flag will be set. Then the physical layer will tell the datalink layer whether to resend the frame or not.
	For the server, the application layer receives packets from the datalink layer, then writes the packet data to the file. The datalink layer receives frames from the physical layer, sends an ACK to physical layer for each frame received, constructs packets out of the those frames, and sends each packet to the application layer. The physical layer received frames from and send ACK to the client. In our program, we conbined the functionality of datalink layer and physical layer together, since the physical layer merely do receiving and sending.
'''

## How to compile and run:
'''
	To compile the program, do

		make

	To run the client, do

		./client <server machine> <client ID> <number of photos to send>

	To run the server, do

		./server

	Note that the client and server programs are located in seperate directories.
'''

## Discussions:
'''
	The time of transmission depends on the procession speed of the machine as well as the size of the photo. The "time out" we are using right now is 300000 microseconds. It takes a fairly amount of time to transmit the photo from the client to the server. When we set the "time out" really low, the photo cannot be correctly transmitted. However, if sending from local to local, "time out" can be much lower. 

	The program works very well on our own machines(MAC OS). However, we do not have time to fully test it on the CCC machine. During our test on CCC, we run the server on CCC and send photos to it through the client on our local machine, or we run the client on CCC and send photos to the server on our local machine. The program works fine for some "time out" but not for the others. When we run both client and server on CCC, the program always fails and we cannot have enough time to fix this.
'''
