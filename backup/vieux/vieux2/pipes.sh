#!/bin/sh
#MATRIX=2048-1024-4.txt
MATRIX=1024-512-7.txt
#CLIENTES=48
RSBYTES=223
FILE=scrambler.c
SCRAMBLEBLOCK=$(echo "${RSBYTES}*4" | bc)
for CLIENTES in `seq 100 5 2000`;
do
	echo "----------------------------------------"
	python -c "print 'Aprovechamiento: %f' % (((${CLIENTES}/256.0)/2)*(${RSBYTES}/(${RSBYTES} + 32.0)))"

	#filtro via pipes
	#./rsenc <test-30-50.txt | ./scrambler ${SCRAMBLEBLOCK} | ./raw2bin | ./ldpcenc ${MATRIX} | ./bfenc ${CLIENTES} | ./bfdec ${CLIENTES} | ./ldpcdec ${MATRIX} |./bin2raw | ./descramble ${SCRAMBLEBLOCK} | ./rsdec >test.out

	# filtro via files
	rm *.out
	./rsenc <${FILE} | ./scrambler ${SCRAMBLEBLOCK} >rs.out
	./raw2bin < rs.out | ./ldpcenc ${MATRIX}  >ldpc.out
	./bfenc ${CLIENTES} < ldpc.out | ./bfdec ${CLIENTES} >bf.out
	./ldpcdec ${MATRIX} < bf.out | ./bin2raw >ldpcdec.out
	./descramble ${SCRAMBLEBLOCK} <ldpcdec.out | ./rsdec >rsdec.out

	echo "BER post-bloomfilter: " `./ber ldpc.out bf.out`
	echo "BER post-ldpc: " `./ber rs.out ldpcdec.out`
	echo "BER post-reed-solomon: " `./ber ${FILE} rsdec.out`

done
