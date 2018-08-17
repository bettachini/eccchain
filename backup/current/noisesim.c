#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "declaraciones.h"

#define BUFFERMAX 512

/*
fcomplex Complex(float re, float im){
  fcomplex c;
  c.r=re;
  c.i=im;
  return c;
}


fcomplex Cmul(fcomplex a, fcomplex b){
  fcomplex c;
  c.r=a.r*b.r-a.i*b.i;
  c.i=a.i*b.r+a.r*b.i;
  return c;
}


float Cabs(fcomplex z){
  float x,y,ans,temp;
  x=fabs(z.r);
  y=fabs(z.i);
  if (x == 0.0) ans=y;
  else if (y == 0.0) ans=x;
  else if (x > y) {
    temp=y/x;
    ans=x*sqrt(1.0+temp*temp);
  } else {
    temp=x/y;
  ans=y*sqrt(1.0+temp*temp);
  }
  return ans;
}
*/

void parameters(GENDRIVE *Carrier, SuperGaussian *gauss, unsigned int *transmitters){
  (*Carrier).NPtos_Bit= pow(2,6);					// # points for each bit slot
  (*Carrier).NPtos_Tot= (*Carrier).NPtos_Bit*(*Carrier).N_Bits;	// Total # points
  (*Carrier).Duty_Cycle= 1.0/3.0;
  (*Carrier).B= 10E9;						// bit rate [bits/s] (10 Gbits/s)
  (*Carrier).Bit_Slot= 1/ (*Carrier).B;				// 1/B B: bit rate [s]
  (*Carrier).dt= (*Carrier).Bit_Slot/( ( (*Carrier).NPtos_Bit- 1)+ 0.5);		// time resolution [s]
//  printf("dt=\t%e\n",1/(*Carrier).dt);
//  (*Carrier).dt= 1/( (*Carrier).B* (*Carrier).NPtos_Bit );		// time resolution [s] [NON OPTIMITISED VERSION]
  //  printf("Bit_Slot= %.3f ps\n",(*Carrier).Bit_Slot*1E12);	
  //  printf("Resolucion temporal= %.3f ps\n",(*Carrier).dt*1E12);


  // OPTICAL PULSE DEFINITION
  (*gauss).P_0= 2.5E-3;						// [mW] www.afop.com/pdf/Active%20Devices/GE-PON%20BOSA.pdf
  (*gauss).A_0= sqrt((*gauss).P_0);				// [sqrt{mW}]
  (*gauss).FWHM_pow= (*Carrier).Duty_Cycle* (*Carrier).Bit_Slot;			// [s] in power (|E|^2)
//  (*gauss).FWHM_pow= 5.887050112577374E-12;			// [s] in electric field amplitude
  (*gauss).T_0_amp= (*gauss).FWHM_pow/(2* sqrt(log(2) ) );	// [s]
  // (*gauss).T_0_amp= 2.5E-12;					// [s]
  //  (*gauss).T_0_amp=0.25*(*Carrier).Duty_Cycle*(*Carrier).Bit_Slot;	// as Duty_Cycle fraction 
  (*gauss).m= 4;						// gaussian=1
  (*gauss).r_ex= -10.0;						// [dB] Extinction ratio [-10 ~ -15 Grosz priv. comm. 090210]

  // SYSTEM
//  *transmitters=10;						// #simultaneous users
}


void Bits2Pulses(GENDRIVE *Carrier, SuperGaussian *gauss, unsigned char InBitString[], double OSNR, float RZPulsesTrain[][2], float RZBit0[], float RZBit1Gauss[], unsigned int transmitters, double *SigmaNoise){
  unsigned int i;

  // '0' SLOT AMPLITUDE
  //float RZBit0[Carrier.NPtos_Bit]
  float LOff=(*gauss).A_0* pow(10,(*gauss).r_ex/20);			// optical amplitude for '0'
  //float LOff= 0.0;							// TEST: no extinction ratio
  for(i=0; i<(*Carrier).NPtos_Bit; i++) RZBit0[i]= transmitters* LOff;		// base level for multiple transmitters

  // Time vector
  float aux=( (1.5- (float)(*Carrier).NPtos_Bit)/ (1- (float)(*Carrier).NPtos_Bit) );
  for(i=0; i<(*Carrier).NPtos_Bit; i++) RZPulsesTrain[i][1]= (*Carrier).dt* ( aux* i+ 0.5) ;	// time for one slot
  unsigned int k;
  for(k= 1;k<(*Carrier).N_Bits;k++) for(i= 0;i<(*Carrier).NPtos_Bit;i++) RZPulsesTrain[k*(*Carrier).NPtos_Bit+ i][1]= RZPulsesTrain[i][1]+ (*Carrier).Bit_Slot* k;		// copies time to following slots

  // '1' SLOT OVER '0' LEVEL AMPLITUDE
  for(i=0; i<(*Carrier).NPtos_Bit; i++) RZBit1Gauss[i]= ((*gauss).A_0- LOff)* exp(-0.5* pow(((RZPulsesTrain[i][1]- (0.5* (*Carrier).Bit_Slot))/ (*gauss).T_0_amp), 2* (*gauss).m));

/*
  // OVERPULSE '1' CHECK
  FILE *TestFile;
  TestFile= fopen("amplitude1.dat", "w");
  // fprintf(TestFile,"#time [s]\tAmplitude [sqrt(mW)]\n");
  for(i=0; i<(*Carrier).NPtos_Bit; i++) fprintf(TestFile,"%3e\n",RZBit1Gauss[i]);
  fclose(TestFile);
*/

  // RZ PULSES TRAIN (Amplitude)
  //float RZPulsesTrain[(*Carrier).NPtos_Tot][2];
  unsigned int senders;
  k=0;
  while(k<(*Carrier).N_Bits){
    senders= InBitString[k];
    for(i=0; i<(*Carrier).NPtos_Bit; i++){
      RZPulsesTrain[k*(*Carrier).NPtos_Bit+i][2]= (float) RZBit0[i];			// '0' base
      RZPulsesTrain[k*(*Carrier).NPtos_Bit+i][2]+= (float) senders* RZBit1Gauss[i];	// '1' pulses addition
    }
    k++;
  }

  //for(i=0; i<(*Carrier).NPtos_Tot; i++) printf("%.3e\t%.3e\n",RZPulsesTrain[i][1],RZPulsesTrain[i][2]);	// TEST

  // ADDITIVE WHITE GAUSSIAN NOISE [SANTIAGO'S GENERATOR]
  // double SigmaNoise;
  /*
  SigmaNoise: [V/m*10-3] width of amplitude gaussian distribution A(t) related to mean noise power as follows:

  white noise autocorrelation = \sigma^2 * delta(t)  [http://en.wikipedia.org/wiki/White_noise]
  Wiener-Khinchin: \phi(\omega) [Power Spectral Density]= Fourier(white noise autocorrelation )  [http://en.wikipedia.org/wiki/Wiener%E2%80%93Khinchin_theorem]
  -> \phi(\omega)= \sigma^2 * Fourier(\delta(t))
  P [Power]= \int d\omega \phi(\omega)  [http://en.wikipedia.org/wiki/Spectral_density]
  -> P= \sigma^2 * Fourier(\delta(t))
  gaussian: Fourier(\delta)=\frac{1}{\sqrt(2*\pi)}
  -> P =\sigma^2


  SigmaNoise is calculated from SNR assuming the following:
  SNR=Power_signal/Power_noise  [http://en.wikipedia.org/wiki/Signal-to-noise_ratio]
  Power_noise=SigmaNoise^2
  Power_signal= 0.5*(Power_'0bit' + Power_'1bit')  assuming equal number of '1' and '0' bits  [Andres, Nacho]
  Power_'0bit'= LOff^2, amplitude for bit '0' due to system extinction ratio
  Power_'1bit'= P1Mean= \frac{1}{(*Carrier).Bit_Slot} \int_0^{(*Carrier).Bit_Slot} dt Pulse1(t)^2  integrated in bit slot
  OR? Power_'1bit'= \frac{1}{LimSup-LimInf} \int_LimInf^{LimSup} dt Pulse1(t)^2  integrated in detection window
  -> SigmaNoise=\sqrt(\frac{LOff^2 + P1Mean}{2 SNR})
  */

  // mean signal power -> gaussian sigma noise
  float Pmean=0;
  for(i=0; i<(*Carrier).NPtos_Bit; i++) Pmean+= RZBit1Gauss[i]* RZBit1Gauss[i];
  Pmean= 0.5* ((Pmean/ (*Carrier).NPtos_Bit )+ LOff* LOff);
  *SigmaNoise= sqrt(Pmean/ OSNR);	// amplitude of noise (Agrawal 4.4.11) [Santiago I.]
  //printf("OSNR= %e\tPmean= %e\tSigmaNoise= %e\n",OSNR ,Pmean, *SigmaNoise);					// TEST

  double NoiseVector[(*Carrier).NPtos_Tot];
  GenRuidoRapido(NoiseVector, *SigmaNoise, (*Carrier).NPtos_Tot);		// NoiseVector amplitude
  //for(i=0; i<(*Carrier).NPtos_Tot; i++) printf("%.3e\t%.3e\n",RZPulsesTrain[i][1],RZPulsesTrain[i][2]);	// TEST
  //for(i=0; i<(*Carrier).NPtos_Tot; i++) printf("%.3e\t\n",(float) NoiseVector[i]);	// TEST

  // Noise addition & amplitude -> power
  for(i=0; i<(*Carrier).NPtos_Tot; i++){
    RZPulsesTrain[i][2]+= NoiseVector[i];			// Noise addition
    // RZPulsesTrain[i][2]*= RZPulsesTrain[i][2];		// amplitude -> power
  }
}


void Pulses2Bits(GENDRIVE *Carrier, float RZPulsesTrain[][2], float RZBit0[], float RZBit1Gauss[], char BitsDisc[], double *SigmaNoise){
  unsigned int i;

/*
  fcomplex test[2];
  test[1].r=1;
  test[1].i=-1;
  test[2].r=1;
  test[2].i=1;

  fcomplex result;  
  result=Cmul(test[1],test[2]);
  printf("r: %f\ti: %f\n",result.r,result.i);
*/

  // Willkommen sum komplex!


  // DETECCION: Filtrado optico pasabajo Butterworth (Como corrijo por Duty Cycle?) [Pendiente]
  /*
  Butteworth code generatedy by: http://www-users.cs.york.ac.uk/~fisher/mkfilter/trad.html
  Order= 3
  Sample rate= 1/ dt = 2.046000e+13
  Corner frequency= 2E9

#define NZEROS 3
#define NPOLES 3
#define GAIN   3.454972336e+10

static float xv[NZEROS+1], yv[NPOLES+1];

static void filterloop()
  { for (;;)
      { xv[0] = xv[1]; xv[1] = xv[2]; xv[2] = xv[3]; 
        xv[3] = next input value / GAIN;
        yv[0] = yv[1]; yv[1] = yv[2]; yv[2] = yv[3]; 
        yv[3] =   (xv[0] + xv[3]) + 3 * (xv[1] + xv[2])
                     + (  0.9987723699 * yv[0]) + ( -2.9975439859 * yv[1])
                     + (  2.9987716158 * yv[2]);
        next output value = yv[3];
      }
  }
  */


  // DISCRIMINATION BY DISPERSION THRESHOLD [Agrawal 4.5]
  float EThresh;
  unsigned short int Pul, NPulThr=5;			// averaged bits to obtain threshold
  double PropInt=0.2;					// CENTRED BIT SLOT FRACTION TO INTEGRATE
  unsigned int step,Min,Max;
  Min= (int) floor((*Carrier).NPtos_Bit*(0.5*(1-PropInt)));
  Max= (int) ceil((*Carrier).NPtos_Bit*(0.5*(1+PropInt)));
  float One[(*Carrier).NPtos_Bit], Zero[(*Carrier).NPtos_Bit];
  float sum0, sum_sqr0, sum1, sum_sqr1;
  float n= Max- Min+1;
  float Id=0, I0, I1, sigma0, sigma1;

  // NOISE VECTOR FOR THRESHOLD DETERMINATION BITS
  double NoiseVector2[2*(*Carrier).NPtos_Bit* NPulThr];
  // printf("SigmaNoise=\t%e\n",*SigmaNoise);			// TEST
  GenRuidoRapido(NoiseVector2, *SigmaNoise, 2*(*Carrier).NPtos_Bit* NPulThr);		// NoiseVector amplitude

  for(Pul=0; Pul<NPulThr; Pul++){
    sum0= 0, sum_sqr0= 0, sum1= 0, sum_sqr1= 0;

    // NOISE ADDITION TO THRESHOLD DETERMINATION BITS
    for(i=0; i<(*Carrier).NPtos_Bit; i++){
      One[i]= RZBit1Gauss[i]+ NoiseVector2[i+(*Carrier).NPtos_Bit*Pul];
      Zero[i]= RZBit0[i]+ NoiseVector2[i+(*Carrier).NPtos_Bit*(Pul+NPulThr)];
      //printf("tot: %d\tlleg: %d\n",(*Carrier).NPtos_Tot,i+(*Carrier).NPtos_Bit*(Pul+NPulThr));      // TEST
    }

    for(step=Min; step<Max+1; step++){
      One[step]*= One[step];	// amplitude-> power
      sum1+= One[step];
      One[step]*= One[step];
      sum_sqr1+= One[step];

      Zero[step]*= Zero[step];	// amplitude-> power
      sum0+= Zero[step];
      Zero[step]*= Zero[step];
      sum_sqr0+= Zero[step];
    }
    I1= sum1/n;		// moyenne1
    I0= sum0/n;		// moyenne0
    // printf("n= %.3e\tsum_sqr0= %.3e\tsum_sqr1= %.3e\tsum0= %.3e\tsum1= %.3e\n",n, sum_sqr0, sum_sqr1, sum0, sum1); // TEST

    sigma1= sqrt(( 1/ ( n- 1)) * (sum_sqr1 - ( (sum1* sum1)/ n) ) );
    sigma0= sqrt(( 1/ ( n- 1)) * (sum_sqr0 - ( (sum0* sum0)/ n) ) );
    if (sigma0 != sigma0) sigma0= 0;	// if sigma0 too small (NaN), sigma0 -> 0
    // printf("Sigma0p= %e\n",sigma0); // TEST
    Id+= (1/ (float) NPulThr)* ((sigma0* I1+ sigma1* I0)/ (sigma0+ sigma1));
  }
  EThresh= Id* (*Carrier).dt;		// energy

/*
  printf("I0= %.3e\tId= %.3e\tI1= %.3e\n",I0, Id, I1); // TEST
  printf("S0= %.3e\t\t\tS1= %.3e\t\n",sigma0,sigma1); // TEST
  printf("Id=%e\tUmbral= %e\n",Id,EThresh); // TEST
*/


    // SAMPLE: CENTRED INTERVAL OF BIT SLOT
    EThresh= n* EThresh;		// Energy threshold for INTERVAL integration
    float Energies[(*Carrier).N_Bits];
    for(i=0; i<(*Carrier).N_Bits; i++){
    Energies[i]=0;
    for(step=Min; step<Max+1; step++) Energies[i]+= RZPulsesTrain[step+ i* (*Carrier).NPtos_Bit][2] * RZPulsesTrain[step+ i* (*Carrier).NPtos_Bit][2];	// amplitude RZPulsesTrain
    Energies[i]*= (*Carrier).dt;			// integrated power -> energy
    // printf("%e\n",Energies[i]); 		// TEST
  }

/*
    // TEST SAMPLE: ONE POINT OF BIT SLOT
    int SamplingPoint= floor(0.5* (*Carrier).NPtos_Bit);			// Samples at bit slot middle point
    float Energies[(*Carrier).N_Bits];
    for(i=0; i<(*Carrier).N_Bits; i++){
      Energies[i]= RZPulsesTrain[SamplingPoint+ i* (*Carrier).NPtos_Bit][2];
      Energies[i]*= RZPulsesTrain[SamplingPoint+ i* (*Carrier).NPtos_Bit][2];
      Energies[i]*= (*Carrier).dt;			// integrated power -> energy
    }
*/

  // DISCRIMINATION BY THRESHOLD
  for(i=0; i<(*Carrier).N_Bits; i++){
    if(Energies[i]>EThresh) BitsDisc[i]=1;
    else BitsDisc[i]=0;
  }
  //BitsDisc[(*Carrier).N_Bits]='\0';
  //for(i=0; i<(*Carrier).N_Bits; i++) prinTf("%c",BitsDisc[i]);
  //printf("\n");
}


int main(int argc, char **argv){
  if (argc<3){
    printf("Usage:noisesim #Tx SNR[dB] <InputBitStream >OutputBitStream\n");
    exit(0);
  }
  unsigned char buf[BUFFERMAX];
  int maxdata=BUFFERMAX;
  unsigned int i;
  GENDRIVE Carrier;
  SuperGaussian gauss;
  unsigned int transmitters= atoi(argv[1]);				// Argument: number of Tx
  double OSNR= pow(10,atof(argv[2])/10);			// Argument: OSNR [dB] -> OSNR [1]
  double SigmaNoise;
  while((Carrier.N_Bits=fread(buf,1,maxdata,stdin))>0){

    // INPUTS READ
    unsigned char InBitString[Carrier.N_Bits];
    for(i=0; i<Carrier.N_Bits; i++) InBitString[i]=buf[i];
    //printf("#bits: %i\tSNR: %f\n",Carrier.N_Bits,OSNR);		// TEST
    //printf("bits: %s\n",InBitString); 			// TEST

    // SYSTEM PARAMETERS
    parameters(&Carrier, &gauss, &transmitters);

    // BITS -> PULSES
    float RZPulsesTrain[Carrier.NPtos_Tot][2];
    float RZBit0[Carrier.NPtos_Bit], RZBit1Gauss[Carrier.NPtos_Bit];
    Bits2Pulses(&Carrier, &gauss, InBitString, OSNR, RZPulsesTrain, RZBit0, RZBit1Gauss, transmitters, &SigmaNoise);

/*
    // POWER VS TIME FILE
    FILE *TrainFile;
    TrainFile= fopen("power.dat", "w");
    // fprintf(TrainFile,"#time [s]\tPower [mW]\n");
    for(i=0; i<Carrier.NPtos_Tot; i++) fprintf(TrainFile,"%3e\t%3e\n",RZPulsesTrain[i][1],RZPulsesTrain[i][2]* RZPulsesTrain[i][2]);
    fclose(TrainFile);
*/

    // PULSES -> BITS
    char BitsDisc[Carrier.N_Bits];
    Pulses2Bits(&Carrier, RZPulsesTrain, RZBit0, RZBit1Gauss, BitsDisc, &SigmaNoise);
    
/*
    // INPUT / OUTPUT COMPARISON
    printf("BitsEntrada:\t");
    for(i=0; i<Carrier.N_Bits; i++) printf("%c",InBitString[i]);
    printf("\nBitsSalida :\t");
    for(i=0; i<Carrier.N_Bits; i++) printf("%c",BitsDisc[i]);
    printf("\nDiferencias:\t");
    for(i=0; i<Carrier.N_Bits; i++){
      if(InBitString[i]==BitsDisc[i]) printf(" ");
      else printf("D");
    }
    printf("\n");
*/

    // OUTPUT AS CHAR STRING
    for(i=0; i<Carrier.N_Bits; i++) buf[i]=BitsDisc[i]; write(1,buf,Carrier.N_Bits);
    }

  return 0;
}

