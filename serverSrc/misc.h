#pragma once
#include "agents.h"
#include "typedefs.h"

int misc_get_file(char *name, char **ptr);
int misc_index_of(char* str, char find, int rev);
int misc_directory_exists( const char* pzPath );
void misc_clean_input(char *input);
void misc_serverinit();
int misc_copy_file(char *filename, char *dest);
char *misc_substring(char *string, int position, int length);
char** misc_str_split(char* a_str, const char a_delim);