#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#define BITS 8

int countbits(char *digits, int span)	{
int i,count=0;
for (i=0;i<BITS;i++) 
	if (digits[i]) count++;
return count;
}

long long int value(char *digits, int x)	{
int c=1,i;
long long int result=0;
for (i=0;i<BITS;i++) {
	switch (digits[i])	{
		case 1: result+=c*1+x;
			break;
		case 2: result-=c*1-x;
			break;
		case 3: result+=c*3+sin(x);
			break;
		case 4: result-=c*3-sin(x);
			break;
		case 5: result+=c*5+x;
			break;
		case 6: result-=c*5-x;
			break;
		}
	c+=c;
	}
return result;
}

void increment(char *digits,int span) {
unsigned int i,q;
i=0;
while(i<BITS)	{
	if (digits[i]<4) {
			digits[i]++;
			break;
			}
	else		{
			digits[i]=0;
			i++;
			}
	}

}

int main(void)	{
printf("Signed digits bit counter: \n");
int i;
char digits[BITS+1];
char *resultdigits[65536];
unsigned char results[65536];
int bits;
long long int numbervalue;

for (i=0;i<65536;i++) resultdigits[i]=(char *)malloc(BITS+1);

int x,tries,q,minbits=9999;
int counter;
for (x=1;x<300;x++) {
	memset(digits,0,sizeof(digits));
	memset(results,255,sizeof(results));
	tries=1;
	//for(i=0;i<x;i++,tries+=tries);
	tries=16777216/16;
	for (i=0;i<tries;i++) {
		numbervalue=value(digits,x);
		bits=countbits(digits,0);
		if ((numbervalue<65535) && (numbervalue>=0))
			if (results[numbervalue]>bits)	{
				results[numbervalue]=bits;
				memcpy(resultdigits[numbervalue],digits,BITS);
				}
		increment(digits,0);
		}
	counter=0;
	for (i=0;i<256;i++) {
		if ((x==108)) {
			//printf("Val: %d bits: %d ",i,results[i]);
			printf("{");
			for (q=0;q<BITS-1;q++) printf("'%d',",resultdigits[i][q]);
			printf("'%d'},\n",resultdigits[i][BITS-1]);
			}
		counter+=results[i];
		}
	if (minbits>counter) minbits=counter;
	printf("Total: tries: %d bits %d %d min %d\n",tries,counter,x,minbits);
	}
return 0;
}
