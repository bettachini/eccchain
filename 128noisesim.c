#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <math.h>
#include <time.h>

#define BUFFERMAX 4096

typedef struct {	// TRANSMISSION SYSTEM DEFINITION
  // PHYSICAL PARAMETERS
  float Bit_Slot;	// bit slot period
  float Duty_Cycle;	
  float B;		// bit rate [bits/s]
  float dt;		// time resolution

  // NUMERICAL PARAMETERS
  unsigned int N_Bits;		// # bits
  unsigned int NPtos_Bit;	// # samples per bit slot
  unsigned long int NPtos_Tot;	// total # samples
} GENDRIVE;


typedef struct {	// DEVICES DESCRIPTION 
  float alpha;		// [dB/km] fibre attenuation coefficient
  float length;		// [km] total fibre path length
  float FAtt1;		// [dB] Attenuation Tx -> EDFAin
  float EDFAgain;	// [dB] Gain at EDFA <fixed approx>
  float FAtt2;		// [dB] Attenuation EDFAout -> Rx
  float M;		// photodetector (APD) amplification
  float R;		// photodetector (PD) responsivity
  int Efilt_CutOff;	// [GHz] PD electrical filter bandwidth
  int Ofilt_CutOff;	// [GHz] PD optical input filter bandwidth
  float CentralNu;	// [Hz] carrier central frequency
  float sim_BW;		// [Hz] simulation bandwidth
  float r_BW;		// [Hz] reference bandwidth
  unsigned int Range; 	// Sampling range: even numbers -> effective range SP +- SP/2
  unsigned int Min; 	// 
} DeviceSTR;


typedef struct {	// GAUSSIAN PULSE DEFINITION
  float P;		// [W] Tx output power
  float P_peak;		// [W] peak power 	 
  float A_peak;		// [sqrt{W}] peak amplitude  |A_peak|^2= P_peak
  float P_0;		// [W] bit 0 power 	 
  float A_0;		// [sqrt{W}] bit 0 amplitude
  float C;		// [1] chirp parameter
  float CentringFactor;	// [1] pulse position factor, as bit slot proportion (<1)
  float FWHM_pow;	// [s] FWHM signal in power
  float T_0_amp;	// [s] T_0=T_{fwhmPOW}/(2* sqrt(ln(2) ) ) half of width at 1/e intensity point = Gaussian sigma
  unsigned int m;	// Super Gaussian pulse shape factor (Gaussian=1)
  float r_ex;		// [1] Extinction ratio (P_1/ P_0) [Agrawal (4.6.1)]
} SuperGaussian;


typedef struct {
  short Eye;
  short LinkVectors;
  short Pmeans;
} FLAGS;

void InputParameters(GENDRIVE *Carrier, SuperGaussian *Gauss, DeviceSTR *Network) {

  // GENDRVIE Carrier
  (*Carrier).NPtos_Bit= pow(2,6);				 		// # points for each bit slot
  (*Carrier).Duty_Cycle= 1.0/3.0;
  (*Carrier).B= 10E9;								// bit rate [bits/s] (10 Gbits/s)
  (*Carrier).Bit_Slot= 1/ (*Carrier).B;						// 1/B [s]
  (*Carrier).dt= (*Carrier).Bit_Slot/ (*Carrier).NPtos_Bit;			// time step [s]
  //  printf("Bit_Slot= %.3f ps\n",(*Carrier).Bit_Slot*1E12);	
  //  printf("Resolucion temporal= %.3f ps\n",(*Carrier).dt*1E12);

  // General
  float c= 299792458;  					// [m/s] light speed
  float Lambda_0= 1.55E-6;				// [m] carrier wavelength
  (*Network).CentralNu= c/ Lambda_0;			// [Hz] carrier frequency
  float Br= 1E-10;					// [m] 0.1 nm reference bandwidth
  (*Network).sim_BW= 1/ (2* (*Carrier).dt);		// [Hz] simulation bandwidth
  (*Network).r_BW= c* Br/ (Lambda_0* Lambda_0);		// [Hz] 0.1 nm reference -> frequency range
  // printf("BW= %e\n",(*Network).r_BW);	  	// TEST
 
  // DeviceSTR Network
  // Attenuations and amplification
  float FAtt1= -2.0 -1.0 -25.0;				// [dB] Tx -> EDFAin 2dB fibre, 1.0dB insertion, 25 dB 128x1 splitter
  (*Network).FAtt1= pow(10, FAtt1/ 20);			// [1] Tx -> EDFAin for amplitude
  float FAtt2= -2.0 -1.0 -25.0;				// [dB] EDFAout -> Rx 2dB fibre, 1.0dB insertion, 25 dB 1x128 splitter
  (*Network).FAtt2= pow(10.0, FAtt2/ 20.0);		// [1] EDFAout -> Rx amplitude
  float EDFAgain= 27.0;					// [dB] Gain at EDFA <fixed approx>
  (*Network).EDFAgain= pow(10.0, EDFAgain/ 20.0);	// [1] Gain at EDFA amplitude
  // printf("FAtt2= %3e\n",(*Network).FAtt2);		// TEST

  // Photodetector
  (*Network).M= 10;					// [1] APD detector amplifier gain
  (*Network).R= 1.2;					// [A/W] detector responsivity for 1550 nm

  // Filters
  (*Network).Efilt_CutOff= 14;				// [GHz] cut-off freq electrical filter
  (*Network).Ofilt_CutOff= 12.5;			// [GHz] cut-off freq optical filter
 
  // Sampling range 
  (*Network).Range= 4;					// even numbers -> effective range SP +- SP/2


  // SuperGaussian Gauss
  // Tx power
  float P= 2.0;						// [dBm] Tx output power < PX2-1541SF > 	 
  (*Gauss).P= pow(10.0, P/ 10.0)/ 1000;			// [W] Tx output power 
  // printf("P[W]= %e\n",(*Gauss).P);	                // TEST

  // Pulse shape
  (*Gauss).m= 4;							// Gaussian=1
  (*Gauss).CentringFactor= 0.25;					// [1] 0.25 -> towards bit slot start edge, 0.5 centres pulse
  (*Gauss).FWHM_pow= (*Carrier).Duty_Cycle* (*Carrier).Bit_Slot;	// [s] in power (|E|^2)
  (*Gauss).T_0_amp= (*Gauss).FWHM_pow/(2* sqrt(log(2) ) );		// [s]
  // (*Gauss).T_0_amp=0.25*(*Carrier).Duty_Cycle*(*Carrier).Bit_Slot;	// as Duty_Cycle fraction 
} 


float Onoise (DeviceSTR *Network) {
  float h= 6.626E-34;					// [J s] Planck constant
  float G= (*Network).EDFAgain* (*Network).EDFAgain;	// [1] EDFA power gain
  float n_sp= 3.5;     					// [1]
  float nu= (*Network).CentralNu;			// [Hz]
  float S_sp= (G- 1)* n_sp* h* nu;			// [J]
  float P_N= S_sp* (*Network).sim_BW;			// [W]
  float P_ASE= 2* P_N;					// [W]
  float OSigmaNoise= sqrt(P_ASE);
  return OSigmaNoise;
}


float Dnoise (DeviceSTR *Network, float Pmean_Rxin, float OSigmaNoise) {
  float q= 1.602E-19;				// [A s]
  float K_B= 1.381E-23;				// [J/K]

  float F_A= 5.5;				// [1] exceess noise factor APD
  float P_in= Pmean_Rxin;
  float I_d= 1E-8;				// [A] APD dark current
  float B_e= 1E9* (*Network).Efilt_CutOff;	// [Hz] effective noise bandwidth (assumed =) electrical filter cut-off frequency

  // Shot noise - ASE power incorporated
  float P_ASE= OSigmaNoise* OSigmaNoise;
  float P_ASE_downstream= P_ASE* (*Network).FAtt2* (*Network).FAtt2;
  float P_ASE_filtered= P_ASE_downstream* (2E9* (*Network).Ofilt_CutOff/ (*Network).sim_BW);
  float sigma_shot_wASE2= 2* q* (*Network).M* (*Network).M* F_A* ((*Network).R* (P_in+ P_ASE_filtered )+ I_d )* B_e;
  // printf("sigma_shot_wASE2[W]= %e\n", sigma_shot_wASE2);	// TEST

  // Thermal noise
  float T= 298.17;     // [K]
  float R_L= 50;     // [J/(s A^2)]
  float sigma_thermal2= 4* K_B* T* B_e/R_L;
  // printf("sigma_thermal2[W]= %e\n", sigma_thermal2);	// TEST

  float DSigmaNoise= sqrt(sigma_thermal2+ sigma_shot_wASE2);
  return DSigmaNoise;
}


void GenRuidoRapido(float *Noise, float Sigma, unsigned long int Tmax) {
  void Gasdev(float *V2, float Sig);
  unsigned long int i;
  static long int sem = -1;	//	Semilla para el generador de numeros random.
  if(sem==-1)	sem =-time(NULL);	
  for(i=0; i<Tmax; i+=2) Gasdev(&(Noise[i]), Sigma);
  for(i=0; i<Tmax; i+=2){
    Gasdev(&(Noise[i]), Sigma);
    Noise[i]*= 2.0* ((float) rand()/ (float) RAND_MAX )- 1.0;   	// phase effect
  }
}


void Gasdev(float *V2, float Sigma) {
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
  // Write1Col("temp.dat",PulsesTrainTime,TrainLength* (*Carrier).NPtos_Bit);
}


void RZTrain(FLAGS *Banderas, float RZPulsesTrain[], GENDRIVE *Carrier, SuperGaussian *Gauss, float RZBit1Gauss[], unsigned char InBitString[], unsigned long int TrainLength, unsigned int transmitters) {

  // RZ PULSES TRAIN (Amplitude)
  unsigned long int i;
  unsigned int senders, k= 0;
  while(k<TrainLength) {
    senders= InBitString[k];
    for(i=0; i<(*Carrier).NPtos_Bit; i++) {
      // Optical intensities addition
      RZPulsesTrain[k*(*Carrier).NPtos_Bit+i]= RZBit1Gauss[i]+ 2* (*Gauss).A_0;
      RZPulsesTrain[k*(*Carrier).NPtos_Bit+i]*= (float) senders* RZBit1Gauss[i];
      RZPulsesTrain[k*(*Carrier).NPtos_Bit+i]+= (float) transmitters * (*Gauss).P_0;
      RZPulsesTrain[k*(*Carrier).NPtos_Bit+i]= sqrt( RZPulsesTrain[k*(*Carrier).NPtos_Bit+i]); 
    }
    k++;
  }
  // Write1Col("amplitude1.dat", RZBit1Gauss, (*Carrier).NPtos_Bit);	// TEST bitslot '1'
  if ((*Banderas).LinkVectors ) Write1Col("RZPulsesTrain.dat", RZPulsesTrain, (*Carrier).NPtos_Tot);	// TEST train to file
  // for(i=0; i<(*Carrier).NPtos_Tot; i++) printf("%.3e\t%.3e\n",RZPulsesTrainTime[i],RZPulsesTrain[i]);	// TEST
}


void OpticalFilter(float SignalVector[], unsigned long int SignalVectorSize) {
  /* Digital filter designed by mkfilter/mkshape/gencode   A.J. Fisher
  Command line: /www/usr/fisher/helpers/mkfilter -Bu -Lp -o 2 -a 1.5625000000e-02 0.0000000000e+00 -l
  passtype    =    Lowpass 
  order    =    6 
  samplerate    =    6.4E11 	= 2^6/ bit slot
  */

  // FILTER Order= 6
  // #define oNZEROS 6
  // #define oNPOLES 6
  // #define oGAIN   8.599958842e+07	// BUTTERWORTH CutOff= 6GHz
  // #define oGAIN   8.599958842e+07	// BUTTERWORTH CutOff= 10GHz
  // #define oGAIN   2.985320090e+07	// BUTTERWORTH CutOff= 12GHz
  // #define oGAIN   3.419013222e+06 	// BUTTERWORTH CutOff= 17.5GHz
  // #define oGAIN   2.357704323e+07 	// BUTTERWORTH CutOff= 12.5GHz
  // #define oGAIN   1.775271324e+05	// BESSEL CutOff= 17.5GHz
  // #define oGAIN   4.570794845e+05 	// BUTTERWORTH CutOff= 25GHz
  
  // FILTER ORDER 2
  #define oNZEROS 2
  #define oNPOLES 2
  #define oGAIN  2.889601535e+02

  static float xvo[oNZEROS+1], yvo[oNPOLES+1];
  unsigned long int i;

  // printf("xv(0)= %3e\txv(1)= %3e\t\n",xv[0], xv[1]); // TEST
  for (i=0; i<SignalVectorSize; i++) {
/*
    // 6 poles
    xvo[0] = xvo[1]; xvo[1] = xvo[2]; xvo[2] = xvo[3]; xvo[3] = xvo[4]; xvo[4] = xvo[5]; xvo[5] = xvo[6];
    xvo[6] = SignalVector[i]/ oGAIN;
    yvo[0] = yvo[1]; yvo[1] = yvo[2]; yvo[2] = yvo[3]; yvo[3] = yvo[4]; yvo[4] = yvo[5]; yvo[5] = yvo[6];
    yvo[6] = (xvo[0] + xvo[6]) + 6 * (xvo[1] + xvo[5]) + 15 * (xvo[2] + xvo[4]) + 20 * xvo[3]
    // + ( -0.6841955724 * yvo[0]) + (  4.3646808658 * yvo[1]) + (-11.6101420990 * yvo[2]) + ( 16.4837640270 * yvo[3]) + (-13.1748195840 * yvo[4]) + (  5.6207116179 * yvo[5]);	// BUTTERWORTH CutOff= 6GHz
    // + ( -0.6841955724 * yvo[0]) + (  4.3646808658 * yvo[1])+ (-11.6101420990 * yvo[2]) + ( 16.4837640270 * yvo[3])+ (-13.1748195840 * yvo[4]) + (  5.6207116179 * yvo[5]);		// BUTTERWORTH CutOff= 10Ghz
    // + ( -0.6341207148 * yvo[0]) + (  4.0933318918 * yvo[1])+ (-11.0212577670 * yvo[2]) + ( 15.8439404200 * yvo[3])+ (-12.8267655930 * yvo[4]) + (  5.5448696188 * yvo[5]);		// BUTTERWORTH CutOff= 12GHz
     + (-0.6221802648* yvo[0]) + (4.0280507862* yvo[1]) + (-10.8782825350* yvo[2]) + (15.6871268060* yvo[3]) + (-12.7406278990* yvo[4]) + (5.5259103928* yvo[5]);		// BUTTERWORTH CutOff= 12.5GHz
    // + ( -0.5143535878 * yvo[0]) + (  3.4274712300 * yvo[1]) + ( -9.5371585039 * yvo[2]) + ( 14.1860709290 * yvo[3]) + (-11.8984008380 * yvo[4]) + (  5.3363520514 * yvo[5]);		// BUTTERWORTH CutOff= 17.5GHz
    // + (-0.2634656628* yvo[0])+ (1.9408892973* yvo[1])+ (-5.9948938374* yvo[2])+ (9.9414861026* yvo[3])+ (-9.3395818843* yvo[4])+ (4.7152054763* yvo[5]);	// BESSEL CutOff= 17.5GHz
    // + (-0.3862279890* yvo[0])+ ( 2.6834487459* yvo[1])+ (-7.8013262392* yvo[2])+ (12.1514352550* yvo[3])+ (-10.6996337410* yvo[4])+ (5.0521639483* yvo[5]);		// BUTTERWORTH CutOff= 25GHz
    SignalVector[i]= yvo[6];
*/
    // 2 poles
    xvo[0] = xvo[1]; xvo[1] = xvo[2]; 
    xvo[2] = SignalVector[i] / oGAIN;
    yvo[0] = yvo[1]; yvo[1] = yvo[2]; 
    yvo[2] = (xvo[0]+ xvo[2] )+ 2* xvo[1]
      +(-0.8406758501 * yvo[0] )+ (1.8268331110* yvo[1] );	// BUTTERWORTH CutOff= 12.5GHz
    SignalVector[i] = yvo[2];
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
      *SamplingPoint= 48;	// opt: 17.5 ord 6 / elec: 7 ord 2
    break;
    case 14:
      eGAIN= 2.326204969e+02; 	// CutOff= 14 GHz
      *SamplingPoint= 40;	// opt: 12.5 ord 2 / elec: 14 ord 2
      // *SamplingPoint= 61;	// opt: 12.5 ord 6 / elec: 14 ord 2
      // *SamplingPoint= 51;	// opt: 17.5 ord 6 / elec: 14 ord 2
    break;
    default:
      eGAIN= 2.326204969e+02; 	// CutOff= 14 GHz
      *SamplingPoint= 40;	// opt: 12.5 ord 2 / elec: 14 ord 2
      // *SamplingPoint= 61;	// opt: 12.5 ord 6 / elec: 14 ord 2
      // *SamplingPoint= 51;	// opt: 17.5 ord 6 / elec: 14 ord 2
    /*
    // Con filtro optico 25 GHz corner, orden 6
    case 25:
      eGAIN= 7.820233128e+01;	// CutOff= 25GHz
      *SamplingPoint= 27;	// 25GHz ord 2 filtro electrico / optico
    break;
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
    // case 5:  eGAIN= 4.441320406e+02;	// CutOff= 5GHz
    // break;
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
    // Con filtro optico 17.5 GHz corner, orden 6
      ( -0.8233495955 * yve[0]) + (  1.8061542062 * yve[1]);  // opt: 17.5 ord 6 / elec: 14 ord 2
    // ( -0.9073853925 * yve[0]) + (  1.9028831033 * yve[1]);  // opt: 17.5 ord 6 / elec: 7 ord 2

    // Con filtro optico 25 GHz corner, orden 6 
    //	( -0.8703674775 * yve[0]) + (  1.8613611468 * yve[1]);	// CutOff= 10GHz
    //  ( -0.9329347318 * yve[0]) + (  1.9306064272 * yve[1]);	// CutOff= 5GHz
    //  ( -0.9200713754 * yve[0]) + (  1.9167412232 * yve[1]);	// CutOff= 6GHz
    //  ( -0.8465319748 * yve[0]) + (  1.8337326589 * yve[1]);  // CutOff= 12GHz
    //  ( -0.7067570632 * yve[0]) + (  1.6556076929 * yve[1]);	// CutOff= 25GHz

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


float Threshold3(GENDRIVE *Carrier, float RZBit1Gauss[], float *OSigmaNoise, float *DSigmaNoise, DeviceSTR *Network, unsigned int transmitters, SuperGaussian *Gauss) {		// TEST Ith comment
// float Threshold3(GENDRIVE *Carrier, float RZBit1Gauss[], float *OSigmaNoise, float *DSigmaNoise, DeviceSTR *Network, unsigned int transmitters, SuperGaussian *Gauss, float IthFrac) {		// TEST Ith uncomment
  unsigned long int i;
  unsigned short int Pul, NPulThr= 5000;                  // averaged bits to obtain threshold
  float One[(*Carrier).NPtos_Bit], Zero[(*Carrier).NPtos_Bit];

/*
  // Optical & Electric IRR filters dummy start
  float One[(*Carrier).NPtos_Bit], Zero[(*Carrier).NPtos_Bit];
  float DummyAux= sqrt( (float) transmitters )* (*Gauss).A_0* (*Network).FAtt1* (*Network).EDFAgain* (*Network).FAtt2;
  for(i=0; i<(*Carrier).NPtos_Bit; i++) Zero[i]= DummyAux;
  OpticalFilter(Zero, (*Carrier).NPtos_Bit);
  DummyAux*= DummyAux;		// Optical: Amplitude -> Power
  DummyAux*= (*Network).M* (*Network).R;	// Optical Power -> Electrical Current 
  for(i=0; i<(*Carrier).NPtos_Bit; i++) Zero[i]= DummyAux;
  int SP;
  ElectricalFilter(Zero, (*Carrier).NPtos_Bit, &SP, Network);
*/

  // OPTICAL NOISE VECTOR FOR THRESHOLD DETERMINATION BITS
  float ONoiseVector[(*Carrier).NPtos_Bit* NPulThr];
  GenRuidoRapido(ONoiseVector, *OSigmaNoise, (*Carrier).NPtos_Bit* NPulThr);		// NoiseVector amplitude
  // Write1Col("ONoiseVector.dat", ONoiseVector, 2*(*Carrier).NPtos_Bit* NPulThr);	// TEST train to file
  // printf("nombre= %i\n",2*(*Carrier).NPtos_Bit* NPulThr);	// TEST

  // DETECTOR (THERMAL) NOISE VECTOR FOR THRESHOLD DETERMINATION BITS
  float DNoiseVector[(*Carrier).NPtos_Bit* NPulThr];
  GenRuidoRapido(DNoiseVector, *DSigmaNoise, (*Carrier).NPtos_Bit* NPulThr);		// NoiseVector amplitude
  // Write1Col("DNoiseVector.dat", DNoiseVector2, 2*(*Carrier).NPtos_Bit* NPulThr);	// TEST train to file

  // DETERMINATION BITS: CREATION, ATTENUATION Tx -> EDFA
  float AuxZero= sqrt( (float) transmitters )* (*Gauss).A_0;
  float AuxOne= (float) transmitters* (*Gauss).P_0;
  for(i=0; i<(*Carrier).NPtos_Bit; i++) {
    Zero[i]= AuxZero;	// zero level for present transmitters

    One[i]= 2* (*Gauss).A_0 + RZBit1Gauss[i];				// one overpulse addition
    One[i]*= RZBit1Gauss[i];
    One[i]+= AuxOne;
    One[i]= sqrt(One[i] );

    // printf("One[%li]= %e\n",i,One[i]); // TEST
    One[i]*= (*Network).FAtt1, Zero[i]*= (*Network).FAtt1;		// ATTENUATION STAGE Tx -> EDFA out
//    One[i]*= (*Network).EDFAgain, Zero[i]*= (*Network).EDFAgain;	// EDFA STAGE EDFAin -> EDFAout
  }

  // Random input bit string
  unsigned N0=0, N1=0;
  unsigned char RanStr[NPulThr];
  RandIntString(RanStr, NPulThr);

  // Super pulses string
  float SuperStr[(*Carrier).NPtos_Bit* NPulThr];
  for(Pul= 0; Pul< NPulThr; Pul++) {
    if (RanStr[Pul]==0) {
      N0++;
      for(i=0; i<(*Carrier).NPtos_Bit; i++) SuperStr[i+ Pul* (*Carrier).NPtos_Bit]= Zero[i];
    }
    else {
      N1++;
      for(i=0; i<(*Carrier).NPtos_Bit; i++) SuperStr[i+ Pul* (*Carrier).NPtos_Bit]= One[i];
    }
  }
  // Write1Col("SuperStr.dat",SuperStr,(*Carrier).NPtos_Bit* NPulThr);

  // Optical noise addition
  for(i=0; i<(*Carrier).NPtos_Bit* NPulThr; i++) SuperStr[i]+= ONoiseVector[i];

  // Attenuation EDFA -> Rx
  for(i=0; i<(*Carrier).NPtos_Bit* NPulThr; i++) SuperStr[i]*= (*Network).FAtt2;

  // Optical filtering
  OpticalFilter(SuperStr, (*Carrier).NPtos_Bit* NPulThr);

  // -> CURRENT + ELECTRICAL NOISE
  for(i=0; i<(*Carrier).NPtos_Bit* NPulThr; i++) {
      SuperStr[i]*= SuperStr[i];				// optical amplitude -> optical power
      SuperStr[i]*= (*Network).M* (*Network).R;				// -> electrical current
      SuperStr[i]+= DNoiseVector[i]; 				// electrical noise addition
  }

  // Electrical filtering
  int SP;
  ElectricalFilter(SuperStr, (*Carrier).NPtos_Bit* NPulThr, &SP, Network);
  // Write1Col("SuperStr.dat",SuperStr,(*Carrier).NPtos_Bit* NPulThr);

  // EYE DIAGRAM REQUIRED TO SELECT MIN & MAX
  // unsigned int (*Network).Range= 2; 	// even numbers -> effective range SP +- SP/2
  (*Network).Min= SP- (*Network).Range/2;
  (*Network).Range++;			// for correct sampling size
  // printf("%i\n",SP); // TEST

/*
  // Manual decision current
  float I0m= AuxZero* (*Network).FAtt1* (*Network).EDFAgain * (*Network).FAtt2;
  I0m*= I0m;					// -> electrical current
  float I1m= sqrt((2* (*Gauss).A_0 + (*Gauss).A_peak )* (*Gauss).A_peak+ AuxOne)* (*Network).FAtt1* (*Network).EDFAgain * (*Network).FAtt2;
  I1m*= I1m;					// -> electrical current
  printf("I0m= %e\t I1m= %e\n",I0m, I1m);		// TEST
  float Idm= I0m+ (I1m- I0m)* IthFrac;
*/

/*
  // with manual current threshold guess
  float Samp0[NPulThr], Samp1[NPulThr], AuxSum;
  unsigned int pos0= 0, pos1=0 ;
  for(Pul= 0; Pul< NPulThr; Pul++) {
    AuxSum= 0;
    for(i= 0; i< Range; i++) {
      AuxSum+= SuperStr[Min+ i+ Pul* (*Carrier).NPtos_Bit];
    }
    AuxSum/= (float) Range;
    if (AuxSum> IthFrac) {
      Samp1[pos1]= AuxSum;
      pos1++;
    }
    else {
      Samp0[pos0]= AuxSum;
      pos0++;
    }
  }
  N0= pos0, N1= pos1;
*/

  //with string previous knowledge
  // Sampling (mean at each bit slot) with string previous knowledge
  float Samp0[N0], Samp1[N1], AuxSum;
  unsigned int pos0= 0, pos1=0 ;
  for(Pul= 0; Pul< NPulThr; Pul++) {
    AuxSum= 0;
    for(i= 0; i< (*Network).Range; i++) {
      AuxSum+= SuperStr[(*Network).Min+ i+ Pul* (*Carrier).NPtos_Bit];
      // printf("%i\t %li\t %li \t %e\n",RanStr[Pul], i, Min+ i+ Pul* (*Carrier).NPtos_Bit, SuperStr[Min+ i+ Pul* (*Carrier).NPtos_Bit]); // TEST
    }

   if (RanStr[Pul]==0) {	 
      Samp0[pos0]= AuxSum/ (float) (*Network).Range;
      // printf("0m %e\n",Samp0[pos0]);		// TEST
      pos0++;
    }
    else {
      Samp1[pos1]= AuxSum/ (float) (*Network).Range;
      // printf("1m %e\n",Samp1[pos1]);		// TEST
      pos1++;
    }
  }
  // Write1Col("Super.dat", SuperStr, NPulThr* (*Carrier).NPtos_Bit);	// TEST train to file
  // Write1Col("Samp0.dat", Samp0, N0);	// TEST train to file
  // Write1Col("Samp1.dat", Samp1, N1);	// TEST train to file


  // Bit 0 statistics
  float sum0= 0, sum_sqr0= 0;
  for(i=0; i< N0; i++) {
    sum0+= Samp0[i];
    Samp0[i]*= Samp0[i];
    sum_sqr0+= Samp0[i];
  }
  float I0= sum0/ (float) N0; 		// moyenne 0
  float sigma0= sqrt(( 1/ ((float) N0 - 1 ) )* (sum_sqr0- ((sum0* sum0 )/ (float) N0 ) ) );
  // printf("I0= %e\t sigma0= %e\n",I0, sigma0); // TEST

  // Bit 1 statistics
  float sum1= 0, sum_sqr1= 0;
  for(i=0; i< N1; i++) {
    sum1+= Samp1[i];
    Samp1[i]*= Samp1[i];
    sum_sqr1+= Samp1[i];
  }
  float I1= sum1/ (float) N1;           // moyenne 1
  float sigma1= sqrt(( 1/ ((float) N1 - 1 ) )* (sum_sqr1- ((sum1* sum1 )/ (float) N1 ) ) );
  // printf("I1= %e\t sigma1= %e\n",I1, sigma1); // TEST


  /*  // Sampling (individual samples)
  float Samp0[Range* N0], Samp1[Range* N1];
  unsigned int pos0= 0, pos1=0 ;
  for(Pul= 0; Pul< NPulThr; Pul++) {
    if (RanStr[Pul]==0) {
      for(i= pos0; i< pos0+ Range; i++) {
        Samp0[i]= SuperStr[Min+ (i- pos0)+ Pul* (*Carrier).NPtos_Bit];
        // printf("0:%li\t %li\n",i, Min+ (i- pos0)+ Pul* (*Carrier).NPtos_Bit); // TEST
      }
      pos0+= Range;
    }
    else {
      for(i= pos1; i< pos1+ Range; i++) {
        Samp1[i]= SuperStr[Min+ (i- pos1)+ Pul* (*Carrier).NPtos_Bit];
        // printf("1: %li\t %li\n",i ,Min+ (i- pos1)+ Pul* (*Carrier).NPtos_Bit); // TEST
      }
      pos1+= Range;
    }
  }
  // printf("N0= %i\tN1= %i\n", N0, N1);
  // Write1Col("Samp0.dat", Samp0, Range* N0);	// TEST train to file
  // Write1Col("Samp1.dat", Samp1, Range* N1);	// TEST train to file

  // Bit 0 statistics
  float sum0= 0, sum_sqr0= 0;
  for(i=0; i< Range* N0; i++) {
    sum0+= Samp0[i];
    Samp0[i]*= Samp0[i];
    sum_sqr0+= Samp0[i];
  }
  float n0= Range* N0;
  float I0= sum0/n0; 		// moyenne 0
  float sigma0= sqrt(( 1/ (n0 - 1 ) )* (sum_sqr0- ((sum0* sum0 )/n0 ) ) );
  // printf("sigma0= %e\n",sigma0); // TEST

  // Bit 1 statistics
  float sum1= 0, sum_sqr1= 0;
  for(i=0; i<Range* N1; i++) {
    sum1+= Samp1[i];
    Samp1[i]*= Samp1[i];
    sum_sqr1+= Samp1[i];
  }
  float n1= Range* N1;
  float I1= sum1/n1;           // moyenne 1
  float sigma1= sqrt(( 1/ (n1 - 1 ) )* (sum_sqr1- ((sum1* sum1 )/n1 ) ) );
  */

  // printf("%e\n",(I1- I0)/ (sigma0+ sigma1));	// TEST QBER 

  // Threshold calculation
  float Id= (sigma0* I1+ sigma1* I0)/ (sigma0+ sigma1);	// TEST Ith comment
  // float Id= I0+ (I1- I0 )* IthFrac;        // TEST Ith uncomment
  // printf("IthFrac= %e\n",IthFrac); // TEST Ith
  // printf("Id= %e\n",Id);	// TEST QBER 
  return Id; 
}



float BitSlots(FLAGS *Banderas, GENDRIVE *Carrier, SuperGaussian *Gauss, DeviceSTR *Network, float OSNR, float *OSigmaNoise, float DSNR, float *DSigmaNoise, unsigned int transmitters, float RZBit1Gauss[], float AuxTemp[]) {	// TEST Ith comment
// float BitSlots(GENDRIVE *Carrier, SuperGaussian *Gauss, DeviceSTR *Network, float OSNR, float *OSigmaNoise, float DSNR, float *DSigmaNoise, unsigned int transmitters, float RZBit1Gauss[], float AuxTemp[], float IthFrac) {		// TEST Ith uncomment

  unsigned long int i;
  // Petit AuxTemp
  for(i=0; i<(*Carrier).NPtos_Bit; i++) AuxTemp[i]= (*Carrier).dt* ( i+ 0.5) ;	// time for one slot

  // Pulse amplitude addition over base [RZ]
  float AuxAmp= (*Gauss).A_peak- (*Gauss).A_0;
  for(i=0; i<(*Carrier).NPtos_Bit; i++) {
    RZBit1Gauss[i]= AuxAmp;
    RZBit1Gauss[i]*= exp(-0.5* pow( ( ( AuxTemp[i]- ( (*Gauss).CentringFactor* (*Carrier).Bit_Slot))/ (*Gauss).T_0_amp), 2* (*Gauss).m));	// SuperGaussian overpulse
    // printf("RZ1[%li]= %e\n",i,RZBit1Gauss[i]);	// TEST
  }

  /*
  // Pulse amplitude addition over base [NRZ]
  for(i=0; i<(*Carrier).NPtos_Bit; i++) RZBit1Gauss[i]= ( (*Gauss).A_peak- (*Gauss).A_0);	// Square [NRZ]
  // for(i=0; i<(*Carrier).NPtos_Bit; i++) printf("RZ1[%li]= %e\n",i,RZBit1Gauss[i]); // TEST
  */

  /*
  // Bit slots test
  float RZBit0[(*Carrier).NPtos_Bit];					// '0' SLOT AMPLITUDE
  float RZBit1[(*Carrier).NPtos_Bit];					// '1' SLOT AMPLITUDE [NRZ]
  for(i=0; i<(*Carrier).NPtos_Bit; i++) {
    RZBit0[i]= (*Gauss).P_0;	
    RZBit1[i]= (*Gauss).A_0+ RZBit1Gauss[i];
    RZBit1[i]*= RZBit1[i];
  }
  Write2Col("Bit01.dat", RZBit0, RZBit1, (*Carrier).NPtos_Bit,0, Carrier);	// TEST one & zero to file
  */

  // PmeanVector
  float PmeanVector[2* (*Carrier).NPtos_Bit];	// PmeanVector
  float auxVector1, auxVector0= transmitters* (*Gauss).P_0;
  float Pmean_sum;
  for(i=0, Pmean_sum= 0; i<(*Carrier).NPtos_Bit; i++) {
    auxVector1= RZBit1Gauss[i]+ 2* (*Gauss).A_0;
    auxVector1*= RZBit1Gauss[i];
    PmeanVector[i]= auxVector1+ auxVector0; 
    PmeanVector[i+ (*Carrier).NPtos_Bit]= auxVector0;
    Pmean_sum+= PmeanVector[i]+ PmeanVector[i+ (*Carrier).NPtos_Bit];
  }
  Pmean_sum*= 1.0/ (2.0* (*Carrier).NPtos_Bit );
  if ((*Banderas).Pmeans ) printf("Pmean_transmitter[W]= %3e\n",Pmean_sum);		// TEST PMEAN

  // ATTENUATION STAGE Tx -> EDFAout
  float aux_Att= (*Network).FAtt1;
  // float aux_Att= (*Network).FAtt1* (*Network).EDFAgain;
  aux_Att*= aux_Att;
  for(i=0, Pmean_sum= 0; i<2* (*Carrier).NPtos_Bit; i++) {
    PmeanVector[i]*= aux_Att; 
    Pmean_sum+= PmeanVector[i];
  }
  float Pmean_EDFAout= Pmean_sum/ (2.0* (*Carrier).NPtos_Bit );
  if ((*Banderas).Pmeans ) printf("Pmean_EDFAout= %3e\n",Pmean_EDFAout);		// TEST PMEAN

  // ONoise calculation
  if (OSNR==0) *OSigmaNoise= Onoise(Network);
  else *OSigmaNoise= sqrt((Pmean_EDFAout/ OSNR)*((*Network).sim_BW/(*Network).r_BW ) );	// optical noise amplitude simulation bandwidth [ TIA/EIA-526-19 ]
  // *OSigmaNoise= sqrt( (Pmean_EDFAout/ OSNR)*( 2* (*Network).Ofilt_CutOff/ (*Network).r_BW )  );	// optical noise amplitude optical filter bandwidth
  if ((*Banderas).Pmeans ) printf("OSigmaNoise^2= %3e\n", *OSigmaNoise* *OSigmaNoise);	// TEST
  if ((*Banderas).Pmeans ) printf("OSNR[dB, 0.1nm]= %3e\n", 10* log10((Pmean_EDFAout/ (*OSigmaNoise* *OSigmaNoise) )* ( 1/((*Network).r_BW* (*Carrier).dt ) ) ) );	// TEST
  // printf("1/dt= %3e\n", 1/(*Carrier).dt);		// TEST

  // Optical & Electric IRR filters dummy start
  float Zero[(*Carrier).NPtos_Bit];
  float DummyAux= sqrt( (float) transmitters )* (*Gauss).A_0* (*Network).FAtt1* (*Network).FAtt2;
  // float DummyAux= sqrt( (float) transmitters )* (*Gauss).A_0* (*Network).FAtt1* (*Network).EDFAgain* (*Network).FAtt2;
  for(i=0; i<(*Carrier).NPtos_Bit; i++) Zero[i]= DummyAux;
  OpticalFilter(Zero, (*Carrier).NPtos_Bit);
  DummyAux*= DummyAux;		// Optical: Amplitude -> Power
  DummyAux*= (*Network).M* (*Network).R;	// Optical Power -> Electrical Current 
  for(i=0; i<(*Carrier).NPtos_Bit; i++) Zero[i]= DummyAux;
  int SP;
  ElectricalFilter(Zero, (*Carrier).NPtos_Bit, &SP, Network);

  // Optical noise addition
  for(i=0, Pmean_sum= 0; i<2* (*Carrier).NPtos_Bit; i++) PmeanVector[i]= sqrt(PmeanVector[i] );		// power -> amplitude 
  float ONoiseVector[2* (*Carrier).NPtos_Bit];
  GenRuidoRapido(ONoiseVector, *OSigmaNoise, 2* (*Carrier).NPtos_Bit);		// NoiseVector amplitude
  for(i=0; i< 2* (*Carrier).NPtos_Bit; i++) PmeanVector[i]+= ONoiseVector[i];

  // ATTENUATION STAGE EDFAout -> Rxin
  for(i=0, Pmean_sum= 0; i<2* (*Carrier).NPtos_Bit; i++) {
    PmeanVector[i]*= (*Network).FAtt2; 
    Pmean_sum+= PmeanVector[i]* PmeanVector[i];
  }
  float Pmean_Rxin2= Pmean_sum/ (2.0* (*Carrier).NPtos_Bit );
  if ((*Banderas).Pmeans ) printf("Pmean_Rxin2= %3e\n",Pmean_Rxin2);		// TEST PMEAN

  // Optical filter at Rx input
  OpticalFilter(PmeanVector, 2* (*Carrier).NPtos_Bit);

  // Pmean post optical filter
  for(i=0, Pmean_sum= 0; i<2* (*Carrier).NPtos_Bit; i++) {
    PmeanVector[i]*= PmeanVector[i]; 
    Pmean_sum+= PmeanVector[i];
  }
  float Pmean_Rxin= Pmean_sum/ (2.0* (*Carrier).NPtos_Bit );
  if ((*Banderas).Pmeans ) printf("Pmean_Rxin= %3e\n",Pmean_Rxin);		// TEST PMEAN

  // DSigmaNoise
  if (DSNR==0) *DSigmaNoise= Dnoise(Network, Pmean_Rxin, *OSigmaNoise);
  else *DSigmaNoise= Pmean_Rxin* (*Network).M* (*Network).R* sqrt(1/ DSNR);	// amplitude of noise (Agrawal 4.4.11) [Santiago I.]
  // printf("DSigmaNoise= %3e\n", *DSigmaNoise);		// TEST
  if ((*Banderas).Pmeans ) printf("DSigmaNoise^2= %3e\n", *DSigmaNoise* *DSigmaNoise);	// TEST
  if ((*Banderas).Pmeans ) printf("DSNR[dB]= %3e\n", 10* log10((Pmean_Rxin* (*Network).M* (*Network).R)* (Pmean_Rxin* (*Network).M* (*Network).R)/ (*DSigmaNoise* *DSigmaNoise ) ) ); // TEST
  if ((*Banderas).Pmeans ) printf("Pmean_photocurrent= %3e\n",Pmean_Rxin* Pmean_Rxin* (*Network).M* (*Network).M* (*Network).R* (*Network).R);		// TEST

  // DETECTOR THRESHOLD
  float Id= Threshold3(Carrier, RZBit1Gauss, OSigmaNoise, DSigmaNoise, Network, transmitters, Gauss);	// TEST Ith comment
  // float Id= Threshold3(Carrier, RZBit1Gauss, OSigmaNoise, DSigmaNoise, Network, transmitters, Gauss, IthFrac);		// TEST Ith uncomment
  return Id;
}


void EyeDiagram(float PulsesTrain[], float PulsesTrainTime[], GENDRIVE *Carrier, float Id) {
  unsigned long int i, iMidi= 0.5*(*Carrier).NPtos_Bit;
  FILE *EYE;
  EYE= fopen("EyeDiagram.dat", "w");
  fprintf(EYE,"# Id= %3e\n",Id);
  for(i=0; i<(*Carrier).NPtos_Tot; i++) {
    ldiv_t thing= ldiv(i,(*Carrier).NPtos_Bit);
    if(thing.rem< iMidi) {
      fprintf(EYE,"%3e\t%3e\t%3e\n",PulsesTrainTime[thing.rem], PulsesTrain[i], Id);
      fprintf(EYE,"%3e\t%3e\t%3e\n",PulsesTrainTime[thing.rem]+ (*Carrier).Bit_Slot, PulsesTrain[i], Id);
      // fprintf(EYE,"%3e\t%3e\n",PulsesTrainTime[thing.rem], PulsesTrain[i]);
      //fprintf(EYE,"%3e\t%3e\n",PulsesTrainTime[thing.rem]+ (*Carrier).Bit_Slot, PulsesTrain[i]);
    }
    else {
      fprintf(EYE,"%3e\t%3e\t%3e\n",PulsesTrainTime[thing.rem], PulsesTrain[i], Id);
      fprintf(EYE,"%3e\t%3e\t%3e\n",PulsesTrainTime[thing.rem]- (*Carrier).Bit_Slot, PulsesTrain[i], Id);
      // fprintf(EYE,"%3e\t%3e\n",PulsesTrainTime[thing.rem], PulsesTrain[i]);
      // fprintf(EYE,"%3e\t%3e\n",PulsesTrainTime[thing.rem]- (*Carrier).Bit_Slot, PulsesTrain[i]);
    }
  }
  fclose(EYE);
}


void Link(FLAGS *Banderas, GENDRIVE *Carrier, float RZPulsesTrain[], float *OSigmaNoise, float *DSigmaNoise, DeviceSTR *Network, int *SamplingPoint, float AuxTemp[]) {
  unsigned long int i;

  for(i=0; i<(*Carrier).NPtos_Tot; i++) RZPulsesTrain[i]*= (*Network).FAtt1; 	// ATTENUATION STAGE Tx -> EDFAin
  // Write1Col("AtEDFAin.dat", RZPulsesTrain, (*Carrier).NPtos_Tot);	// TEST train to file
/*
SigmaNoise: [V/m*10-3] width of amplitude Gaussian distribution A(t) related to mean noise power as follows:

white noise autocorrelation = \sigma^2 * delta(t)  [http://en.wikipedia.org/wiki/White_noise]
Wiener-Khinchin: \phi(\omega) [Power Spectral Density]= Fourier(white noise autocorrelation )  [http://en.wikipedia.org/wiki/Wiener�Khinchin_theorem]
-> \phi(\omega)= \sigma^2 * Fourier(\delta(t))
P [Power]= \int d\omega \phi(\omega)  [http://en.wikipedia.org/wiki/Spectral_density]
-> P= \sigma^2 * Fourier(\delta(t))
Gaussian: Fourier(\delta)=\frac{1}{\sqrt(2*\pi)}
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

  // for(i=0; i<(*Carrier).NPtos_Tot; i++) RZPulsesTrain[i]*= (*Network).EDFAgain; 		// EDFA STAGE EDFAin -> EDFAout
  if ((*Banderas).LinkVectors ) Write1Col("AtEDFAoutWON.dat", RZPulsesTrain, (*Carrier).NPtos_Tot);		// TEST train to file

  // Optical noise addition (AWGN)
  float ONoiseVector[(*Carrier).NPtos_Tot];
  GenRuidoRapido(ONoiseVector, *OSigmaNoise, (*Carrier).NPtos_Tot);		// NoiseVector amplitude
  if ((*Banderas).LinkVectors ) Write1Col("ONoiseVector.dat", ONoiseVector, (*Carrier).NPtos_Tot);		// TEST train to file
  for(i=0; i<(*Carrier).NPtos_Tot; i++) RZPulsesTrain[i]+= ONoiseVector[i];	// Noise addition
  if ((*Banderas).LinkVectors ) Write1Col("AtEDFAout.dat", RZPulsesTrain, (*Carrier).NPtos_Tot);		// TEST train to file

  /* // Photodetected ASE noise TEST
  float ONoiseVector2[(*Carrier).NPtos_Bit];
  for(i=0; i<(*Carrier).NPtos_Bit; i++) {
  ONoiseVector2[i]= ONoiseVector[i];
  // ONoiseVector2[i]*= (*Network).FAtt2;		// downstream attenuation
  }
  //  OpticalFilter(ONoiseVector2, (*Carrier).NPtos_Bit);
  float PD_ASE=0;
  for(i=0; i<(*Carrier).NPtos_Bit; i++) {
  ONoiseVector2[i]*= ONoiseVector2[i];
  PD_ASE+= ONoiseVector2[i];
  }
  printf("PD_ASE[W]= %e3\n",PD_ASE);
  */

  /*		// EDFA output check
  float RZPulsesTrainTime[(*Carrier).NPtos_Tot];			// Time vector  
  TimeVectorGenerator(Carrier, (*Carrier).N_Bits, AuxTemp, RZPulsesTrainTime);
  float Id=0;
  EyeDiagram(RZPulsesTrain, RZPulsesTrainTime, Carrier, Id);	// TEST Eye diagram
  */

  // Attenuation EDFA -> Rx
  for(i=0; i<(*Carrier).NPtos_Tot; i++) RZPulsesTrain[i]*= (*Network).FAtt2; 		// ATTENUATION STAGE EDFAout -> Rx

  // Optical filtering
  OpticalFilter(RZPulsesTrain, (*Carrier).NPtos_Tot);
  if ((*Banderas).LinkVectors ) Write1Col("OptFilt.dat", RZPulsesTrain, (*Carrier).NPtos_Tot);		// TEST train to file

  /*		// Optical filter output check
  float RZPulsesTrainTime[(*Carrier).NPtos_Tot];			// Time vector  
  TimeVectorGenerator(Carrier, (*Carrier).N_Bits, AuxTemp, RZPulsesTrainTime);
  float Id=0;
  EyeDiagram(RZPulsesTrain, RZPulsesTrainTime, Carrier, Id);	// TEST Eye diagram
  */

  // Optical -> Electrical signal (current)
  for(i=0; i<(*Carrier).NPtos_Tot; i++) {
    RZPulsesTrain[i]*= RZPulsesTrain[i];	// optical amplitude-> optical power
    RZPulsesTrain[i]*= (*Network).M* (*Network).R;		// -> electrical current
  }
  if ((*Banderas).LinkVectors ) Write1Col("PhotodetectedI.dat", RZPulsesTrain, (*Carrier).NPtos_Tot);		// TEST train to file

  // Detector electrical noise
  float DNoiseVector[(*Carrier).NPtos_Tot];					// Thermal noise vector
  GenRuidoRapido(DNoiseVector, *DSigmaNoise, (*Carrier).NPtos_Tot);	 	// NoiseVector amplitude
  for(i=0; i<(*Carrier).NPtos_Tot; i++)	RZPulsesTrain[i]+= DNoiseVector[i];	// Noise addition to electrical current signal
  if ((*Banderas).LinkVectors ) Write1Col("DNoiseVector.dat", DNoiseVector, (*Carrier).NPtos_Tot);		// TEST train to file
  // printf("DSNR=\t%e\tDSigmaNoise=\t%e\n",DSNR, DSigmaNoise);			// TEST
  if ((*Banderas).LinkVectors ) Write1Col("atElectFiltInput.dat", RZPulsesTrain, (*Carrier).NPtos_Tot);		// TEST train to file

  ElectricalFilter(RZPulsesTrain, (*Carrier).NPtos_Tot, SamplingPoint, Network);
  if ((*Banderas).LinkVectors ) Write1Col("ElecFilt.dat", RZPulsesTrain, (*Carrier).NPtos_Tot);	// TEST train to file
}


int main(int argc, char **argv) {

  int Erste= 0;			// write file auxiliary
  unsigned long int i;
  unsigned char buf[BUFFERMAX];
  int maxdata=BUFFERMAX;
  int SamplingPoint;		// Decision point (varies for each filter)

  // Simulation parameters declaration
  GENDRIVE Carrier;
  DeviceSTR Network;
  SuperGaussian Gauss;
  InputParameters(&Carrier, &Gauss, &Network);		// SYSTEM PARAMETERS

  // Flags and default values
  int c;
  unsigned int transmitters= 128;	// Default number of Tx
  Gauss.r_ex= 10;			// Default [dB] Extinction ratio [10~15 Grosz priv. comm. 090210]
  float DSNR=0;
  float OSNR=0;
  FLAGS Banderas; 
  Banderas.Eye= 0;		// Generates eye diagramme file
  Banderas.LinkVectors= 0;	// Generates intermediates steps files
  Banderas.Pmeans= 0;		// Intermediate mean powers -> stdout

  // Command line options (using getopt)
  while ((c= getopt(argc, argv, "elpc:d:o:r:") ) != -1) {
    switch (c) {
      case 'e':
        Banderas.Eye= 1;                		// Generates eye diagramme file
        break;
      case 'l':
        Banderas.LinkVectors= 1;        		// Generates intermediates steps files
        break;
      case 'p':
        Banderas.Pmeans= 1;             		// Intermediate mean powers -> stdout
        break;
      case 'c':
 	transmitters= atoi(optarg);			// Argument: number of Tx
	break;
      case 'd':
 	DSNR= pow(10.0,atof(optarg)/10.0);		// Argument: DSNR [dB] -> DSNR [1]
	break;
      case 'o':
 	OSNR= pow(10.0,atof(optarg)/10.0);		// Argument: OSNR [dB] -> OSNR [1]
	break;
      case 'r':
  	Gauss.r_ex= atof(optarg);			// Argument:Extinction ratio [dB]
	break;
      case '?':
        if (optopt == 'c')
          fprintf (stderr, "Option -%c requires argument: # transmitters.\n", optopt);
        if (optopt == 'd')
          fprintf (stderr, "Option -%c requires an argument: DSNR[dB].\n", optopt);
        if (optopt == 'o')
          fprintf (stderr, "Option -%c requires an argument: OSNR[dB].\n", optopt);
        if (optopt == 'r')
          fprintf (stderr, "Option -%c requires an argument: Extinction ratio[dB].\n", optopt);
        else if (isprint (optopt))
          fprintf (stderr, "Unknown option `-%c'.\n", optopt);
        else
          fprintf (stderr,"Unknown option character `\\x%x'.\n",optopt);
          return 1;
      default:
        abort ();
    }
  }


  // Rx power
  Gauss.r_ex = pow(10.0, Gauss.r_ex/ 10.0);         	// [1] Extinction ratio
  // (*Gauss).r_ex= 1.1;				// TEST 
  //printf("r_ex[dB]= %e\n",Gauss.r_ex);               	// TEST
  Gauss.P_0= 2.0* Gauss.P/ (1.0+ Gauss.r_ex);		// [W] '0' 
  // printf("P_0[W]= %e\n",Gauss.P_0);               	// TEST
  Gauss.A_0= sqrt( Gauss.P_0 );				// [sqrt{W}] '0' bit amplitude
  float P_1NRZ= Gauss.P_0* Gauss.r_ex;			// [W] '1' bit power full duty cycle
  // Gauss.P_peak= P_1NRZ/ Carrier.Duty_Cycle;		// [W] duty cycle corrected '1' bit power (square pulse aprox)
  Gauss.P_peak= P_1NRZ;					// [W] '1' bit power (square pulse aprox)
  Gauss.A_peak= sqrt( Gauss.P_peak);			// [sqrt{W}] duty cycle corrected '1' bit amplitude
  // printf("P_0= %e\tA_0= %e\tP_p= %e\tA_p= %e \n",Gauss.P_0, Gauss.P_peak, Gauss.A_0, Gauss.A_peak);	// TEST
  // printf("Tx= %i\tDSNR= %e \t OSNR= %e \t r_ex= %e\n",transmitters, DSNR, OSNR, Gauss.r_ex);	// TEST
  

  // float IthFrac= atof( argv[4]);
  // float IthFrac= atof( argv[4])/100;            // TEST Ith comment
  // printf("IthFrac= %e\n",IthFrac); 	// TEST Ith

/*
  // Attenuation
  float FAtt= 1.0;
  float FAtt= exp(-0.5* pow(10, Network.alpha/ 10)* Network.length );
  printf("FAtt=\t%e\n",FAtt); 		// TEST
*/

  float RZBit1Gauss[Carrier.NPtos_Bit];
  float AuxTemp[Carrier.NPtos_Bit];
  float OSigmaNoise, DSigmaNoise;
  float Id= BitSlots(&Banderas, &Carrier, &Gauss, &Network, OSNR, &OSigmaNoise, DSNR, &DSigmaNoise, transmitters, RZBit1Gauss, AuxTemp);	// TEST Ith comment
  // float Id= BitSlots(&Carrier, &Gauss, &Network, OSNR, &OSigmaNoise, DSNR, &DSigmaNoise, transmitters, RZBit1Gauss, AuxTemp, IthFrac);		// TEST Ith uncomment


  while( ( Carrier.N_Bits= fread(buf,1,maxdata,stdin) )> 0) {
    /* 	// INPUTS READ
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
    RZTrain(&Banderas, RZPulsesTrain, &Carrier, &Gauss, RZBit1Gauss, InBitString, Carrier.N_Bits, transmitters);

    // Network elements 
    Link(&Banderas, &Carrier, RZPulsesTrain, &OSigmaNoise, &DSigmaNoise, &Network, &SamplingPoint, AuxTemp);

    // Output check
    if (Banderas.Eye ) {
      float RZPulsesTrainTime[Carrier.NPtos_Tot];			// Time vector  
      TimeVectorGenerator(&Carrier, Carrier.N_Bits, AuxTemp, RZPulsesTrainTime);
      EyeDiagram(RZPulsesTrain, RZPulsesTrainTime, &Carrier, Id);	// TEST Eye diagram
      // Write2Col("train.dat", RZPulsesTrainTime, RZPulsesTrain, Carrier.NPtos_Tot, Erste, &Carrier);	// TEST train to file
    }

    // Rx: BIT DETERMINATION
    char BitsDisc[Carrier.N_Bits];
    float AuxSum;
    unsigned short int k;
    for(i= 0; i< Carrier.N_Bits; i++) {
      AuxSum= 0;
      for(k= 0; k< Network.Range; k++) {
        AuxSum+= RZPulsesTrain[Network.Min+ k+ i* Carrier.NPtos_Bit];
      }
      AuxSum/= (float) Network.Range;
      if(AuxSum> Id) BitsDisc[i]=1;
      else BitsDisc[i]=0;
      // printf("AuxSum[%li]= %e \t %i \t %i \n",i, AuxSum, buf[i], BitsDisc[i]);	// TEST
    }
    // printf("Id= %3e\t Min=%i \t Range= %i\n",Id, Network.Min, Network.Range);

    // OUTPUT AS CHAR STRING
    for(i=0; i<Carrier.N_Bits; i++) buf[i]=BitsDisc[i];
    write(1,buf,Carrier.N_Bits);	// TEST QBER COMMENT
  
    // Write file auxiliary
    Erste++;
    }
  return 0;
}

