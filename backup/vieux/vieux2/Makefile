all: genMatrix rsenc bfenc raw2bin ldpcenc scrambler ber noisesim hammingenc PRNGenc

clean:
	rm rsenc rsdec bfenc bfdec raw2bin bin2raw ldpcenc ldpcdec genMatrix scrambler descramble ber noisesim hammingenc hammingdec PRNGenc PRNGdec

#CFLAGS= -Wall -O3 -march=native
CFLAGS= -Wall -O0 -g -march=native

genMatrix: genMatrix.cpp
	g++ $(CFLAGS) -o genMatrix genMatrix.cpp

rsenc: rsenc.c reedsolomon.c
	gcc $(CFLAGS) -o rsenc rsenc.c reedsolomon.c
	ln -s rsenc rsdec

bfenc: bfenc.c 
	g++ $(CFLAGS) -o bfenc bfenc.c
	ln -s bfenc bfdec

PRNGenc: PRNGenc.c 
	g++ $(CFLAGS) -o PRNGenc PRNGenc.c
	ln -s PRNGenc PRNGdec

raw2bin: raw2bin.c
	gcc $(CFLAGS) -o raw2bin raw2bin.c
	ln -s raw2bin bin2raw

ldpcenc: ldpcenc.cpp
	g++ $(CFLAGS) -o ldpcenc ldpcenc.cpp
	ln -s ldpcenc ldpcdec

scrambler: scrambler.c
	gcc $(CFLAGS) -o scrambler scrambler.c
	ln -s scrambler descramble

ber: ber.c
	gcc $(CFLAGS) -o ber ber.c

noisesim: noisesim.c GenRuidoRapido.c dran1.c
	gcc $(CFLAGS) -lm -o noisesim noisesim.c GenRuidoRapido.c dran1.c

hammingenc: hammingenc.c
	g++ $(CFLAGS) -o hammingenc hammingenc.c 
	ln -s hammingenc hammingdec

