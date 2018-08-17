#!/bin/sh
# Esta version no utiliza LDPC
SCRAMBLEBLOCK=$(echo "${RSBYTES}*4" | bc)
for CLIENTES in `seq 0.001 0.001 1 `;
do
	echo "----------------------------------------"
	# filtro via files
	#rm *.out
	./raw2bin < test.txt >testbin.out

	./scrambler 256 0 < testbin.out >scrambler1.out
	./hamenc hamming5.txt S< scrambler1.out > hamming1.out
	 
	./bin2bin ${CLIENTES} < hamming1.out >bf.out

	./hamdec hamming5.txt S<bf.out >hammingdec1.out
	./descramble 256 0 <hammingdec1.out >descramble1.out

	./bin2raw <descramble1.out >testraw.out
	#sleep 100000
	echo "\nBER post-introduccion-error: " `./ber hamming1.out bf.out`
	echo "BER post-hamming3: " `./ber scrambler1.out hammingdec1.out`

done
