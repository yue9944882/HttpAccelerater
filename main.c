
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


#define HEAD_GRAM "HEAD %s HTTP/1.1\r\nHost: %s\r\nUser-Agent: %s\r\n\r\n"

#define GET_GRAM "GET %s HTTP/1.1\r\nHost: %s\r\nUser-Agent: %s\r\nRange: bytes=%lld-\r\n\r\nConnection: close\r\n\r\n"


/* Global Variable Defination */


struct URLinfo{

//// info analysis from url pattern
	char szHostname[1024];
	char szURLname[1024];
	char szFilename[1024];
	char szIPv4addr[16];

	int iPort;

//// info analysis from head reply
	long long llContentLen;

}gURLinfo;




struct Threadinfo{
	struct sockaddr_in sin;
	long long llBeginPos;
	long long llCurrentPos;
	long long llEndPos;
	long long llContentLen;
	pthread_t tid;
	int iFlag;
}gThreadinfo;








/***********
 *	Main Entrance
 ***********/

int main(int argc,char**argv){
	
	int i,j;

	////Analysing the parameters	



	////URL parsing
	if(strlen(argv[1])<7||strncmp(argv[1],"http://",7)!=0){
		fprintf(stderr,"Http Support Only( Not Https )\n");
		exit(-1);
	}
		
	gURLinfo.iPort=80;
	
	char*s=argv[1]+7;

	for(i=0;*s!='/'&&i<1024;s++,i++){
		gURLinfo.szHostname[i]=*s;
	}gURLinfo.szHostname[i]='\0';

	for(i=0;*s!='\0'&&i<1024;s++,i++){
		gURLinfo.szURLname[i]=*s;
	}gURLinfo.szURLname[i]='\0';

	--s;

	char tmpname[1024]={};

	for(i=0;*s!='/'&&i<1024;i++,s--){
		tmpname[i]=*s;
	}tmpname[i]='\0';

	int tmplen=strlen(tmpname);

	for(j=0,i=tmplen-1;i>=0;i--,j++){
		gURLinfo.szFilename[j]=tmpname[i];
	}gURLinfo.szFilename[j]='\0';

	printf("Hostname:\t%s\nURLname:\t%s\nFilename:\t%s\n",gURLinfo.szHostname,gURLinfo.szURLname,gURLinfo.szFilename);

	////Headers send & recv

	char*sendBuf=(char*)malloc(1024*sizeof(char));
	char*recvBuf=(char*)malloc(1024*sizeof(char));
	
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
		exit(-1);
	}

	if(connect(sockdesc,(const struct sockaddr*)sock,sizeof(struct sockaddr_in))==-1){
		fprintf(stderr,"Socket Connection Failure!\n");
		exit(-1);
	}

	printf("Connect Successfully!\n");

	sprintf(sendBuf,HEAD_GRAM,gURLinfo.szURLname,gURLinfo.szHostname,"HttpDownloader");

	if(send(sockdesc,sendBuf,strlen(sendBuf),0)==-1){
		fprintf(stderr,"Header Sending Failure!\n");
		exit(-1);
	}

	if(recv(sockdesc,recvBuf,1024,0)==-1){
		fprintf(stderr,"Header Recving Failure!\n");
		exit(-1);
	}
	
	char*tok=strtok(recvBuf,"\r\n");
	if(strstr(tok,"HTTP/1.1 200")!=NULL){
		while(tok=strtok(NULL,"\r\n")){
			if(strncasecmp(tok,"Content-Length:",15)==0){
				char*tmp=(tok+15);
				gURLinfo.llContentLen=atoll(tmp);
			}
		}
	}else{
		fprintf(stderr,"Connection Rejected Failure!\n");
		exit(-1);
	}
	
	printf("HeaderLen:%lld\n",gURLinfo.llContentLen);

	free(sendBuf);
	free(recvBuf);

	////This line may never reach
	return 0;

}





