#pragma once

// common used libraries
#include <assert.h>
#include <dirent.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <openssl/sha.h>
#include <pthread.h>
#include <signal.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <time.h>
#include <unistd.h>
#include <vector>
#include <queue>

#include <stdarg.h>
#include "log.h"

class Common
{
private:
    class Log *logger;
public:
    Common(/* args */);
    ~Common();

    int log(const char *format, const char *id, ...);
};
