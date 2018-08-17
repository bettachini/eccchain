/* Minimal ones table generator */
/* Alfredo Ortega  2009    */


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#define BITS 7
#define BITWIDTH 4

// Perfecto 256
//#define BITS 8
//#define BITWIDTH 3


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
	if (digits[i]<BITWIDTH) {
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
printf("Tabla de unos minimos: \n");
int i;
char digits[BITS+1];
unsigned int results[65536];
int bits;
long long int numbervalue;

int x,tries,q,minbits=9999;
int counter,allowprint;
int bits3=0;
	memset(digits,0,sizeof(digits));
	memset(results,0,sizeof(results));
	tries=pow(BITWIDTH+1,BITS);// 16777216/16;
	printf("Tries: %d\n",tries);
	for (i=0;i<tries;i++) {
		//numbervalue=value(digits,x);
		bits=countbits(digits,0);
		results[bits]++;
		allowprint=0;
		if (bits<=2) allowprint=1;
		if (bits==3) {
			bits3++;
			if (bits3<15)
				allowprint=1;
			}
		if (allowprint) {
			printf(" {");
			for (q=0;q<BITS-1;q++) printf("'%d',",digits[q]);
			printf("'%d'},\n",digits[BITS-1]);
			}

		increment(digits,0);
		}
	counter=0;
	printf("Words / unos\n");
	for (i=0;i<BITS;i++) 
		printf(" %d: %d\n",i,results[i]);
return 0;
}
