#include <time.h>
#include <math.h>
#include <stdio.h>
#include "estructurasVAB.h"
#include "nrutil.h"
void gaussiana(double* cosa1,unsigned int Tmax);


int main() {
unsigned long int i;
unsigned long int Tmax=100;	// numero de puntos en vector de intensidad
double cosa1[Tmax];

// GENDRIVE gen;
// gen.F0=1.1;

gaussiana(cosa1,Tmax);
for(i=0; i<Tmax; i++) printf("%f\n",cosa1[i]);
return 0;
}

void gaussiana(double* cosa1,unsigned int Tmax){
  double i;
  double sigma=5;
  for(i=0; i<(double) Tmax; i+=1.0) {
    cosa1[(int) i]=exp(-(i- 0.5*(double) Tmax)*(i- 0.5*(double) Tmax)/(2*sigma*sigma));
  }
}

