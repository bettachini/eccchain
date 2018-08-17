#!/bin/sh
#MATRIX=2048-1024-4.txt
MATRIX=1024-512-4.txt
#CLIENTES=48
RSBYTES=223
SCRAMBLEBLOCK=$(echo "${RSBYTES}*4" | bc)
for CLIENTES in `seq 18 60`;
do
	echo "----------------------------------------"
	python -c "print 'Aprovechamiento: %f' % (((${CLIENTES}/128.0)/1)*(${RSBYTES}/(${RSBYTES} + 32.0)))"

	# filtro via files
	rm *.out
	./rsenc <test-30-50.txt  >rs.out
	./raw2bin < rs.out  >ldpc.out
	./bfenc ${CLIENTES} < ldpc.out | ./bfdec ${CLIENTES} >bf.out
	./bin2raw <bf.out >ldpcdec.out
	./rsdec <ldpcdec.out >rsdec.out

	echo "BER post-bloomfilter: " `./ber ldpc.out bf.out`
	echo "BER post-ldpc: " `./ber rs.out ldpcdec.out`
	echo "BER post-reed-solomon: " `./ber test-30-50.txt rsdec.out`

done
