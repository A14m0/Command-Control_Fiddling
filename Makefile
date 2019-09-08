CC=gcc
CFLAGSSERV=-lssh -lpthread
CFLAGSCLI=-lssh

hellomake: 
	$(CC) -o serverSrc/server serverSrc/server.c serverSrc/misc.c serverSrc/agents.c $(CFLAGSSERV)
	$(CC) -o clientSrc/client clientSrc/client.c clientSrc/agent.c $(CFLAGSCLI)
