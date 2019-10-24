#pragma once
#include <stdio.h>
#include <openssl/sha.h>
#include <assert.h>
#include <string.h>
#include <stdlib.h>

#define DATA_FILE "agents/agents.dat"

struct ret
{
    char *usr;
    char *passwd;
};

int authenticate(char *usr, char *pass);
char *digest(char *input);
char** str_split(char* a_str, const char a_delim);