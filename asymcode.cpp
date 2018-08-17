#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <iostream>
#include <fstream>
//#define NDEBUG
//#define BOOST_UBLAS_NDEBUG
#include <boost/numeric/ublas/vector.hpp>
#include <boost/numeric/ublas/matrix.hpp>
#include <boost/numeric/ublas/io.hpp>

#define DEBUG  0

using namespace boost::numeric::ublas ;

#define WIDTH 10

#define CODEWIDTH 5// 256

   matrix<int> H(1<<WIDTH,WIDTH);
   matrix<int> onecount(1<<WIDTH,2);
   matrix<int> CODE(1<<CODEWIDTH,WIDTH);

//genera matriz de codigo
void genMatrix()
{
int i,x;
  for(i=0;i<(1<<WIDTH);i++)
	for(x=0;x<WIDTH;x++)
		H(i,x) = (i & (1<<x))? 1:0;
}

int hammingAsym(vector<int> V1,vector<int>V2)
{
unsigned int i,cnt;
unsigned int max=V1.size();
if (V2.size()<max) max=V2.size();
for (i=0,cnt=0;i<max;i++)	
	if (V1(i)!=V2(i))
		cnt++;
return cnt;
}


// Genera codigo con mas ceros que unos
void genCode()	{
  int i,x;

  // Genera matrix
  genMatrix();

  // calcula numero de unos por codeword
  for(i=0;i<(1<<WIDTH);i++)	{
	int cnt=0;
	for(x=0;x<WIDTH;x++)
		cnt+=H(i,x);
	onecount(i,0)=cnt;
	onecount(i,1)=i;
	}
//std::sort(onecount.data().begin(),onecount.data().end());

// elimina los que tienen demasiados unos
int count= 1<<WIDTH; // unos iniciales
int target= 1<<CODEWIDTH; //objetivo (cantidad de codewords que hay que lograr, tipicament la mitad)

for (x=WIDTH;x>0;x--)
  {
  for(i=0;i<(1<<WIDTH);i++)	{
	if (onecount(i,0)==x) {
		onecount(i,0)=-1;
		count--;
		if (count==target)
			break;
		}
	}
  if (count==target)
	break;
  }

 // Copiamos los bits que tienen menos ceros a la matriz CODE
  for(i=0,count=0;i<(1<<WIDTH);i++)	{
	//printf("%d , %d\n",onecount(i,0),onecount(i,1));
	if (onecount(i,0)>0)	{
		for(x=0;x<WIDTH;x++)
			CODE(count,x)=H(i,x);
		count++;
		}
	}

}

int main(int argc, char **argv) 
{
vector<int> V1(WIDTH),V2(WIDTH);
unsigned int len,i,val,q;
unsigned char in[CODEWIDTH];
unsigned char out[WIDTH];

genCode();

if (strstr(argv[0],"asymenc"))
	{ // codifica un canal
	while((len=fread(in,1,CODEWIDTH,stdin))==CODEWIDTH)	{
			// bits->val
			for (i=0,val=0;i<CODEWIDTH;i++)
				val+=(in[i]-0x30)<<i;

			// val->out
			for (i=0;i<WIDTH;i++)
				out[i]=CODE(val,i)+0x30;

			if ((len=write(1,out,WIDTH)) !=WIDTH)
				fprintf(stderr,"Error: write\n");
		}
	}
else	{// decodifica un bloomfilter en un canal
	int total=0,ones=0;
	while (1)	{
			len=fread(out,1,WIDTH,stdin);
			if (len!=WIDTH) //exit 
				{
				fprintf(stderr,"relacion unos/ceros: %f\n",((float)ones)/((float)total));
				return 0;
				}
			// Buscamos cual codigo se parece mas buscando el hamming minimo
			int minhamming=999,selected=0,ham;

			total+=WIDTH;
			for (i=0;i<WIDTH;i++) {
				V1(i)=out[i]-0x30;
				if (out[i]=='1') ones++;	
				}

			for (i=0;i<(1<<CODEWIDTH);i++) {
				// Convertimos fila de la matriz en vector para llamar a hamAsym
				for (q=0;q<WIDTH;q++) 
					V2(q)=CODE(i,q);
				ham=hammingAsym(V1,V2);
				if (ham<minhamming)
					{
					minhamming=ham;
					selected=i;
					}
				}
			if (minhamming>0) // se produjo un error
				fprintf(stderr,"Error: %d bits\n",minhamming);
			// Tenemos el codeword que mas se aproxima a la entrada,
			// convertimos a bits y sacamos
							
			for (i=0;i<CODEWIDTH;i++)	{ 
				in[i]=selected&(1<<i)?'1':'0';
				}

			if ((len=write(1,in,CODEWIDTH)) !=CODEWIDTH)
				fprintf(stderr,"Error: write\n");
		}
	}
return 0;
} 
