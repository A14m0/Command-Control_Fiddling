CC=gcc
CFLAGSSERV=-lssh -lpthread -lcrypto
CFLAGSCLI=-lssh -lcurl
CFLAGSDBG=-ggdb

hellomake: 
	$(CC) -o serverSrc/server.out serverSrc/server.c serverSrc/misc.c serverSrc/agents.c serverSrc/list.c $(CFLAGSSERV)
	$(CC) -o clientSrc/client.out clientSrc/client.c clientSrc/agent.c $(CFLAGSCLI)

debug:
	$(CC) -o serverSrc/server.out serverSrc/server.c serverSrc/misc.c serverSrc/agents.c serverSrc/list.c $(CFLAGSSERV) $(CFLAGSDBG)
	$(CC) -o clientSrc/client.out clientSrc/client.c clientSrc/agent.c $(CFLAGSCLI) $(CFLAGSDBG)
