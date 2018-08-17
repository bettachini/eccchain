#!/bin/sh

for CLIENTES in `seq 50 1 150 `;
do
	./pipes-clientserver.sh ${CLIENTES} > /tmp/salidacliente${CLIENTES}-1.txt
done
