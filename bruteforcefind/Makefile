all: ldpcenc genMatrixHG genMatrixGH ldpcencAccel modifMatrix

clean:
	rm ldpcenc genMatrixHG genMatrixGH ldpcencAccel modifMatrix

CFLAGS= -Wall -O3 -march=native
#CFLAGS= -Wall -O0  -g 

ldpcenc: ldpcenc.cpp
	g++ $(CFLAGS) -o ldpcenc ldpcenc.cpp

ldpcencAccel: ldpcencAccel.cpp
	g++ $(CFLAGS) -o ldpcencAccel ldpcencAccel.cpp

genMatrixHG: genMatrixHG.cpp  
	g++ $(CFLAGS) -o genMatrixHG genMatrixHG.cpp

genMatrixGH: genMatrixGH.cpp  
	g++ $(CFLAGS) -o genMatrixGH genMatrixGH.cpp

modifMatrix: modifMatrix.cpp  
	g++ $(CFLAGS) -o modifMatrix modifMatrix.cpp

