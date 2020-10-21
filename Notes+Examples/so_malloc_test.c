#include <stdlib.h>
#include <stdio.h>

void *example(int num_bytes){
    void *addr = malloc(num_bytes);
    printf("Address: %p\n", addr);
    return addr;
}

