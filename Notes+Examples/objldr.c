#include <stdio.h>
#include <stdlib.h>
#include <dlfcn.h>


int main(int argc, char **argv){
    void *handle = dlopen("/home/wave/Desktop/Coding_Projects/C/Command-Control_Fiddling/Notes+Examples/libexample.so", RTLD_NOW);

    char *err = dlerror();
    if(err){
        printf("ERROR: %s\n", err);
        return 1;
    }

    void *(*example)(int);

    example = (void *(*)(int))dlsym(handle, "example");

    err = dlerror();
    if(err){
        printf("ERROR: %s\n", err);
        return 1;
    }

    void *malloc_addr = example(1024);
    printf("Address: %p\n", malloc_addr);
    free(malloc_addr);
    return 0;
}