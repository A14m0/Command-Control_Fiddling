#include "authenticate.h"

char *digest(char *input){
    unsigned char digest[SHA512_DIGEST_LENGTH];
    SHA512((unsigned char*)input, strlen(input), (unsigned char*)&digest);
    
    char *ret = malloc(SHA512_DIGEST_LENGTH*2+1);

    for(int i = 0; i < SHA512_DIGEST_LENGTH; i++)
         sprintf(&ret[i*2], "%02x", (unsigned int)digest[i]);
 
    return ret;
}

char** str_split(char* a_str, const char a_delim)
{
    char** result    = 0;
    size_t count     = 0;
    char* tmp        = a_str;
    char* last_comma = 0;
    char delim[2];
    delim[0] = a_delim;
    delim[1] = 0;

    /* Count how many elements will be extracted. */
    while (*tmp)
    {
        if (a_delim == *tmp)
        {
            count++;
            last_comma = tmp;
        }
        tmp++;
    }

    /* Add space for trailing token. */
    count += last_comma < (a_str + strlen(a_str) - 1);

    /* Add space for terminating null string so caller
       knows where the list of returned strings ends. */
    count++;

    result = malloc(sizeof(char*) * count);

    if (result)
    {
        size_t idx  = 0;
        char* token = strtok(a_str, delim);

        while (token)
        {
            assert(idx < count);
            *(result + idx++) = strdup(token);
            token = strtok(0, delim);
        }
        if(idx == count -1){
            *(result + idx) = 0;
        }
    }

    return result;
}



int authenticate(char *usr, char *pass){
    // initialize variables
    char** tokens = NULL;
    char** subtokens = NULL;
    char buff[1000];
    char thing[2] = "A\0";
    FILE *fd;
    
    // read in data into buffer
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

    tokens = str_split(buff, '\n');

    // begin going through usernames and hashes to attemtp to authenticate
    if(tokens){
        int j;
        for (j = 0; *(tokens + j); j++)
        {
            subtokens = str_split(*(tokens + j), ':');
            if(!strcmp(usr, *(subtokens))){
                if(!strcmp(digest(pass), *(subtokens+1))){
                    printf("Server: ID %s successfully authenticated\n", usr);
                    for (int i = 0; *(tokens + i); i++)
                    {
                        printf("Doing free of tokens (%p)\n", (tokens +i));
                        //if(*(tokens + i) == NULL)
                        free(*(tokens + i));
                        printf("Completed free\n");
                    }
                    for (int i = 0; *(subtokens+i); i++)
                    {
                        printf("Doing free of subtokens\n");
                        free(*(subtokens+i));
                    }
                    printf("Doing free of parent array ptrs\n");
                    free(subtokens);
                    printf("Last one\n");
                    free(tokens);
                    return 1;
                }
                else {
                    // note here that we do not break the loop, to prevent brute forcing of IDs through response time correlation
                    printf("Server: ID %s failed to pass password authentication\n");
                }
            } else {
                printf("Server: ID %s unknown to this server\n", usr);
            }
        }
    }

    for (int i = 0; *(tokens + i); i++)
    {
        printf("Doing free of tokens (failure)\n");
        free(*(tokens + i));
    }
    for (int i = 0; *(subtokens+i); i++)
    {
        printf("Doing free of subtokens (failure)\n");
        free(*(subtokens+i));
    }
    printf("Doing free ptr (failure)\n");
    free(subtokens);
    printf("Done (failure)\n");
    free(tokens);


    return 0;
}


struct ret *gen_creds(){
    FILE *fd;
    fd = fopen(DATA_FILE, "a");
    struct ret *buf = malloc(sizeof(struct ret));
    memset(buf, 0, sizeof(struct ret));
    char *usr = malloc(13);
    memset(usr, 0, 13);
    char *pwd = malloc(13);
    memset(usr, 0, 13);

    usr[13] = '\0';
    pwd[13] = '\0';

    buf->usr = usr;
    buf->passwd = pwd;


    for(int i = 0; i < 12; i++){
        usr[i] = 'A' + (random() % 26);
    }

    for(int i = 0; i < 12; i++){
        pwd[i] = 'A' + (random() % 26);
    }

    fwrite(buf, 1, sizeof(buf), fd);
    fclose(fd);
    return buf;
}