#include "server.h"

/*Global variables and data structures*/
pthread_t thread_array[MAX_CONN];


/*handy little print function for debugging pClientDat structures*/
void print_clientDat(pClientDat str){
    printf("\n\nID: %s\n", str->id);
    printf("Type: %d\n\n\n", str->type);
}

Server::Server(){
    this->sessions = new std::vector<ConnectionInstance *>(0);
    this->shell_queue = new std::queue<ConnectionInstance *>();
    this->logger = new Log();

    int ret;
    char* last;
	char result[4096];
    char dir[4096];
    unsigned long index;
    struct stat st = {0};
    int type = 0;
    int opt = 0;

    if( (this->master_socket = socket(AF_INET , SOCK_STREAM , 0)) == 0){   
        perror("Server: Socket Failure");      
    }

    if(setsockopt(master_socket, SOL_SOCKET, SO_REUSEADDR, (char *)&opt, sizeof(opt)) < 0 ){   
        perror("Server: Setsockopt Failure");
    }

    


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

void Server::add_instance(ConnectionInstance *instance){
    if (instance == nullptr){
        printf("Nullptr instance caught\n");
    } else {
        this->sessions->push_back(instance);
    }
    
}

class Log *Server::get_log(){
    return this->logger;
}

int Server::listen_instance(int index){
    pthread_t thread;
    
    class ConnectionInstance *instance = this->sessions->at(index);
    if(instance == nullptr){
        printf("instance is still nullptr\n");
        return 1;
    }

    // to keep a reference to the thread in memory
    instance->set_thread(thread);
    // pass connection to handler thread
    if(pthread_create(&thread, NULL, instance->handle_connection, instance)){
        printf("Error creating thread\n");
        delete instance;
        return 1;
    }
    
    return 0;
}

int Server::listen_instance(class ConnectionInstance *instance){
    pthread_t thread;
    
    //instance->get_transport()->listen(this->master_socket);
    instance->set_thread(thread);
    // pass connection to handler thread
    if(pthread_create(&thread, NULL, instance->handle_connection, instance)){
        printf("Error creating thread\n");
        delete instance;
        return 1;
    }
    
    return 0;
}

std::queue<ConnectionInstance *> *Server::get_shell_queue(){
    return this->shell_queue;
}

/*Funny enough, this is the main function*/
int main(int argc, char **argv){
    // TODO: UPDATE SIGHANDLERS TO MODERN STUFF

    // initialize variables
    Server *server = new Server();

    // Example load .so module
    void *handle = dlopen("./shared/example_module.so", RTLD_LAZY);
    if(!handle) {
        printf("failed to load .so file\n");    
        return -1;
    }
    
    int type = *(int *)dlsym(handle, "type");
    if(!type) {
        printf("Failed to find type symbol!\n");
        return 1;
    }
    else printf("Detected type of object: %d\n", type);

    ptransport_t transport;
    class ConnectionInstance *instance;
    void (*entrypoint)();
    pClientDat init_data = (pClientDat)malloc(sizeof(ClientDat));
    memset(init_data, 0, sizeof(ClientDat));

    switch(type){
        case MODULE:
            printf("Detected module type\n");
            
            entrypoint = (void (*)())dlsym(handle, "entrypoint");
            if (!entrypoint){
                printf("Failed to locate the module's entrypoint function");
                return 1;
            }
            (*entrypoint)();
            break;
        case TRANSPORT:
            printf("Detected transport type\n");
            transport = (ptransport_t)dlsym(handle, "transport_api");
            if(!transport) {
                printf("Failed to find transport api\n"); 
                return 1;
            }
            transport->init(init_data);
            instance = new ConnectionInstance(server);
            instance->set_transport(transport);
            server->add_instance(instance);
            server->listen_instance(0);
            break;

        default:
            printf("Unknown type: %d\n", type);
            return 1;

    }
    
    // initialize the controller socket
    //ConnectionInstance *instance = new ConnectionInstance(server);
    //ServerTransport *def_transport = new Ssh_Transport(instance);

    //instance->set_transport(def_transport);
    //server->add_instance(instance);
    //server->listen_instance(0);
    

    
    // accept connections
    // TODO: FIX THIS SO LOOP BIND WORKS LOL
  
    while(1){
        sleep(1);
    }
    
    printf("Server: Terminated successfully\n");
    return 0;
}
