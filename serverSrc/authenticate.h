#pragma once
#include "common.h"
#include "misc.h"
#include "typedefs.h"

int authenticate_doauth(const char *usr, const char *pass);
char *authenticate_digest(char *input);
