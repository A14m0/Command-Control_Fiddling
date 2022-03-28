#include "authenticate.h"

/* Creates a SHA512 hash of `input`*/
char* Authenticate::digest(const char *input){
    unsigned char digest[SHA512_DIGEST_LENGTH];

    // get hash of `input`
    SHA512((unsigned char*)input, strlen(input), (unsigned char*)&digest);
    
    char *ret = (char *)malloc(SHA512_DIGEST_LENGTH*2+1);
    ret[SHA512_DIGEST_LENGTH*2] = '\0';

    // format it for use 
    for(int i = 0; i < SHA512_DIGEST_LENGTH; i++)
         sprintf(&ret[i*2], "%02x", (unsigned int)digest[i]);
 
    return ret;
}

/* Checks if `usr` and `pass` should be allowed to connect */
int Authenticate::doauth(const char *usr, const char *pass){
    // initialize variables
    char** tokens = NULL;
    char** subtokens = NULL;
    char *buff;
    int size = 0;
    
    // read in data into buffer
    size = misc_get_file("agents/agents.dat", &buff);
    
    if(size == 0){
        printf("Server: Found default agent file. Register agents by compiling them with this install\n");
        return 0;
    }

    tokens = misc_str_split(buff, '\n');

    // begin going through usernames and hashes to attemtp to authenticate
    if(tokens){
        int j;

        // loop over each line in file
        for (j = 0; *(tokens + j); j++)
        {
            // Split at `:` (to get username and hash separeately)
            subtokens = misc_str_split(*(tokens + j), ':');

            // check if `usr` is the current line's username
            if(!strcmp(usr, *(subtokens))){
                // check if the digest of `pass` is the same as the hash
                char *auth = Authenticate::digest(pass);
                if(!strcmp(auth, *(subtokens+1))){
                    free(auth);
                    // success and free
                    printf("Server: ID %s successfully authenticated\n", usr);
                    for (int i = 0; *(tokens + i); i++)
                    {
                        free(*(tokens + i));
                    }
                    for (int i = 0; *(subtokens+i); i++)
                    {
                        free(*(subtokens+i));
                    }
                    free(subtokens);
                    free(tokens);
                    return 1;
                }
                else {
                    free(auth);
                    // note here that we do not break the loop, 
                    //t o prevent brute forcing of IDs through response time correlation
                    printf("Server: ID %s failed to pass password authentication\n", usr);
                    return 0;
                }
            }
        }
        // unknown username
        printf("Server: ID %s unknown to this server\n", usr);
    }

    // free tokens
    for (int i = 0; *(tokens + i); i++)
    {
        free(*(tokens + i));
    }
    for (int i = 0; *(subtokens+i); i++)
    {
        free(*(subtokens+i));
    }
    free(subtokens);
    free(tokens);

    return 0;
}
