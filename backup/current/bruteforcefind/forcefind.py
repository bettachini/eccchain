#!/usr/bin/python
import os,sys
# Usage
if len(sys.argv)<4:
	print "Usage: %s <Width> <Height> <ones>" % (sys.argv[0])
	exit(0)

bestError=99999
MatrixWidth=int(sys.argv[1])
MatrixHeight=int(sys.argv[2])
MatrixOnes=int(sys.argv[3])
matrixFile="brute-%d-%d-%d-test.txt" % (MatrixWidth,MatrixHeight,MatrixOnes)
errorsFile="brute-errors.txt"
genCommand="./genMatrixHG %d %d %d > %s" % (MatrixWidth,MatrixHeight,MatrixOnes,matrixFile)
testCommand="./ldpcencAccel %s 2> %s" % (matrixFile,errorsFile)
while(1):
	os.system(genCommand);
	os.system(testCommand);
	a=open(errorsFile,"rb")
	b=a.read()
	a.close()
	error=int(b)
	if (error<bestError):
		bestError=error;
		os.system("cp %s brute-best-%d-%d-%d-test.txt" % (matrixFile,MatrixWidth,MatrixHeight,MatrixOnes))
		print "******* Best: %d" % error

