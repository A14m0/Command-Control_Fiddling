CPPC = g++
CC = gcc


EXECOUTDIR = out
ODIR = server/build

CFLAGSSERV = -lpthread -lcrypto -ldl -fpermissive
CFILESSERV = server/misc.cpp server/agents.cpp server/authenticate.cpp server/b64.cpp server/log.cpp server/connection.cpp server/server.cpp server/common.cpp server/server_module.cpp server/shell.cpp

CFLAGSCLI = -lssh -lcurl
CFILESCLI = agent/client.c agent/agent.c agent/b64.c agent/beacon.c agent/shell.c

CFLAGSREL = -s
CFLAGSDBG = -ggdb -Wall

CFLAGSSO = -c -fpic -static -lssh -lcrypto
CFILESSO = server/ssh_transport/ssh_transport.cpp server/b64.cpp server/authenticate.cpp server/misc.cpp server/agents.cpp

OBJECT_FILES = $(CFILESSO:%.cpp=$(ODIR)/%.o)

release: $(OBJECT_FILES)
ifeq (,$(wildcard out))
	@mkdir -p out/shared
endif
	$(CPPC) -o $(EXECOUTDIR)/server.out $(CFILESSERV) $(CFLAGSSERV) $(CFLAGSREL)
	$(CC) -o $(EXECOUTDIR)/client.out $(CFILESCLI) $(CFLAGSCLI) $(CFLAGSREL)

	$(CPPC) -shared -o $(EXECOUTDIR)/shared/ssh_transport.so $(OBJECT_FILES) -lcrypto -lssh
	/bin/bash ./python/update_uis.sh

server_release: $(OBJECT_FILES)
ifeq (,$(wildcard out))
	@mkdir -p out/shared
endif
	$(CPPC) -o $(EXECOUTDIR)/server.out $(CFILESSERV) $(CFLAGSSERV) $(CFLAGSREL)
	$(CPPC) -shared -o $(EXECOUTDIR)/shared/ssh_transport.so $(OBJECT_FILES) -lcrypto -lssh


debug: $(OBJECT_FILES)
ifeq (,$(wildcard out))
	@mkdir -p out/shared
endif
	$(CPPC) -o $(EXECOUTDIR)/server.out $(CFILESSERV) $(CFLAGSSERV) $(CFLAGSDBG)
	$(CC) -o $(EXECOUTDIR)/client.out $(CFILESCLI) $(CFLAGSCLI) $(CFLAGSDBG)
	$(CPPC) -shared -o $(EXECOUTDIR)/shared/ssh_transport.so $(OBJECT_FILES) -lcrypto -lssh
	@/bin/bash ./python/update_uis.sh

clean:
	rm -r $(ODIR)

.PHONY: debug release clean

$(OBJECT_FILES): $(ODIR)/%.o: %.cpp
	@echo "Compiling $<"
	@mkdir -p $(@D)
	@$(CPPC) $(CFLAGSSO) -o $@ $<
