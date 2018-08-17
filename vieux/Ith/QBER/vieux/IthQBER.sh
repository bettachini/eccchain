#!/bin/bash

# Paths
BASE_PATH="/home/vbettachini/Documentos/PONs"
NOISESIM_PATH="${BASE_PATH}/eccchain/Ith/QBER"	# Ith noisesim version
BN_PATH="${BASE_PATH}/simulations/Graphs"

STRINGS_PATH="/home/vbettachini/Documentos/strings2"
OUTPUT_PATH=${NOISESIM_PATH}

# Clients
CLIENTS=1
BITS=1
INPUTFILE=AT_${CLIENTS}_${BITS}.dat

# SNR [dB]
DSNR=15.8
OSNR=20.5

# Ith Fraction
IFRSTART=10 
IFRSTEP=2
IFRSTOP=90

# Main cycle
OUTPUTFILE="QBER_${INPUTFILE}"
echo -e "#IFR\tQ">${OUTPUT_PATH}/${OUTPUTFILE}
# OUTPUTFILE="ITH_${INPUTFILE}"
#echo -e "#IFR\tBER">${OUTPUT_PATH}/${OUTPUTFILE}
for IFR in `seq ${IFRSTART} ${IFRSTEP} ${IFRSTOP}`;
do
  echo -e "calculando IthFrac=\t${IFR/,/.}"
#  ${NOISESIM_PATH}/noisesim ${CLIENTS} ${DSNR/,/.} ${OSNR/,/.} ${IFR/,/.} <${STRINGS_PATH}/${INPUTFILE} >${OUTPUT_PATH}/aux${INPUTFILE}
  echo -e -n "${IFR/,/.}\t">>${OUTPUT_PATH}/${OUTPUTFILE}
#  ${BN_PATH}/BN ${STRINGS_PATH}/${INPUTFILE} ${OUTPUT_PATH}/aux${INPUTFILE} >>${OUTPUT_PATH}/${OUTPUTFILE}
  ${NOISESIM_PATH}/noisesim ${CLIENTS} ${DSNR/,/.} ${OSNR/,/.} ${IFR/,/.} <${STRINGS_PATH}/${INPUTFILE} >>${OUTPUT_PATH}/${OUTPUTFILE}
#  rm ${OUTPUT_PATH}/aux${INPUTFILE}
done
