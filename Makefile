CC=gcc
CFLAGSSERV=-lssh -lpthread
CFLAGSCLI=-lssh -lcurl

hellomake: 
	$(CC) -o serverSrc/server.out serverSrc/server.c serverSrc/misc.c serverSrc/agents.c $(CFLAGSSERV)
	$(CC) -o clientSrc/client.out clientSrc/client.c clientSrc/agent.c $(CFLAGSCLI)
