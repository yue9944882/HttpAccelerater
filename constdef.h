#ifndef CONSTDEF_H
#define CONSTDEF_H


#define HEAD_GRAM "HEAD %s HTTP/1.1\r\nHost: %s\r\nUser-Agent: %sConnection: close\r\n\r\n"

#define GET_GRAM "GET %s HTTP/1.1\r\nHost: %s\r\nUser-Agent: %s\r\nRange: bytes=%lld-\r\nConnection: keep-alive\r\n\r\n"


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
	//long long llContentLen;
	char*msg;
	pthread_t tid;
	int iFlag;
};


//long long*llThreadOffset;



#endif
