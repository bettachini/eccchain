#include <stdio.h>
#include <math.h>

void gaussiana(double* pulso1,unsigned int Tmax);

int main(void) {
unsigned long int i;
unsigned long int Tmax=1024;		// puntos muestreo vector de campo
double pulso1[Tmax],pulso0[Tmax];	// vectores campo para '1' y '0'
double re=1;				// Extinction ratio, razon potencia '1' y '0'
double T;				// Duracion pulso
double D=1;				// Duty Cycle

gaussiana(pulso1,Tmax);			// genera pulso '1'
for(i=0; i<Tmax; i++) pulso0[i]= re;	// genera pulso '0'

for(i=0; i<Tmax; i++) {
  printf("%f\n",pulso1[i]);
}

return 0;
}

void gaussiana(double* pulso1,unsigned int Tmax){
  double t;
//  double sigma=5;
  double m=1.0	// m=1 Gaussiana 
  for(t=0; t<(double) Tmax; t+=1.0) {
    pulso1[(int) t]=exp(-0.5*pow(t/To,2*m)); // SuperGaussiana (real)

//    pulso1[(int) t]=exp(-(t- 0.5*(double) Tmax)*(t- 0.5*(double) Tmax)/(2*sigma*sigma)); // Gaussiana
  }
}

