// Name: Ashutosh Somaiya
// CampusID: W1609207

#include<sys/socket.h> // for socket related functions and APIs
#include<netinet/in.h> // for address related details
#include<stdio.h> 
#include<strings.h>
#include<string.h>
#include<stdint.h>
#include<stdlib.h>
#include<unistd.h>

#define PORT 8081
#define PACKETID 0XFFFF // Start of Packet Identifier
#define CLIENTID 0XFF // Client Identfier
#define DATATYPE 0XFFF1 // Data Packet
#define ENDPACKETID 0XFFFF // End Packet Identifier
#define TIMEOUT 3 // Set Timeout for ACK timer to 3 sec
#define ACKPACKET 0XFFF2 // ACK Packet
#define REJECTPACKETCODE 0XFFF3 // REJECT Packet
#define LENGTHMISMATCHCODE 0XFFF5 // Length Mismatch sub code
#define ENDPACKETIDMISSINGCODE 0XFFF6 // End of Packet missing sub code
#define OUTOFSEQUENCECODE 0XFFF4 // Out of Sequence sub code
#define DUPLICATECODE 0XFFF7 // Duplicate Packet sub code

struct datapacket{
	uint16_t packetID;
	uint8_t clientID;
	uint16_t type;
	uint8_t segment_No;
	uint8_t length;
	char payload[255];
	uint16_t endpacketID;
};
struct ackpacket {
	uint16_t packetID;
	uint8_t clientID;
	uint16_t type;
	uint8_t segment_No;
	uint16_t endpacketID;
};
struct rejectpacket {
	uint16_t packetID;
	uint8_t clientID;
	uint16_t type;
	uint16_t subcode;
	uint8_t segment_No;
	uint16_t endpacketID;
};
// shows the packet information.
void show(struct datapacket data) {
	printf("Received Packet Details\n");
	printf("Start of Packet ID: %hx\n",data.packetID);
	printf("Client ID : %hhx\n",data.clientID);
	printf("Data: %x\n",data.type);
	printf("Segment Number: %d\n",data.segment_No);
	printf("Length: %d\n",data.length);
	printf("Payload: %s\n",data.payload);
	printf("End of Packet ID: %x\n",data.endpacketID);
}
// creates a reject packet to send to the client 
struct rejectpacket generaterejectpacket(struct datapacket data) {
	struct rejectpacket reject;
	reject.packetID = data.packetID;
	reject.clientID = data.clientID;
	reject.segment_No = data.segment_No;
	reject.type = REJECTPACKETCODE;
	reject.endpacketID = data.endpacketID;
	return reject;
}
// creates an ack packet to send to the client
struct ackpacket generateackpacket(struct datapacket data) {
	struct ackpacket ack;
	ack.packetID = data.packetID;
	ack.clientID = data.clientID;
	ack.segment_No = data.segment_No;
	ack.type = ACKPACKET ;
	ack.endpacketID = data.endpacketID;
	return ack;
}

int main()
{
	int Server_Socket,n;
	struct sockaddr_in serverAddr;
	struct sockaddr_storage serverStorage;
	socklen_t addr_size;
	struct datapacket data;
	struct ackpacket  ack;
	struct rejectpacket reject;

	// a buffer to check if the packet has already been received before and send an Ack or an error message accordingly.
	int buffer[34];
	int j;	
	for(j=0;j<34;j++) 
	{
		buffer[j] = 0;
	}
	Server_Socket=socket(AF_INET,SOCK_DGRAM,0);
	int expectedPacket = 1;
	bzero(&serverAddr,sizeof(serverAddr));
	serverAddr.sin_family = AF_INET;
	serverAddr.sin_addr.s_addr=htonl(INADDR_ANY);
	serverAddr.sin_port=htons(PORT);
	bind(Server_Socket,(struct sockaddr *)&serverAddr,sizeof(serverAddr));
	addr_size = sizeof serverAddr;
	printf("Server connected\n");

	// an infinite loop to keep listening to the incoming packets
	for(;;) {
		printf("\n");
		// a function to receive data from the client.
		n = recvfrom(Server_Socket,&data,sizeof(struct datapacket),0,(struct sockaddr *)&serverStorage, &addr_size);
		printf("\n");
		show(data);
		buffer[data.segment_No]++;
		int length = strlen(data.payload);

		if(buffer[data.segment_No] != 1) 
		{
			reject = generaterejectpacket(data);
			reject.subcode = DUPLICATECODE;
			sendto(Server_Socket,&reject,sizeof(struct rejectpacket),0,(struct sockaddr *)&serverStorage,addr_size);
			printf("DUPLICATE PACKET RECIEVED \n\n");
		}

		else if(length != data.length) 
		{
			reject = generaterejectpacket(data);
			reject.subcode = LENGTHMISMATCHCODE ;
			sendto(Server_Socket,&reject,sizeof(struct rejectpacket),0,(struct sockaddr *)&serverStorage,addr_size);
			printf("LENGTH MISMATCH ERROR \n\n");
		}
		else if(data.endpacketID != ENDPACKETID ) 
		{
			reject = generaterejectpacket(data);
			reject.subcode = ENDPACKETIDMISSINGCODE ;
			sendto(Server_Socket,&reject,sizeof(struct rejectpacket),0,(struct sockaddr *)&serverStorage,addr_size);
			printf("END OF PACKET IDENTIFIER MISSING \n\n");
		}
		else if(data.segment_No != expectedPacket && data.segment_No != 10 && data.segment_No != 11) 
		{
			reject = generaterejectpacket(data);
			reject.subcode = OUTOFSEQUENCECODE;
			sendto(Server_Socket,&reject,sizeof(struct rejectpacket),0,(struct sockaddr *)&serverStorage,addr_size);
			printf("OUT OF SEQUENCE ERROR \n\n");
		}
		else 
		{
			ack = generateackpacket(data);
			sendto(Server_Socket,&ack,sizeof(struct ackpacket),0,(struct sockaddr *)&serverStorage,addr_size);
		}
		expectedPacket++;
		printf("@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@\n");
	}
}
