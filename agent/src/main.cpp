#include "agent.h"
#include "config.h"
#include "beacon.h"


int main(int argc, char* argv[]){
	Agent *agent = new Agent();
	agent->run();
	delete agent;
	printf("Successfully disconnected from server\n");
    return 0;
}

