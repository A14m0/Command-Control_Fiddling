#include "misc.h"
#include "authenticate.h"
#include "log.h"

/* Gets the index of `find` in `str`. Goes backwards if `rev` is !0*/
int misc_index_of(const char* str, const char find, int rev){
    // check if we are going backwards 
    if (rev)
    {
        // get the length of string to iterate over
        int end = strlen(str);
        int ctr = 0;

        // loop over each character
        while(end - ctr > 0){
            if(str[end-ctr] == find){
                // found it so write index 
                return end - ctr;
            }
            ctr++;
        }
    } else {
        int i = 0;

        // loop while there is no nullterm
        while (str[i] != '\0') {
            if (str[i] == find) {
                return i;
            }

            i++;
        }
    }

    // Failed to find character
    return -1;
}

/* Checks if directory at `path` exists*/
int misc_directory_exists( const char* pzPath ){
    struct stat st = {0};

    // initialize a stats struct and checks if it succeeds
    // If it fails, it doesnt exist 
    if (stat(pzPath, &st) == -1) {
        return 0;
    }
    
    // directory exists
    return 1;
    
}

/* removes everything before the last occurance of a slash in `input`*/
void misc_clean_input(char *input){
    int offset = misc_index_of(input, '/', 1);
    input = input +offset;
}

/* Copies data from `filename` to `dest` */
int misc_copy_file(const char *filename, const char *dest){
    char ch;

    FILE* in = fopen( filename, "rb" ) ;
    FILE* out = fopen( dest, "wb" ) ;
    
    // check that the files successfully opened
    if( in == NULL || out == NULL )
    {
        perror( "An error occured while opening files" ) ;
        in = out = 0;
        return 1;
    }

    // loop through the first file and write all data to second
    while ((ch = fgetc(in)) != EOF)
        fputc(ch, out);
    

    // close and return
    fclose(in);
    fclose(out);
    
    return 0 ;
}

/* Gets substring from `string` given a position and length*/
char *misc_substring(const char *string, int position)
{
    char *pointer;
    int c;
 
    // allocate memory for substring
    pointer = (char *)malloc(strlen(string));
   
    if (pointer == NULL)
    {
        // malloc failed
        printf("Unable to allocate memory.\n");
        exit(1);
    }
 
    // sets all characters before position 
    // to corresponding indexes in return string
    for (c = 0 ; c < position ; c++)
    {
        *(pointer+c) = *(string);      
        string++;  
    }
 
    // null terminates the new string and returns it
    *(pointer+c) = '\0';
 
    return pointer;
}

/*Reads `name`, stores data in `ptr` and returns the file size*/
int misc_get_file(const char *name, char **ptr){
    int size = 0;
    FILE *file = NULL;

    // open the file
    file = fopen(name, "rb");
    if (file == NULL)
    {
        printf("Failed to open file %s\n", name);
        return -1;
    }

    // get the file size
    fseek(file, 0L, SEEK_END);
    size = ftell(file);

    // creates new memory allocation for data
    *ptr = (char *)malloc(size);
    memset(*ptr, 0, size);
    rewind(file);

    // read and close
    fread(*ptr, 1, size, file);
    fclose(file);

    return size;
}

/* Splits `a_str` at `a_delim` and returns all of the substrings */
char** misc_str_split(char* a_str, const char a_delim)
{
    char** result    = 0;
    size_t count     = 0;
    char* tmp        = a_str;
    char* last_comma = 0;
    char delim[2];
    delim[0] = a_delim;
    delim[1] = 0;

    // Count how many elements will be extracted
    while (*tmp)
    {
        if (a_delim == *tmp)
        {
            count++;
            last_comma = tmp;
        }
        tmp++;
    }

    // Add space for trailing token
    count += last_comma < (a_str + strlen(a_str) - 1);

    /* Add space for terminating null string so caller
       knows where the list of returned strings ends. */
    count++;

    result = (char **)malloc(sizeof(char*) * count);
    memset(result, 0, sizeof(char*) * count);

    if (result)
    {
        size_t idx  = 0;
        char* token = strtok(a_str, delim);

        // loop over tokens and put them in the returned array
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

