#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>
 
#define BUFFERMAX 4096

typedef struct {	// TRANSMISSION SYSTEM DEFINITION
  // PHYSICAL PARAMETERS
  float Bit_Slot;		// bit slot period
  float Duty_Cycle;	
  float B;		// bit rate [bits/s]
  float dt;		// time resolution

  // NUMERICAL PARAMETERS
  unsigned int      N_Bits;	// # bits
  unsigned int      NPtos_Bit;	// # samples per bit slot
  unsigned long int NPtos_Tot;	// total # samples
} GENDRIVE;


typedef struct {	// DEVICES DESCRIPTION 
  float alpha;		// [dB/km] fibre attenuation coefficient
  float length;		// [km] total fibre path length
  float FAtt1;		// [dB] Attenuation Tx -> EDFAin
  float EDFAgain;	// [dB] Gain at EDFA <fixed approx>
  float FAtt2;		// [dB] Attenuation EDFAout -> Rx
  float R;		// photodetector (PD) responsivity
  int Efilt_CutOff;	// [GHz] PD electrical filter bandwidth
  int Ofilt_CutOff;	// [GHz] PD optical input filter bandwidth
  float r_BW;		// reference bandwidth
} DeviceSTR;


typedef struct {	// GAUSSIAN PULSE DEFINITION
  float P;		// [mW] Tx output power
  float P_peak;		// [mW] peak power 	 
  float A_peak;		// [sqrt{mW}] peak amplitude  |A_peak|^2= P_peak
  float P_0;		// [mW] bit 0 power 	 
  float A_0;		// [sqrt{mW}] bit 0 amplitude
  float C;		// [1] chirp parameter
  float CentringFactor;	// [1] pulse position factor, as bit slot proportion (<1)
  float FWHM_pow;	// [s] FWHM signal in power
  float T_0_amp;	// [s] T_0=T_{fwhmPOW}/(2* sqrt(ln(2) ) ) half of width at 1/e intensity point = gaussian sigma
  unsigned int m;	// Super gaussian pulse shape factor (gaussian=1)
  float r_ex;		// [1] Extinction ratio (P_1/ P_0) [Agrawal (4.6.1)]
} SuperGaussian;


void InputParameters(GENDRIVE *Carrier, SuperGaussian *gauss, DeviceSTR *Network) {

  (*Carrier).NPtos_Bit= pow(2,6);				 		// # points for each bit slot
  // (*Carrier).NPtos_Tot= (*Carrier).NPtos_Bit*(*Carrier).N_Bits;		// Total # points
  (*Carrier).Duty_Cycle= 1.0/3.0;
  (*Carrier).B= 10E9;								// bit rate [bits/s] (10 Gbits/s)
  (*Carrier).Bit_Slot= 1/ (*Carrier).B;						// 1/B [s]
  // (*Carrier).dt= (*Carrier).Bit_Slot/( ( (*Carrier).NPtos_Bit- 1)+ 0.5);	// time step [s] [OPTIMISED]
  (*Carrier).dt= (*Carrier).Bit_Slot/ (*Carrier).NPtos_Bit;			// time step [s] [NON OPTIMISED]
  //  printf("Bit_Slot= %.3f ps\n",(*Carrier).Bit_Slot*1E12);	
  //  printf("Resolucion temporal= %.3f ps\n",(*Carrier).dt*1E12);

  // (*Network).alpha= 0.21;				// [dB/km] Corning ClearCurve ITU-T G.652
  // (*Network).length= 20;				// [km] max total path length

  float FAtt1= -2.0 -1.0 -25.0;				// [dB] Tx -> EDFAin 2dB fibre, 1.0dB insertion, 25 dB 128x1 splitter
  (*Network).FAtt1= pow(10, FAtt1/ 20);			// [1] Tx -> EDFAin for amplitude
  float FAtt2= -2.0 -1.0 -25.0;				// [dB] EDFAout -> Rx 2dB fibre, 1.0dB insertion, 25 dB 1x128 splitter
  (*Network).FAtt2= pow(10.0, FAtt2/ 20.0);		// [1] EDFAout -> Rx amplitude
  float EDFAgain= 27.0;					// [dB] Gain at EDFA <fixed approx>
  (*Network).EDFAgain= pow(10.0, EDFAgain/ 20.0);	// [1] Gain at EDFA amplitude

  (*Network).R= 1;					// detector responsivity R= 1

  (*Network).Efilt_CutOff= 7;				// [GHz] cut-off freq electrical filter
  (*Network).Ofilt_CutOff= 17.5;			// [GHz] cut-off freq optical filter
  float c= 299792458;  					// [m/s] light speed
  float Lambda_0= 1550;					// [nm] carrier wavelength
  float Br= 0.1;					// [nm] 0.1 nm reference bandwidth
  (*Network).r_BW= 1E9* c* Br/ (Lambda_0* Lambda_0);	// [Hz] 0.1 nm reference -> frequency range
  // printf("BW= %e\n",(*Network).r_BW);			// TEST

  (*gauss).m= 4;					// gaussian=1
  // float r_ex= 3.5;					// [dB] Extinction ratio [minimum at datasheets]
  float r_ex= 99;					// [dB] Extinction ratio [10~15 Grosz priv. comm. 090210]
  // (*gauss).r_ex= 999.0;				// TEST 
  (*gauss).r_ex= pow(10.0, r_ex/ 10.0);			// [1] Extinction ratio
  // printf("val= %e\n",(*gauss).r_ex);			// TEST
 
  float P= 2.0;						// [dBm] Tx output power < PX2-1541SF > 	 
  (*gauss).P= pow(10.0, P/ 10.0);			// [mW] Tx output power 
  (*gauss).P_0= 2.0* P/ (1.0+ (*gauss).r_ex);		// [mW] '0' 
  (*gauss).A_0= sqrt( (*gauss).P_0 );				// [sqrt{mW}] '0' bit amplitude
  float P_1NRZ= (*gauss).P_0* (*gauss).r_ex;			// [mW] '1' bit power full duty cycle
  // (*gauss).P_peak= P_1NRZ/ (*Carrier).Duty_Cycle;	// [mW] duty cycle corrected '1' bit power (square pulse aprox)
  (*gauss).P_peak= P_1NRZ;				// [mW] '1' bit power (square pulse aprox)
  (*gauss).A_peak= sqrt( (*gauss).P_peak);		// [sqrt{mW}] duty cycle corrected '1' bit amplitude

  (*gauss).CentringFactor= 0.25;					// [1] 0.25 -> towards bit slot start edge, 0.5 centres pulse
  (*gauss).FWHM_pow= (*Carrier).Duty_Cycle* (*Carrier).Bit_Slot;	// [s] in power (|E|^2)
  (*gauss).T_0_amp= (*gauss).FWHM_pow/(2* sqrt(log(2) ) );		// [s]
  // (*gauss).T_0_amp=0.25*(*Carrier).Duty_Cycle*(*Carrier).Bit_Slot;	// as Duty_Cycle fraction 
} 


void GenRuidoRapido(float *Noise, float Sigma, unsigned long int Tmax) {
  void Gasdev(float *V2, float Sig, long *idum);
  unsigned long int i;
  static long int sem = -1;	//	Semilla para el generador de numeros random.
  if(sem==-1)	sem =-time(NULL);	
  for(i=0; i<Tmax; i+=2) Gasdev(&(Noise[i]), Sigma, &sem);
}


void Gasdev(float *V2, float Sigma, long *idum) {
  float fac,rsq,v1,v2;
  do {
    // generador stdlib.h [-1,1] 
    v1= 2.0* ((float) rand()/ (float) RAND_MAX )- 1.0;
    v2= 2.0* ((float) rand()/ (float) RAND_MAX )- 1.0;
    rsq=v1*v1+v2*v2;
  } while (rsq >= 1.0 || rsq == 0.0);
  fac=Sigma*sqrt(-2.0*log(rsq)/rsq);
  V2[0] = v2*fac;
  V2[1] = v1*fac;
}


void Write1Col(char FileName[], float Train[], unsigned int TrainSize) {
  unsigned long int i;
  FILE *File;
  File= fopen(FileName, "w");
  for(i=0; i<TrainSize; i++) fprintf(File,"%3e\n", Train[i]);
  fclose(File);
}


void Write2Col(char FileName[], float TrainTime[], float Train[], unsigned int TrainSize, int Erste, GENDRIVE *Carrier) {
  char FileStatus[2];
  FileStatus[0]='w';
  FileStatus[1]='\0';
  unsigned long int i;
  FILE *File;
  if (Erste !=0){
    FileStatus[0]= 'a';
    for (i= 0;i< TrainSize; i++) TrainTime[i]+= Erste* BUFFERMAX* (*Carrier).Bit_Slot; 
  }
  File= fopen(FileName, FileStatus);
  for(i=0; i<TrainSize; i++) fprintf(File,"%3e\t%3e\n", TrainTime[i], Train[i]);
  fclose(File);
}


void TimeVectorGenerator(GENDRIVE *Carrier, unsigned long int TrainLength, float AuxTemp[], float PulsesTrainTime[]) {
  unsigned long int i;
  unsigned int k;
  for(k= 0;k<TrainLength;k++) for(i= 0;i<(*Carrier).NPtos_Bit;i++) PulsesTrainTime[k*(*Carrier).NPtos_Bit+ i]= AuxTemp[i]+ (*Carrier).Bit_Slot* k; // copies time to following slots
  Write1Col("temp.dat",PulsesTrainTime,TrainLength* (*Carrier).NPtos_Bit);
}


void RZTrain(float RZPulsesTrain[], GENDRIVE *Carrier, SuperGaussian *gauss, float RZBit1Gauss[], unsigned char InBitString[], unsigned long int TrainLength, unsigned int transmitters) {

  // RZ PULSES TRAIN (Amplitude)
  unsigned long int i;
  unsigned int senders, k= 0;
  while(k<TrainLength) {
    senders= InBitString[k];
    for(i=0; i<(*Carrier).NPtos_Bit; i++) {
      // Optical intensities addition
      RZPulsesTrain[k*(*Carrier).NPtos_Bit+i]= RZBit1Gauss[i]+ 2* (*gauss).A_0;
      RZPulsesTrain[k*(*Carrier).NPtos_Bit+i]*= (float) senders* RZBit1Gauss[i];
      RZPulsesTrain[k*(*Carrier).NPtos_Bit+i]+= (float) transmitters * (*gauss).P_0;
      RZPulsesTrain[k*(*Carrier).NPtos_Bit+i]= sqrt( RZPulsesTrain[k*(*Carrier).NPtos_Bit+i]); 
    }
    k++;
  }
  // Write1Col("amplitude1.dat", RZBit1Gauss, (*Carrier).NPtos_Bit);	// TEST bitslot '1'
  // Write1Col("RZPulsesTrain.dat", RZPulsesTrain, (*Carrier).NPtos_Tot);	// TEST train to file
  // for(i=0; i<(*Carrier).NPtos_Tot; i++) printf("%.3e\t%.3e\n",RZPulsesTrainTime[i],RZPulsesTrain[i]);	// TEST
}


void OpticalFilter(float SignalVector[], unsigned long int SignalVectorSize) {
/* Digital filter designed by mkfilter/mkshape/gencode   A.J. Fisher
  Command line: /www/usr/fisher/helpers/mkfilter -Bu -Lp -o 2 -a 1.5625000000e-02 0.0000000000e+00 -l

  filtertype    =    Butterworth 
  passtype    =    Lowpass 
  order    =    6 
  samplerate    =    6.4E11 	= 2^6/ bit slot
  corner1    =    25E9
*/

  // FILTER ORDER 6
  #define oNZEROS 6
  #define oNPOLES 6
  // CutOff= 6GHz order 6
    // #define oGAIN   8.599958842e+07
  // CutOff= 10GHz order 6
    // #define oGAIN   8.599958842e+07
  // CutOff= 12GHz order 6
    // #define oGAIN   2.985320090e+07
  // CutOff= 17.5GHz order 6
    #define oGAIN   3.419013222e+06 
  // CutOff= 25GHz order 6
    // #define oGAIN   4.570794845e+05 

/*
  // FILTER ORDER 2
  #define oNZEROS 2
  #define oNPOLES 2
  #define oGAIN   7.820233128e+01
*/

  static float xvo[oNZEROS+1], yvo[oNPOLES+1];
  unsigned long int i;

  // printf("xv(0)= %3e\txv(1)= %3e\t\n",xv[0], xv[1]);	// TEST
  for (i=0; i<SignalVectorSize; i++) {

    // 6 poles
    xvo[0] = xvo[1]; xvo[1] = xvo[2]; xvo[2] = xvo[3]; xvo[3] = xvo[4]; xvo[4] = xvo[5]; xvo[5] = xvo[6];
    xvo[6] = SignalVector[i]/ oGAIN;
    yvo[0] = yvo[1]; yvo[1] = yvo[2]; yvo[2] = yvo[3]; yvo[3] = yvo[4]; yvo[4] = yvo[5]; yvo[5] = yvo[6];
    yvo[6] =   (xvo[0] + xvo[6]) + 6 * (xvo[1] + xvo[5]) + 15 * (xvo[2] + xvo[4]) + 20 * xvo[3]

    // CutOff= 6GHz
    //  + ( -0.6841955724 * yvo[0]) + (  4.3646808658 * yvo[1]) + (-11.6101420990 * yvo[2]) + ( 16.4837640270 * yvo[3]) + (-13.1748195840 * yvo[4]) + (  5.6207116179 * yvo[5]);

    // CutOff= 10Ghz
    //  + ( -0.6841955724 * yvo[0]) + (  4.3646808658 * yvo[1])+ (-11.6101420990 * yvo[2]) + ( 16.4837640270 * yvo[3])+ (-13.1748195840 * yvo[4]) + (  5.6207116179 * yvo[5]);

    // CutOff= 12GHz
    //  + ( -0.6341207148 * yvo[0]) + (  4.0933318918 * yvo[1])+ (-11.0212577670 * yvo[2]) + ( 15.8439404200 * yvo[3])+ (-12.8267655930 * yvo[4]) + (  5.5448696188 * yvo[5]);

    // CutOff= 17.5GHz
      + ( -0.5143535878 * yvo[0]) + (  3.4274712300 * yvo[1]) + ( -9.5371585039 * yvo[2]) + ( 14.1860709290 * yvo[3]) + (-11.8984008380 * yvo[4]) + (  5.3363520514 * yvo[5]);

   // CutOff= 25GHz
   //  + ( -0.3862279890 * yvo[0]) + (  2.6834487459 * yvo[1])+ ( -7.8013262392 * yvo[2]) + ( 12.1514352550 * yvo[3])+ (-10.6996337410 * yvo[4]) + (  5.0521639483 * yvo[5]);


    SignalVector[i]= yvo[6];
/*
    // 2 poles
    xvo[0] = xvo[1]; xvo[1] = xvo[2]; 
    xvo[2] = SignalVector[i] / oGAIN;
    yvo[0] = yvo[1]; yvo[1] = yvo[2]; 
    yvo[2] =   (xvo[0] + xvo[2]) + 2 * xvo[1]
          + ( -0.7067570632 * yvo[0]) + (  1.6556076929 * yvo[1]);
    SignalVector[i] = yvo[2];
*/
  }
}


void ElectricalFilter(float SignalVector[], unsigned long int SignalVectorSize, int *SamplingPoint, DeviceSTR *Network) {
  /* Digital filter designed by mkfilter/mkshape/gencode   A.J. Fisher
   Command line: /www/usr/fisher/helpers/mkfilter -Bu -Lp -o 2 -a 1.5625000000e-02 0.0000000000e+00 -l

   filtertype    =    Butterworth 
   passtype    =    Lowpass 
   order    =    2 
   samplerate    =    6.4E11 	= 2^6/ bit slot
 */

/*
// FILTER ORDER 6
// #define NZEROSe 6
// #define NPOLESe 6
// CutOff= 6GHz order 6
  // #define eGAIN   8.599958842e+07
  // *SamplingPoint= 3;		// 6GHz ord 6 filtro electrico
// CutOff= 10GHz order 6
  // #define eGAIN   8.599958842e+07
  // *SamplingPoint= 4;		// 10GHz ord 6 filtro electrico
// CutOff= 12GHz order 6
  // #define eGAIN   2.985320090e+07
  // *SamplingPoint= 4;		// 12GHz ord 6 filtro electrico
// CutOff= 25GHz order 6
  //  #define eGAIN   4.570794845e+05 
  // *SamplingPoint= 4;		// 25GHz ord 6 filtro electrico
*/

  // FILTER ORDER 2
  #define NZEROSe 2
  #define NPOLESe 2
  float eGAIN;
  switch ((*Network).Efilt_CutOff ) { 
    case 7:
      eGAIN= 8.884369142e+02; 	// CutOff= 7 GHz
      *SamplingPoint= 43;	// opt: 17.5 ord 6 / elec: 7 ord 2
      break;

    // Con filtro optico 25 GHz corner, orden 6
    case 25:
      eGAIN= 7.820233128e+01;	// CutOff= 25GHz
      *SamplingPoint= 27;	// 25GHz ord 2 filtro electrico / optico
      break;
/*
    case 12:
      eGAIN= 3.125167034e+02;	// CutOff= 12GHz
      *SamplingPoint= 35;	// 12GHz ord 2 filtro electrico / optico
      break;
    case 10:
      eGAIN= 4.441320406e+02;	// CutOff= 10GHz
      *SamplingPoint= 37;	// 10GHz ord 2 filtro electrico / optico
      break;
    case 6:
      eGAIN= 4.441320406e+02;	// CutOff= 6GHz
      *SamplingPoint= 45;	// 6GHz ord 2 filtro electrico / optico
      break;
    //case 5:  eGAIN= 4.441320406e+02;	// CutOff= 5GHz
    //  break;
*/
  }
  // printf("eGAIN= %e\n",eGAIN);		// TEST


  static float xve[NZEROSe+1], yve[NPOLESe+1];
  unsigned long int i;
  for (i=0; i<SignalVectorSize; i++) {
    // 2 poles
    xve[0] = xve[1]; xve[1] = xve[2]; 
    xve[2] = SignalVector[i] / eGAIN;
    yve[0] = yve[1]; yve[1] = yve[2];
    yve[2] =   (xve[0] + xve[2]) + 2 * xve[1]+
    //  + ( -0.9073853925 * yve[0]) + (  1.9028831033 * yve[1]);  // opt: 17.5 ord 6 / elec: 7 ord 2

    // Con filtro optico 25 GHz corner, orden 6 
    //	( -0.8703674775 * yve[0]) + (  1.8613611468 * yve[1]);	// CutOff= 10GHz
    //  ( -0.9329347318 * yve[0]) + (  1.9306064272 * yve[1]);	// CutOff= 5GHz
    //  ( -0.9200713754 * yve[0]) + (  1.9167412232 * yve[1]);	// CutOff= 6GHz
    //  ( -0.8465319748 * yve[0]) + (  1.8337326589 * yve[1]);  // CutOff= 12GHz
      ( -0.7067570632 * yve[0]) + (  1.6556076929 * yve[1]);	// CutOff= 25GHz
    SignalVector[i] = yve[2];

    /*
    // 6 poles
    xve[0] = xve[1]; xve[1] = xve[2]; xve[2] = xve[3]; xve[3] = xve[4]; xve[4] = xve[5]; xve[5] = xve[6]; 
    xve[6] = SignalVector[i]/ eGAIN;
    yve[0] = yve[1]; yve[1] = yve[2]; yve[2] = yve[3]; yve[3] = yve[4]; yve[4] = yve[5]; yve[5] = yve[6]; 
    yve[6] =   (xve[0] + xve[6]) + 6 * (xve[1] + xve[5]) + 15 * (xve[2] + xve[4]) + 20 * xve[3]
    //    + ( -0.6841955724 * yve[0]) + (  4.3646808658 * yve[1]) + (-11.6101420990 * yve[2]) + ( 16.4837640270 * yve[3]) + (-13.1748195840 * yve[4]) + (  5.6207116179 * yve[5]);    // CutOff= 6GHz
    //      + ( -0.6841955724 * yve[0]) + (  4.3646808658 * yve[1])+ (-11.6101420990 * yve[2]) + ( 16.4837640270 * yve[3])+ (-13.1748195840 * yve[4]) + (  5.6207116179 * yve[5]);  // CutOff= 10GHz
    //      + ( -0.6341207148 * yve[0]) + (  4.0933318918 * yve[1])+ (-11.0212577670 * yve[2]) + ( 15.8439404200 * yve[3])+ (-12.8267655930 * yve[4]) + (  5.5448696188 * yve[5]);  // CutOff= 12GHz 
+ ( -0.3862279890 * yve[0]) + (  2.6834487459 * yve[1])+ ( -7.8013262392 * yve[2]) + ( 12.1514352550 * yve[3])+ (-10.6996337410 * yve[4]) + (  5.0521639483 * yve[5]);  // CutOff= 25GHz
    SignalVector[i]= yve[6];
   */

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


float Threshold1(GENDRIVE *Carrier, float RZBit1Gauss[], float *OSigmaNoise, float *DSigmaNoise, DeviceSTR *Network, unsigned int transmitters, SuperGaussian *gauss) { 

  // DISCRIMINATION BY DISPERSION THRESHOLD [Agrawal 4.5]
  unsigned long int i;
  unsigned short int Pul, NPulThr=100;			// averaged bits to obtain threshold

  float One[(*Carrier).NPtos_Bit], Zero[(*Carrier).NPtos_Bit];
  float sum0, sum_sqr0, sum1, sum_sqr1;
  int SP;
  float Id=0, I0, I1, sigma0, sigma1;
  float Idp;
  unsigned short int reject=0;
  float Qd=0, Qdp;    // TEST BER via Q

  // OPTICAL NOISE VECTOR FOR THRESHOLD DETERMINATION BITS
  float ONoiseVector[2*(*Carrier).NPtos_Bit* NPulThr];
  GenRuidoRapido(ONoiseVector, *OSigmaNoise, 2*(*Carrier).NPtos_Bit* NPulThr);		// NoiseVector amplitude
  // Write1Col("ONoiseVector.dat", ONoiseVector, 2*(*Carrier).NPtos_Bit* NPulThr);	// TEST train to file

  // DETECTOR (THERMAL) NOISE VECTOR FOR THRESHOLD DETERMINATION BITS
  float DNoiseVector2[2*(*Carrier).NPtos_Bit* NPulThr];
  GenRuidoRapido(DNoiseVector2, *DSigmaNoise, 2*(*Carrier).NPtos_Bit* NPulThr);		// NoiseVector amplitude
  // Write1Col("DNoiseVector.dat", DNoiseVector2, 2*(*Carrier).NPtos_Bit* NPulThr);	// TEST train to file

  // Optical & Electric IRR filters dummy start
  float DummyAux= sqrt( (float) transmitters )* (*gauss).A_0* (*Network).FAtt1* (*Network).EDFAgain* (*Network).FAtt2;
  for(i=0; i<(*Carrier).NPtos_Bit; i++) Zero[i]= DummyAux;
  OpticalFilter(Zero, (*Carrier).NPtos_Bit);
  DummyAux*= DummyAux;		// Optical: Amplitude -> Power
  DummyAux*= (*Network).R;	// Optical Power -> Electrical Current 
  for(i=0; i<(*Carrier).NPtos_Bit; i++) Zero[i]= DummyAux;
  ElectricalFilter(Zero, (*Carrier).NPtos_Bit, &SP, Network);
  // Min=40; Max=50; // EYE DIAGRAM REQUIRED TO SELECT MIN & MAX
  unsigned int Min= SP- 5,Max= SP+ 5;
  float n= Max- Min+1;
  // printf("SamplingPoint=\t%i\n", SP);  // TEST
  
  // main cycle
  for(Pul=0; Pul<NPulThr; Pul++) {
  //  Pul=0;		// TEST one cycle

    // DETERMINATION BITS: CREATION, ATTENUATION & OPTICAL NOISE
    for(i=0; i<(*Carrier).NPtos_Bit; i++) {
      Zero[i]= sqrt( (float) transmitters )* (*gauss).A_0;	// zero level for present transmitters
      One[i]= Zero[i]+ RZBit1Gauss[i];				// one overpulse addition
      // printf("One[%li]= %e\n",i,One[i]); // TEST
      One[i]*= (*Network).FAtt1, Zero[i]*= (*Network).FAtt1;		// ATTENUATION STAGE Tx -> EDFA out
      One[i]*= (*Network).EDFAgain, Zero[i]*= (*Network).EDFAgain;	// EDFA STAGE EDFAin -> EDFAout
      One[i]+= ONoiseVector[i+ (*Carrier).NPtos_Bit* Pul], Zero[i]+= ONoiseVector[i+ (*Carrier).NPtos_Bit* (Pul+ NPulThr)];	// optical noise addition
      // printf("One[%li]= %e\n",i,One[i]); // TEST
      One[i]*= (*Network).FAtt2, Zero[i]*= (*Network).FAtt2;		// ATTENUATION STAGE EDFA -> Rx
    }
    // printf("Zero(64)= %3e\tOne(64)= %3e\n",Zero[(*Carrier).NPtos_Bit- 1],One[(*Carrier).NPtos_Bit- 1]);		// TEST
    // printf("FAtt1= %3e\tFAtt2= %3e\n",(*Network).FAtt1, (*Network).FAtt2);		// TEST
    // Write2Col("Zero1.dat", Zero, One, (*Carrier).NPtos_Bit,0, Carrier);	// TEST one & zero to file

    // Optical filtering
    OpticalFilter(Zero, (*Carrier).NPtos_Bit);
    OpticalFilter(One, (*Carrier).NPtos_Bit);
    // Write2Col("Zero2.dat", Zero, One, (*Carrier).NPtos_Bit,0, Carrier);	// TEST one & zero to file

    // -> CURRENT + ELECTRICAL NOISE
    for(i=0; i<(*Carrier).NPtos_Bit; i++) {
      One[i]*= One[i];							// optical amplitude -> optical power
      One[i]*= (*Network).R;						// -> electrical current
      One[i]+= DNoiseVector2[i+(*Carrier).NPtos_Bit*Pul]; 		// electrical noise addition

      Zero[i]*= Zero[i];				  		// optical amplitude -> optical power
      Zero[i]*= (*Network).R;						// -> electrical current
      Zero[i]+= DNoiseVector2[i+(*Carrier).NPtos_Bit*(Pul+NPulThr)]; 	// electrical noise addition
    }
    // Write2Col("Zero3.dat", Zero, One, (*Carrier).NPtos_Bit,0, Carrier);	// TEST one & zero to file

    // Electrical filtering
    ElectricalFilter(One, (*Carrier).NPtos_Bit, &SP, Network);
    ElectricalFilter(Zero, (*Carrier).NPtos_Bit, &SP, Network);
    // Write2Col("Zero4.dat", Zero, One, (*Carrier).NPtos_Bit,0, Carrier);	// TEST one & zero to file

    // Contraptions
    sum0= 0, sum_sqr0= 0, sum1= 0, sum_sqr1= 0;
    // printf("n= %f\tmin= %i\tmax= %i\n",n,Min,Max); // TEST
    for(i=Min; i<Max+1; i++) {
      // printf("One[%li]= %e\n",i,One[i]); // TEST
      sum1+= One[i];
      One[i]*= One[i];
      sum_sqr1+= One[i];

      sum0+= Zero[i];
      Zero[i]*= Zero[i];
      sum_sqr0+= Zero[i];
    }			// TEST one cycle
    // printf("sum0= %e\tsum1= %e\n",sum0, sum1); // TEST
    // printf("sum_sqr0= %e\tsum_sqr1= %e\n",sum_sqr0, sum_sqr1); // TEST

    sigma1= sqrt(( 1/ (n- 1 ) )* (sum_sqr1- ((sum1* sum1 )/ n ) ) );
    sigma0= sqrt(( 1/ (n- 1 ) )* (sum_sqr0- ((sum0* sum0 )/ n ) ) );
    // printf("Sigma0p= %e\tSigma1p= %e\n",sigma0,sigma1); // TEST
/*
    if (sigma0 != sigma0 && sigma1 != sigma1) reject++;
    else {
      if (sigma0 != sigma0) sigma0= 0;	// if sigma0 too small (NaN), sigma0 -> 0
      if (sigma1 != sigma1) sigma1= 0;	// if sigma1 too small (NaN), sigma1 -> 0
    }
*/
    I1= sum1/n;		// moyenne1
    I0= sum0/n;		// moyenne0
    // printf("I0p= %e\tI1p= %e\n",I0,I1); // TEST

    Idp= (sigma0* I1+ sigma1* I0)/ (sigma0+ sigma1);
    Qdp= (I1- I0)/ (sigma0+ sigma1);
    // printf("Idp= %e\n",Idp); // TEST
    if (Idp != Idp) reject++;	// NaN rejection
    else{
      Id+= Idp;
      // Id+= (sigma0* I1+ sigma1* I0)/ (sigma0+ sigma1);
      // printf("Id[%i]= %e\n",Pul,Id); // TEST
      Qd+= Qdp;
    }
  }  // TEST one cycle
  Id*= (1/ (float) (NPulThr- reject) );
  // Id*= 1/ (float) NPulThr;
  // printf("Id= %e\n",Id); // TEST
  Qd*= (1/ (float) (NPulThr- reject) );
  printf("%e\n",Qd); // TEST
  return Id;
}


float BitSlots(GENDRIVE *Carrier, SuperGaussian *gauss, DeviceSTR *Network, float OSNR, float *OSigmaNoise, float DSNR, float *DSigmaNoise, unsigned int transmitters, float RZBit1Gauss[], float AuxTemp[]) {	
  unsigned long int i;

  // Petit AuxTemp
  // float aux=( (1.5- (float)(*Carrier).NPtos_Bit)/ (1- (float)(*Carrier).NPtos_Bit) );	// OPTIMISED SAMPLING
  // for(i=0; i<(*Carrier).NPtos_Bit; i++) AuxTemp[i]= (*Carrier).dt* ( aux* i+ 0.5) ;	// OPTIMISED SAMPLING time for one slot
  for(i=0; i<(*Carrier).NPtos_Bit; i++) AuxTemp[i]= (*Carrier).dt* ( i+ 0.5) ;	// NON OPTIMISED SAMPLING time for one slot

  // Pulse amplitude addition over base [RZ]
  float AuxAmp= (*gauss).A_peak- (*gauss).A_0;
  for(i=0; i<(*Carrier).NPtos_Bit; i++) {
    RZBit1Gauss[i]= AuxAmp;
    RZBit1Gauss[i]*= exp(-0.5* pow( ( ( AuxTemp[i]- ( (*gauss).CentringFactor* (*Carrier).Bit_Slot))/ (*gauss).T_0_amp), 2* (*gauss).m));	// SuperGaussian overpulse
    // printf("RZ1[%li]= %e\n",i,RZBit1Gauss[i]);	// TEST
  }

/*
  // Pulse amplitude addition over base [NRZ]
  for(i=0; i<(*Carrier).NPtos_Bit; i++) RZBit1Gauss[i]= ( (*gauss).A_peak- (*gauss).A_0);	// Square [NRZ]
  // for(i=0; i<(*Carrier).NPtos_Bit; i++) printf("RZ1[%li]= %e\n",i,RZBit1Gauss[i]); // TEST
 */

/*
  // Bit slots test
  float RZBit0[(*Carrier).NPtos_Bit];					// '0' SLOT AMPLITUDE
  float RZBit1[(*Carrier).NPtos_Bit];					// '1' SLOT AMPLITUDE [NRZ]
  float A_0aux= (*gauss).A_0;
  A_0aux*= A_0aux;
  for(i=0; i<(*Carrier).NPtos_Bit; i++) {
    RZBit0[i]= A_0aux;	
    RZBit1[i]= (*gauss).A_0+ RZBit1Gauss[i];
    RZBit1[i]*= RZBit1[i];
  }
  Write2Col("Bit01.dat", RZBit0, RZBit1, (*Carrier).NPtos_Bit,0, Carrier);	// TEST one & zero to file
  printf("dt= %3e\n",(*Carrier).dt);
*/

  // Mean 1 Tx power
  float aux, Pmean;
  for(i=0, Pmean=0; i<(*Carrier).NPtos_Bit; i++) {
    aux= RZBit1Gauss[i]+ 2* (*gauss).A_0;
    aux*= RZBit1Gauss[i];
    Pmean+= aux;
  }
  Pmean*= 1.0/ (2.0* (*Carrier).NPtos_Bit );
  Pmean+= transmitters* (*gauss).P_0; 
  // printf("Pmean= %3e\n",Pmean);		// TEST

  // ATTENUATION STAGE Tx -> EDFA output
  float Pmean_EDFAout;
  Pmean_EDFAout= Pmean;
  Pmean_EDFAout*= (*Network).FAtt1;
  Pmean_EDFAout*= (*Network).FAtt1;
  Pmean_EDFAout*= (*Network).EDFAgain;
  Pmean_EDFAout*= (*Network).EDFAgain;
  // printf("Pmean_EDFAout= %3e\n",Pmean_EDFAout);		// TEST

  // OSigmaNoise
  // *OSigmaNoise= sqrt( (Pmean_EDFAout/ OSNR)*( 2* (*Network).Ofilt_CutOff/ (*Network).r_BW )  );	// optical noise amplitude [ TIA/EIA-526-19 ]
  *OSigmaNoise= sqrt( (Pmean_EDFAout/ OSNR)*( 1/((*Network).r_BW* (*Carrier).dt ) )  );	// optical noise amplitude [ TIA/EIA-526-19 ]
  // *OSigmaNoise= sqrt( Pmean_ASEin/ OSNR);	// optical noise amplitude (Agrawal 4.4.11) [Santiago I.]
  // printf("OSigmaNoise= %3e\n", *OSigmaNoise);		// TEST

  // ATTENUATION STAGE EDFA output -> Rx
  float Pmean_Rxin;
  Pmean_Rxin= Pmean_EDFAout;
  Pmean_Rxin*= (*Network).FAtt2;
  Pmean_Rxin*= (*Network).FAtt2;
  // printf("Pmean_Rxin= %3e\n",Pmean_Rxin);		// TEST
  
  // DSigmaNoise
  *DSigmaNoise= Pmean_Rxin* (*Network).R* sqrt(1/ DSNR);	// amplitude of noise (Agrawal 4.4.11) [Santiago I.]
  // printf("DSigmaNoise= %3e\n", *DSigmaNoise);		// TEST

  // DETECTOR THRESHOLD
  float Id= Threshold1(Carrier, RZBit1Gauss, OSigmaNoise, DSigmaNoise, Network, transmitters, gauss);
  return Id;
}


void Link(GENDRIVE *Carrier, float RZPulsesTrain[], float *OSigmaNoise, float *DSigmaNoise, DeviceSTR *Network, int *SamplingPoint) {
  unsigned long int i;

  for(i=0; i<(*Carrier).NPtos_Tot; i++) RZPulsesTrain[i]*= (*Network).FAtt1; 	// ATTENUATION STAGE Tx -> EDFAin
  // Write1Col("AtEDFAin.dat", RZPulsesTrain, (*Carrier).NPtos_Tot);	// TEST train to file

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
SNR= Power_signal/Power_noise  [http://en.wikipedia.org/wiki/Signal-to-noise_ratio]
Power_noise= SigmaNoise^2
Power_signal= 0.5*(Power_'0bit' + Power_'1bit')  assuming equal number of '1' and '0' bits  [Andres, Nacho]
Power_'0bit'= LOff^2, amplitude for bit '0' due to system extinction ratio
Power_'1bit'= P1Mean= \frac{1}{(*Carrier).Bit_Slot} \int_0^{(*Carrier).Bit_Slot} dt Pulse1(t)^2  integrated in bit slot
OR? Power_'1bit'= \frac{1}{LimSup-LimInf} \int_LimInf^{LimSup} dt Pulse1(t)^2  integrated in detection window
-> SigmaNoise=\sqrt(\frac{LOff^2 + P1Mean}{2 SNR})
*/

  for(i=0; i<(*Carrier).NPtos_Tot; i++) RZPulsesTrain[i]*= (*Network).EDFAgain; 		// EDFA STAGE EDFAin -> EDFAout
  
  // OPTICAL NOISE (AWGN)
  float ONoiseVector[(*Carrier).NPtos_Tot];
  GenRuidoRapido(ONoiseVector, *OSigmaNoise, (*Carrier).NPtos_Tot);		// NoiseVector amplitude
  // Write1Col("ONoiseVector.dat", ONoiseVector, (*Carrier).NPtos_Tot);		// TEST train to file
  for(i=0; i<(*Carrier).NPtos_Tot; i++){
    RZPulsesTrain[i]+= ONoiseVector[i];	// Noise addition
    RZPulsesTrain[i]*= (*Network).FAtt2; 		// ATTENUATION STAGE EDFAout -> Rx
  }
  // Write1Col("AtEDFAout.dat", RZPulsesTrain, (*Carrier).NPtos_Tot);		// TEST train to file

  // Optical filtering
  OpticalFilter(RZPulsesTrain, (*Carrier).NPtos_Tot);
  // Write1Col("OptFilt.dat", RZPulsesTrain, (*Carrier).NPtos_Tot);		// TEST train to file

  // Optical -> Electrical signal (current)
  for(i=0; i<(*Carrier).NPtos_Tot; i++) {
    RZPulsesTrain[i]*= RZPulsesTrain[i];	// optical amplitude-> optical power
    RZPulsesTrain[i]*= (*Network).R;		// -> electrical current (detector responsivity R= 1)
  }

  // 'THERMAL' NOISE AT DETECTOR
  float DNoiseVector[(*Carrier).NPtos_Tot];					// Thermal noise vector
  GenRuidoRapido(DNoiseVector, *DSigmaNoise, (*Carrier).NPtos_Tot);	 	// NoiseVector amplitude
  for(i=0; i<(*Carrier).NPtos_Tot; i++)	RZPulsesTrain[i]+= DNoiseVector[i];	// Noise addition to electrical current signal
  // Write1Col("DNoiseVector.dat", DNoiseVector, (*Carrier).NPtos_Tot);		// TEST train to file
  // printf("DSNR=\t%e\tDSigmaNoise=\t%e\n",DSNR, DSigmaNoise);			// TEST

  ElectricalFilter(RZPulsesTrain, (*Carrier).NPtos_Tot, SamplingPoint, Network);
  // Write1Col("ElecFilt.dat", RZPulsesTrain, (*Carrier).NPtos_Tot);	// TEST train to file

}


void EyeDiagram(float PulsesTrain[], float PulsesTrainTime[], GENDRIVE *Carrier, float Id) {
  unsigned long int i, iMidi= 0.5*(*Carrier).NPtos_Bit;
  FILE *EYE;
  EYE= fopen("EyeDiagram.dat", "w");
  fprintf(EYE,"# Id= %3e\n",Id);
  for(i=0; i<(*Carrier).NPtos_Tot; i++) {
    ldiv_t thing= ldiv(i,(*Carrier).NPtos_Bit);
    if(thing.rem< iMidi) {
      fprintf(EYE,"%3e\t%3e\n",PulsesTrainTime[thing.rem], PulsesTrain[i]);
      fprintf(EYE,"%3e\t%3e\n",PulsesTrainTime[thing.rem]+ (*Carrier).Bit_Slot, PulsesTrain[i]);
    }
    else {
      fprintf(EYE,"%3e\t%3e\n",PulsesTrainTime[thing.rem], PulsesTrain[i]);
      fprintf(EYE,"%3e\t%3e\n",PulsesTrainTime[thing.rem]- (*Carrier).Bit_Slot, PulsesTrain[i]);
    }
  }
  fclose(EYE);
}


int main(int argc, char **argv) {
  if (argc<4){
    printf("Usage:noisesim #Tx DSNR[dB] OSNR[dB] <InputBitStream >OutputBitStream\n");
    exit(0);
  }
  unsigned long int i;
  int Erste= 0;			// write file auxiliary
  unsigned char buf[BUFFERMAX];
  int maxdata=BUFFERMAX;
  GENDRIVE Carrier;
  DeviceSTR Network;
  SuperGaussian gauss;
  unsigned int transmitters= atoi(argv[1]);					// Argument: number of Tx
  float DSNR= pow(10,atof(argv[2])/10), OSNR= pow(10,atof(argv[3])/10);		// Argument: SNR [dB] -> SNR [1]
  float OSigmaNoise, DSigmaNoise;
  int SamplingPoint;								// Decision, varies by filter

/*
  // Attenuation
  float FAtt= 1.0;
  float FAtt= exp(-0.5* pow(10, Network.alpha/ 10)* Network.length );
  printf("FAtt=\t%e\n",FAtt); 		// TEST
*/
  InputParameters(&Carrier, &gauss, &Network);		// SYSTEM PARAMETERS
  float RZBit1Gauss[Carrier.NPtos_Bit];
  float AuxTemp[Carrier.NPtos_Bit];
  float Id= BitSlots(&Carrier, &gauss, &Network, OSNR, &OSigmaNoise, DSNR, &DSigmaNoise, transmitters, RZBit1Gauss, AuxTemp);

  while( ( Carrier.N_Bits= fread(buf,1,maxdata,stdin) )> 0) {
  // INPUTS READ
/*
    unsigned char InBitString[Carrier.N_Bits+50];
    for(i=0; i<50; i++) InBitString[i]='0';
    for(i=0; i<Carrier.N_Bits; i++) InBitString[i+49]=buf[i];
*/
    unsigned char InBitString[Carrier.N_Bits];
    for(i=0; i<Carrier.N_Bits; i++) InBitString[i]=buf[i];
    Carrier.NPtos_Tot= Carrier.NPtos_Bit* Carrier.N_Bits;	// Total # points
    // printf("#bits: %i\tdt: %3e\n",Carrier.N_Bits,Carrier.dt);	// TEST

    // Tx 
    float RZPulsesTrain[Carrier.NPtos_Tot];
    RZTrain(RZPulsesTrain, &Carrier, &gauss, RZBit1Gauss, InBitString, Carrier.N_Bits, transmitters);

    // Network elements 
    Link(&Carrier, RZPulsesTrain, &OSigmaNoise, &DSigmaNoise, &Network, &SamplingPoint);

/*
    // Output check
    float RZPulsesTrainTime[Carrier.NPtos_Tot];			// Time vector  
    TimeVectorGenerator(&Carrier, Carrier.N_Bits, AuxTemp, RZPulsesTrainTime);
    EyeDiagram(RZPulsesTrain, RZPulsesTrainTime, &Carrier, Id);	// TEST Eye diagram
//    Write2Col("train.dat", RZPulsesTrainTime, RZPulsesTrain, Carrier.NPtos_Tot, Erste, &Carrier);	// TEST train to file
*/

    // Rx: BIT DETERMINATION
    char BitsDisc[Carrier.N_Bits];
    for(i= 0; i< Carrier.N_Bits; i++) {
      if(RZPulsesTrain[i* Carrier.NPtos_Bit+ SamplingPoint]> Id) BitsDisc[i]=1;
      else BitsDisc[i]=0;
    }
    // printf("Id= %3e\n",Id);

    // OUTPUT AS CHAR STRING
    for(i=0; i<Carrier.N_Bits; i++) buf[i]=BitsDisc[i];
    // write(1,buf,Carrier.N_Bits);
  
    // Write file auxiliary
    Erste++;
    }
  return 0;
}

