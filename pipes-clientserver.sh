#!/bin/sh
MATRIX=1024-512-8.txt
CLIENTES=$1
RSBYTES=223
FILE=test.txt

# generamos archivo random,32 Megabits
dd if=/dev/urandom of=test.txt count=16

SCRAMBLEBLOCK=$(echo "${RSBYTES}*4" | bc)
	echo "----------------------------------------"
	echo "Scrambler block:" ${SCRAMBLEBLOCK}
	python -c "print 'Aprovechamiento: %f' % (((${CLIENTES}/330.0)/1)*(${RSBYTES}/(${RSBYTES} + 32.0)))"

	# filtro via files
	rm *.out
	./rsenc <${FILE} | ./scrambler ${SCRAMBLEBLOCK} >rs.out
	./raw2bin < rs.out >ldpc.out
	#./bfenc ${CLIENTES} < ldpc.out | ./noisesim 120 100db 100db | ./bfdec ${CLIENTES} >bf.out
	./bfenc ${CLIENTES} < ldpc.out | ./bfdec ${CLIENTES} >bf.out
	#./bfenc-orig ${CLIENTES} < ldpc.out | ./bfdec-orig ${CLIENTES} >bf.out
	./bin2raw <bf.out  >ldpcdec.out
	./descramble ${SCRAMBLEBLOCK} <ldpcdec.out | ./rsdec >rsdec.out

	echo "BER post-bloomfilter: " `./ber ldpc.out bf.out`
	echo "BER post-ldpc: " `./ber rs.out ldpcdec.out`
	echo "BER post-reed-solomon: " `./ber ${FILE} rsdec.out`
	echo ${CLIENTES} " " `./ber ${FILE} rsdec.out`
