CPPC=g++
CC=gcc

CFLAGSSERV=-lssh -lpthread -lcrypto 
CFILESSERV=serverSrc/server.cpp serverSrc/misc.cpp serverSrc/agents.cpp serverSrc/list.cpp serverSrc/authenticate.cpp serverSrc/b64.cpp serverSrc/log.cpp serverSrc/ssh_transport.cpp

CFLAGSCLI=-lssh -lcurl
CFILESCLI=clientSrc/client.c clientSrc/agent.c clientSrc/b64.c clientSrc/beacon.c

CFLAGSREL=-s
CFLAGSDBG=-ggdb -Wall

hellomake: 
	$(CPPC) -o serverSrc/server.out $(CFILESSERV) $(CFLAGSSERV) $(CFLAGSREL)
	$(CC) -o clientSrc/client.out $(CFILESCLI) $(CFLAGSCLI) $(CFLAGSREL)
	/bin/bash ./python/update_uis.sh

debug:
	$(CPPC) -o serverSrc/server.out $(CFILESSERV) $(CFLAGSSERV) $(CFLAGSDBG)
	$(CC) -o clientSrc/client.out $(CFILESCLI) $(CFLAGSCLI) $(CFLAGSDBG)
	/bin/bash ./python/update_uis.sh
