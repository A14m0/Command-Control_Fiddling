#pragma once

#include "common.h"

class B64
{
private:
    /* data */
public:
    static size_t enc_size(size_t inlen);
    static size_t dec_size(const char *in);
    static void encode(const unsigned char *in, size_t len, char **buff);
    static int decode(const char *in, unsigned char *out, size_t outlen);
    static int isvalidchar(char c);
    static const char b64chars[];
    static const int b64invs[];
};
