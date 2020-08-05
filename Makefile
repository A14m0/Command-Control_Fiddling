CPPC = g++
CC = gcc


EXECOUTDIR = out
ODIR = serverSrc/build

CFLAGSSERV = -lssh -lpthread -lcrypto -ldl -fpermissive
CFILESSERV = serverSrc/misc.cpp serverSrc/agents.cpp serverSrc/authenticate.cpp serverSrc/b64.cpp serverSrc/log.cpp serverSrc/connection.cpp serverSrc/server.cpp serverSrc/common.cpp serverSrc/server_module.cpp

CFLAGSCLI = -lssh -lcurl
CFILESCLI = clientSrc/client.c clientSrc/agent.c clientSrc/b64.c clientSrc/beacon.c clientSrc/shell.c

CFLAGSREL = -s
CFLAGSDBG = -ggdb -Wall

CFLAGSSO = -c -fpic -static -lssh -lcrypto
CFILESSO = serverSrc/ssh_transport/ssh_transport.cpp serverSrc/b64.cpp serverSrc/authenticate.cpp serverSrc/misc.cpp serverSrc/agents.cpp

OBJECT_FILES = $(CFILESSO:%.cpp=$(ODIR)/%.o)

release: $(OBJECT_FILES)
ifeq (,$(wildcard out))
	@mkdir out
endif
	$(CPPC) -o $(EXECOUTDIR)/server.out $(CFILESSERV) $(CFLAGSSERV) $(CFLAGSREL)
	$(CC) -o $(EXECOUTDIR)/client.out $(CFILESCLI) $(CFLAGSCLI) $(CFLAGSREL)

	$(CPPC) -shared -o $(EXECOUTDIR)/shared/ssh_transport.so $(OBJECT_FILES)
	/bin/bash ./python/update_uis.sh

server_release: $(OBJECT_FILES)
ifeq (,$(wildcard out))
	@mkdir out
endif
	$(CPPC) -o $(EXECOUTDIR)/server.out $(CFILESSERV) $(CFLAGSSERV) $(CFLAGSREL)
	$(CPPC) -shared -o $(EXECOUTDIR)/shared/ssh_transport.so $(OBJECT_FILES)


debug: $(OBJECT_FILES)
ifeq (,$(wildcard out))
	@mkdir out
endif
	$(CPPC) -o $(EXECOUTDIR)/server.out $(CFILESSERV) $(CFLAGSSERV) $(CFLAGSDBG)
	$(CC) -o $(EXECOUTDIR)/client.out $(CFILESCLI) $(CFLAGSCLI) $(CFLAGSDBG)
	$(CPPC) -shared -o $(EXECOUTDIR)/shared/ssh_transport.so $(OBJECT_FILES)
	@/bin/bash ./python/update_uis.sh

clean:
	rm -r $(ODIR)

.PHONY: debug release clean

$(OBJECT_FILES): $(ODIR)/%.o: %.cpp
	@echo "Compiling $<"
	@mkdir -p $(@D)
	@$(CPPC) $(CFLAGSSO) -o $@ $<
