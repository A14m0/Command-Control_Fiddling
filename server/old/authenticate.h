#pragma once
#include "common.h"
#include "misc.h"
#include "typedefs.h"

class Authenticate {
private:
public:
    static int doauth(const char *usr, const char *passwd);
    static char* digest(const char *input);
};

