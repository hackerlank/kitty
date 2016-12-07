#include <unistd.h>
#include <iostream>
#include <vector>
#include <map>
//#include "zArray.h"

void printfmsg(int p1, bool=true, unsigned int dwTempID=0, unsigned char byType=0, bool addCartoon=false, bool=false, const char* desc=NULL);

int main(int argc,char *argv[])
{
/*	
	array<int> ta(10);
	memcpy(&ta[0], &tb[0], sizeof(int)*15);
	for (int i=0; i<16; i++)
		std::cout << ta.storage[i] << std::endl;
		*/

/*	
	__asm__
	(
	 "cld"
	 );
	 */



	printfmsg(1,true, 0, 0, true, true, __PRETTY_FUNCTION__);
	

	return 0;
}


void printfmsg(int p1, bool addPet, unsigned int dwTempID, unsigned char byType, bool addCartoon, bool adult, const char* desc)
{
	std::cout << p1 << addPet << dwTempID << byType << addCartoon << adult << desc <<std::endl;
}

