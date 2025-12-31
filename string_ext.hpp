#ifndef _STRING_EXT_H___
#define _STRING_EXT_H___

#include <stdlib.h>
#include <string.h>
#include <assert.h>

void str_replace(char* text, char to_find, char to_replace_with);
char** str_split(char* a_str, const char a_delim);
char** string_split(char* string, const char delimiter);
void free_splitted_string(char** splitted_strings);

#endif