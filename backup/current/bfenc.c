#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#include <boost/random/mersenne_twister.hpp>
#include <boost/random/uniform_int.hpp>
#include <boost/random/variate_generator.hpp>


  boost::mt19937 rng;                 // produces randomness out of thin air
                                      // see pseudo-random number generators
  boost::uniform_int<> six(0,256*256*256);  // distribution that maps to 1..6
                                      // see random number distributions
  boost::variate_generator<boost::mt19937&, boost::uniform_int<> >
           irand(rng, six);             // glues randomness with mapping



/* prng usando rand() */
void lfsr(unsigned short int *reg)      {
        *reg=irand();
        }


        #define M 256
        #define K 3
        #define CLIENTS M
	#define BITCOUNT 1
        #define DEBUG 0

void bloomenc(unsigned char *bit,unsigned bitcount,unsigned char *bloomFilter, unsigned clients)
	{
        unsigned int i,q,k;
        unsigned char clientData[CLIENTS][bitcount]; // Datos a transmitir

        /* Seeds para el lfsr */
        unsigned short int clientSeeds[CLIENTS][K];
	/* Posiciones de nuestro bloomfilter */
	unsigned short int clientPos[K];

        /* Inicializamos PRNG de los clientes */
        for(i=0;i<clients;i++)
                for (q=0;q<K;q++)
                        clientSeeds[i][q]=irand() & 0xffff;

                /* Inicializamos BloomFilter */
		memset(bloomFilter,0,M);

                /* Generamos datos random */
                for(i=0;i<clients;i++)
			for (q=0;q<bitcount;q++)
                        	clientData[i][q]=irand() & 1;

                /* El cliente 0 tiene los datos reales */
		for (q=0;q<bitcount;q++)
                	clientData[0][q]=bit[q]-0x30;
                /* Simulamos transmision */
                for(i=0;i<clients;i++)
                        for (k=0;k<K;k++)       {
                        /* Averiguamos posicion en la que tenemos que marcar */
                        lfsr(&clientSeeds[i][k]);
                        unsigned int pos = clientSeeds[i][k]&(M-1);
			/* Guardamos posicion de nuestro cliente */
			if (!i)	clientPos[k]=pos;
                        if (DEBUG)
                                fprintf(stderr,"Pos[%d][%d]: %d\n",i,k,pos);
                        /* Marcamos la posicion que corresponda */
			for (q=0;q<bitcount;q++)
	                        if(clientData[i][q])
        	                        bloomFilter[ (pos+q) % M ]++;
                        }
                if (DEBUG)
                        for(i=0;i<sizeof(bloomFilter);i++)
                                printf("bloomFilter[%d]=%d\n",i,bloomFilter[i]);
		/* Copiamos a la salida solamente los bits que corresponden a nuestro cliente */
		for (k=0;k<K;k++)
			clientPos[k]=bloomFilter[ (clientPos[k]) % M];
		for (k=0;k<K;k++)
			bloomFilter[k]=clientPos[k];
	}

void bloomdec(unsigned char *bit,unsigned bitcount,unsigned char *bloomFilter, unsigned clients)
        {
        unsigned int q,k;
        unsigned char clientDataReceived[CLIENTS][bitcount]; // Datos recibidos


                /* Ahora en el bloomfilter tenemos todos los datos, tratamos de descifrarlos: */
                        /* Caso por defecto: Uno */
			for (q=0;q<bitcount;q++)
                        	clientDataReceived[0][q]=1;
                        for (k=0;k<K;k++)       {
                                /* La posicion es la misma que la anterior */
                                unsigned int pos = k;
				for (q=0;q<bitcount;q++)
                                	if (bloomFilter[(pos+q) % M]==0) clientDataReceived[0][q]=0;
                                }
	for (q=0;q<bitcount;q++)
		bit[q] = clientDataReceived[0][q];
	}

void usage(void)	{
	printf("Usage: bfenc <clients> (takes input from stdin and outputs to stdout)\n");
	exit(1);
}

int main(int argc, char **argv) 
{
int len,q,i;
unsigned char in[BITCOUNT];
unsigned char rin[BITCOUNT];
unsigned char out[M];

int clients=128;

srand(0);

if (argc<2)	usage();

clients=atoi(argv[1]);

fprintf(stderr,"bloomfilter: %d clients \n",clients);

if (strstr(argv[0],"bfenc"))
	{ // codifica un canal en un bloomfilter
	while((len=fread(in,1,BITCOUNT,stdin))==BITCOUNT)	{
		for (q=0;q<BITCOUNT;q++)	{
			bloomenc(in,1,out,clients);
			if ((len=write(1,out,K)) !=K)
				fprintf(stderr,"Error: write\n");
			}
		}
	}
else	{// decodifica un bloomfilter en un canal
	while (1)	{
		for (q=0;q<BITCOUNT;q++)	
			in[q]='1';
		for (q=0;q<BITCOUNT;q++)	{
			len=fread(out,1,K,stdin);
			if (len!=K) return 0;
			bloomdec(rin,1,out,clients);
			for (i=0;i<BITCOUNT;i++)
				if (rin[i]==0) in[i]='0';
			}
		if ((len=write(1,in,BITCOUNT)) !=BITCOUNT)
			fprintf(stderr,"Error: write\n");
		}
	}
return 0;
}
