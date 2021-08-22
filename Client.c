// Name: Ashutosh Somaiya
// CampusID: W1609207

#include<sys/socket.h>
#include<netinet/in.h>
#include<stdio.h>
#include<stdlib.h>
#include<strings.h>
#include<string.h>
#include<time.h>
#include<stdint.h>

#define PORTNO 8081
#define TIMEOUT 3 // In case there is no response from the server in 3 seconds, an error message will be sent.

//Defining the Primitives.
#define STARTPACKETID 0XFFFF // Start of Packet Identifier
#define ENDPACKETID 0XFFFF // End of Packet Identifier
#define CLIENTID 0XFF // Client Identifier

//Types of packets
#define DATATYPE 0XFFF1 // Data Packet
#define ACKPACKET 0XFFF2 // ACK Packet
#define REJECTPACKETCODE 0XFFF3 // REJECT Packet

//Reject sub codes
#define OUTOFSEQUENCECODE 0XFFF4 // Out of Sequence sub code
#define LENGTHMISMATCHCODE 0XFFF5 // Length Mismatch sub code
#define ENDPACKETIDMISSINGCODE 0XFFF6 // End of Packet missing sub code
#define DUPLICATECODE 0XFFF7 // Duplicate Packet sub code



struct Datapacket {
	uint16_t startPacketID;
	uint8_t clientID;
	uint16_t type;
	uint8_t segment_number;
	uint8_t length;
	char payload[255];
	uint16_t endpacketID;
};
struct ackpacket {
	uint16_t startPacketID;
	uint8_t clientID;
	uint16_t type;
	uint8_t segment_number;
	uint16_t endpacketID;
};
struct rejectpacket {
	uint16_t startPacketID;
	uint8_t clientID;
	uint16_t type;
	uint16_t subcode;
	uint8_t segment_number;
	uint16_t endpacketID;
};

// function to load the packet with the data
struct Datapacket initialise() {
	struct Datapacket data;
	data.startPacketID = STARTPACKETID;
	data.clientID = CLIENTID;
	data.type = DATATYPE;
	data.endpacketID = ENDPACKETID;
	return data;
}
struct ackpacket ackinitialise() {
	struct ackpacket data;
	data.startPacketID = STARTPACKETID;
	data.clientID = CLIENTID;
	data.type = ACKPACKET;
	data.endpacketID = ENDPACKETID;
	return data;
}
struct rejectpacket rejinitialise() {
	struct rejectpacket data;
	data.startPacketID = STARTPACKETID;
	data.clientID = CLIENTID;
	data.type = ACKPACKET;
	data.endpacketID = ENDPACKETID;
	return data;
}
// Print Client Packet Data
void print(struct Datapacket data) {
    printf("\nSending packet:\n");
	printf("Client ID: %hhx\n",data.clientID);
	printf("Packet ID: %x\n",data.startPacketID);
	printf("Segment Number: %d \n",data.segment_number);
	printf("Data: %x\n",data.type);
	printf("Length: %d\n",data.length);
	printf("Payload: %s",data.payload);
	printf("End of Packet ID: %x\n",data.endpacketID);
	printf("\n");
}

// Print Server ACK Packet Data
void ackprint(struct ackpacket adata) {
    
	printf("Packet ID: %x\n",adata.startPacketID);
	printf("Client ID: %x\n",adata.clientID);
	printf("ACK of Packet: %x\n",adata.type);
	printf("Segment Number: %d \n",adata.segment_number);
	printf("End of Packet ID: %x\n",adata.endpacketID);
	printf("\n");
}

// Print Server REJECT Packet Data
void rejprint(struct rejectpacket rdata) {
	printf("Packet ID: %x\n",rdata.startPacketID);
	printf("Client ID: %hhx\n",rdata.clientID);
	printf("Reject Type of Packet: %x\n",rdata.type);
	printf("Segment Number: %d \n",rdata.segment_number);
	printf("Reject : fff3\n");
	printf("Subcode : %x\n",rdata.subcode);
	printf("End of Packet ID: %x\n",rdata.endpacketID);
	printf("\n");
}


int main(){
	struct Datapacket data;
	struct rejectpacket recievedpacket;
	struct sockaddr_in cliaddr;
	struct ackpacket ackn;
	struct rejectpacket rej;
	socklen_t addr_size;
	FILE *fp;


	char line[255];
	int Client_Socket;
	int n = 0;
	int counter = 0;
	int segmentNo = 1;
	int pacCount = 0;
	
	Client_Socket = socket(AF_INET,SOCK_DGRAM,0);  //establish connection to socket AF_INET: SOCK_DGRAM:used for UDP (domain, type, protocol(this is 0 for Internet Protocol)) 
	if(Client_Socket < 0) {
		printf("Cannot connect to socket\n");
	}
	bzero(&cliaddr,sizeof(cliaddr)); // Reset and define connection
	cliaddr.sin_family = AF_INET;
	cliaddr.sin_addr.s_addr = htonl(INADDR_ANY);
	cliaddr.sin_port=htons(PORTNO);
	addr_size = sizeof cliaddr ;
	
	struct timeval tv;
	tv.tv_sec = TIMEOUT;
	tv.tv_usec = 0;
	
	setsockopt(Client_Socket, SOL_SOCKET, SO_RCVTIMEO, (const char*)&tv,sizeof(struct timeval));  //Set socket options, used to allocated buffer space, control timeout, permit socket data broadcast
	
	
	fp = fopen("input.txt", "rt");
	if(fp == NULL)
	{
		printf("Cannot open the file\n");
		exit(0);
	}
	
	while(1)
	{
		printf("\n");
		printf("Options:\n");
		printf("1. Send 5 correct packets\n");
		printf("2. Send 4 incorrect and 1 correct packet\n");
		printf("3. Length Mismatch Error\n");
		printf("4. End of Packet Error\n");
		printf("5. Duplicate Packet Error\n");
		printf("\n");
		
		int i;
		scanf("%d",&i);
		
		switch (i)
		{
		case 1:
			for(pacCount=0;pacCount<5;pacCount++){
				data = initialise();
				ackn = ackinitialise();
				rej=rejinitialise();
				if(fgets(line, sizeof(line), fp) != NULL) 
				{
					n = 0;
					counter = 0;
					printf("%s",line);
					data.segment_number = segmentNo;
					strcpy(data.payload,line);     
					data.length = strlen(data.payload);
					data.endpacketID = ENDPACKETID;
				}
				while(n<=0 && counter<3)
				{
					sendto(Client_Socket,&data,sizeof(struct Datapacket),0,(struct sockaddr *)&cliaddr,addr_size);
					n = recvfrom(Client_Socket,&recievedpacket,sizeof(struct rejectpacket),0,NULL,NULL);
					if(n <= 0 )
					{
						printf("The server hasn't responded for %d seconds. Trying again.\n",TIMEOUT);
						counter ++;
					}
					else if(recievedpacket.type == ACKPACKET  ) 
					{
						print(data);
						printf("Acknoledgement recieved \n");
						ackn.segment_number = data.segment_number;
						ackprint(ackn);
						printf("\n");
					}
					else if(recievedpacket.type == REJECTPACKETCODE ) 
					{
						printf("Recieved a Reject Packet \n");
						rej.subcode=recievedpacket.subcode;
						rej.segment_number = data.segment_number;
						if(recievedpacket.subcode == LENGTHMISMATCHCODE ) 
						{
							printf("LENGTH MISMATCH ERROR\n");
						}
						if(recievedpacket.subcode == ENDPACKETIDMISSINGCODE ) 
						{
							printf("END OF PACKET IDENTIFIER MISSING ERROR \n");
						}
						if(recievedpacket.subcode == OUTOFSEQUENCECODE ) 
						{
							printf("OUT OF SEQUENCE ERROR \n");
						}
						if(recievedpacket.subcode == DUPLICATECODE) 
						{
							printf("DUPLICATE PACKET\n");
						}
						rejprint(rej);
					}
				}
				if(counter >= 3 ) 
				{
					printf("Server is down.\n");
					exit(0);
				}
				segmentNo++;
				printf("________________________________________________________________________________________\n");
			}
			break;
					
		case 3:
			data = initialise();
			ackn = ackinitialise();
			rej=rejinitialise();
			if(fgets(line, sizeof(line), fp) != NULL) 
			{
				n = 0;
				counter = 0;
				printf("%s",line);
				data.segment_number = segmentNo;
				strcpy(data.payload,line);
				data.length = strlen(data.payload);
				data.endpacketID = ENDPACKETID;
			}
			data.length++;   // Changing length of data
			while(n<=0 && counter<3)
			{
				sendto(Client_Socket,&data,sizeof(struct Datapacket),0,(struct sockaddr *)&cliaddr,addr_size);
				n = recvfrom(Client_Socket,&recievedpacket,sizeof(struct rejectpacket),0,NULL,NULL);
				if(n <= 0 )
				{
					printf("The server hasn't responded for %d seconds. Trying again.\n",TIMEOUT);
					counter ++;
				}
				else if(recievedpacket.type == ACKPACKET  ) 
				{
					print(data);
					printf("Acknoledgement recieved \n ");
					ackn.segment_number = data.segment_number;
					ackprint(ackn);
				}
				else if(recievedpacket.type == REJECTPACKETCODE ) 
				{
					printf("Recieved a Reject Packet \n");
					rej.subcode=recievedpacket.subcode;
					rej.segment_number = data.segment_number;
					if(recievedpacket.subcode == LENGTHMISMATCHCODE ) 
					{
						printf("LENGTH MISMATCH ERROR\n");
					}
					if(recievedpacket.subcode == ENDPACKETIDMISSINGCODE ) 
					{
						printf("END OF PACKET IDENTIFIER MISSING ERROR\n");
					}
					if(recievedpacket.subcode == OUTOFSEQUENCECODE ) 
					{
						printf("OUT OF SEQUENCE ERROR \n");
					}
					if(recievedpacket.subcode == DUPLICATECODE) 
					{
						printf("DUPLICATE PACKET\n");
					}
					rejprint(rej);
				}
			}
			if(counter >= 3 ) 
			{
				printf("Server is down.\n");
				exit(0);
			}
			segmentNo++;
			printf("________________________________________________________________________________________\n");
			break;

		case 4:
			data = initialise();
			ackn = ackinitialise();
			rej=rejinitialise();
			if(fgets(line, sizeof(line), fp) != NULL) 
			{
				n = 0;
				counter = 0;
				printf("%s",line);
				data.segment_number = segmentNo;
				strcpy(data.payload,line);
				data.length = strlen(data.payload);
				data.endpacketID = ENDPACKETID;
			}
			data.endpacketID= 0;  
			while(n<=0 && counter<3)
			{
				sendto(Client_Socket,&data,sizeof(struct Datapacket),0,(struct sockaddr *)&cliaddr,addr_size);
				n = recvfrom(Client_Socket,&recievedpacket,sizeof(struct rejectpacket),0,NULL,NULL);
				if(n <= 0 )
				{
					printf("The server hasn't responded for %d seconds. Trying again.\n",TIMEOUT);
					counter ++;
				}
				else if(recievedpacket.type == ACKPACKET  ) 
				{
					print(data);
					printf("Acknoledgement recieved \n ");
					ackn.segment_number = data.segment_number;
					ackprint(ackn);
				}
				else if(recievedpacket.type == REJECTPACKETCODE ) 
				{
					printf("Recieved a Reject Packet \n");
					rej.subcode=recievedpacket.subcode;
					rej.segment_number = data.segment_number;
					if(recievedpacket.subcode == LENGTHMISMATCHCODE ) 
					{
						printf("LENGTH MISMATCH ERROR\n");
					}
					if(recievedpacket.subcode == ENDPACKETIDMISSINGCODE ) 
					{
						printf("END OF PACKET IDENTIFIER MISSING ERROR\n");
					}
					if(recievedpacket.subcode == OUTOFSEQUENCECODE ) 
					{
						printf("OUT OF SEQUENCE ERROR \n");
					}
					if(recievedpacket.subcode == DUPLICATECODE) 
					{
						printf("DUPLICATE PACKET \n");
					}
					rejprint(rej);
				}
			}
			if(counter >= 3 ) 
			{
				printf("Server is down.\n");
				exit(0);
			}
			segmentNo++;
			printf("________________________________________________________________________________________\n");
			break;

		case 5:
			data = initialise();
			ackn = ackinitialise();
			rej=rejinitialise();
			if(fgets(line, sizeof(line), fp) != NULL) 
			{
				n = 0;
				counter = 0;
				printf("%s",line);
				data.segment_number = segmentNo;
				strcpy(data.payload,line);
				data.length = strlen(data.payload);
				data.endpacketID = ENDPACKETID;
			}
			data.segment_number = 3;
			while(n<=0 && counter<3)
			{
				sendto(Client_Socket,&data,sizeof(struct Datapacket),0,(struct sockaddr *)&cliaddr,addr_size);
				n = recvfrom(Client_Socket,&recievedpacket,sizeof(struct rejectpacket),0,NULL,NULL);
				if(n <= 0)
				{
					printf("ERROR! The server hasn't responded for %d seconds. Trying again.\n",TIMEOUT);
					counter ++;
				}
				else if(recievedpacket.type == ACKPACKET  ) 
				{
					print(data);
					printf("Acknoledgement recieved \n ");
					ackn.segment_number = data.segment_number;
					ackprint(ackn);
				}
				else if(recievedpacket.type == REJECTPACKETCODE ) 
				{
					printf("Recieved a Reject Packet \n");
					rej.subcode=recievedpacket.subcode;
					rej.segment_number = data.segment_number;
					if(recievedpacket.subcode == LENGTHMISMATCHCODE ) 
					{
						printf("LENGTH MISMATCH ERROR\n");
					}
					if(recievedpacket.subcode == ENDPACKETIDMISSINGCODE ) 
					{
						printf("END OF PACKET IDENTIFIER MISSING ERROR\n");
					}
					if(recievedpacket.subcode == OUTOFSEQUENCECODE ) 
					{
						printf("OUT OF SEQUENCE ERROR \n");
					}
					if(recievedpacket.subcode == DUPLICATECODE) 
					{
						printf("DUPLICATE PACKET\n");
					}
					rejprint(rej);
				}
			}
			if(counter >= 3 ) 
			{
				printf("Server is down.\n");
				exit(0);
			}
			segmentNo++;
			printf("_______________________________________________________________\n");
			break;
		case 6:
			data = initialise();
			ackn = ackinitialise();
			rej=rejinitialise();
			if(fgets(line, sizeof(line), fp) != NULL) 
			{
				n = 0;
				counter = 0;
				printf("%s",line);
				data.segment_number = segmentNo;
				strcpy(data.payload,line);
				data.length = strlen(data.payload);
				data.endpacketID = ENDPACKETID;
			}
			data.segment_number = 27;
			while(n<=0 && counter<3)
			{
				sendto(Client_Socket,&data,sizeof(struct Datapacket),0,(struct sockaddr *)&cliaddr,addr_size);
				n = recvfrom(Client_Socket,&recievedpacket,sizeof(struct rejectpacket),0,NULL,NULL);
				if(n <= 0)
				{
					printf("ERROR! The server hasn't responded for %d seconds. Trying again\n",TIMEOUT);
					counter ++;
				}
				else if(recievedpacket.type == ACKPACKET  ) 
				{
					print(data);
					printf("Acknoledgement recieved \n ");
					ackn.segment_number = data.segment_number;
					ackprint(ackn);
				}
				else if(recievedpacket.type == REJECTPACKETCODE ) 
				{
					printf("Recieved a Reject Packet \n");
					rej.subcode=recievedpacket.subcode;
					rej.segment_number = data.segment_number;
					if(recievedpacket.subcode == LENGTHMISMATCHCODE ) 
					{
						printf("LENGTH MISMATCH ERROR\n");
					}
					if(recievedpacket.subcode == ENDPACKETIDMISSINGCODE ) 
					{
						printf("END OF PACKET IDENTIFIER MISSING ERROR\n");
					}
					if(recievedpacket.subcode == OUTOFSEQUENCECODE ) 
					{
						printf("OUT OF SEQUENCE ERROR \n");
					}
					if(recievedpacket.subcode == DUPLICATECODE) 
					{
						printf("DUPLICATE PACKET\n");
					}
					rejprint(rej);
				}
			}
			if(counter >= 3 ) 
			{
				printf("Server is down.\n");
				exit(0);
			}
			segmentNo++;
			printf("_______________________________________________________________\n");
			break;
				
		case 2:
			for(pacCount=0;pacCount<5;pacCount++){
				data = initialise();
				ackn = ackinitialise();
				rej=rejinitialise();
				if(fgets(line, sizeof(line), fp) != NULL) 
				{
					n = 0;
					counter = 0;
					printf("%s",line);
					data.segment_number = segmentNo;
					strcpy(data.payload,line);
					data.length = strlen(data.payload);
					data.endpacketID = ENDPACKETID;
				}
				switch(pacCount+1){
					case 1:
						data.length++;
						break;
					case 2:
						data.endpacketID= 0;
						break;
					case 3:
						data.segment_number = 1;
						break;
					case 5:
						data.segment_number = 20;
						break;
				}
				
				while(n<=0 && counter<3)
				{
					sendto(Client_Socket,&data,sizeof(struct Datapacket),0,(struct sockaddr *)&cliaddr,addr_size);
					n = recvfrom(Client_Socket,&recievedpacket,sizeof(struct rejectpacket),0,NULL,NULL);
					if(n <= 0 )
					{
						printf("The server hasn't responded for %d seconds. Trying again.\n",TIMEOUT);
						counter ++;
					}
					else if(recievedpacket.type == ACKPACKET  ) 
					{
						print(data);
						printf("Acknoledgement recieved \n ");
						ackn.segment_number = data.segment_number;
						ackprint(ackn);
					}
					else if(recievedpacket.type == REJECTPACKETCODE ) 
					{
						printf("Recieved a Reject Packet \n");
						rej.subcode=recievedpacket.subcode;
						rej.segment_number = data.segment_number;
						if(recievedpacket.subcode == LENGTHMISMATCHCODE ) 
						{
							printf("LENGTH MISMATCH ERROR\n");
						}
						if(recievedpacket.subcode == ENDPACKETIDMISSINGCODE ) 
						{
							printf("END OF PACKET IDENTIFIER MISSING \n");
						}
						if(recievedpacket.subcode == OUTOFSEQUENCECODE ) 
						{
							printf("OUT OF SEQUENCE\n");
						}
						if(recievedpacket.subcode == DUPLICATECODE) 
						{
							printf("DUPLICATE PACKET\n");
						}
						rejprint(rej);
					}
				}
				if(counter >= 3 ) 
				{
					printf("ERROR! Server is down.\n");
					exit(0);
				}
				segmentNo++;
				printf("___________________________________________________________\n");
			}
			break;

		default:
			printf("\n \n Please select a valid option \n \n!");
		}
	}
}
