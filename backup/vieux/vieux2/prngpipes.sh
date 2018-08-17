#!/bin/sh
RSBYTES=223
MAXCLIENTES=256.0
FILE=rsenc.c
SCRAMBLEBLOCK=$(echo "${RSBYTES}*4" | bc)
for CLIENTES in `seq 130 10 2000`;
do
	echo "----------------------------------------"
	python -c "print 'Aprovechamiento: %f' % (((${CLIENTES}/${MAXCLIENTES}))*(${RSBYTES}/(${RSBYTES} + 32.0)) * (8.0/9.0))"

	# filtro via files
	rm *.out
	./rsenc <${FILE} | ./scrambler ${SCRAMBLEBLOCK} >rs.out
	./raw2bin < rs.out >ldpc.out
	./PRNGenc ${CLIENTES} < ldpc.out | ./PRNGdec ${CLIENTES} >bf.out
	./bin2raw <bf.out >ldpcdec.out
	./descramble ${SCRAMBLEBLOCK} <ldpcdec.out | ./rsdec >rsdec.out

	echo "BER post-bloomfilter: " `./ber ldpc.out bf.out`
	echo "BER post-ldpc: " `./ber rs.out ldpcdec.out`
	echo "BER post-reed-solomon: " `./ber ${FILE} rsdec.out`

done
