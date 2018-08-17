#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
//#include "mtrand/mtrand.h"


//************
//
//Compilar con:
//
//gcc -o encoder template.c
//gcc -o decoder template.c
//
//El programa es el mismo, pero segun el nombre (que esta en argv[0]) hace una u otra cosa.
//Tambien es mucho mas eficiente hacer un hard-link que hacer una copia entera del ejecutable.
//
//************

#define NOMBRE "testenc"
#define BUFFERMAX 256

int main(int argc, char **argv) 
{
int len;
unsigned char buf[BUFFERMAX];
int maxdata=BUFFERMAX;

char pbuf=&buf;

if (strstr(argv[0],NOMBRE))
	{ // codifica
	count=0;
	while((len=fread(buf,1,maxdata,stdin))>0)	{
		//****************** 
		//	Hacer encoder aca!!
		//
		//
		//******************
		write(1,buf,len);
//		printf("len: %i\n",len);
		}
	}
else	{// decodifica 
	while((len=fread(buf,1,maxdata,stdin))==maxdata)	{
 		printf("%s\n",argv[0]);
		//****************** 
		//	Hacer decoder aca!!
		//
		//
		//******************
		write(1,buf,maxdata);
		}
	}
return 0;
}

