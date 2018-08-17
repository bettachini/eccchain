all: testenc

clean:
	rm testenc testdec 

CFLAGS= -Wall -O3 -march=native
#CFLAGS= -Wall -O0 -g -march=native


testenc: testenc.c mtrand/mtrand.cpp
	g++ $(CFLAGS) -o testenc testenc.c mtrand/mtrand.cpp
	ln -s testenc testdec
