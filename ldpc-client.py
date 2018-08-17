# Ldpc client program
# Este programa se encarga de recibir comandos del servidor que consisten en ejecutar las tareas dictadas por el mismo y devolver la salida
import socket,os,struct

#-------- configuracion ---------------
HOST = 'alerce.itba.edu.ar'    # The remote host
PORT = 50008              # The same port as used by the server
#--------------------------------------



print "Connecting to %s:%d" % (HOST,PORT)
s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
s.connect((HOST, PORT))
while(True):
	data = s.recv(1)
	if len(data)==0: # no more data?
		exit(0)
	print 'Received command %s' % (data)
	if (data=='Q'): # Q= comando de salir
		exit(0)
	if (data=='E'): # E= comando de eco
		s.send('E')
	if (data=='C'): # C= comando de empezar tarea
		clientsnumber = ord(s.recv(1))
		command = './pipes-clientserver.sh %d > salidaclient%d.txt' % (clientsnumber,clientsnumber)
		print "Executing command '%s'" % command
		os.system(command)
	if (data=='R'): # R= comando de devolver datos
		clientsnumber = ord(s.recv(1))
		outfile= 'salidaclient%d.txt' % clientsnumber
		print "Sending file '%s' to server" % outfile
		try:
			fil=open(outfile,'rb')
			data=fil.read()
			fil.close()
			os.unlink(outfile)
		except: data=''
		s.send(struct.pack("<L",len(data)))
		s.sendall(data)
s.close()


