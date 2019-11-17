#include "authenticate.h"

char *authenticate_digest(char *input){
    unsigned char digest[SHA512_DIGEST_LENGTH];
    SHA512((unsigned char*)input, strlen(input), (unsigned char*)&digest);
    
    char *ret = malloc(SHA512_DIGEST_LENGTH*2+1);
    ret[SHA512_DIGEST_LENGTH*2] = '\0';

    for(int i = 0; i < SHA512_DIGEST_LENGTH; i++)
         sprintf(&ret[i*2], "%02x", (unsigned int)digest[i]);
 
    return ret;
}

int authenticate_doauth(const char *usr, const char *pass){
    // initialize variables
    char** tokens = NULL;
    char** subtokens = NULL;
    char buff[1000];
    char thing[2] = "A\0";
    FILE *fd;
    
    // read in data into buffer
    // TODO: STACK SMASHING DETECTED HERE FOR WHATEVER FUCKING REASON
    fd = fopen(DATA_FILE, "r");
    if (fd == NULL)
    {
        printf("Server: FAILED TO OPEN AGENT DATABASE\n");
        return 0;
    }

    memset(buff, 0, 1000);
    int ctr = 0;
    if(fd != NULL)
    {
        while((thing[0] = getc(fd)) != EOF)
        {
            strcat(buff, thing);
            ctr++;
        }
        fclose(fd);
    }

    if(ctr == 0){
        printf("Server: Found default agent file. Register agents by compiling them with this install\n");
        return 0;
    }

    tokens = misc_str_split(buff, '\n');

    // begin going through usernames and hashes to attemtp to authenticate
    if(tokens){
        int j;
        for (j = 0; *(tokens + j); j++)
        {
            subtokens = misc_str_split(*(tokens + j), ':');
            if(!strcmp(usr, *(subtokens))){
                if(!strcmp(authenticate_digest(pass), *(subtokens+1))){
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
                    // note here that we do not break the loop, to prevent brute forcing of IDs through response time correlation
                    printf("Server: ID %s failed to pass password authentication\n", usr);
                    return 0;
                }
            }
        }
        printf("Server: ID %s unknown to this server\n", usr);
    }

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
