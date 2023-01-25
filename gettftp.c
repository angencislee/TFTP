#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <netdb.h>
#include <string.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <fcntl.h>

// ./gettftp addres server filename
int main(int argc, char *argv[]) {  //Q1 
	argc++;
	char* server = argv[1];
	char* filename= argv[2];

	
	char RRQ[256];	
	
	struct addrinfo filtre,* result;
	int code_error, sock;

	//2. Call getaddrinfo to get the server address;
	memset(&filtre, 0,sizeof(filtre));//The function set all the value at 0, that allow to initialized the filter.
	filtre.ai_family = AF_UNSPEC;    /* allow IPv4 or IPv6 */
	filtre.ai_socktype = SOCK_DGRAM; 
	filtre.ai_flags = AI_PASSIVE;    
	filtre.ai_protocol = 0;          
	filtre.ai_canonname = NULL;
	filtre.ai_addr = NULL;
	filtre.ai_next = NULL;
	
	//We obtain the server address
	code_error=getaddrinfo(server,"1069",&filtre,&result);
	if (code_error != 0) {
		fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(code_error));
		exit(EXIT_FAILURE);
	}else{
		fprintf(stderr, "getaddrinfo: ok\n");
	}
	
	if(result ==NULL){
		fprintf(stderr, "we can't connect \n");
		exit(EXIT_FAILURE);
	}else{
		fprintf(stderr, "connection: ok\n");
	}
	//3. Creation of a connection socket to the server;
	sock= socket(result->ai_family, result-> ai_socktype, result->ai_protocol);
	//error of creation of the socket
	if(sock==-1){
		fprintf(stderr, "error of creation of socket\n");
		exit(EXIT_FAILURE);
	}
	//4.
	
	// read request
	RRQ[0]=0;RRQ[1]=1; 							//opcode		2 bytes 	1 Read request (RRQ)||2 Write request (WRQ)||3 Data (DATA) ||4 Acknowledgment (ACK)||5 Error (ERROR)
	strcpy(&RRQ[2],filename);					//filename		string
	RRQ[2+strlen(filename)]=0;					//0				1 byte	
	strcpy(&RRQ[3+strlen(filename)],"octet");	//Mode			string
	RRQ[8+strlen(filename)]=0;					//0				1 byte 
	int lenght_RRQ= 9+strlen(filename);
	
	if(!sendto(sock,RRQ,lenght_RRQ,0,result->ai_addr, result->ai_addrlen)){
		fprintf(stderr, "read request error \n");
	}
	

	char recv[516];


	int lenght_recv=recvfrom(sock, recv, 516, 0,result->ai_addr,&(result->ai_addrlen));
	//2 bytes -> Opcode    2 bytes  -> Block #     n bytes -> Data

	fprintf(stderr,"Opcode: %d,%d  Block %d,%d\nData lenght : %d\n",recv[0],recv[1],recv[2],recv[3],lenght_recv-4);
	
	char ACK[100];
	ACK[0]=0;ACK[1]=4; 				//opcode -> 2 bytes
	
	int lenght_ACK=4;
	int file =open(filename,O_CREAT|O_WRONLY,0x777);
	write(file,recv+4,lenght_recv-4);
	while(lenght_recv==516){
		ACK[2]=recv[2];ACK[3]=recv[3];		
		
		sendto(sock,ACK,lenght_ACK,0,result->ai_addr, result->ai_addrlen);
		
		lenght_recv=recvfrom(sock, recv, 516, 0,result->ai_addr,&(result->ai_addrlen));
		write(file,recv+4,lenght_recv-4);
		fprintf(stderr,"Opcode: %d,%d  Block %d,%d\Data lenght : %d\n",recv[0],recv[1],recv[2],recv[3],lenght_recv-4);
				
	}
	exit(EXIT_SUCCESS); 	
}