#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#define NDEBUG
#define BOOST_UBLAS_NDEBUG
#include <boost/numeric/ublas/io.hpp>
#include <boost/numeric/ublas/matrix.hpp>


using namespace boost::numeric::ublas ;

int main(int argc, char **argv)	
{
  srand(0);

  int i,j,q;
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

 // Constructing H = (At|I(x-y))
 matrix<int> H(y+(x-y),x-y);

 // I(x-y)
 for (i=y;i<y+(x-y);i++)
		H(i,i-y)=1;
 // At
 matrix<int> Gt(y,x-y);
 for (j=0;j<(x-y);j++)
	for(q=0;q<ones-1;q++)	{
		int index=rand() % y;
		Gt(index,j)=1;
		}
 for (i=0;i<y;i++)
	for (j=0;j<(x-y);j++)
		H(i,j)=Gt(i,j);

 int Hx = H.size1();
 int Hy = H.size2();
 int chk;

  for(j=0;j<Hy-(x-y);j++) {
	chk=0;
  	for(i=0;i<Hy;i++) 
		chk+=H(j,i);
	if (chk%2)
		H(j,rand() % y)=1;
	if (chk==0)
		H(j,rand() % y)=1;
	}
/*
  for(i=0;i<Hy;i++)	{
	chk=0;
  	for(j=0;j<Hy-(x-y);j++) 
		chk+=H(j,i);
	if (chk%2)
		H(rand() % Hx,i)=1;
	if (chk==0)
		H(rand() % Hx,i)=1;
	}
*/
 int a,b,c;
 for(j=0;j<Hy;j++) 
 	for (i=0;i<Hx;i++) 
		if (H(i,j))	
			for (a=i;a<Hx;a++)
				if (H(a,j))
					for (b=j;b<Hy;b++)
						if (H(a,b))
							for (c=a;c>0;c--)
								if (H(c,b))
									H(c,b)=0;

 std::cout<<trans(H)<<std::endl;
 
  // Constructing G = (Iy|A)
  // Iy
 for(i=0;i<y;i++)
	G(i,i)=1;
 matrix<int> Gp = trans(Gt);
 for(i=0;i<y;i++)
	for (j=0;j<(x-y);j++)
		G(j+y,i)=Gp(j,i);
 std::cout<<trans(G)<<std::endl;

 // Check, R must be all 0
 R=prod(trans(G),H);

}
