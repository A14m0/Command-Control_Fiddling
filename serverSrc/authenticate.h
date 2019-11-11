#pragma once
#include <stdio.h>
#include <openssl/sha.h>
#include <assert.h>
#include <string.h>
#include <stdlib.h>
#include "misc.h"

#define DATA_FILE "agents/agents.dat"

struct ret
{
    char *usr;
    char *passwd;
};

int authenticate_doauth(const char *usr, const char *pass);
char *authenticate_digest(char *input);
