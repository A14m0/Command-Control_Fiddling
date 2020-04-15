CPPC = g++
CC = gcc

ODIR = serverSrc/build

CFLAGSSERV = -lssh -lpthread -lcrypto -ldl
CFILESSERV = serverSrc/misc.cpp serverSrc/agents.cpp serverSrc/authenticate.cpp serverSrc/b64.cpp serverSrc/log.cpp serverSrc/connection.cpp serverSrc/server.cpp

CFLAGSCLI = -lssh -lcurl
CFILESCLI = clientSrc/client.c clientSrc/agent.c clientSrc/b64.c clientSrc/beacon.c clientSrc/shell.c

CFLAGSREL = -s
CFLAGSDBG = -ggdb -Wall

CFLAGSSO = -c -fpic -static -lssh -lcrypto
CFILESSO = serverSrc/ssh_transport/ssh_transport.cpp serverSrc/authenticate.cpp serverSrc/list.cpp serverSrc/log.cpp serverSrc/b64.cpp serverSrc/agents.cpp serverSrc/connection.cpp

OBJECT_FILES = $(CFILESSO:%.cpp=$(ODIR)/%.o)

build: $(OBJECT_FILES)
	$(CPPC) -o serverSrc/server.out $(CFILESSERV) $(CFLAGSSERV) $(CFLAGSREL)
	$(CC) -o clientSrc/client.out $(CFILESCLI) $(CFLAGSCLI) $(CFLAGSREL)

	$(CPPC) -shared -o serverSrc/shared/ssh_transport.so $(OBJECT_FILES)
	/bin/bash ./python/update_uis.sh

debug: $(OBJECT_FILES)
	$(CPPC) -o serverSrc/server.out $(CFILESSERV) $(CFLAGSSERV) $(CFLAGSDBG)
	$(CC) -o clientSrc/client.out $(CFILESCLI) $(CFLAGSCLI) $(CFLAGSDBG)
	$(CPPC) -shared -o serverSrc/shared/ssh_transport.so $(OBJECT_FILES)
	@/bin/bash ./python/update_uis.sh

clean: 
	rm -r $(ODIR)

.PHONY: build debug clean

$(OBJECT_FILES): $(ODIR)/%.o: %.cpp
	@echo "Compiling $<"
	@mkdir -p $(@D)
	@$(CPPC) $(CFLAGSSO) -o $@ $<