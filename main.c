#include"stdio.h"
#include"preconn.h"
#include"singledl.h"
#include"multidl.h"





/***********
 *	Main Entrance
 ***********/

int main(int argc,char**argv){

	////Analysing the parameters	
	preconn(argv[1]);
	//singledl();
	multidl();
	
	return 0;
}



