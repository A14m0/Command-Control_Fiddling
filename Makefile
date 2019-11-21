CC=gcc

CFLAGSSERV=-lssh -lpthread -lcrypto
CFILESSERV=serverSrc/server.c serverSrc/misc.c serverSrc/agents.c serverSrc/list.c serverSrc/authenticate.c serverSrc/b64.c

CFLAGSCLI=-lssh -lcurl
CFILESCLI=clientSrc/client.c clientSrc/agent.c clientSrc/b64.c clientSrc/beacon.c
CFLAGSDBG=-ggdb -Wall

hellomake: 
	$(CC) -o serverSrc/server.out $(CFILESSERV) $(CFLAGSSERV)
	$(CC) -o clientSrc/client.out $(CFILESCLI) $(CFLAGSCLI)
	/bin/bash ./python/update_uis.sh

debug:
	$(CC) -o serverSrc/server.out $(CFILESSERV) $(CFLAGSSERV) $(CFLAGSDBG)
	$(CC) -o clientSrc/client.out $(CFILESCLI) $(CFLAGSCLI) $(CFLAGSDBG)
	/bin/bash ./python/update_uis.sh
