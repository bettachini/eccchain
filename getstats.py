import os

os.system("ls -1 sali*>files.txt")
a=open("files.txt","rb")
aprov = [0] * 200
error = [0] * 200
noerror = [0] * 200
numsampl = [0] * 200
for line in a.readlines():
	line=line[:-1]
	print "processing %s" % line
	pos=line.find("-")
	client = int(line[13:pos])
	b=open(line,"rb").readlines()
	try:
		aprov[client]=float(b[2][17:])
		est= b[6].split()[2][1:-1]
		err=int(est[:est.find("/")])
		noerr=int(est[est.find("/")+1:])
		error[client]+=err
		noerror[client]+=noerr
		numsampl[client]+=1
	except: pass
for i in range(200):
	if numsampl[i]>0:
	#	print "%i %i %i" % (i,float(error[i]),float(noerror[i]))
		print "%i %.15f %.7f" % (i,float(error[i])/float(134213888),aprov[i])
	#	print "%i %.15f %.7f" % (i,float(error[i])/float(noerror[i]),aprov[i])
os.unlink("files.txt")
