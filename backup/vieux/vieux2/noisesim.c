#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "declaraciones.h"

#define BUFFERMAX 256

void parameters(GENDRIVE *Erste, SuperGaussian *gauss, unsigned int *clients){
  (*Erste).NPtos_Bit=64;					// # points for each bit slot
  (*Erste).NPtos_Tot=(*Erste).NPtos_Bit*(*Erste).N_Bits;	// Total # points
  (*Erste).Duty_Cycle=0.5;
  (*Erste).B=40E9;						// bit rate [bits/s] (40 Gbits/s)
  (*Erste).Bit_Slot=1/(*Erste).B;				// 1/B B: bit rate [s]
  (*Erste).dt=1/( (*Erste).B* (*Erste).NPtos_Bit );		// time resolution [s]
  //  printf("Bit_Slot= %.3f ps\n",(*Erste).Bit_Slot*1E12);	
  //  printf("Resolucion temporal= %.3f ps\n",(*Erste).dt*1E12);

  // OPTICAL PULSE DEFINITION
  (*gauss).P_0=2.5;					// [mW] www.afop.com/pdf/Active%20Devices/GE-PON%20BOSA.pdf
  (*gauss).A_0=sqrt((*gauss).P_0);			// [sqrt{mW}]
  (*gauss).T_0=2.5E-12;					// [s]
  //  (*gauss).T_0=0.25*(*Erste).Duty_Cycle*(*Erste).Bit_Slot;	// as Duty_Cycle fraction 
  (*gauss).m=1;						// gaussian=1
  (*gauss).r_ex=-10.0;					// [dB] Extinction ratio [-10 ~ -15 Grosz priv. comm. 090210]

  // SYSTEM
//  *clients=10;						// #simultaneous users
}



void Bits2Pulses(GENDRIVE *Erste, SuperGaussian *gauss, unsigned char InBitString[], double OSNR, float RZPulsesTrain[][2], float RZBit0[], float RZBit1Gauss[], double NoiseVector[], unsigned int clients){
  unsigned int i;

  // '0' LEVEL
  //float RZBit0[Erste.NPtos_Bit]
  float LOff=(*gauss).A_0* pow(10,(*gauss).r_ex/20);			// optical amplitude for '0'
  for(i=0; i<(*Erste).NPtos_Bit; i++) RZBit0[i]= clients* LOff* LOff;	// base level for multiple clients in power


  // '1' OVER LEVEL PULSE
  float Q0_Pts=-0.5* pow( (*Erste).dt/ (*gauss).T_0,2* (*gauss).m);
  int j;
  for(i=0,j=-Q0_Pts ; i<=Q0_Pts; i++,j++){
    RZBit1Gauss[i]=  ( (*gauss).A_0- LOff)* exp( -Q0_Pts* pow(j,2* (*gauss).m) );		// gaussian over base line
    RZBit1Gauss[i]*= RZBit1Gauss[i];	// amplitude->power
    RZBit1Gauss[(*Erste).NPtos_Bit-i-1]= RZBit1Gauss[i];
  }

  
  /*
  // '1' OVER LEVEL PULSE
  //float RZBit1Gauss[Erste.NPtos_Bit];
  float Q0_Pts= 1/(2* (*Erste).NPtos_Bit* (*Erste).B* (*gauss).T_0);
  Q0_Pts*=Q0_Pts;
  int j, q=0.5*(*Erste).NPtos_Bit;
  for(i=0,j=-q ; i<=q; i++,j++){
    RZBit1Gauss[i]=  ((*gauss).A_0- LOff)* exp(-Q0_Pts* j* j);		// gaussian over base line
    //RZBit1Gauss[i]= LOff+ ( (*gauss).A_0- LOff)* exp(-Q0_Pts* j* j);
    RZBit1Gauss[i]*=RZBit1Gauss[i];	// amplitude->power
//    RZBit1Gauss[i]+=RZBit0[i];		// adding base power
    RZBit1Gauss[(*Erste).NPtos_Bit-i-1]=RZBit1Gauss[i];
  }
  //for(i=0; i<(*Erste).NPtos_Bit; i++) printf("%i\t%.3e\t%.3e\n",i,RZBit0[i],RZBit1Gauss[i]);	// TEST
  */


  // OVERPULSE '1' CHECK
  FILE *TestFile;
  TestFile= fopen("power1.dat", "w");
  fprintf(TestFile,"#time [s]\tPower [mW]\n");
  for(i=0; i<(*Erste).NPtos_Bit; i++) fprintf(TestFile,"%3e\n",RZBit1Gauss[i]);
  fclose(TestFile);


  // RZ PULSES TRAIN (POWER)
  //float RZPulsesTrain[(*Erste).NPtos_Tot][2];
  for(i=0; i<(*Erste).NPtos_Tot; i++) RZPulsesTrain[i][1]=(*Erste).dt*i;	// time vector
  unsigned int k=0, senders;
  while(k<(*Erste).N_Bits){
    senders= InBitString[k];
    for(i=0; i<(*Erste).NPtos_Bit; i++){
      RZPulsesTrain[k*(*Erste).NPtos_Bit+i][2]= (float) RZBit0[i];
      RZPulsesTrain[k*(*Erste).NPtos_Bit+i][2]+= (float) senders* RZBit1Gauss[i];
    }
    k++;
  }

  //for(i=0; i<(*Erste).NPtos_Tot; i++) printf("%.3e\t%.3e\n",RZPulsesTrain[i][1],RZPulsesTrain[i][2]);	// TEST

  // ADDITIVE WHITE GAUSSIAN NOISE [SANTIAGO'S GENERATOR]
  double SigmaNoise;
  /*
  SigmaNoise: [V/m*10-3] width of amplitude gaussian distribution A(t) related to mean noise power as follows:

  white noise autocorrelation = \sigma^2 * delta(t)  [http://en.wikipedia.org/wiki/White_noise]
  Wiener-Khinchin: \phi(\omega) [Power Spectral Density]= Fourier(white noise autocorrelation )  [http://en.wikipedia.org/wiki/Wiener�Khinchin_theorem]
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
  Power_'1bit'= P1Mean= \frac{1}{(*Erste).Bit_Slot} \int_0^{(*Erste).Bit_Slot} dt Pulse1(t)^2  integrated in bit slot
  OR? Power_'1bit'= \frac{1}{LimSup-LimInf} \int_LimInf^{LimSup} dt Pulse1(t)^2  integrated in detection window
  -> SigmaNoise=\sqrt(\frac{LOff^2 + P1Mean}{2 SNR})
  */
  float Pmean=0;
  for(i=0; i<(*Erste).NPtos_Bit; i++) Pmean+= 0.5* ( (RZBit0 [i]+ RZBit1Gauss[i] )/ (float) (*Erste).NPtos_Bit);
  SigmaNoise= sqrt( Pmean /OSNR );	// power noise (Agrawal 4.4.11) [Santiago I.]
  //double NoiseVector[(*Erste)..NPtos_Tot];
  GenRuidoRapido(NoiseVector, SigmaNoise, (*Erste).NPtos_Tot);
  for(i=0; i<(*Erste).NPtos_Tot; i++) RZPulsesTrain[i][2]+= (float) (NoiseVector[i]);
  //for(i=0; i<(*Erste).NPtos_Tot; i++) printf("%.3e\t%.3e\n",RZPulsesTrain[i][1],RZPulsesTrain[i][2]);	// TEST
  //for(i=0; i<(*Erste).NPtos_Tot; i++) printf("%.3e\t\n",(float) NoiseVector[i]);	// TEST
}


void Pulses2Bits(GENDRIVE *Erste, float RZPulsesTrain[][2], float RZBit0[], float RZBit1Gauss[], double NoiseVector[], char BitsDisc[]){
  unsigned int i;

  // DETECCION: Filtrado optico pasabajo Butterworth (Como corrijo por Duty Cycle?) [Pendiente]
  double PropInt=0.4;					// CENTRED BIT SLOT FRACTION TO INTEGRATE

  // PULSE INTEGRATION IN A CENTRED BIT SLOT INTERVAL
  float Energies[(*Erste).N_Bits];
  unsigned int step,Min,Max;
  Min= (int) floor((*Erste).NPtos_Bit*(0.5*(1-PropInt)));
  Max= (int) ceil((*Erste).NPtos_Bit*(0.5*(1+PropInt)));
  for(i=0; i<(*Erste).N_Bits; i++){
    Energies[i]=0;
        for(step=Min; step<Max+1; step++) Energies[i]+= RZPulsesTrain[step+ i* (*Erste).NPtos_Bit][2];		// power RZPulsesTrain
    //for(step=Min; step<Max+1; step++) Energies[i]+= RZPulsesTrain[step+ i* (*Erste).NPtos_Bit][2] * RZPulsesTrain[step+ i* (*Erste).NPtos_Bit][2];	// amplitude RZPulsesTrain
    Energies[i]*= (*Erste).dt* 1E+3;
  }

  // DISCRIMINATION BY DISPERSION THRESHOLD [Agrawal 4.5]
  float EThresh;
  unsigned short int NPulThr, Pul;
  if((*Erste).N_Bits<10) NPulThr=floor((*Erste).N_Bits/2);
  else NPulThr=5;
  float One[(*Erste).NPtos_Bit], Zero[(*Erste).NPtos_Bit];
  float sum0= 0, sum_sqr0= 0, sum1= 0, sum_sqr1= 0;
  float n= Max- Min+1;
  float Id=0, I0=0, I1=0, sigma0=0, sigma1=0;
  for(Pul=0; Pul<NPulThr; Pul++){
    for(i=0; i<(*Erste).NPtos_Bit; i++){
      One[i]=RZBit1Gauss[i]+NoiseVector[i+(*Erste).NPtos_Bit*Pul];
      Zero[i]=RZBit0[i]+NoiseVector[i+(*Erste).NPtos_Bit*(Pul+NPulThr)];
      //printf("tot: %d\tlleg: %d\n",(*Erste).NPtos_Tot,i+(*Erste).NPtos_Bit*(Pul+NPulThr));
    }
    sum0= 0, sum_sqr0= 0, sum1= 0, sum_sqr1= 0;
    for(step=Min; step<Max+1; step++){
//      One[step]*=One[step];	// for amplitude pulses
      sum1+=One[step];
      One[step]*=One[step];
      sum_sqr1+= One[step];
//      Zero[step]*=Zero[step];	// for amplitude pulses
      sum0+= Zero[step];
      Zero[step]*=Zero[step];
      sum_sqr0+= Zero[step];
    }
    I1= sum1/n;		// moyenne1
    I0= sum0/n;		// moyenne0
    //printf("n= %.3e\tsum_sqr0= %.3e\tsum_sqr1= %.3e\tsum0= %.3e\tsum1= %.3e\n",n, sum_sqr0, sum_sqr1, sum0, sum1); // TEST
    sigma1= sqrt(( 1/ ( n- 1)) * (sum_sqr1 + (sum1* sum1)/ n) );
    sigma0= sqrt(( 1/ ( n- 1)) * (sum_sqr0 + (sum0* sum0)/ n) );
    //printf("Sigma0p= %e\n",sigma0); // TEST
    Id+= (1/ (float) NPulThr)* ((sigma0* I1+ sigma1* I0)/ (sigma0+ sigma1));
  }
  EThresh= Id* n* (*Erste).dt* 1E3;		// energie
  //printf("I0= %.3e\tId= %.3e\tI1= %.3e\n",I0, Id, I1); // TEST
  //printf("S0= %.3e\t\t\tS1= %.3e\t\n",sigma0,sigma1); // TEST
  //printf("Umbral= %e\n",Id,EThresh); // TEST

  // THRESHOLD DISCRIMINATION
  for(i=0; i<(*Erste).N_Bits; i++){
    if(Energies[i]>EThresh) BitsDisc[i]=1;
    else BitsDisc[i]=0;
  }
  //BitsDisc[(*Erste).N_Bits]='\0';
  //for(i=0; i<(*Erste).N_Bits; i++) printf("%c",BitsDisc[i]);
  //printf("\n");
}


int main(int argc, char **argv) 
{
  if (argc<3){
    printf("Usage:\nnoisesim #Tx SNR[dB] <InputBitStream >OutputBitStream\n");
    exit(0);
  }
  unsigned char buf[BUFFERMAX];
  int maxdata=BUFFERMAX;
  unsigned int i;
  GENDRIVE Erste;
  SuperGaussian gauss;
  unsigned int clients= atoi(argv[1]);				// number of Tx
  double OSNR= pow(10,atof(argv[2])/10);			// OSNR [dB] AS ARGUMENT
  while((Erste.N_Bits=fread(buf,1,maxdata,stdin))>0){

    // INPUTS READ
    unsigned char InBitString[Erste.N_Bits];
    for(i=0; i<Erste.N_Bits; i++) InBitString[i]=buf[i];
    //printf("#bits: %i\tSNR: %f\n",Erste.N_Bits,OSNR);
    //printf("bits: %s\n",InBitString);

    // SYSTEM PARAMETERS
    parameters(&Erste, &gauss, &clients);

    // BITS -> PULSES
    float RZPulsesTrain[Erste.NPtos_Tot][2];
    float RZBit0[Erste.NPtos_Bit], RZBit1Gauss[Erste.NPtos_Bit];
    double NoiseVector[Erste.NPtos_Tot];
    Bits2Pulses(&Erste, &gauss, InBitString, OSNR, RZPulsesTrain, RZBit0, RZBit1Gauss, NoiseVector, clients);

    // PULSES -> BITS
    char BitsDisc[Erste.N_Bits];
    Pulses2Bits(&Erste, RZPulsesTrain, RZBit0, RZBit1Gauss, NoiseVector, BitsDisc);

    // PROGRAMME OUTPUTS
    // for(i=0; i<Erste.NPtos_Tot; i++) printf("%3e\t%3e\n",RZBit1Gauss[i],RZPulsesTrain[i][2]);


    // POWER VS TIME FILE
    FILE *TrainFile;
    TrainFile= fopen("power.dat", "w");
    fprintf(TrainFile,"#time [s]\tPower [mW]\n");
    for(i=0; i<Erste.NPtos_Tot; i++) fprintf(TrainFile,"%3e\t%3e\n",RZPulsesTrain[i][1],RZPulsesTrain[i][2]);
    fclose(TrainFile);

    
    /*
    // INPUT / OUTPUT COMPARISON
    printf("BitsEntrada:\t");
    for(i=0; i<Erste.N_Bits; i++) printf("%c",InBitString[i]);
    printf("\nBitsSalida :\t");
    for(i=0; i<Erste.N_Bits; i++) printf("%c",BitsDisc[i]);
    printf("\nDiferencias:\t");
    for(i=0; i<Erste.N_Bits; i++){
      if(InBitString[i]==BitsDisc[i]) printf(" ");
      else printf("D");
    }
    printf("\n");
    */

    // OUTPUT AS CHAR STRING
    for(i=0; i<Erste.N_Bits; i++) buf[i]=BitsDisc[i];
    write(1,buf,Erste.N_Bits);
    }
  //printf("\nclients: %i\n",clients);
  return 0;
}

