/**
* Vector Lab
* CS 241 - Fall 2018
*/

#include "sstring.h"
#include "vector.h"

#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif

#include <assert.h>
#include <string.h>
#include <stdlib.h>

struct sstring 
{
    char* string;
    int size;
};

sstring *cstr_to_sstring(char *input) {
    // your code goes here
    assert(input);

    sstring* result = (sstring*) malloc(sizeof(sstring));
    result -> string = strdup(input);
    result -> size = strlen(input);
    
    return result;
}

char *sstring_to_cstr(sstring *input) {
    // your code goes here
    assert(input);
    return strdup(input -> string);
}

int sstring_append(sstring *this, sstring *addition) {
    // your code goes here
    assert(this);
    assert(addition);
    this -> size = this -> size + addition -> size;

    this -> string = (char*) realloc(this -> string, this -> size + 1);

    strcat(this -> string, addition -> string);
    
    return this -> size;
}

vector *sstring_split(sstring *this, char delimiter) {
    // your code goes here
    assert(this);

    vector* result = string_vector_create();
    char* tmp_string = strdup(this->string);

    char* token = NULL;
    char* saveptr = tmp_string;

    while((token = strtok_r(saveptr, &delimiter, &saveptr)))
    {
        vector_push_back(result, token);
    }

    free(tmp_string);


    return result;
}

int sstring_substitute(sstring *this, size_t offset, char *target,
                       char *substitution) {
    // your code goes here
    assert(this);
    assert(target);
    assert(substitution);
    assert(offset >= 0 && offset < (size_t) this -> size + 1);

    char* pch = strstr((this->string)+offset, target);

    if(!pch)
        return -1;

    int pos = pch - (this->string);

    this -> size = this -> size + strlen(substitution) - strlen(target); 
    char* new_string = (char*) malloc(this->size + 1);

    memcpy(new_string, this->string, pos);
    memcpy(new_string+pos, substitution, strlen(substitution));
    memcpy(new_string+pos+strlen(substitution), this->string+pos+strlen(target), 
            this->size - strlen(substitution) - pos +1);

    free(this->string);
    this -> string = new_string;

    return 0;
}

char *sstring_slice(sstring *this, int start, int end) {
    assert(this);
    // your code goes here

    char* result = (char*) malloc(end-start+1);
    result[end-start] = '\0';

    memcpy(result, (this->string)+start, end-start);
    
    return result;
}

void sstring_destroy(sstring *this) {
    // your code goes here
    free(this -> string);
    free(this);
}
