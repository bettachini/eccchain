//	Esta funcion genera el "Ruido Blanco Gaussiano" para cada particula
#ifndef GENRUIDORAPIDO_C
#define GENRUIDORAPIDO_C
#include <time.h>
#include <math.h>

void GenRuidoRapido(double *Noise, double Sigma, unsigned long int Tmax)
{
	void Gasdev(double *V2, double Sig, long *idum);
	unsigned long int i;
	static long int sem = -1;	//	Semilla para el generador de numeros random.

	if(sem==-1)	sem =-time(NULL);	
	for(i=0; i<Tmax; i+=2)
		Gasdev(&(Noise[i]), Sigma, &sem);
}

void Gasdev(double *V2, double Sigma, long *idum)
{
	double dran1(long *idum);
	double fac,rsq,v1,v2;

	do {
		v1=2.0*dran1(idum)-1.0;
		v2=2.0*dran1(idum)-1.0;
		rsq=v1*v1+v2*v2;
	} while (rsq >= 1.0 || rsq == 0.0);
	fac=Sigma*sqrt(-2.0*log(rsq)/rsq);
	V2[0] = v2*fac;
	V2[1] = v1*fac;
}
#endif
