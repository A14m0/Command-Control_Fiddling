#pragma once

#include "agents.h"
#include "typedefs.h"

int misc_get_file(const char *name, char **ptr);
int misc_index_of(const char* str, char find, int rev);
int misc_directory_exists( const char* pzPath );
void misc_clean_input(char *input);
void misc_serverinit();
int misc_copy_file(const char *filename, const char *dest);
char *misc_substring(const char *string, int position);
char** misc_str_split(char* a_str, const char a_delim);