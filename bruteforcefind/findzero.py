#!/usr/bin/python
import os,sys
# Usage
if len(sys.argv)<4:
	print "Find the first matrix that give zero errors"
	print "Usage: %s <Width> <Height> <ones>" % (sys.argv[0])
	exit(0)

bestError=99999
MatrixWidth=int(sys.argv[1])
MatrixHeight=int(sys.argv[2])
MatrixOnes=int(sys.argv[3])
matrixFile="brute-%d-%d-%d-test.txt" % (MatrixWidth,MatrixHeight,MatrixOnes)
errorsFile="brute-errors.txt"
while(bestError>1):
	print "Triying matrix %d-%d-%d" %  (MatrixWidth,MatrixHeight,MatrixOnes)
	genCommand="./genMatrixHG %d %d %d > %s" % (MatrixWidth,MatrixHeight,MatrixOnes,matrixFile)
	testCommand="./ldpcencAccel %s 2> %s" % (matrixFile,errorsFile)
	os.system(genCommand);
	os.system(testCommand);
	a=open(errorsFile,"rb")
	b=a.read()
	a.close()
	error=int(b)
	MatrixWidth+=512
	if (error<bestError):
		bestError=error;
		print "******* Best: %d" % error
print "Zero error found at matrix %d-%d-%d" %  (MatrixWidth,MatrixHeight,MatrixOnes)

