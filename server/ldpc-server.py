# LDPC simulation tcp server

# Crea una lista de trabajos y los envia a los posibles clientes
# Utilizando un protocolo simple
# Usar en conjunto con ldpc-client.py

import SocketServer,struct,time

#--------- configuracion ----------------------

# rango de clientes
clientsMin=10
clientsMax=20
# repeticiones
clientsRep=2
#server host is a tuple ('host', port)
listenon=('',50008)

#-------------------------------------------------
joblist=[]

#inicializamos la lista de trabajos que necesitamos
for i in range(clientsMin,clientsMax): # rango de clientes a probar
	for q in range(clientsRep): # Repeticiones para aumentar los bits a probar
		joblist.append(i)


class LdpcRequestHandler(SocketServer.BaseRequestHandler ):

    def ConnectionOk(self):
	self.request.send('E')
	resp=self.request.recv(1)
	if resp=='E':
		return 1
	return 0

    def setup(self):
        print self.client_address, 'connected!'
	if self.ConnectionOk():
		print 'Connection Ok.'

    def handle(self):
	while len(joblist)>0:
		clients=joblist.pop() # pop es thread-safe en teoria
		print time.ctime()
		print "Sending job %d" % clients
		# orden de iniciar tarea
		self.request.send('C')
		self.request.send(chr(clients))
		# orden de devolver datos
		self.request.send('R')
		self.request.send(chr(clients))
		datalen=struct.unpack('<L',self.request.recv(4))[0]
		if datalen==0: # si paso algun error, reintroducir trabajo y mandarlo a otro cliente
			joblist.append(clients)
		data=self.request.recv(datalen)
		#guarda archivo
		filecounter=len(joblist)
		filename='salidacliente%d-%d.txt' % (clients,filecounter)
		print "writing file %s" % filename
		try:
			fil=open(filename,'wb')
			fil.write(data)
			fil.close()
		except: print "Can't write file!"
	self.request.send('Q') # quit
		
    def server_bind(self):
	server.sock.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
		
    def finish(self):
        print self.client_address, 'disconnected!'

print "Listening on (server,port): ",listenon
server = SocketServer.ThreadingTCPServer(listenon, LdpcRequestHandler)
server.serve_forever()

