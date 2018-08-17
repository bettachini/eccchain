#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <iostream>
#include <fstream>
#define NDEBUG
#define BOOST_UBLAS_NDEBUG
#include <boost/numeric/ublas/io.hpp>
#include <boost/numeric/ublas/matrix.hpp>

#include <boost/random/mersenne_twister.hpp>
#include <boost/random/uniform_int.hpp>
#include <boost/random/variate_generator.hpp>


  boost::mt19937 rng;                 // produces randomness out of thin air
                                      // see pseudo-random number generators
  boost::uniform_int<> six(0,256*256*256);  // distribution that maps to 1..6
                                      // see random number distributions
  boost::variate_generator<boost::mt19937&, boost::uniform_int<> >
           irand(rng, six);             // glues randomness with mapping

using namespace boost::numeric::ublas ;


// Evencol algorithm:
// Modifies the number of ones on the columns to make them the same as the number of ones in the rows.

void evenCol(matrix<int> &Gt)	{
 int x=Gt.size1(); 
 int y=Gt.size2(); 
 int j,q,i;
//count ones per column
 int ones_per_column=0;
 for (q=0;q<y;q++)	
 	for (j=0;j<x;j++)
		ones_per_column+=Gt(q,j);
 ones_per_column/=y;
 int ones_this_column;
 for (q=0;q<y;q++)	{
	ones_this_column=0;
 	for (j=0;j<x;j++) ones_this_column+=Gt(q,j);
	int cont=0;
	// Mas de la media
	while (ones_this_column>ones_per_column)	{
 		for (j=0;j<x;j++) 
		//j = irand() % x;
		if (Gt(q,j))	{ //swap
				i=irand() % (q-y);
				i+=q;
				if (!Gt(i,j))	{
					Gt(i,j)=1;
					Gt(q,j)=0;
					}
			break;
			}
		ones_this_column=0;
 		for (j=0;j<x;j++) ones_this_column+=Gt(q,j);
		if (cont++>1000) break;
		//std::cout<<"cont: "<<cont<<" "<<ones_this_column<<std::endl;
		}
	// Menos de la media
	while (ones_this_column<ones_per_column)	{
		j = irand() % x;
		if (!Gt(q,j))	{ //swap
			i=irand() % (q-y);
			i+=q;
			if (Gt(i,j))	{
				Gt(i,j)=0;
				Gt(q,j)=1;
				}
			}
		ones_this_column=0;
 		for (j=0;j<x;j++) ones_this_column+=Gt(q,j);
		if (cont++>10000) break;
		//std::cout<<"cont: "<<cont<<" "<<ones_this_column<<std::endl;
		}

	} 
 
}
// Show ones on the matrix
void showOnes(matrix<int> &Gt)	{
 int x=Gt.size1(); 
 int y=Gt.size2(); 
 int j,q;
 
 std::cerr<<x<<"-"<<y<<std::endl;
 // ones per row
 for (j=0;j<x;j++)
	{
	int index=0;
	for (q=0;q<y;q++)	{
		index+=Gt(q,j);
		}
	std::cerr<<index<<" ";
	} 
 std::cerr<<std::endl;

 // ones per column
 for (q=0;q<y;q++)	{
	int index=0;
 	for (j=0;j<x;j++){
		index+=Gt(q,j);
		}
	std::cerr<<index<<" ";
	} 
 std::cerr<<std::endl;
}

void usage(void)	{
	std::cerr<<"Usage: modifMatrix <matrix> <rows>"<<std::endl;
	exit(1);
}

// LDPC simulator
  matrix<int> H; //parity check
  matrix<int> G;


void readMatrixes(char *file)	{
   std::ifstream thematrix;
   thematrix.open (file);
   // Cargamos H (Parity Check Matrix)
   thematrix>>H;
   // Cargamos G (Generator Matrix)
   thematrix>>G;
   thematrix.close();
}



int main(int argc, char **argv)	
{
  // Get random seed
  std::ifstream rando("/dev/urandom");
  unsigned int r;
  rando>>r;
  rando.close();
  rng.seed(r);


  if (argc<3)	usage();
  readMatrixes(argv[1]);
  matrix<int> Gt = trans(G);
  H = trans(H);

  int i,j,q,q1,s;
  int x =H.size1();
  int y =H.size2();
  int rows = atoi(argv[2]);

  std::cerr<<"Modifyng"<<std::endl;

  for(i=0;i<rows;i++)	{
	/* either add or remove a bit */
	if (irand() % 3)	{
		// Add bit
		q=irand() % y;
		q1=irand() % (x-y);
		H(q,q1)=1;
		}
	else	{
		// Remove bit
		for (s=0;s<10000;s++)	{
			q=irand() % y;
			q1=irand() % (x-y);	
			if (H(q,q1))	{
				H(q,q1)=0;
				break;
				}
			}
		}
	
	}
  //evenCol(H);
   

 std::cout<<trans(H)<<std::endl;





  // Constructing G = (Iy|A)
  matrix<int> G(x,y); //generator

  // init matrix
 for(i=0;i<(int)G.size2();i++)
 	for(j=0;j<(int)G.size1();j++)
		G(j,i)=0;
 
  // Iy
 for(i=0;i<y;i++)
	G(i,i)=1;

 // Copy H to At
 for (i=0;i<y;i++)
	for (j=0;j<(x-y);j++)
		Gt(i,j)=H(i,j);

 matrix<int> Gp = trans(Gt);

 for(i=0;i<y;i++)
	for (j=0;j<(x-y);j++)
		G(j+y,i)=Gp(j,i);

 std::cout<<trans(G)<<std::endl;

 // Check, R must be all 0
 /*
 matrix<int> R;
 q=0;
 R=prod(trans(G),H);
 for(i=0;i<(int)R.size1();i++)
 	for(j=0;j<(int)R.size2();j++) {
		R(i,j)%=2;
		q+=R(i,j);
		}
 std::cout<<"Sum R: "<<q<<std::endl;
 */
}
