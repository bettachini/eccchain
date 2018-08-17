#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#define MAXBLOCKSIZE 65536

int main(int argc, char **argv) 
{
int len1,len2,i;
unsigned char buf1[MAXBLOCKSIZE],buf2[MAXBLOCKSIZE];
FILE *file1,*file2;

if (argc!=3)	{
	printf("Usage: %s file1 file2\n",argv[0]);
	exit(1);
	}

float errors=0,count=0;

file1=fopen(argv[1],"r");
file2=fopen(argv[2],"r");

if (strstr(argv[0],"ber"))
	{ // ber of bits
	while((len1=fread(buf1,1,MAXBLOCKSIZE,file1))>0)	{
		len2=fread(buf2,1,MAXBLOCKSIZE,file2);
		if (len2<len1) len1=len2;
		for (i=0;i<len1;i++)	{
			count++;
			if (buf1[i]!=buf2[i])
				errors++;
			}
		}
	printf("%f\n",errors/count);
	}
return 0;
}
