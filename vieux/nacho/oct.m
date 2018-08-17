function oct

% carga de vectores
RZPulsesTrain= load('RZPulsesTrain.dat');
AtEDFAoutWON= load('AtEDFAoutWON.dat');
ONoiseVector= load('ONoiseVector.dat');
AtEDFAout= load('AtEDFAout.dat');
OptFilt= load('OptFilt.dat');
PhotodetectedI= load('PhotodetectedI.dat');
DNoiseVector= load('DNoiseVector.dat');
atElectFiltInput= load('atElectFiltInput.dat');
ElecFilt= load('ElecFilt.dat');


% potencias opticas medias
Pmean_transmitter= 3.739473e-02 
Pmean_transmitter_oct= mean(RZPulsesTrain.^2)

Pmean_EDFAoutWON= 2.970370e-02
Pmean_EDFAoutWON_oct= mean(AtEDFAoutWON.^2)

% potencia media ruido optico
OSigmaNoise2= 2.869678e-04
OSigmaNoise2_oct= var(ONoiseVector(:))

% potencias opticas medias
% Pmean_EDFAout= 1 	% (no hay equivalente)
Pmean_EDFAout_oct= mean(AtEDFAoutWON.^2)

Pmean_Rxin= 4.707718e-05 
Pmean_Rxin_oct= mean(OptFilt.^2)

% potencias electricas medias
Pmean_Photocurrent= 3.191416e-07 
Pmean_Photocurrent_oct= mean(PhotodetectedI.^2)

%  potencia media ruido detector 
DSigmaNoise2= 7.144691e-11
DSigmaNoise2_oct= var(DNoiseVector(:))

% potencias electricas medias
% Pmean_NoisyPhotocurrent= 1
Pmean_NoisyPhotocurrent_oct= mean(atElectFiltInput.^2)

% Pmean_NoisyPhotocurrent= 1
Pmean_FilteredNoisyPhotocurrent_oct= mean(ElecFilt.^2)
 
