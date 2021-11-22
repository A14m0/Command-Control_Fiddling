#include "misc.h"

// returns all the things you could want from a file
int Misc::get_file(char *name, char **ptr){
    int size = 0;
    FILE *file = NULL;
    
    // open the file for reading
    file = fopen(name, "rb");
    if (file == NULL) {
        printf("Failed to open file %s\n", name);
        return -1;
    }

    // deterine the filesize
    fseek(file, 0L, SEEK_END);
    size = ftell(file);

    // save file to memory
    *ptr = (char*)malloc(size);
    memset(*ptr, 0, size);
    rewind(file);
    fread(*ptr, 1, size, file);
    fclose(file);

    return size;
}

// returns the index of `find` in the string `str`. If `rev=1`, then goes backwards
int Misc::index_of(char* str, char find, int rev){
    if (rev) {
        // goes backwards
        int end = strlen(str);
        int ctr = 0;
        while(end - ctr > 0){
            if(str[end-ctr] == find) {
                return end - ctr;
            }
            ctr++;
        }
        return -1;
    } else {
        // goes forwards
        int i = 0;
        while (str[i] != '\0') {
            if (str[i] == find){
                return i;
            }
            i++;
        }
        return -1;
    }
}

// removes a 
void Misc::remchar(char *msg, char rem, char *buff){
    int size;

    for(size = 0; msg[size] != '\0'; ++size);

    int loop = 0;
	int index = 0;
	while(loop <= size){
        if (msg[loop] != rem){
        	buff[index] = msg[loop];
			index++;
		}
        loop++;
	}
}