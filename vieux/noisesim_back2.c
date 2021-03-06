#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>
#include "declaraciones.h"

#define BUFFERMAX 4096


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
  (*gauss).r_ex= -99.0;						// [dB] Extinction ratio [-10 ~ -15 Grosz priv. comm. 090210]

  // SYSTEM
//  *transmitters=10;						// #simultaneous users
}


void WriteFile(char FileName[], float TrainTime[], float Train[], unsigned int TrainSize){
  unsigned long int i;
  FILE *File;
  File= fopen(FileName, "w");
  for(i=0; i<TrainSize; i++) fprintf(File,"%3e\t%3e\n", TrainTime[i], Train[i]);
  fclose(File);
}


void RZPulsesTrainGenerator(float RZPulsesTrain[], float PulsesTrainTime[], GENDRIVE *Carrier, SuperGaussian *gauss, float RZBit0[], float RZBit1Gauss[], float *LOff, unsigned char InBitString[], unsigned long int TrainLength, unsigned int transmitters){
  unsigned long int i;

  // '0' SLOT AMPLITUDE
  //float RZBit0[Carrier.NPtos_Bit]
  *LOff=(*gauss).A_0* pow(10,(*gauss).r_ex/20);			// optical amplitude for '0'
  //float LOff= 0.0;							// TEST: no extinction ratio
  for(i=0; i<(*Carrier).NPtos_Bit; i++) RZBit0[i]= transmitters* *LOff;		// base level for multiple transmitters

  // Time vector
  float aux=( (1.5- (float)(*Carrier).NPtos_Bit)/ (1- (float)(*Carrier).NPtos_Bit) );
  for(i=0; i<(*Carrier).NPtos_Bit; i++) PulsesTrainTime[i]= (*Carrier).dt* ( aux* i+ 0.5) ;	// time for one slot
  unsigned int k;
  for(k= 1;k<TrainLength;k++) for(i= 0;i<(*Carrier).NPtos_Bit;i++) PulsesTrainTime[k*(*Carrier).NPtos_Bit+ i]= PulsesTrainTime[i]+ (*Carrier).Bit_Slot* k;		// copies time to following slots

  // '1' SLOT OVER '0' LEVEL AMPLITUDE
  for(i=0; i<(*Carrier).NPtos_Bit; i++) RZBit1Gauss[i]= ((*gauss).A_0- *LOff)* exp(-0.5* pow(((PulsesTrainTime[i]- (0.25* (*Carrier).Bit_Slot))/ (*gauss).T_0_amp), 2* (*gauss).m));	// SuperGaussian
  //for(i=0; i<(*Carrier).NPtos_Bit; i++) RZBit1Gauss[i]= ((*gauss).A_0- *LOff);	// Square [NRZ]

  // OVERPULSE '1' CHECK
  //WriteFile("amplitude1.dat", PulsesTrainTime, RZBit1Gauss, (*Carrier).NPtos_Bit);	// TEST train to file


  // RZ PULSES TRAIN (Amplitude)
  unsigned int senders;
  k=0;
  while(k<TrainLength){
    senders= InBitString[k];
    for(i=0; i<(*Carrier).NPtos_Bit; i++){
      RZPulsesTrain[k*(*Carrier).NPtos_Bit+i]= (float) RZBit0[i];			// '0' base
      RZPulsesTrain[k*(*Carrier).NPtos_Bit+i]+= (float) senders* RZBit1Gauss[i];	// '1' pulses addition
      // Electric field addition is assumed instead of a intensity sum
    }
    k++;
  }
  //for(i=0; i<(*Carrier).NPtos_Tot; i++) printf("%.3e\t%.3e\n",RZPulsesTrainTime[i],RZPulsesTrain[i]);	// TEST
}




void Bits2Pulses(GENDRIVE *Carrier, SuperGaussian *gauss, unsigned char InBitString[], double OSNR, float *Pmean, double *OSigmaNoise, float RZPulsesTrain[], float RZPulsesTrainTime[], float RZBit0[], float RZBit1Gauss[], unsigned int transmitters, float *LOff){

  unsigned long int i;
//  float LOff;

  RZPulsesTrainGenerator(RZPulsesTrain, RZPulsesTrainTime, Carrier, gauss, RZBit0, RZBit1Gauss, LOff, InBitString, (*Carrier).N_Bits, transmitters);

  // OPTICAL NOISE (AWGN)

  // ADDITIVE WHITE GAUSSIAN NOISE [SANTIAGO'S GENERATOR]
  // double SigmaNoise;
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
  Power_'1bit'= P1Mean= \frac{1}{(*Carrier).Bit_Slot} \int_0^{(*Carrier).Bit_Slot} dt Pulse1(t)^2  integrated in bit slot
  OR? Power_'1bit'= \frac{1}{LimSup-LimInf} \int_LimInf^{LimSup} dt Pulse1(t)^2  integrated in detection window
  -> SigmaNoise=\sqrt(\frac{LOff^2 + P1Mean}{2 SNR})
  */


  // mean signal power -> gaussian sigma noise
  // float Pmean=0;
  for(i=0; i<(*Carrier).NPtos_Bit; i++) *Pmean+= RZBit1Gauss[i]* RZBit1Gauss[i];
  *Pmean= 0.5* ((*Pmean/ (*Carrier).NPtos_Bit )+ *LOff* *LOff);
  *OSigmaNoise= sqrt(*Pmean/ OSNR);	// amplitude of noise (Agrawal 4.4.11) [Santiago I.]
  //printf("OSNR= %e\tPmean= %e\tOSigmaNoise= %e\n",OSNR ,Pmean, *OSigmaNoise);		// TEST

  double ONoiseVector[(*Carrier).NPtos_Tot];
  GenRuidoRapido(ONoiseVector, *OSigmaNoise, (*Carrier).NPtos_Tot);		// NoiseVector amplitude
  //for(i=0; i<(*Carrier).NPtos_Tot; i++) printf("%.3e\t%.3e\n",PulsesTrainTime[i],RZPulsesTrain[i]);	// TEST
  //for(i=0; i<(*Carrier).NPtos_Tot; i++) printf("%.3e\t\n",(float) NoiseVector[i]);	// TEST
  WriteFile("ONoiseVector.dat", RZPulsesTrain, ONoiseVector, (*Carrier).NPtos_Tot);	// TEST train to file

  // Noise addition
  for(i=0; i<(*Carrier).NPtos_Tot; i++) RZPulsesTrain[i]+= ONoiseVector[i];		// Noise addition
}


void OpticalFilter(float SignalVector[], unsigned long int SignalVectorSize) {
  /* Digital filter designed by mkfilter/mkshape/gencode   A.J. Fisher
  Command line: /www/usr/fisher/helpers/mkfilter -Bu -Lp -o 2 -a 1.5625000000e-02 0.0000000000e+00 -l

  filtertype    =    Butterworth 
   passtype    =    Lowpass 
   order    =    2 
   samplerate    =    6.4E11 	= 2^6/ bit slot
   corner1    =    25E9
  */

  #define NZEROS 2
  #define NPOLES 2
  #define oGAIN   7.820233128e+01
  static float xv[NZEROS+1], yv[NPOLES+1];
  unsigned long int i;

  for (i=0; i<SignalVectorSize; i++) {
    xv[0] = xv[1]; xv[1] = xv[2]; 
    xv[2] = SignalVector[i] / oGAIN;
    yv[0] = yv[1]; yv[1] = yv[2]; 
    yv[2] =   (xv[0] + xv[2]) + 2 * xv[1]
      + ( -0.7067570632 * yv[0]) + (  1.6556076929 * yv[1]);
    SignalVector[i] = yv[2];
  }
}


void ElectricalFilter(float SignalVector[], unsigned long int SignalVectorSize) {
  /* Digital filter designed by mkfilter/mkshape/gencode   A.J. Fisher
  Command line: /www/usr/fisher/helpers/mkfilter -Bu -Lp -o 2 -a 1.5625000000e-02 0.0000000000e+00 -l

  filtertype    =    Butterworth 
   passtype    =    Lowpass 
   order    =    2 
   samplerate    =    6.4E11 	= 2^6/ bit slot
  */

  #define NZEROS 2
  #define NPOLES 2
  static float xve[NZEROS+1], yve[NPOLES+1];
  // BW= 10GHz
  // #define eGAIN   4.441320406e+02
  // BW= 5GHz
  // #define eGAIN   1.717988320e+03
  // BW= 6GHz
  #define eGAIN   1.201146288e+03

  unsigned long int i;
  for (i=0; i<SignalVectorSize; i++) {
    xve[0] = xve[1]; xve[1] = xve[2]; 
    xve[2] = SignalVector[i] / eGAIN;
    yve[0] = yve[1]; yve[1] = yve[2];
    yve[2] =   (xve[0] + xve[2]) + 2 * xve[1]
//      + ( -0.8703674775 * yve[0]) + (  1.8613611468 * yve[1]);	// BW= 10GHz
//      + ( -0.9329347318 * yve[0]) + (  1.9306064272 * yve[1]);	// BW= 5GHz
      + ( -0.9200713754 * yve[0]) + (  1.9167412232 * yve[1]);	// BW= 6GHz
    SignalVector[i] = yve[2];
  }
}


void RandIntString(unsigned char ThrStr[], int NPulThr){
  unsigned long int i;
  srand ( time(NULL) );
  for(i= 0; i< NPulThr; i++){
    if ( (rand()/(RAND_MAX/2))==1) ThrStr[i]= 1;
    else ThrStr[i]= 0;
  }
}


float Threshold1(GENDRIVE *Carrier, float RZBit0[], float RZBit1Gauss[], double *OSigmaNoise, double DSNR, double *DSigmaNoise, float *Pmean){
  // DISCRIMINATION BY DISPERSION THRESHOLD [Agrawal 4.5]
  unsigned long int i;
  unsigned short int Pul, NPulThr=5;			// averaged bits to obtain threshold
  unsigned int Min,Max;
 
  // EYE DIAGRAM REQUIRED TO SELECT MIN & MAX
  Min= 40;
  Max= 50;

  float One[(*Carrier).NPtos_Bit], Zero[(*Carrier).NPtos_Bit];
  float sum0, sum_sqr0, sum1, sum_sqr1;
  float n= Max- Min+1;
  float Id=0, I0, I1, sigma0, sigma1;

  // OPTICAL NOISE VECTOR FOR THRESHOLD DETERMINATION BITS
  double ONoiseVector[2*(*Carrier).NPtos_Bit* NPulThr];
  // printf("OSigmaNoise=\t%e\n",*OSigmaNoise);			// TEST
  GenRuidoRapido(ONoiseVector, *OSigmaNoise, 2*(*Carrier).NPtos_Bit* NPulThr);		// NoiseVector amplitude

  // DETECTOR (THERMAL) NOISE VECTOR FOR THRESHOLD DETERMINATION BITS
  double DNoiseVector2[2*(*Carrier).NPtos_Bit* NPulThr];
  *DSigmaNoise= *Pmean* sqrt(1/ DSNR);	// amplitude of noise (Agrawal 4.4.11) [Santiago I.]
  // printf("DSigmaNoise=\t%e\n",*SigmaNoise);			// TEST
  GenRuidoRapido(DNoiseVector2, *DSigmaNoise, 2*(*Carrier).NPtos_Bit* NPulThr);		// NoiseVector amplitude

  // int reject=0;
  for(Pul=0; Pul<NPulThr; Pul++){
    sum0= 0, sum_sqr0= 0, sum1= 0, sum_sqr1= 0;

    // DETERMINATION BITS: CREATION & NOISE ADDITION
    for(i=0; i<(*Carrier).NPtos_Bit; i++){
      One[i]= RZBit1Gauss[i]+ ONoiseVector[i+(*Carrier).NPtos_Bit*Pul];	// optical noise addition      
      Zero[i]= RZBit0[i]+ ONoiseVector[i+(*Carrier).NPtos_Bit*(Pul+NPulThr)];	// optical noise addition
    }
    OpticalFilter(One, (*Carrier).NPtos_Bit);
    OpticalFilter(Zero, (*Carrier).NPtos_Bit);

    // DETERMINATION BITS: CREATION & NOISE ADDITION
    for(i=0; i<(*Carrier).NPtos_Bit; i++){
      One[i]*= One[i];							// optical amplitude -> optical power= electrical current
      One[i]+= DNoiseVector2[i+(*Carrier).NPtos_Bit*Pul]; 		// electrical noise addition
      Zero[i]*= Zero[i];				  		// optical amplitude -> optical power= electrical current
      Zero[i]+= DNoiseVector2[i+(*Carrier).NPtos_Bit*(Pul+NPulThr)]; 	// electrical noise addition
    }
    ElectricalFilter(One, (*Carrier).NPtos_Bit);
    ElectricalFilter(Zero, (*Carrier).NPtos_Bit);

    for(i=Min; i<Max+1; i++){
      // printf("One%i= %e\n",i,One[i]); // TEST
      sum1+= One[i];
      One[i]*= One[i];
      sum_sqr1+= One[i];

      sum0+= Zero[i];
      Zero[i]*= Zero[i];
      sum_sqr0+= Zero[i];
    }
    I1= sum1/n;		// moyenne1
    I0= sum0/n;		// moyenne0

    //printf("n= %f\n",n); // TEST
    //printf("sum0= %.3e\tsum1= %.3e\n",sum0, sum1); // TEST
    //printf("sum_sqr0= %.3e\tsum_sqr1= %.3e\n",sum_sqr0, sum_sqr1); // TEST

    sigma1= sqrt(( 1/ ( n- 1)) * (sum_sqr1 - ( (sum1* sum1)/ n) ) );
    sigma0= sqrt(( 1/ ( n- 1)) * (sum_sqr0 - ( (sum0* sum0)/ n) ) );

    /*
    if (sigma0 != sigma0 && sigma1 != sigma1) reject++;
    else {
      if (sigma0 != sigma0) sigma0= 0;	// if sigma0 too small (NaN), sigma0 -> 0
      if (sigma1 != sigma1) sigma1= 0;	// if sigma1 too small (NaN), sigma1 -> 0
    */
    //printf("sum0= %e\tsum1= %e\n",sum0,sum1); // TEST
    //printf("Sigma0p= %e\tSigma1p= %e\n",sigma0,sigma1); // TEST
    //printf("I0p= %e\tI1p= %e\n",I0,I1); // TEST
    //}
    Id+= (sigma0* I1+ sigma1* I0)/ (sigma0+ sigma1);
    //printf("Id[%i]= %e\n",Pul,Id); // TEST
  }
  //Id*= (1/ (float) (NPulThr- reject) );
  Id*= 1/ (float) NPulThr;

  return Id;
}



float Threshold2(GENDRIVE *Carrier, SuperGaussian *gauss, float RZBit0[], float RZBit1Gauss[], float *LOff, double *OSigmaNoise, double DSNR, double *DSigmaNoise, float *Pmean){
  unsigned long int i;
  unsigned short int Pul, NPulThr=50;		// averaged bits to obtain threshold
  unsigned char ThrStr[NPulThr];
  float RZPulsesTrainThr[(*Carrier).NPtos_Bit* NPulThr], RZPulsesTrainThrTime[(*Carrier).NPtos_Bit* NPulThr];

  RandIntString(ThrStr, NPulThr);	// RANDOM BITS TRAIN GENERATOR
  RZPulsesTrainGenerator(RZPulsesTrainThr, RZPulsesTrainThrTime, Carrier, gauss, RZBit0, RZBit1Gauss, LOff, ThrStr,NPulThr, 1);

  // OPTICAL NOISE FOR THRESHOLD DETERMINATION BITS
  double ONoiseVector[(*Carrier).NPtos_Bit* NPulThr];
  // printf("OSigmaNoise=\t%e\n",*OSigmaNoise);			// TEST
  GenRuidoRapido(ONoiseVector, *OSigmaNoise, (*Carrier).NPtos_Bit* NPulThr);
  for(i=0; i<(*Carrier).NPtos_Bit* NPulThr; i++) RZPulsesTrainThr[i]+= ONoiseVector[i];   // NOISE ADDITION

  // OPTICAL FILTER
  OpticalFilter(RZPulsesTrainThr, (*Carrier).NPtos_Bit* NPulThr);

  // DETECTOR (THERMAL) NOISE VECTOR FOR THRESHOLD DETERMINATION BITS
  *DSigmaNoise= *Pmean* sqrt(1/ DSNR);	// amplitude of noise (Agrawal 4.4.11) [Santiago I.]
  //printf("DSNR= %e\tPmean= %e\tDSigmaNoise= %e\n",DSNR ,Pmean, *DSigmaNoise);					// TEST
  double DNoiseVector[(*Carrier).NPtos_Bit* NPulThr];
  // printf("DSigmaNoise=\t%e\n",*SigmaNoise);			// TEST
  GenRuidoRapido(DNoiseVector, *DSigmaNoise, (*Carrier).NPtos_Bit* NPulThr);
  for(i=0; i<(*Carrier).NPtos_Bit* NPulThr; i++){
    RZPulsesTrainThr[i]*= RZPulsesTrainThr[i];
    RZPulsesTrainThr[i]+= DNoiseVector[i];	  // NOISE ADDITION
  }

  // ELECTRICAL FILTER
  ElectricalFilter(RZPulsesTrainThr, (*Carrier).NPtos_Bit* NPulThr);

  // SAMPLING RANGE (SELECTED AT EYE DIAGRAM)
  //unsigned int Min= 47, Max= 58; 		// centred pulse
  unsigned int Min= 40, Max= 50;	// 0.25 centred pulse
  unsigned short int nZero=0, nOne=0;
  float n= Max- Min+1;
  float sum, sum_sqr, I, sigma;
  float I0= 0, I1= 0, sigma0= 0, sigma1= 0;
  float Id;

  for(Pul=0; Pul<NPulThr; Pul++){
    sum= 0, sum_sqr= 0;
    for(i= (*Carrier).NPtos_Bit*Pul+ Min; i<(*Carrier).NPtos_Bit*Pul+ Max; i++){
      sum+= RZPulsesTrainThr[i];
      RZPulsesTrainThr[i]*= RZPulsesTrainThr[i];
      sum_sqr+= RZPulsesTrainThr[i];
    }
    I= sum/ n;
    sigma= sqrt(( 1/ ( n- 1)) * (sum_sqr - ( (sum* sum)/ n) ) );
    //printf("I= %e\tsigma= %e\n",I, sigma); 		// TEST

    if(ThrStr[Pul]== 0){
      nZero++;
      I0+= I;
      sigma0+= sigma;
    }
    else{
      nOne++;
      I1+= I;
      sigma1+= sigma;
    }
  }
  I0*= 1/(float) nZero;
  sigma0*= 1/(float) nZero;
  I1*= 1/(float) nOne;
  sigma1*= 1/(float) nOne;
  Id= (sigma0* I1+ sigma1* I0)/ (sigma0+ sigma1);
  //printf("Id= %e\n",Id); 		// TEST

  return Id;
}


void Pulses2Bits(GENDRIVE *Carrier, float RZPulsesTrain[], float RZPulsesTrainTime[], float Id, double *DSigmaNoise, char BitsDisc[]){
  unsigned long int i;

  OpticalFilter(RZPulsesTrain, (*Carrier).NPtos_Tot);

  // Optical -> Electrical signal
  for(i=0; i<(*Carrier).NPtos_Tot; i++) RZPulsesTrain[i]*= RZPulsesTrain[i];	// optical amplitude-> optical power= electrical current (detector responsivity R= 1)

  // 'THERMAL' NOISE AT DETECTOR
  double DNoiseVector[(*Carrier).NPtos_Tot];						// Thermal noise vector
  GenRuidoRapido(DNoiseVector, *DSigmaNoise, (*Carrier).NPtos_Tot);		 	// NoiseVector amplitude
  for(i=0; i<(*Carrier).NPtos_Tot; i++)	RZPulsesTrain[i]+= DNoiseVector[i];		// Noise addition to electrical current signal

  //WriteFile("antes.dat", RZPulsesTrainTime, RZPulsesTrain, (*Carrier).NPtos_Tot);	// TEST train to file

  ElectricalFilter(RZPulsesTrain, (*Carrier).NPtos_Tot);

  //WriteFile("despues.dat", RZPulsesTrainTime, RZPulsesTrain, (*Carrier).NPtos_Tot);	// TEST train to file

  // SAMPLE: CENTRAL SINGLE POINT OF BIT SLOT
  //int SamplingPoint= floor(0.5* (*Carrier).NPtos_Bit);		// bit slot middle point
  //int SamplingPoint= 52;		// centred pulse
  int SamplingPoint= 45;		// 0.25 centred pulse
  float Courrants[(*Carrier).N_Bits];
//  printf("Id= %e\n",Id); 		// TEST

  for(i=0; i<(*Carrier).N_Bits; i++){
    Courrants[i]= RZPulsesTrain[SamplingPoint+ i* (*Carrier).NPtos_Bit];
//    printf("I[%lu]= %e\n",i,Courrants[i]); 		// TEST
  }

  // DISCRIMINATION BY THRESHOLD
  for(i=0; i<(*Carrier).N_Bits; i++){
    if(Courrants[i]>Id) BitsDisc[i]=1;
    else BitsDisc[i]=0;
  }
}


void EyeDiagram(float PulsesTrain[], float PulsesTrainTime[], GENDRIVE *Carrier){
  unsigned long int i, iMidi= 0.5*(*Carrier).NPtos_Bit;
  FILE *EYE;
  EYE= fopen("EyeDiagram.dat", "w");
  for(i=0; i<(*Carrier).NPtos_Tot; i++){
    ldiv_t thing= ldiv(i,(*Carrier).NPtos_Bit);
    if(thing.rem< iMidi){
      fprintf(EYE,"%3e\t%3e\n",PulsesTrainTime[thing.rem], PulsesTrain[i]);
      fprintf(EYE,"%3e\t%3e\n",PulsesTrainTime[thing.rem]+ (*Carrier).Bit_Slot, PulsesTrain[i]);
    }
    else{
      fprintf(EYE,"%3e\t%3e\n",PulsesTrainTime[thing.rem], PulsesTrain[i]);
      fprintf(EYE,"%3e\t%3e\n",PulsesTrainTime[thing.rem]- (*Carrier).Bit_Slot, PulsesTrain[i]);
    }
  }
  fclose(EYE);
}


int main(int argc, char **argv){
  if (argc<4){
    printf("Usage:noisesim #Tx DSNR[dB] OSNR[dB] <InputBitStream >OutputBitStream\n");
    exit(0);
  }
  unsigned char buf[BUFFERMAX];
  int maxdata=BUFFERMAX;
  unsigned long int i;
  GENDRIVE Carrier;
  SuperGaussian gauss;
  unsigned int transmitters= atoi(argv[1]);			// Argument: number of Tx
  double DSNR= pow(10,atof(argv[2])/10);			// Argument: DSNR [dB] -> DSNR [1]
  double OSNR= pow(10,atof(argv[3])/10);			// Argument: OSNR [dB] -> OSNR [1]
  double OSigmaNoise, DSigmaNoise;
  float Pmean=0;
  while((Carrier.N_Bits=fread(buf,1,maxdata,stdin))>0){

    // INPUTS READ
    unsigned char InBitString[Carrier.N_Bits];
    for(i=0; i<Carrier.N_Bits; i++) InBitString[i]=buf[i];
    //printf("#bits: %i\tSNR: %f\n",Carrier.N_Bits,DSNR);		// TEST
    //printf("bits: %s\n",InBitString); 			// TEST

    // SYSTEM PARAMETERS
    parameters(&Carrier, &gauss, &transmitters);

    // BITS -> PULSES
    float RZPulsesTrain[Carrier.NPtos_Tot];
    float RZPulsesTrainTime[Carrier.NPtos_Tot];
    float RZBit0[Carrier.NPtos_Bit], RZBit1Gauss[Carrier.NPtos_Bit];
    float LOff;
    Bits2Pulses(&Carrier, &gauss, InBitString, OSNR, &Pmean, &OSigmaNoise, RZPulsesTrain, RZPulsesTrainTime, RZBit0, RZBit1Gauss, transmitters, &LOff);

    // DETECTOR THRESHOLD
    float Id= Threshold1(&Carrier, RZBit0, RZBit1Gauss, &OSigmaNoise, DSNR, &DSigmaNoise, &Pmean);
    //float Id= Threshold2(&Carrier, &gauss, RZBit0, RZBit1Gauss, &LOff, &OSigmaNoise, DSNR, &DSigmaNoise, &Pmean);

    // PULSES -> BITS
    char BitsDisc[Carrier.N_Bits];
    Pulses2Bits(&Carrier, RZPulsesTrain, RZPulsesTrainTime, Id, &DSigmaNoise, BitsDisc);

    //EyeDiagram(RZPulsesTrain, RZPulsesTrainTime, &Carrier);	// TEST Eye diagram
    //WriteFile("train.dat", RZPulsesTrainTime, RZPulsesTrain, Carrier.NPtos_Tot);	// TEST train to file

    // OUTPUT AS CHAR STRING
    for(i=0; i<Carrier.N_Bits; i++) buf[i]=BitsDisc[i]; write(1,buf,Carrier.N_Bits);
    }

  return 0;
}

