#ifndef _PULSO1_C_
  #define _PULSO1_C_

#include "Estructuras.h"

void gaussiana(double* pulso1,unsigned int Tmax){
  double t;
//  double sigma=5;
  double m=1.0	// m=1 Gaussiana 
  for(t=0; t<(double) Tmax; t+=1.0) {
    pulso1[(int) t]=exp(-0.5*pow(t/To,2*m)); // SuperGaussiana (real)

//    pulso1[(int) t]=exp(-(t- 0.5*(double) Tmax)*(t- 0.5*(double) Tmax)/(2*sigma*sigma)); // Gaussiana
  }