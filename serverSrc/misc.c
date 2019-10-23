#include "misc.h"
#include "authenticate.h"


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
    // seed random number generator
    srand(time(NULL));

	// gets current file path, so data will be written to correct folder regardless of where execution is called
	char result[4096];
	memset(result, 0, sizeof(result));
	ssize_t count = readlink( "/proc/self/exe", result, 4096);

	char dir[4096];
	memset(dir, 0, sizeof(dir));
	char* last;
	last = strrchr(result, '/');

	unsigned long index = last - result;
	strncpy(dir, result, index);
	
	printf("[i] Determined directory: %s\n", dir);
	
	int ret = chdir(dir);


	if(ret < 0){
		perror("[-] Failed to change directory");
		exit(-1);
	}


    struct stat st = {0};
    int rc = 0;
    mode_t i = umask(0);

    if (stat("agents", &st) == -1) {
        mkdir("agents", 0755);
        printf("Server: initialized directory 'agents'\n");
    }

    if (stat("out", &st) == -1) {
        mkdir("out", 0755);
        printf("Server: initialized directory 'out'\n");
    }

    if (stat(DATA_FILE, &st) == -1) {
        int fd2 = open(DATA_FILE, O_RDWR | O_CREAT, S_IRUSR | S_IRGRP | S_IROTH);
        printf("Server: initialized agent authentication file\n");
        close(fd2);
    }


}

int copy_file(char *filename, char *dest){
    size_t len = 0 ;
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

char *substring(char *string, int position, int length)
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
