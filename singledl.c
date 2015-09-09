
#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<unistd.h>

#include<netinet/in.h>
#include<signal.h>
#include<resolv.h>
#include<netdb.h>

#include<sys/types.h>
#include<sys/socket.h>

#include<arpa/nameser.h>
#include<arpa/inet.h>


#include"constdef.h"


int singledl(void){
	
	char*sendBuf=(char*)malloc(sizeof(char)*4096);
	char*recvBuf=(char*)malloc(sizeof(char)*4096);

	struct hostent*host=gethostbyname(gURLinfo.szHostname);
	if(host==NULL){
		fprintf(stderr,"Resolve Hostname Failure!\n");
		exit(-1);
	}

	strncpy(gURLinfo.szIPv4addr,inet_ntoa(*(struct in_addr*)host->h_addr),16);

	struct sockaddr_in*sock=(struct sockaddr_in*)malloc(sizeof(struct sockaddr_in));

	bzero(sock,sizeof(struct sockaddr_in));

	sock->sin_family=AF_INET;
	sock->sin_addr.s_addr=inet_addr(gURLinfo.szIPv4addr);
	sock->sin_port=htons(gURLinfo.iPort);

	int sockdesc=socket(AF_INET,SOCK_STREAM,0);

	if(sockdesc=-1){
		fprintf(stderr,"Socket Creation Failure!\n");
		exit(-1);
	}

	if(connect(sockdesc,(const struct sockaddr*)sock,sizeof(struct sockaddr_in))==-1){
		fprintf(stderr,"Socket Creation Failure!\n");
		exit(-1);
	}

	

	free(sendBuf);
	free(recvBuf);

	return 0;
}




