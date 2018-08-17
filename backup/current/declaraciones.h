#ifndef _DECLARACIONES_H_
  #define _DECLARACIONES_H_

typedef struct  {	// DEFINICION DEL SISTEMA DE TRANSMISION
	// PHYSICAL PARAMETERS (parámetros físicos)
	float Bit_Slot;			//	Tamaño de cada Bit.
	float Duty_Cycle;	
	float Total_Time;	
	float B;			// bit rate [bits/s]
	float dt;			//	Resolución en tiempo.

	// NUMERICAL PARAMETERS (parámetros numéricos)
	unsigned int      N_Bits;		//	Número de bits.
	unsigned int      N_BitsP;		//	Número de bits positivos.
	unsigned int      NPtos_Bit;		
	unsigned long int NPtos_Tot;	//	Número total de puntos.

	unsigned int NTau;	
	unsigned int NT1;
} GENDRIVE;

typedef struct {	// GAUSSIAN PULSE DEFINITION
  float P_0;		// [mW] Optical peak power 
  float A_0;		// [sqrt{mW}] peak amplitude  |A_0|^2=P_0 Optical power
  float C;		// chirp parameter
  float FWHM_pow;	// [s] FWHM signal in power
  float T_0_amp;	// [s] T_0=T_{fwhmPOW}/(2* sqrt(ln(2) ) ) half of width at 1/e intensity point = gaussian sigma
  unsigned int m;	// Super gaussian pulse shape factor (gaussian=1)
  float r_ex;		// [dB] Extinction ratio= 10* log(P_0/ P_1) [Agrawal (4.6.1)]

} SuperGaussian;

typedef struct FCOMPLEX {float r,i;} fcomplex;

void GenRuidoRapido(double *Noise, double Sigma, unsigned long int Tmax);


#endif
