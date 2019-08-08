/* This is a sample implementation of a libssh based SSH server */
/*
Copyright 2003-2009 Aris Adamantiadis
This file is part of the SSH Library
You are free to copy this file, modify it in any way, consider it being public
domain. This does not apply to the rest of the library though, but it is
allowed to cut-and-paste working code from this file to any license of
program.
The goal is to show the API in action. It's not a reference on how terminal
clients must be made or how a client should react.




https://github.com/substack/libssh
https://www.libssh.org/
http://api.libssh.org/stable/libssh_tutorial.html

*/

#include "config.h"
#include "execs.h"

#include <libssh/libssh.h>
#include <libssh/server.h>

#include <pthread.h>

#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>

#include <sys/stat.h>
#include <time.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>


#ifdef HAVE_ARGP_H
#include <argp.h>
#endif
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#ifndef KEYS_FOLDER
#ifdef _WIN32
#define KEYS_FOLDER
#else
#define KEYS_FOLDER "/etc/ssh/"
#endif
#endif

#ifdef WITH_PCAP
const char *pcap_file="debug.server.pcap";
ssh_pcap_file pcap;

void set_pcap(ssh_session session);
void set_pcap(ssh_session session){
	if(!pcap_file)
		return;
	pcap=ssh_pcap_file_new();
	if(ssh_pcap_file_open(pcap,pcap_file) == SSH_ERROR){
		printf("Error opening pcap file\n");
		ssh_pcap_file_free(pcap);
		pcap=NULL;
		return;
	}
	ssh_set_pcap_file(session,pcap);
}

void cleanup_pcap(void);
void cleanup_pcap(){
	ssh_pcap_file_free(pcap);
	pcap=NULL;
}
#endif

ssh_session session_array[MAX_CONN];
pthread_t thread_array[MAX_CONN];
pthread_mutex_t session_lock;



struct clientDat{
    int id;
    ssh_session session;
    int trans_id;
    ssh_channel chan;
    int type;
};



static int auth_password(const char *user, const char *password){
    if(strcmp(user,"aris"))
        return 0;
    if(strcmp(password,"lala"))
        return 0;
    return 1; // authenticated
}
#ifdef HAVE_ARGP_H
const char *argp_program_version = "libssh server example "
  SSH_STRINGIFY(LIBSSH_VERSION);
const char *argp_program_bug_address = "<libssh@libssh.org>";

/* Program documentation. */
static char doc[] = "libssh -- a Secure Shell protocol implementation";

/* A description of the arguments we accept. */
static char args_doc[] = "BINDADDR";

/* The options we understand. */
static struct argp_option options[] = {
  {
    .name  = "port",
    .key   = 'p',
    .arg   = "PORT",
    .flags = 0,
    .doc   = "Set the port to bind.",
    .group = 0
  },
  {
    .name  = "hostkey",
    .key   = 'k',
    .arg   = "FILE",
    .flags = 0,
    .doc   = "Set the host key.",
    .group = 0
  },
  {
    .name  = "dsakey",
    .key   = 'd',
    .arg   = "FILE",
    .flags = 0,
    .doc   = "Set the dsa key.",
    .group = 0
  },
  {
    .name  = "rsakey",
    .key   = 'r',
    .arg   = "FILE",
    .flags = 0,
    .doc   = "Set the rsa key.",
    .group = 0
  },
  {
    .name  = "verbose",
    .key   = 'v',
    .arg   = NULL,
    .flags = 0,
    .doc   = "Get verbose output.",
    .group = 0
  },
  {NULL, 0, 0, 0, NULL, 0}
};

/* Parse a single option. */
static error_t parse_opt (int key, char *arg, struct argp_state *state) {
  /* Get the input argument from argp_parse, which we
   * know is a pointer to our arguments structure.
   */
  ssh_bind sshbind = state->input;

  switch (key) {
    case 'p':
      ssh_bind_options_set(sshbind, SSH_BIND_OPTIONS_BINDPORT_STR, arg);
      break;
    case 'd':
      ssh_bind_options_set(sshbind, SSH_BIND_OPTIONS_DSAKEY, arg);
      break;
    case 'k':
      ssh_bind_options_set(sshbind, SSH_BIND_OPTIONS_HOSTKEY, arg);
      break;
    case 'r':
      ssh_bind_options_set(sshbind, SSH_BIND_OPTIONS_RSAKEY, arg);
      break;
    case 'v':
      ssh_bind_options_set(sshbind, SSH_BIND_OPTIONS_LOG_VERBOSITY_STR, "3");
      break;
    case ARGP_KEY_ARG:
      if (state->arg_num >= 1) {
        /* Too many arguments. */
        argp_usage (state);
      }
      ssh_bind_options_set(sshbind, SSH_BIND_OPTIONS_BINDADDR, arg);
      break;
    case ARGP_KEY_END:
      if (state->arg_num < 1) {
        /* Not enough arguments. */
        argp_usage (state);
      }
      break;
    default:
      return ARGP_ERR_UNKNOWN;
  }

  return 0;
}

/* Our argp parser. */
static struct argp argp = {options, parse_opt, args_doc, doc, NULL, NULL, NULL};
#endif /* HAVE_ARGP_H */

int index_of(char* str, char find){
    int i = 0;

    while (str[i] != '\0')
    {
        if (str[i] == find)
        {
            return i;
        }

        i++;
    }

    return -1;

}

int parse_command(char* comm, struct clientDat clistruct){
    char main[16];
    char params[4096];
    int rc = -1;

    rc = index_of(comm, ' ');

    if (rc<0)
    {
        printf("Illegal command passsed to server\n");
        return -1;
    }

    strncpy(main, comm, rc);

    // in case the command string is larger than the receiving buffer
    if (strlen(comm) > 4096)
    {
/*MAYBE AT SOME POINT REPLACE THIS WITH IT ALL BEING LOADED INTO MEMORY OR SOMETHING???*/
        strncat(params, comm+rc+1, 4096-rc+1);
    } else
    {
        strncat(params, comm+rc+1, strlen(comm) - rc+1);
    }
    
    
    if (clistruct.type == AGENT_TYPE)
    {
        // Agent commands
        if (strcmp(main, "download")) {
            // Downloads a file from the client
            int size = 0;
            int recv = 0;
            struct stat st = {0};
            
            
            // gets file size from client
            ssh_channel_read(clistruct.chan, &size, 2056, 0);

            // allocates memory for file contents
            unsigned char* data = malloc(size); 
            ssh_channel_write(clistruct.chan, "ok", 3);
            int nbytes = 0;

            // read the data
            while (nbytes < size)
            {
                recv = ssh_channel_read(clistruct.chan, data, sizeof(data), 0);
                if (recv == SSH_ERROR)
                {
                    printf("Failed to read data from channel\n");
                    return -1;
                }
                nbytes += recv;
                
            }

            if (stat("agent_files", &st) == -1)
            {
                mkdir("agent_files", 0666);
            } 

            char formatstr[FILENAME_MAX] = "client%d_%d.data";
            char filename[FILENAME_MAX];
            
            sprintf(filename, formatstr, clistruct.id, clistruct.trans_id);
            

            FILE* fd = fopen(filename, "wb");

            fprintf(fd, data);
            fclose(fd);            
            

            // send ok 
            ssh_channel_write(clistruct.chan, "ok", 3);
            

            return AGENT_DOWN_FILE;
        } else if (strcmp(main, "shell")) {
            return AGENT_REV_SHELL;
        } else if (strcmp(main, "exec")) {
            return AGENT_EXEC_SC;
        } else if (strcmp(main, "upload")) {
            return AGENT_UP_FILE;
        } else if (strcmp(main, "end"))
        {
            return END_CONN;
        }
        
    } else if (clistruct.type == MANAG_TYPE) {
        printf("TYPE = MANAGER\n");        
    }


    return -1;
}


void *ssh_handler(void* sess){
    struct clientDat pass = *(struct clientDat*) sess;

    ssh_session session = pass.session;
    int id = pass.id;
    ssh_channel chan;
    ssh_message message;
    int sftp = 0;
    int i = 0;
    int msgType = REQ_NONE;
    char buf[4096];
    srand(time(NULL));
    pass.trans_id = rand();
    pass.chan = chan;

    do {
		//printf("entered message loop\n");
        message=ssh_message_get(session);
        if(message){
            switch(ssh_message_type(message)){
                case SSH_REQUEST_CHANNEL_OPEN:
					printf("Client %d: Got request for opening channel\n", id); 
                    if(ssh_message_subtype(message)==SSH_CHANNEL_SESSION){
                        chan=ssh_message_channel_request_open_reply_accept(message);
                        break;
                    }
                default:
                ssh_message_reply_default(message);
            }
            ssh_message_free(message);
        }
    } while(message && !chan);
    
	if(!chan){
        printf("Client %d: Channel error : %s\n", id, ssh_get_error(session));
        ssh_finalize();
        return NULL;
    }
    
	do {
		//printf("Entered second message loop\n");
        message=ssh_message_get(session);
        if(message && ssh_message_type(message)==SSH_REQUEST_CHANNEL &&
           ssh_message_subtype(message)==SSH_CHANNEL_REQUEST_SHELL){
//            if(!strcmp(ssh_message_channel_request_subsystem(message),"sftp")){
				printf("Client %d: Got shell request\n", id);
                sftp=1;
                msgType = REQ_SHELL;
                ssh_message_channel_request_reply_success(message);
                break;
 //           }
        }

		if (message && ssh_message_type(message) == SSH_REQUEST_CHANNEL && ssh_message_subtype(message) == SSH_CHANNEL_REQUEST_EXEC){
			printf("Client %d: Got request for execution\n", id);
            ssh_message_channel_request_reply_success(message);
            sftp=1;
            msgType = REQ_EXEC;
			break;
		}
		

        if(!sftp){
            ssh_message_reply_default(message);
        }
        ssh_message_free(message);
    } while (message && !sftp);
    
	if(!sftp){
		printf("Client %d: SFTP error : %s\n", id, ssh_get_error(session));
        return NULL;
    }
    //printf("it works !\n");
    
    switch (msgType)
    {
    case REQ_EXEC:
        printf("Client %d: waiting for command\n", id);
        const char* test = ssh_message_channel_request_command(message);
        printf("Client %d: got: %s\n", id, test);

        ssh_channel_write(chan, test, strlen(test));
        printf("Client %d: wrote data to channel\n", id);
        break;
    case REQ_SHELL:
        do{
		    i=ssh_channel_read(chan,buf, sizeof(buf), 0);
		    if (!strncmp(buf, "exit", 4)) {
			    printf("Caught exit from client. Doing so...\n");
                pthread_mutex_lock(&session_lock);
                session_array[id] = NULL;
                pthread_mutex_unlock(&session_lock);
                ssh_message_free(message);
			    return NULL;
		    }
		
            if(i>0) {
                ssh_channel_write(chan, buf, i);
			    if (write(1,buf,i) < 0) {
                    printf("error writing to buffer\n");
                    return NULL;
                }
            }
        } while (i>0);
        break;
    
    default:
        printf("Client %d: got unknown message type: %d\n", id, msgType);
        break;
    }

    printf("Client %d: closing channels...\n", id);
    ssh_message_free(message);
    ssh_disconnect(session);

	
    return NULL;
}

int get_free(){
    pthread_mutex_lock(&session_lock);
    for (size_t i = 0; i < MAX_CONN; i++)
    {
        if(session_array[i] == NULL){
            pthread_mutex_unlock(&session_lock);
            return i;
        }
    }
    pthread_mutex_unlock(&session_lock);
    return -1;
}


int main(int argc, char **argv){
    
    ssh_session session;
    ssh_bind sshbind;
    ssh_message message;
    ssh_channel chan=0;
    char buf[2048];
    int auth=0;
    int sftp=0;
    int i;
    int r;
    int opt = 1;
    int quitting = 0;
	int port=1337;
    int msgType = REQ_EXEC;
    int master_socket;
    int ctr = 0;
    pthread_t thread;
    
    for (size_t i = 0; i < MAX_CONN; i++)
    {
        session_array[i] = NULL;
    }
    

    struct sockaddr_in address;

    if( (master_socket = socket(AF_INET , SOCK_STREAM , 0)) == 0){   
        perror("socket failed");   
        exit(EXIT_FAILURE);   
    }

    if(setsockopt(master_socket, SOL_SOCKET, SO_REUSEADDR, (char *)&opt, sizeof(opt)) < 0 ){   
        perror("setsockopt");   
        exit(EXIT_FAILURE);   
    }



    sshbind=ssh_bind_new();
    session=ssh_new();

    ssh_options_set(session, SSH_OPTIONS_FD, &master_socket);
	
    ssh_bind_options_set(sshbind, SSH_BIND_OPTIONS_DSAKEY, KEYS_FOLDER "ssh_host_dsa_key");
    ssh_bind_options_set(sshbind, SSH_BIND_OPTIONS_RSAKEY, KEYS_FOLDER "ssh_host_rsa_key");
    //ssh_bind_options_set(sshbind, SSH_BIND_OPTIONS_BINDPORT, &port);
	printf("Initialized Variables...\n");
#ifdef HAVE_ARGP_H
    /*
     * Parse our arguments; every option seen by parse_opt will
     * be reflected in arguments.
     */
    argp_parse (&argp, argc, argv, 0, 0, sshbind);
#else
    (void) argc;
    (void) argv;
#endif
#ifdef WITH_PCAP
    set_pcap(session);
#endif

    // bind the listener to the port
    if(ssh_bind_listen(sshbind)<0){
        printf("Error listening to socket: %s\n", ssh_get_error(sshbind));
        return 1;
    }
    printf("Server: Bound to listening port\n");
    
    // accept connections
    while (!quitting){
        ctr = get_free();
        if (ctr == -1) {
            printf("Server: Hit max concurrent connections. Waiting for free sessions...\n");
            sleep(10);
            ctr = get_free();
        
        } else {
            printf("Server: Found free session at index %d\n", ctr);
            session=ssh_new();

            r=ssh_bind_accept(sshbind,session);
	        printf("Server: Accepting connection\n");
            if(r==SSH_ERROR){
      	        printf("error accepting a connection : %s\n",ssh_get_error(sshbind));
      	        return 1;
            }
            if (ssh_handle_key_exchange(session)) {
                printf("ssh_handle_key_exchange: %s\n", ssh_get_error(session));
                return 1;
            }

            auth = 0;
            do {
                message=ssh_message_get(session);
                if(!message)
                    break;
                switch(ssh_message_type(message)){
                    case SSH_REQUEST_AUTH:
                        switch(ssh_message_subtype(message)){
                            case SSH_AUTH_METHOD_PASSWORD:
                                printf("Server: User %s wants to auth with pass %s\n", ssh_message_auth_user(message), ssh_message_auth_password(message));
                                if(auth_password(ssh_message_auth_user(message), ssh_message_auth_password(message))){
                                    auth=1;
                                    ssh_message_auth_reply_success(message,0);
                                    break;
                           	    }
                            // not authenticated, send default message
                            case SSH_AUTH_METHOD_NONE:
                            default:
                                ssh_message_auth_set_methods(message,SSH_AUTH_METHOD_PASSWORD);
                                ssh_message_reply_default(message);
                                break;
                        }
                        break;
                    default:
                        ssh_message_reply_default(message);
                }
                ssh_message_free(message);
            } while (!auth);
    
            // Check if the client authenticated successfully
	        if(!auth){
                printf("auth error: %s\n",ssh_get_error(session));
                ssh_disconnect(session);
                break;
            }
            pthread_mutex_lock(&session_lock);
            session_array[ctr] = session;
            pthread_mutex_unlock(&session_lock);

            struct clientDat pass;
            pass.id = ctr;
            pass.session = session_array[ctr];
    
            // Pass the connection off to the handler
            if(pthread_create(&thread, NULL, ssh_handler, &pass)){
                printf("Error creating thread\n");
                ssh_disconnect(session);
                break;

            }

            printf("Server: Passed session to thread...\n");

            thread_array[ctr] = thread;	
        }
    }
        
        
    for (size_t i = 0; i < ctr; i++){
        if(pthread_join(thread_array[i], NULL)){
            printf("Failed to join thread at index %d\n", i);
        }
    }

    ssh_bind_free(sshbind);

#ifdef WITH_PCAP
    cleanup_pcap();
#endif
    ssh_finalize();

    return 0;
}
