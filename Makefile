all:pge_LServer pge_LClient
	rm pge_LServer.o
	rm pge_LClient.o
	rm DieWithMessage.o
	rm AddressUtility.o
	
pge_LServer: pge_LServer.o  AddressUtility.o DieWithMessage.o
	cc -o pge_LServer pge_LServer.o  AddressUtility.o DieWithMessage.o

pge_LClient:pge_LClient.o DieWithMessage.o
	cc -o pge_LClient pge_LClient.o DieWithMessage.o

pge_LServer.o: pge_LServer.c Practical.h
	cc -c pge_LServer.c

pge_LClient.o: pge_LClient.c Practical.h
	cc -c pge_LClient.c 
	
DieWithMessage.o: DieWithMessage.c
	cc -c DieWithMessage.c 	
	
AddressUtility.o: AddressUtility.c
	cc -c AddressUtility.c	
