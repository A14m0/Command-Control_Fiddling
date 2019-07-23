CC=gcc
CFLAGSSERV=-lssh -lpthread
CFLAGSCLI=-lssh

hellomake: clientSrc/client.o serverSrc/server.o
	$(CC) -o serverSrc/server serverSrc/server.o $(CFLAGSSERV)
	$(CC) -o clientSrc/client clientSrc/client.o $(CFLAGSCLI)
