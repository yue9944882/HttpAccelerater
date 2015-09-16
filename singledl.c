
#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<unistd.h>

#include<netinet/in.h>
#include<signal.h>
#include<resolv.h>
#include<netdb.h>
#include<errno.h>
#include<fcntl.h>


#include<sys/types.h>
#include<sys/socket.h>

#include<arpa/nameser.h>
#include<arpa/inet.h>


#include"constdef.h"


int singledl(void){

	long long dr=0,dw=0;

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

	if(sockdesc==-1){
		fprintf(stderr,"Socket Creation Failure!\n");
		printf("%d\n...",errno);
		exit(-1);
	}

	if(connect(sockdesc,(const struct sockaddr*)sock,sizeof(struct sockaddr_in))==-1){
		fprintf(stderr,"Socket Connection Failure!\n");
		exit(-1);
	}

	bzero(sendBuf,4096);

	
	sprintf(sendBuf,GET_GRAM,gURLinfo.szURLname,gURLinfo.szHostname,"HttpDownloader",0);


	printf("%s",sendBuf);

	if(send(sockdesc,sendBuf,strlen(sendBuf),0)==-1){
		fprintf(stderr,"Header Sending Failure!\n");
		exit(-1);
	}

	if((dr=recv(sockdesc,recvBuf,4096,0))==-1){
		fprintf(stderr,"Header Recving Failure!\n");
		exit(-1);
	}
	



	////Strip the HTTP header
	

	char*skiphead=recvBuf;
	long long headlength=0;
	long long curPos=0;

	printf("%s\n",recvBuf);

	while(1){
		if(*skiphead=='\n'&&*(skiphead-1)=='\r'&&*(skiphead-2)=='\n'&&*(skiphead-3)=='\r'){
			skiphead++;
			headlength++;
			break;
		}
		skiphead++;
		headlength++;
	}


	char*tmpptr=skiphead;
	long long reallen=0;


	/*while(1){
		if(*tmpptr=='\0'||reallen>=4096)break;
		reallen++;
		tmpptr++;
	}*/

	printf("\nHead Length:\t%d\n",headlength);
	//printf("Real Length:\t%d\n",reallen);

	/* File IO */

	int file=open(gURLinfo.szFilename,O_CREAT|O_RDWR,S_IRWXU);
	
	if(file==-1){
		fprintf(stderr,"File Open Failure!\n");
		exit(-1);
	}

	dw=pwrite(file,skiphead,dr-headlength,0);

	printf("first time dw:\t%d\n",dw);

	curPos+=dw;

	printf("\n%s\n",recvBuf);

	printf("offset:\t%d\ndw:\t%d\n",curPos,dw);

	while(curPos<gURLinfo.llContentLen){
		dr=recv(sockdesc,recvBuf,4096,0);
		if(dr+curPos>gURLinfo.llContentLen){
			dw=pwrite(file,recvBuf,gURLinfo.llContentLen-headlength-curPos,curPos);	
			curPos+=dw;
			//printf("offset:\t%d\ndw:\t%d\n",curPos,dw);
			break;
		}else{
			dw=pwrite(file,recvBuf,dr,curPos);
			curPos+=dw;
		}
		//printf("offset:\t%d\ndw:\t%d\n",curPos,dw);
	}
	



	close(file);

	free(sendBuf);
	free(recvBuf);
	return 0;
}




