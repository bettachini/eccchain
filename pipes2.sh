#!/bin/sh
# Esta version no utiliza LDPC
RSBYTES=223
SCRAMBLEBLOCK=$(echo "${RSBYTES}*4" | bc)
for CLIENTES in `seq 40 1 200 `;
do
	echo "----------------------------------------"
	python -c "print 'Aprovechamiento: %f' % (((${CLIENTES}/256.0)/1)*(${RSBYTES}/(${RSBYTES} + 32.0)))"

	# filtro via files
	#rm *.out
	./scrambler 8192 0 < test.txt >scrambler1.out
	./rsenc ${RSBYTES} <scrambler1.out  >rs.out
	./raw2bin < rs.out  >ldpc.out
	./bfenc ${CLIENTES} < ldpc.out | ./bfdec ${CLIENTES} >bf.out
	./bin2raw <bf.out >ldpcdec.out
	./rsdec ${RSBYTES} <ldpcdec.out >rsdec.out
	./descramble 8192 0 <rsdec.out >descramble1.out

	echo "BER post-bloomfilter: " `./ber ldpc.out bf.out`
	echo "BER post-reed-solomon: " `./ber test.txt descramble1.out`

done
