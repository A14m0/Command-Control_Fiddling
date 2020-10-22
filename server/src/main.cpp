/* Main function */
#include "server.h"


int main(int argc, char **argv){
    Server *srv = new Server();

    srv->MainLoop();

    printf("Server completed execution\n");
    return 0;
}