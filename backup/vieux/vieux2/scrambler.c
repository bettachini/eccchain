#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#define MAXBLOCKSIZE 65536

int main(int argc, char **argv) 
{
int len,i;
unsigned char buf[MAXBLOCKSIZE],c;
unsigned int perm[MAXBLOCKSIZE];

int blocksize=4096;

// Read lenght of input block (max MAXBLOCKSIZE)
if (argc==2)	
	blocksize=atoi(argv[1]);

// create permutations
srand(0);
for (i=0;i<blocksize;i++)
	perm[i]=rand() % blocksize;	

if (strstr(argv[0],"scrambler"))
	{ // scrambler
	//fprintf(stderr," scrambling block: %d\n",blocksize);
	while((len=fread(buf,1,blocksize,stdin))>0)	{
		for (i=0;i<blocksize;i++)	{
			// swap
			c=buf[perm[i]];
			buf[perm[i]]=buf[i];
			buf[i]=c;
			}
		write(1,buf,blocksize);
		}
	}
else	{// decodifica reed-solomon gf(8)
	while((len=fread(buf,1,blocksize,stdin))==blocksize)	{
		for (i=blocksize-1;i>=0;i--)	{
			// swap
			c=buf[perm[i]];
			buf[perm[i]]=buf[i];
			buf[i]=c;
			}
		write(1,buf,blocksize);
		}
	}
return 0;
}
