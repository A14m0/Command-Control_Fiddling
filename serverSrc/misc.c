#include "misc.h"
#include "authenticate.h"


int misc_index_of(char* str, char find, int rev){
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

int misc_directory_exists( const char* pzPath ){
    /*Tests if a directory exists in the file system */
    struct stat st = {0};

    if (stat(pzPath, &st) == -1) {
        return 0;
    }
    return 1;
    
}

void misc_clean_input(char *input){
    int offset = misc_index_of(input, '/', 1);
    input = input +offset;
}


void misc_serverinit() {
    // seed random number generator
    srand(time(NULL));

	// gets current file path, so data will be written to correct folder regardless of where execution is called
	char result[4096];
	memset(result, 0, sizeof(result));
	readlink( "/proc/self/exe", result, 4096);

	char dir[4096];
	memset(dir, 0, sizeof(dir));
	char* last;
	last = strrchr(result, '/');

	unsigned long index = last - result;
	strncpy(dir, result, index);
	
	int ret = chdir(dir);


	if(ret < 0){
		perror("Failed to change directory");
		exit(-1);
	}


    struct stat st = {0};
    umask(0);

    if (stat("agents", &st) == -1) {
        mkdir("agents", 0755);
        printf("Server: initialized directory 'agents'\n");
    }

    if (stat("out", &st) == -1) {
        mkdir("out", 0755);
        printf("Server: initialized directory 'out'\n");
    }

    if (stat("modules", &st) == -1) {
        mkdir("modules", 0755);
        printf("Server: initialized directory 'modules'\n");
    }

    if (stat(DATA_FILE, &st) == -1) {
        int fd2 = open(DATA_FILE, O_RDWR | O_CREAT, S_IRUSR | S_IRGRP | S_IROTH);
        printf("Server: initialized agent authentication file\n");
        close(fd2);
    }


}

int misc_copy_file(char *filename, char *dest){
    char ch;

    printf("Moving file: %s -> %s\n", filename, dest);

    FILE* in = fopen( filename, "rb" ) ;
    FILE* out = fopen( dest, "wb" ) ;
    if( in == NULL || out == NULL )
    {
        perror( "An error occured while opening files" ) ;
        in = out = 0 ;
    }
    while ((ch = fgetc(in)) != EOF)
        fputc(ch, out);
    
    fclose(in);
    fclose(out);
    
    return 0 ;
}

char *misc_substring(char *string, int position, int length)
{
    char *pointer;
    int c;
 
    pointer = malloc(length+1);
   
    if (pointer == NULL)
    {
        printf("Unable to allocate memory.\n");
        exit(1);
    }
 
    for (c = 0 ; c < position ; c++)
    {
        *(pointer+c) = *(string);      
        string++;  
    }
 
    *(pointer+c) = '\0';
 
    return pointer;
}

int misc_get_file(char *name, char **ptr){
    int size = 0;
    FILE *file = NULL;
    file = fopen(name, "rb");

    if (file == NULL)
    {
        printf("Failed to open file %s\n", name);
        return -1;
    }
    fseek(file, 0L, SEEK_END);
    size = ftell(file);

    *ptr = malloc(size);
    memset(*ptr, 0, size);
    rewind(file);
    fread(*ptr, 1, size, file);
    fclose(file);

    return size;
}

char** misc_str_split(char* a_str, const char a_delim)
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
    memset(result, 0, sizeof(char*) * count);

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

