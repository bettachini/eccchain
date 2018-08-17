#!/bin/bash

# Inputs 
CLIENTS=1
BITS=1

# Input file
INPUTFILE=AT_${CLIENTS}_${BITS}.dat
#echo $INPUTFILE

# File paths
BASE_PATH="/home/vbettachini/Documentos/PONs"
NOISESIM_PATH="${BASE_PATH}/eccchain/DT"
BN_PATH="${BASE_PATH}/simulations/Graphs"
STRINGS_PATH="/home/vbettachini/Documentos/strings2"
# STRINGS_PATH="${BASE_PATH}/simulations/Strings"
OUTPUT_PATH="${BASE_PATH}/eccchain/DT"

# # Output folder creation
# N_FILE=1
# FOLDER_NAME=$(date +%y%m%d)_${CLIENTS}_${BITS}_${N_FILE}
# while [ -d $OUTPUT_PATH/$FOLDER_NAME ]; do
#   let N_FILE=N_FILE+1
#   FOLDER_NAME=$(date +%y%m%d)_${CLIENTS}_${BITS}_${N_FILE}
# done
# mkdir $OUTPUT_PATH/$FOLDER_NAME
# OUTPUT_PATH=${OUTPUT_PATH}/$FOLDER_NAME
# # echo $OUTPUT_PATH

# outres
OUTPUTFILE="QSNR${INPUTFILE}"
# cp $NOISESIM_PATH/noisesim.c $OUTPUT_PATH/noisesim_back.c
# cp $BN_PATH/ber_noise.sh $OUTPUT_PATH/ber_noise_back.sh

# vero SNR -> added into noisesim.c
# B :bit rate
# BW= Nbit* B   :total bandwidth
# Pnoise_0.1= Pnoise_BW* K
# OSNRv[dB]= OSNR[dB]- 10* log_10(K)
# delta_lambda=0.1E-9;
# delta_vNoise= vNoise(delta_lambda);   # ~/Documentos/PONs/Reporte/Graphs/matlab/vNoise.m
# B= 10E9;
# NPtos_Bit= 2^6;
# K= delta_vNoise/ (B* NPtos_Bit);
# K= 0.027315           # ~/Documentos/PONs/Reporte/Graphs/matlab/ConvPnoise.m
# toOSNRvero= -10* log_10(K)= 15.636
# toOSNRvero= 15.636

# OSNR parameters [dB] [added \simeq K]
OSNRSTART=45
#OSNRSTART=-250
#OSNRSTEP=5E-1  # linear step
OSNRSTEP=-1     # linear step
OSNRSTOP=-2

# DSNR parameters [dB]
DSNR=15.8
#DSNR=39
#DSNR=42

# Main cycle
echo -e "#OSNR\tBER">${OUTPUTFILE}
for OSNR in `seq ${OSNRSTART} ${OSNRSTEP} ${OSNRSTOP}`;
do
  echo -e "calculando OSNR=\t${OSNR/,/.}"
  echo -e -n "${OSNR/,/.}\t">>${OUTPUT_PATH}/${OUTPUTFILE}
  ${NOISESIM_PATH}/noisesimDT ${CLIENTS} ${DSNR} ${OSNR/,/.} <${STRINGS_PATH}/${INPUTFILE} >>${OUTPUT_PATH}/${OUTPUTFILE}
done

