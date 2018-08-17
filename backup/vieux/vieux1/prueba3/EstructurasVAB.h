#ifndef estructurasVAB_H
#define estructurasVAB_H

typedef struct {

	//	PHYSICAL PARAMETERS (parámetros físicos)

	float F0;			//	Amplitud del Bit.
	float Bit_Slot;			//	Tamaño de cada Bit.
	float Duty_Cycle;	
	float Total_Time;	
	float dt;			//	Resolución en tiempo.

	//	NUMERICAL PARAMETERS (parámetros numéricos)
	
	unsigned int      N_Bits;		//	Número de bits.
	unsigned int      N_BitsP;		//	Número de bits positivos.
	unsigned int      NPtos_Bit;		
	unsigned long int NPtos_Tot;	//	Número total de puntos.

	unsigned int NTau;	
	unsigned int NT1;
} GENDRIVEVAB;

#endif
