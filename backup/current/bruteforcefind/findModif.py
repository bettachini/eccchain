#!/usr/bin/python
import os,sys
# Usage
if len(sys.argv)<3:
	print "Generate and test matrix modifications"
	print "Usage: %s <matrix> <rows>" % (sys.argv[0])
	exit(0)

bestError=99999
# Setup filenames
OrigMatrixFile=sys.argv[1]
MatrixFile="modif-%s" % OrigMatrixFile
TestMatrixFile="modif-test-%s" % OrigMatrixFile
os.system("cp %s %s" % (OrigMatrixFile,MatrixFile))
os.system("cp %s %s" % (OrigMatrixFile,TestMatrixFile))
errorsFile="brute-errors.txt"
cont=0
while(bestError>1):
	print "Triying matrix modification"
	# Generate matrix modification and test it
	genCommand="./modifMatrix %s %s > %s" % (MatrixFile,sys.argv[2],TestMatrixFile)
	testCommand="./ldpcencAccel %s 2> %s" % (TestMatrixFile,errorsFile)
	if (cont>0): os.system(genCommand)
	cont+=1
	os.system(testCommand)
	a=open(errorsFile,"rb")
	b=a.read()
	a.close()
	error=int(b)
	if (error<bestError):
		# The modification is better than the original
		bestError=error;
		print "******* Best: %d" % error
		# Make a copy
		os.system("cp %s best-%s" % (TestMatrixFile,MatrixFile)) 
		# Now this is our base matrix
		os.system("cp %s %s" % (TestMatrixFile,MatrixFile))
print "Zero error found at matrix %d-%d-%d" %  (MatrixWidth,MatrixHeight,MatrixOnes)

