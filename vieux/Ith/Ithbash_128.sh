#!/bin/bash

# Paths
BASE_PATH="/home/vbettachini/Documentos/PONs"
NOISESIM_PATH="${BASE_PATH}/eccchain"
BN_PATH="${BASE_PATH}/simulations/Graphs"

STRINGS_PATH="/home/vbettachini/Documentos/strings2"
OUTPUT_PATH=${NOISESIM_PATH}/Ith

# Clients
CLIENTS=128
BITS=1000
INPUTFILE=AT_${CLIENTS}_${BITS}.dat

# SNR [dB]

# r_ex=20dB
DSNR=26.5
OSNR=28.16

# r_ex=10dB
# DSNR=36.5
# OSNR=37.2

# Ith Fraction
IFRSTART=0 
IFRSTEP=2
IFRSTOP=100

# Main cycle
OUTPUTFILE="ITH_${INPUTFILE}"
echo -e "#IFR\tBER">${OUTPUT_PATH}/${OUTPUTFILE}
for IFR in `seq ${IFRSTART} ${IFRSTEP} ${IFRSTOP}`;
do
  echo -e "calculando IthFrac=\t${IFR/,/.}"
  ${NOISESIM_PATH}/noisesim ${CLIENTS} ${DSNR/,/.} ${OSNR/,/.} ${IFR/,/.} <${STRINGS_PATH}/${INPUTFILE} >${OUTPUT_PATH}/aux${INPUTFILE}
  echo -e -n "${IFR/,/.}\t">>${OUTPUT_PATH}/${OUTPUTFILE}
  ${BN_PATH}/BN ${STRINGS_PATH}/${INPUTFILE} ${OUTPUT_PATH}/aux${INPUTFILE} >>${OUTPUT_PATH}/${OUTPUTFILE}
  rm ${OUTPUT_PATH}/aux${INPUTFILE}
done
