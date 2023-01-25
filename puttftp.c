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
int main(int argc, char *argv[]) {
	argc++;
	char* server = argv[1];
	char* filename= argv[2];
	
	int size_file=0;
	
	char WRQ[256];
		
	struct addrinfo filtre,* result;
	int code_error, sock;
	
	memset(&filtre, 0,sizeof(filtre));
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
	//*Creation of a connection socket
	sock= socket(result>ai_family, result-> ai_socktype, result->ai_protocol);
	//error of creation of the socket
	if(sock==-1){
		fprintf(stderr, "error of creation of socket\n");
		exit(EXIT_FAILURE);
	}
	
	
	//5. Write request
	WRQ[0]=0;WRQ[1]=2; 							//opcode		2 bytes 	1 Read request (RRQ)||2 Write request (WRQ)||3 Data (DATA) ||4 Acknowledgment (ACK)||5 Error (ERROR)
	strcpy(&WRQ[2],filename);					//filename		string
	WRQ[2+strlen(filename)]='\0';				//0				1 byte	
	strcpy(&WRQ[3+strlen(filename)],"octet");	//Mode			string
	WRQ[8+strlen(filename)]='\0';				//0				1 byte 
	int lenght_WRQ= 9+strlen(filename);
	
	if(!sendto(sock,WRQ,lenght_WRQ,0,result->ai_addr, result->ai_addrlen)){
		fprintf(stderr, "write request error \n");
	}
		
	char recv[516];
	int lenght_recv=recvfrom(sock, recv, 15, 0,result->ai_addr,&(result->ai_addrlen));
	fprintf(stderr,"Opcode: %d,%d  Block %d,%d \nData lenght: %d\n",recv[0],recv[1],recv[2],recv[3],lenght_recv-4);
	size_file=size_file+lenght_recv-4;
		
	int file = open(filename,O_RDWR,0x777);
	char DATA[516];
	memset( DATA, '\0', sizeof(DATA) );	 
	DATA[0]=0;DATA[1]=3; 				//opcode -> 2 bytes
	int block =1;
	DATA[2]=0,DATA[3]=block;	
	int nb_read =read(file,&DATA[4],512);				
	fprintf(stderr,"nbread: %d\n",nb_read);
	sendto(sock,DATA,nb_read+4,0,result->ai_addr, result->ai_addrlen);	
	recvfrom(sock, recv, 15, 0,result->ai_addr,&(result->ai_addrlen));
	fprintf(stderr,"Opcode: %d,%d  Block %d,%d \nData lenght: %d\n",recv[0],recv[1],recv[2],recv[3],lenght_recv-4);
	
	while(nb_read==512){
		block++;
		DATA[2]=0,DATA[3]=block;
		nb_read =read(file,&DATA[4],512);
		fprintf(stderr,"nbread: %d\n",nb_read);				
		sendto(sock,DATA,nb_read+4,0,result->ai_addr, result->ai_addrlen);
		recvfrom(sock, recv, 15, 0,result->ai_addr,&(result->ai_addrlen));
		fprintf(stderr,"Opcode: %d,%d  Block %d,%d \nData lenght: %d\n",recv[0],recv[1],recv[2],recv[3],lenght_recv-4);				
		
	}

}