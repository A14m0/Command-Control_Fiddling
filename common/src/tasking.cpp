#include "tasking.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>


// constructor for our AgentJob class
AgentJob::AgentJob(unsigned char type, unsigned long len, void* data) {
    // sanity check the len to make sure it isn't too big
    if( (len & 0xff00000000000000) != 0) {
        printf("Len too long!");
        exit(1);
    }
    this->type = type;
    this->len = len;
    this->data = data;
}

AgentJob::AgentJob(unsigned long combined, void* data) {
    // Type: (char) ((combined & 0xff00000000000000) >>7)
    unsigned char t = (unsigned char) ((combined & 0xff00000000000000) >> 56);
    unsigned long l = (unsigned long) (combined & 0x00ffffffffffffff);
    this->type = t;
    this->len = l;
    this->data = data;
}

AgentJob::AgentJob(ptask_t task) {
    this->type = task->type;
    this->len = task->length;
    this->data = task->data;
}

AgentJob::~AgentJob(){
    if(this->data != nullptr) {
        free(data);
    }
}


// helper function to get the type of an agent job strucutre
unsigned char AgentJob::get_type() {
    return this->type;
}

// helper function to get the length of an agent job's data
unsigned long AgentJob::get_len() {
    return this->len;
}

// helper function to get the raw data of an agent job
void *AgentJob::get_data() {
    return this->data;  
}

// helper function to set the data of the job
void AgentJob::set_data(void *data) {
    this->data = data;
} 

// helper function that generates a header long for the agent job
unsigned long AgentJob::encode_header(){
    unsigned long t = (unsigned long)(this->type);
    t = t << 56;
    return t | this->len;
}

// converts an unsigned long integer to bytes
unsigned char *AgentJob::long_to_bytes(unsigned long v) {
    unsigned char *buffer = (unsigned char*)malloc(8);
    unsigned long mask = 0xfflu << 56;
    
    // fetch the bits we need from the long
    for(int i = 7; i >= 0; i--){
        buffer[i] = (unsigned char) ((v & mask) >> (8*i)) & 0xff;
        mask = mask >> 8;
    }
    return buffer;
}


// converts 7 bytes to an unsigned long integer
unsigned long AgentJob::bytes_to_long(unsigned char *t) {
    unsigned long p1 = t[0]+/*skip t[0], as it is opcode*/ (t[1] << 8) + (t[2] << 16) + (t[3] << 24); 
    unsigned long p2 = (t[4] + (t[5] << 8) + (t[6] << 16) + (t[7] << 24)); // shift 32?
    p2 = p2 << 32;
    
    return p1 + p2;
}

// packs the data of the job into one structure
void *AgentJob::pack() {
    size_t packet_len = this->get_len() + 8;
    void *packet = malloc(packet_len);
    memset(packet, 0, packet_len);

    // get the header
    unsigned char *header_bytes = AgentJob::long_to_bytes(this->encode_header());
    memcpy(packet, header_bytes, 8);
    free(header_bytes);

    // copy the payload
    memcpy(packet+8, this->data, this->get_len());

    // return the packet 
    return packet;
}