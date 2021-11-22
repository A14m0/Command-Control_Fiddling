#pragma once
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <libssh/libssh.h>


// collection of miscilaneous functions 
class Misc {
public:
    static int get_file(char *name, char **buffer);
    static int index_of(char * str, char target, int reverse);
    static void remchar(char *msg, char target, char *buffer);
};