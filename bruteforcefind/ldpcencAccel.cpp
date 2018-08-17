#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <iostream>
#include <fstream>
#define NDEBUG
#define BOOST_UBLAS_NDEBUG
#include <boost/numeric/ublas/vector.hpp>
#include <boost/numeric/ublas/matrix.hpp>
#include <boost/numeric/ublas/io.hpp>

#define DEBUG  0

using namespace boost::numeric::ublas ;

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
void lfsr(unsigned short int *reg)	{
	*reg=irand();
	}

// Devuelve la cantidad de errores de paridad del vector cw segun la matriz de paridad H
int checkParity(vector<int> cw, matrix<int> H)	{
	unsigned int errors = 0,q;
   	vector<int> vc;
    	vc = prod(H,cw);
    	for (q=0;q<vc.size();q++) vc(q)=vc(q) % 2;
    	for (q=0;q<vc.size();q++) errors+=vc(q);
	return errors;
}

// LDPC decode
// cw: received codeword 
// H: parity-check matrix
//
char *decode(char *cw, char *Hc,int Hx,int Hy) {
    int i,q;
    int numVnodes=Hx; // == dataLength
    int numFnodes=Hy;
    char *vnode= (char *) malloc(numVnodes);
    char *response = (char *) malloc(numVnodes*numFnodes);
    int MAXROUNDS=10;
    for (int r=0;r<MAXROUNDS;r++) {
	for(i=0;i<numVnodes;i++) {
		if (!r) vnode[i]=cw[i]; //first round
		else	{ // sum
			double nzeros = 0,nones=0;
			if (cw[i]==0) continue; // assymetric
			if (cw[i]) nones=1; else nzeros=1; // count the original value
			for(q=0;q<numFnodes;q++)
				if (Hc[i+numVnodes*q]) {// Link between f-node and v-node
					if (response[i+numVnodes*q]) nones++;
					else nzeros++;
					}
			//std::cout<<nzeros<<" "<<nones<<std::endl;
			//decision, is one or zero?
			if (nzeros>nones) vnode[i]=0;
			else	if (nzeros<=nones) vnode[i]=1;
			if (DEBUG>5) std::cerr<<" 0:"<<nzeros<<" 1:"<<nones<<"   ";
			}
		}
	if (DEBUG>5) std::cerr<<std::endl;
	char parityerrors=0,parity=0;
	for (int f=0;f<numFnodes;f++) {
		for(i=0;i<numVnodes;i++)	{
			if (cw[i]==0) continue; //assymetric
			if (  Hc[i+f*numVnodes]  ) {// Link between f-node and v-node
				parity=0;
				for (int j=0;j<numVnodes;j++)	{
					if (j==i) continue; //ignore current vnode
					if (Hc[j+f*numVnodes]) {// Link between f-node and v-node
						parity+=vnode[j];
						}
					}
				parity%=2; // Response is the value that we 
					  // need for the sum to be zero
				response[i+f*numVnodes]=parity;
				if (response[i+f*numVnodes] != vnode[i]) parityerrors++;
				}
			}
		}
        if (DEBUG>1) std::cerr<<"Round: "<<r<<" vnodes: "<<vnode<<" parityerrors: "<<parityerrors<<std::endl;
	if (!parityerrors) break; // all parity checks OK
	}
free(response);
return vnode;
};

void usage(void)	{
	std::cerr<<"Usage: ldpcenc <matrix> (takes input from stdin and outputs to stdout)"<<std::endl;
	exit(1);
}

// LDPC simulator
  matrix<int> H; //parity check
  matrix<int> G;


// Convert boost matrix to C matrix
char *MatrixBoost2C(matrix<int> M)
	{
	char *R;
 	int x,y;
    	int numXnodes=M.size1();
    	int numYnodes=M.size2();
	int tsize = (numXnodes+1)*(numYnodes+1);
	R = (char *) malloc(tsize);
	memset(R,0,tsize);
	for (y=0;y<=numYnodes;y++)
		for (x=0;x<=numXnodes;x++)
			R[y*numXnodes+x]=M(x,y);
	return R;
	}

char *VectorBoost2C(vector<int> M)
	{
	char *R;
 	int x;
    	int numXnodes=M.size(); 
	R = (char *) malloc(numXnodes+1);
	memset(R,0,numXnodes);
	for (x=0;x<=numXnodes;x++) 
		R[x]=M(x);
	return R;
	}


void readMatrixes(char *file)	{
   std::ifstream thematrix;
   thematrix.open (file);
   // Cargamos H (Parity Check Matrix)
   thematrix>>H;
   // Cargamos G (Generator Matrix)
   thematrix>>G;
   thematrix.close();
   if (DEBUG)
   	std::cerr<<"Loaded "<<file<<std::endl;
}

int main(int argc, char **argv)	
{
  srand(0);

  if (argc<2)	usage();
  readMatrixes(argv[1]);

   unsigned int dataLenght = G.size1();
   unsigned int codewordLenght = G.size2();

   vector<int> v(dataLenght);
   vector<int> cw(codewordLenght);
   vector<int> cwerr(codewordLenght);
   matrix<int> Gt = trans(G);
   matrix<int> Ht = trans(H);
   char *Hc;
   unsigned int i,q,berpre,berpost,totalpre=0,totalpost=0,bits=0;

   //convert to C matrix    
   Hc = MatrixBoost2C(Ht);

   for(i=0;i<350;i++)	{
		#define RANDOMINPUT

		#ifdef RANDOMINPUT
		//Genera vector v()
	    	for (q=0;q<dataLenght;q++)	
			v(q)= rand() % 2;
		#else
		// lee vector v() de stdin
		char *rbuf = (char *)malloc(dataLenght+1);
		q=fread(rbuf,1,dataLenght,stdin);
		if (q!=dataLenght)
			break; //no more data
	    	for (q=0;q<dataLenght;q++)	
			v(q)= (rbuf[q]-0x30);
		free(rbuf);
		#endif

    		// Code
		// CodeWord = Gt*v
	    	cw = prod(Gt,v);
	    	for (q=0;q<cw.size();q++) {
			cw(q)=cw(q) % 2; 
			}
		
		// Error
	    	for (q=0;q<cw.size();q++) 
			if ((irand() % 100)<20)
				cwerr(q)=1;
			else	cwerr(q)=cw(q);

		// Decode
   		char *cwc = VectorBoost2C(cwerr);
		char *decoC = decode(cwc,Hc,Ht.size1(),Ht.size2());
		free(cwc);
    		vector<int> deco(Ht.size1());
    		for (q=0;q<dataLenght;q++) 
			deco(q)=decoC[q];
		free(decoC);

		// Compara
		berpost=0;berpre=0;
	    	for (q=0;q<dataLenght;q++) {
			v(q)!=cwerr(q)?berpre++:0;
			v(q)!=deco(q)?berpost++:0;
			}
		bits+=dataLenght;

		std::cout<<" Berpre: "<<berpre<<" Berpost: "<<berpost<<std::endl;	
		totalpost+=berpost;
		totalpre+=berpre;
		}
	std::cerr<<totalpost<<std::endl;	
	std::cout<<"Total Pre: "<<totalpre<<" Total Post: "<<totalpost<<" "<<(float)totalpost/(float)totalpre<<" BER: "<<(float)totalpre/(float)bits<<" BER_POST: "<<(float)totalpost/(float)bits<<std::endl;	
free(Hc);
}
