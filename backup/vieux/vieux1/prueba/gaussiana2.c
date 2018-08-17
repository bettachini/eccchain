#ifndef gaussiana2_c
#define gaussiana2_c
#include "Estructuras.h"

void gaussiana2(double* pulso1,unsigned int Tmax){
  double t;
//  double sigma=5;
  double m=1.0	// m=1 Gaussiana 
  for(t=0; t<(double) Tmax; t+=1.0) {
    pulso1[(int) t]=exp(-0.5*pow(t/To,2*m)); // SuperGaussiana (real)
  }