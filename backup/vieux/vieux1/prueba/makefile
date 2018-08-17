prueba : prueba.o nrutil.o
	gcc prueba.o nrutil.o -Wall -lm -o prueba

prueba.o : prueba.c estructurasVAB.h
	gcc -Wall -c prueba.c

nrutil.o: nrutil.c nrutil.h nr.h
	gcc -Wall   -c nrutil.c

dran1.o: dran1.c 
	gcc -Wall   -c dran1.c
