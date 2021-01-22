/* Defines non-overridden functionality of Common class */
#include "common.h"

// dummy constructors and destructors
Common::Common(){}
Common::~Common(){}


/* Gets the index of `find` in `str`. Goes backwards if `rev` is !0*/
int Common::index_of(const char* str, const char find, int rev){
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
int Common::directory_exists( const char* pzPath ){
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
void Common::clean_input(char *input){
    int offset = Common::index_of(input, '/', 1);
    input = input +offset;
}

/* Copies data from `filename` to `dest`
int Common::copy_file(const char *filename, const char *dest){
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
} */

/* Gets substring from `string` given a position and length*/
char *Common::substring(const char *string, int position)
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

/*Reads `name`, stores data in `ptr` and returns the file size
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
}*/

/* Splits `a_str` at `a_delim` and returns all of the substrings */
char** Common::str_split(char* a_str, const char a_delim)
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



/* returns the size of the buffer
given the `inlen` of the raw buffer */
size_t B64::enc_size(size_t inlen)
{
	size_t ret;

	ret = inlen;
	if (inlen % 3 != 0)
		ret += 3 - (inlen % 3);
	ret /= 3;
	ret *= 4;

	return ret;
}

/* returns the size of the decoded buffer
given the `in` string */
size_t B64::dec_size(const char *in)
{
	size_t len;
	size_t ret;
	size_t i;

	if (in == NULL)
		return 0;

	len = strlen(in);
	ret = len / 4 * 3;

	for (i=len; i-->0; ) {
		if (in[i] == '=') {
			ret--;
		} else {
			break;
		}
	}

	return ret;
}

/*Encodes the `in` buffer of size `len`
and writes encoded string to `buff`*/
void B64::encode(const unsigned char *in, size_t len, char **buff)
{
	char   *out;
	size_t  elen;
	size_t  i;
	size_t  j;
	size_t  v;

	if (in == NULL || len == 0){
		printf("INNODE IS NULL!\n");
		return;
	}

	elen = B64::enc_size(len);
	out  = (char*)malloc(elen+1);
	out[elen] = '\0';

	for (i=0, j=0; i<len; i+=3, j+=4) {
		v = in[i];
		v = i+1 < len ? v << 8 | in[i+1] : v << 8;
		v = i+2 < len ? v << 8 | in[i+2] : v << 8;

		out[j]   = B64::b64chars[(v >> 18) & 0x3F];
		out[j+1] = B64::b64chars[(v >> 12) & 0x3F];
		if (i+1 < len) {
			out[j+2] = B64::b64chars[(v >> 6) & 0x3F];
		} else {
			out[j+2] = '=';
		}
		if (i+2 < len) {
			out[j+3] = B64::b64chars[v & 0x3F];
		} else {
			out[j+3] = '=';
		}
	}

	*buff = out;
	return;
}

/* Checks if `c` is a valid character */
int B64::isvalidchar(char c)
{
	if (c >= '0' && c <= '9')
		return 1;
	if (c >= 'A' && c <= 'Z')
		return 1;
	if (c >= 'a' && c <= 'z')
		return 1;
	if (c == '+' || c == '/' || c == '=')
		return 1;
	return 0;
}

/* Decodes `in` and writes to `out`, storing length in `outlen`*/
int B64::decode(const char *in, unsigned char *out, size_t outlen)
{
	size_t len;
	size_t i;
	size_t j;
	int    v;

	if (in == NULL || out == NULL)
		return 0;

	len = strlen(in);
	if (outlen < B64::dec_size(in) || len % 4 != 0)
		return 0;

	for (i=0; i<len; i++) {
		if (!B64::isvalidchar(in[i])) {
			return 0;
		}
	}

	for (i=0, j=0; i<len; i+=4, j+=3) {
		v = B64::b64invs[in[i]-43];
		v = (v << 6) | B64::b64invs[in[i+1]-43];
		v = in[i+2]=='=' ? v << 6 : (v << 6) | B64::b64invs[in[i+2]-43];
		v = in[i+3]=='=' ? v << 6 : (v << 6) | B64::b64invs[in[i+3]-43];

		out[j] = (v >> 16) & 0xFF;
		if (in[i+2] != '=')
			out[j+1] = (v >> 8) & 0xFF;
		if (in[i+3] != '=')
			out[j+2] = v & 0xFF;
	}

	return 1;
}

const char B64::b64chars[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

const int B64::b64invs[] = { 62, -1, -1, -1, 63, 52, 53, 54, 55, 56, 57, 58,
	59, 60, 61, -1, -1, -1, -1, -1, -1, -1, 0, 1, 2, 3, 4, 5,
	6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20,
	21, 22, 23, 24, 25, -1, -1, -1, -1, -1, -1, 26, 27, 28,
	29, 30, 31, 32, 33, 34, 35, 36, 37, 38, 39, 40, 41, 42,
	43, 44, 45, 46, 47, 48, 49, 50, 51 };


const char *CONST_NA = "NA";

/* Initializes an agent's working directory */
int Common::init_agent(const char *agent_id){

    // initialize and zero needed variables and buffers
    FILE *manifest = NULL;
    char parent_dir[2048];
    char tmp_buff[BUFSIZ];
    char *tmp = NULL;
    char *buff = NULL;
    int rc = 0;

    memset(parent_dir, 0, sizeof(parent_dir));
    memset(tmp_buff, 0, sizeof(tmp_buff));

    // find correct agent directory
    sprintf(parent_dir, "%s/agents/%s", getcwd(tmp_buff, sizeof(tmp_buff)), agent_id);

    // duplicate parent directory into multiple buffers for use
    buff = strdup(parent_dir);
    tmp = strdup(parent_dir);

    // create dir with correct perms
    rc = mkdir(parent_dir, 0755);
    if(rc != 0){
        perror("Failed to create agent's directory");
        return 1;
    }

    // creates the loot directory
    rc = mkdir(strcat(parent_dir, "/loot"), 0755);
    if(rc != 0){
        perror("Failed to create loot directory");
        return 1;
    }

    // create the tasking directory
    rc = mkdir(strcat(tmp, "/tasking"), 0755);
    if(rc != 0){
        perror("Failed to create tasking directory");
        return 1;
    }

    // open and write default agent manifest
    manifest = fopen(strcat(buff, "/agent.mfst"), "w");
    if (!manifest){
        perror("Failed to create agent manifest");
        return 1;
    }
    fwrite("NULL :)", 1, sizeof("NULL :)"), manifest);
    fclose(manifest);

    // write default agent information to its new info file
    Common::write_agent_beacon(agent_id, "NA\nNA\nNA\nNA\nNA\n");

    return 0;
}

/* Registers a new set of credentials with the server*/
int Common::_register_agent(const char *username, const char *password){
    FILE *file;

    // open credential store
    file = fopen("agents/agents.dat", "a");
    if (!file)
    {
        printf("Failed to open file thing\n");
        return 1;
    }

    // go to the end and write data
    fseek(file, 0L, SEEK_END);
    fprintf(file, "%s:%s\n", username, Common::digest(password));

    // close and return
    fclose(file);
    return 0;
}

/* Registers agent using single line format */
int Common::register_agent(char *line){

    char *id;
    char *passwd;

    // find the delimeter character ':'
    int delim = Common::index_of(line, ':', 0);

    // the password is everything after the delim
    passwd = line + delim + 1;

    // zero the delimeter so we can terminate the id string
    line[delim] = '\0';
    id = line;

    // register the agent
    printf("Registering agents: %s,%s\n", id, passwd);

    Common::_register_agent(id, passwd);

    return 0;
}

/* Retrieves the tasking information from agent with ID `agent_id` */
char *Common::get_agent_tasking(const char *agent_id){
    char file[2048];
    char cwd_buf[BUFSIZ];
    char *mem_dump = NULL;
    int size = 0;
    FILE *fd = NULL;

    // get target agent's manifest
    memset(cwd_buf, 0, sizeof(cwd_buf));
    memset(file, 0, sizeof(file));
    sprintf(file, "%s/agents/%s/agent.mfst", getcwd(cwd_buf, sizeof(cwd_buf)),agent_id);

    // open it
    fd = fopen(file, "rb");
    if(fd == NULL) return NULL;

    // get the file's size
    fseek(fd, 0L, SEEK_END);
    size = ftell(fd);
    rewind(fd);

    // allocate heap memory for the data and read it
    mem_dump = (char *)malloc(size+1);
    memset(mem_dump, 0, size+1);
    fread(mem_dump, 1, size, fd);

    // close the file
    fclose(fd);
    //write_format(file);
    return mem_dump;
}

/* Generates a new username and password set */
ppasswd_t Common::gen_agent_creds(){
    // allocate heap memory and zero fields
    struct ret *buf = (struct ret *) malloc(sizeof(struct ret));
    memset(buf, 0, sizeof(struct ret));
    char *usr = (char *)malloc(13);
    memset(usr, 0, 13);
    char *pwd = (char *)malloc(13);
    memset(usr, 0, 13);

    // assign structure values to allocated chunks
    buf->usr = usr;
    buf->passwd = pwd;

    // generate a 12 character random username
    for(int i = 0; i < 12; i++){
        usr[i] = 'A' + (random() % 26);
    }

    // generate a 12 character random password
    // Note: should probably increase this at some point
    for(int i = 0; i < 12; i++){
        pwd[i] = 'A' + (random() % 26);
    }

    return buf;
}

/* Adds `operation` to `agent` tasking queue with `opt`*/
int Common::task_agent(const int operation,
                                  const char *agent,
                                  const char *opt){
    FILE *file = NULL;
    char buffer[BUFSIZ];
    char tmpbuff[BUFSIZ];

    // get path to agent's manifest
    memset(buffer, 0, sizeof(buffer));
    sprintf(buffer, "%s/agents/%s/agent.mfst",
            getcwd(tmpbuff, sizeof(tmpbuff)), agent);

    // open file
    file = fopen(buffer, "a");
    if(!file){
        perror("Server failed to read agent tasking file");
        return 1;
    }

    printf("Server: Tasking %s with operation %d\n", agent, operation);

    // format, write, and close
    fprintf(file, "%d|%s\n", operation, opt);
    fclose(file);

    return 0;
}

/* Writes received agent beacon data to agent's info.txt*/
int Common::write_agent_beacon(const char *id,
                                          const char *beacon){
    FILE *fd = NULL;
    char buff[2048];
    char cwd[BUFSIZ];

    memset(buff, 0, sizeof(buff));
    memset(cwd, 0, sizeof(cwd));

    // get path to agent's info
    sprintf(buff, "%s/agents/%s/info.txt", getcwd(cwd, sizeof(cwd)), id);

    fd = fopen(buff, "w");
    if (fd == NULL)
    {
        perror("");
        return 1;
    }

    // write, close, return
    fwrite(beacon, 1,strlen(beacon), fd);
    fclose(fd);

    return 0;
}

/* Writes the default manifest of the agent*/
int Common::write_default_agent_manifest(char *path){
    FILE *fd;

    fd = fopen(path, "w");
    if (!fd)
    {
        perror("");
        return 1;
    }

    fwrite("NULL :)\n", 1, sizeof("NULL :)"), fd);
    fclose(fd);
    return 0;
}

/* Creates a SHA512 hash of `input`*/
char* Common::digest(const char* input){
    unsigned char digest[SHA512_DIGEST_LENGTH];

    // get hash of `input`
    SHA512((unsigned char*)input, strlen(input), (unsigned char*)&digest);
    char* ret = (char*)malloc(SHA512_DIGEST_LENGTH*2+1);
    ret[SHA512_DIGEST_LENGTH*2] = '\0';

    // format for use
    for(int i = 0; i < SHA512_DIGEST_LENGTH; i++)
        sprintf(&ret[i*2], "%02x", (unsigned int)digest[i]);

    return ret;
}
