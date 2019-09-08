#include "misc.h"

int auth_password(const char *user, const char *password){
    /*
    * How this is gonna work:
    * File named 'users.dat'
    * Format is:
    *   Unique_ID:Hashed_passphrase(Maybe SHA512?)
    * 
    * Unique ID is shipped with each executable
    * Hashed passphrase is pretty straight forward
    */

    if(strcmp(user,"aris"))
        return 0;
    if(strcmp(password,"lala"))
        return 0;
    return 1; // authenticated
}

int index_of(char* str, char find, int rev){
    if (rev)
    {
        int end = strlen(str);
        int ctr = 0;
        while(end - ctr > 0){
            if(str[end-ctr] == find){
                return end - ctr;
            }
            ctr++;
        }
        return -1;
    } else {
        int i = 0;

        while (str[i] != '\0')
        {
            if (str[i] == find)
            {
                return i;
            }

            i++;
        }

        return -1;
    }
}

int directory_exists( const char* pzPath ){
    /*Tests if a directory exists in the file system */
    struct stat st = {0};

    if (stat(pzPath, &st) == -1) {
        return 0;
    }
    return 1;
    
}

void clean_input(char *input){
    int offset = index_of(input, '/', 1);
    input = input +offset;
}


void init() {
    struct stat st = {0};
    int rc = 0;
    mode_t i = umask(0);

    if (stat("agents", &st) == -1) {
        mkdir("agents", 0755);
        printf("Server: initialized directory 'agents'\n");
    }

}