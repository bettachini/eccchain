#include <stdio.h>
#include <stdlib.h>
#include <iostream>
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

int main(int argc, char **argv)	
{
  rng.seed(time(NULL));

  int i,j;
  int x =400;
  int y =100;
  int ones=5;
  if (argc>1) x = atoi(argv[1]);
  if (argc>2) y = atoi(argv[2]);
  if (argc>3) ones = atoi(argv[3]);
  if (argc==1)	{
	printf("Usage: %s <width> <height> <ones per row>\n",argv[0]);
	exit(-1);
	}
  

  matrix<int> G(x,y); //generator
  matrix<int> R;


  // Constructing G = (Iy|A)

  // Ik
  for(i=0;i<y;i++)
	G(i,i)=1;
  // A
  for(i=0;i<y;i++) {
	for(j=0;j<ones;j++)	{
		int index=irand() % (x-y);
		G(y+index,i)=1;
		}
	}
 // Check for even columns in A
 evenCol(G);

 // Constructing H = (At|I(x-y))
 matrix<int> Gp(x-y,y);
 // At
 for (i=0;i<y;i++)
	for (j=0;j<(x-y);j++)
		Gp(j,i)=G(y+j,i);

 matrix<int> H(y+(x-y),x-y);
 matrix<int> Gt = trans(Gp);

 for (i=0;i<y;i++)
	for (j=0;j<(x-y);j++)
		H(i,j)=Gt(i,j);
 
 // I(x-y)
 for (i=y;i<y+(x-y);i++)
		H(i,i-y)=1;

 // Check, R must be all 0
 //R=prod(trans(G),H);

 // Output 
 std::cout<<trans(H)<<std::endl;
 std::cout<<trans(G)<<std::endl;

}

