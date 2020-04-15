CPPC=g++
CC=gcc

CFLAGSSERV=-lssh -lpthread -lcrypto -ldl
CFILESSERV= serverSrc/misc.cpp serverSrc/agents.cpp serverSrc/authenticate.cpp serverSrc/b64.cpp serverSrc/log.cpp serverSrc/connection.cpp serverSrc/server.cpp

CFLAGSCLI=-lssh -lcurl
CFILESCLI=clientSrc/client.c clientSrc/agent.c clientSrc/b64.c clientSrc/beacon.c clientSrc/shell.c

CFLAGSREL=-s
CFLAGSDBG=-ggdb -Wall

hellomake: 
	$(CPPC) -o serverSrc/server.out $(CFILESSERV) $(CFLAGSSERV) $(CFLAGSREL)
	$(CC) -o clientSrc/client.out $(CFILESCLI) $(CFLAGSCLI) $(CFLAGSREL)
	$(CPPC) -o serverSrc/ssh_transport/ssh_transport.o -c -fpic serverSrc/ssh_transport/ssh_transport.cpp
	$(CPPC) -shared -o serverSrc/shared/ssh_transport.so serverSrc/ssh_transport/ssh_transport.o
	/bin/bash ./python/update_uis.sh

debug:
	$(CPPC) -o serverSrc/server.out $(CFILESSERV) $(CFLAGSSERV) $(CFLAGSDBG)
	$(CC) -o clientSrc/client.out $(CFILESCLI) $(CFLAGSCLI) $(CFLAGSDBG)
	$(CPPC) -o serverSrc/ssh_transport/ssh_transport.o -c -fpic serverSrc/ssh_transport/ssh_transport.cpp
	$(CPPC) -shared -o serverSrc/shared/ssh_transport.so serverSrc/ssh_transport/ssh_transport.o
	/bin/bash ./python/update_uis.sh
