/**
* Text Editor Lab
* CS 241 - Fall 2018
*/

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "document.h"
#include "vector.h"

struct document {
    vector *vector;
};

document *document_create() {
    document *this = (document *)malloc(sizeof(document));
    assert(this);
    this->vector = vector_create(string_copy_constructor, string_destructor,
                                 string_default_constructor);
    return this;
}

void document_write_to_file(document *this, const char *path_to_file) {
    assert(this);
    assert(path_to_file);
    // see the comment in the header file for a description of how to do this!
    // TODO: your code here!
    
    FILE* convert = fopen(path_to_file, "w");

    for (size_t i = 0; i < vector_size(this->vector); i++)
    {
        fprintf(convert, "%s\n", vector_get(this->vector, i));
    }

    fclose(convert);

}

document *document_create_from_file(const char *path_to_file) {
    assert(path_to_file);
    // this function will read a file which is created by document_write_to_file
    // TODO: your code here!

    document* result = document_create();
    
    FILE* convert = fopen(path_to_file, "r");
    
    if(convert)
    {
        char* buffer = NULL;
        size_t size = 0;

        while(getline(&buffer, &size, convert) != -1)
        {
            char* line = (char*) malloc(strlen(buffer));
            strncpy(line, buffer, strlen(buffer));
            line[strlen(buffer)-1] = '\0';
            vector_push_back(result->vector, (void*)line);
            free(line);
        }

        free(buffer);
        fclose(convert);
    }

    return result;

}

void document_destroy(document *this) {
    assert(this);
    vector_destroy(this->vector);
    free(this);
}

size_t document_size(document *this) {
    assert(this);
    return vector_size(this->vector);
}

void document_set_line(document *this, size_t line_number, const char *str) {
    assert(this);
    assert(str);
    size_t index = line_number - 1;
    vector_set(this->vector, index, (void *)str);
}

const char *document_get_line(document *this, size_t line_number) {
    assert(this);
    assert(line_number > 0);
    size_t index = line_number - 1;
    return (const char *)vector_get(this->vector, index);
}

void document_insert_line(document *this, size_t line_number, const char *str) {
    assert(this);
    assert(str);
    // TODO: your code here!
    // How are you going to handle the case when the user wants to
    // insert a line past the end of the document?
    char* insert = strdup(str);

    if(line_number <= vector_size(this->vector))
    {
	    vector_insert(this->vector, line_number-1, insert);
        free(insert);
    }
    else
    {
	    for(size_t i = vector_size(this->vector); i < line_number-1; i++)
	    {
	        vector_push_back(this->vector, "");
	    }
	
	    vector_push_back(this->vector, insert);
        free(insert);
    }

}

void document_delete_line(document *this, size_t line_number) {
    assert(this);
    assert(line_number > 0);
    size_t index = line_number - 1;
    vector_erase(this->vector, index);
}
