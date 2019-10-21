#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <openssl/sha.h>
#include <assert.h>

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
        assert(idx == count - 1);
        *(result + idx) = 0;
    }

    return result;
}
int main(){
    char** tokens;
    char** subtokens;

    char *id = "aris";
    char *pass = "lala";
    printf("Password hash: %s\n", digest(pass));

    FILE *fd;
    fd = fopen("test.dat", "r");
    if (fd == NULL)
    {
        printf("Server: FAILED TO OPEN AGENT DATABASE\n");
        return 0;
    }

    char buff[1000];
    memset(buff, 0, 1000);
    printf("Buff contents before %s\n", buff);
    char symbol;
    if(fd != NULL)
    {
        while((symbol = getc(fd)) != EOF)
        {
            strcat(buff, &symbol);
        }
        fclose(fd);
    }

    printf("Buffer contents: [%s]\n\n", buff);

    tokens = str_split(buff, '\n');

    if (tokens)
    {
        int i;
        for (i = 0; *(tokens + i); i++)
        {
            subtokens = str_split(*(tokens + i), ':');
            if(subtokens){
                for(int j = 0; *(subtokens + j); j++){
                    printf("Split String: %s", *(subtokens + j));
                    free(*(subtokens));
                }
            }
            printf("User=[%s]\n", *(tokens + i));
            free(*(tokens + i));
        }
        printf("\n");
        free(tokens);
    }
    return 0;
}
/*
int main(){
    char *id = "aris";
    char *pass = "lala";

    printf("Password hash: %s\n", digest(pass));

    FILE *fd;
    fd = fopen("test.dat", "r");
    if (fd == NULL)
    {
        printf("Server: FAILED TO OPEN AGENT DATABASE\n");
        return 0;
    }
    
    char buf[SHA512_DIGEST_LENGTH];
    char thing = '\0';
    char id_buf[SHA512_DIGEST_LENGTH];
    char thing2 = '\0';
    char pass_buf[SHA512_DIGEST_LENGTH]; 
    memset(buf, 0, SHA512_DIGEST_LENGTH);
    memset(id_buf, 0, SHA512_DIGEST_LENGTH);
    memset(pass_buf, 0, SHA512_DIGEST_LENGTH);
    int count = 1;
    for (size_t i = 0; i < count; i++)
    {
        fscanf(fd,"%[^\n]", buf);
        printf("%s\n", buf);
        sscanf(buf, "%[^:]", id_buf);
        if(!strcmp(id, id_buf)){
            printf("Found agent ID '%s' in database. Testing password...\n", id);
            char term = ':';
            int i = 0;
            while(buf[i] != term){
                printf("Not the character: '%c'\n", buf[i]);
                i++;
            }
            printf("Found the character: '%c'\n", buf[i]);
            printf("Thing: %s\n", buf + i);
            sscanf(pass_buf, "%[^\n]", buf + i);
            printf("Heres the pass: '%s'\n", pass_buf);
            
        }

        memset(buf, 0, 100);
    }
    return 0;
}*/
