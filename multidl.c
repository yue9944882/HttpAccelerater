
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
#include<pthread.h>

#include<sys/types.h>
#include<sys/socket.h>

#include<arpa/nameser.h>
#include<arpa/inet.h>


#include"constdef.h"



int partition();

void*partget(void*);


int multidl(void){

	int iPart=partition();
	struct Threadinfo*tis=(struct Threadinfo*)malloc(sizeof(struct Threadinfo)*iPart);
	
	////Begin Offset & Ending Offset Calculating
	int i;
	for(i=0;i<iPart;i++){
		tis[i].llBeginPos=i*(gURLinfo.llContentLen/iPart);
		tis[i].llEndPos=(i+1)*(gURLinfo.llContentLen/iPart);
		if(i==iPart-1)tis[0].llEndPos=gURLinfo.llContentLen;
		printf("Thread %d : %d - %d\n",i,tis[i].llBeginPos,tis[i].llEndPos);
	}
	
	
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

	for(i=0;i<iPart;i++){
		tis[i].sin=*sock;
		//create
		tis[i].msg=(char*)malloc(sizeof(char)*4096);
		sprintf(tis[i].msg,GET_GRAM,gURLinfo.szURLname,gURLinfo.szHostname,"HttpDownloader",tis[i].llBeginPos);
		pthread_create(&(tis[i].tid),NULL,partget,&(tis[i]));
	}


	///wait for all the downloading threads
	
	int iSuccess=0;
	for(i=0;i<iPart;i++){
		pthread_join(tis[i].tid,NULL);
		if(tis[i].iFlag==1)iSuccess++;
	}

	if(iSuccess==iPart)printf("Download Success!\n");
	else printf("Download Failure!\n");

	free(sendBuf);
	free(recvBuf);
	free(tis);
	return 0;
}



int partition(){
	if(gURLinfo.llContentLen>1024*1024)return 20;
	else if(gURLinfo.llContentLen>512*1024)return 10;
	else if(gURLinfo.llContentLen>128*1024)return 5;
	else return 3;
}



void*partget(void*arg){

	struct Threadinfo*ti=(struct Threadinfo*)arg;

	int dw=0,dr=0;

	ti->tid=pthread_self();
	
	char*sendBuf=(char*)malloc(sizeof(char)*4096);
	char*recvBuf=(char*)malloc(sizeof(char)*4096);

	int sockdesc=socket(AF_INET,SOCK_STREAM,0);
	if(sockdesc==-1){
		fprintf(stderr,"Socket Creation Failure!\n");
		exit(-1);
	}

	if((connect(sockdesc,(const struct sockaddr*)(&ti->sin),sizeof(struct sockaddr_in)))==-1){
		fprintf(stderr,"Socket Connection Failure!\n");
		exit(-1);
	}

	bzero(sendBuf,4096);

	sprintf(sendBuf,GET_GRAM,gURLinfo.szURLname,gURLinfo.szHostname,"HttpDownloader",0);

	if((send(sockdesc,sendBuf,strlen(sendBuf),0))==-1){
		fprintf(stderr,"Header Sending Failure!\n");
		exit(-1);
	}

	if((dr=recv(sockdesc,recvBuf,4096,0))==-1){
		fprintf(stderr,"Header Recving Failure!\n");
		exit(-1);
	}

	char*skiphead=recvBuf;
	long long headlength=0;
	long long curPos=0;

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
	
	ti->llCurrentPos=ti->llBeginPos;

	int file=open(gURLinfo.szFilename,O_CREAT|O_RDWR,S_IRWXU);

	if(file==-1){
		fprintf(stderr,"File Open Failure!\n");
		exit(-1);
	}
	if(dr-headlength>ti->llEndPos)dw=pwrite(file,skiphead,ti->llEndPos-headlength,ti->llCurrentPos);
	else dw=pwrite(file,skiphead,dr-headlength,ti->llCurrentPos);

	ti->llCurrentPos+=dw;


	printf("%d - %d First Time DW: %d\n",ti->llBeginPos,ti->llEndPos,dw);

	while(ti->llCurrentPos<ti->llEndPos){
		dr=recv(sockdesc,recvBuf,4096,0);
		
		if(dr==0||dr==-1)printf("?????");
		if(dr+ti->llCurrentPos>gURLinfo.llContentLen){
			dw=pwrite(file,recvBuf,gURLinfo.llContentLen-headlength-ti->llCurrentPos,ti->llCurrentPos);
			ti->llCurrentPos+=dw;
			break;
		}else{
			dw=pwrite(file,recvBuf,dr,ti->llCurrentPos);
			ti->llCurrentPos+=dw;
		}
	}

	ti->iFlag=1;//for Okay

	close(file);

	free(sendBuf);
	free(recvBuf);

}




