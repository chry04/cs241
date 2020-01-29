/**
* Mini Valgrind Lab
* CS 241 - Fall 2018
*/

#include "mini_valgrind.h"
#include <stdio.h>
#include <string.h>

meta_data *head = NULL;
size_t total_memory_requested = 0;
size_t total_memory_freed = 0;
size_t invalid_addresses = 0;

void *mini_malloc(size_t request_size, const char *filename,
                  void *instruction) {
    // your code here

    if(request_size == 0)
    {
	    return NULL;
    }

    //request memory
    meta_data* result = malloc(sizeof(meta_data)+request_size);
    result->request_size = request_size;
    result->filename = filename;
    result->instruction = instruction;
    result->next = NULL;
    
    //link the list
    if(!head)
    {
        head = result;
    }
    else
    {
        meta_data* ptr = head;
        while (ptr->next)
        {
            ptr = ptr->next;
        }

        ptr->next = result;//connect
    }


    total_memory_requested += request_size;
    
    return (void*)(result+1);
}

void *mini_calloc(size_t num_elements, size_t element_size,
                  const char *filename, void *instruction) {
    // your code here
    
    void* result = mini_malloc(num_elements*element_size, filename, instruction);

    char* tmp = (char*) result;

    //initialize
    for (size_t i = 0; i < num_elements*element_size; i++)
    {
        tmp[i] = 0;
    }

    return result;

}

void *mini_realloc(void *payload, size_t request_size, const char *filename,
                   void *instruction) {
    // your code here    
    meta_data* tmp = head;
    meta_data* next = NULL;
    int valid = 0;

    //search the pointer in the list d
    if (tmp+1 == payload)
    {
        valid = 1;
    }
    else if (tmp && tmp -> next)
    {
        while (tmp -> next != NULL)
        {
	        if((tmp->next)+1 == (meta_data*)payload)
            {
	            valid = 1;
                next = tmp;
	        }
    	    tmp = tmp->next;
        }
    }
    
    if(payload && valid == 0)
    {
	    invalid_addresses++;
        return NULL;
    }
    
    if(request_size == 0)
    {
        mini_free(payload);
        return NULL;
    }
    else if(request_size > 0)
    {
        int init = 0;
        meta_data* pt = (meta_data*)payload-1;

        if(!payload)
        {
            init = 1;
            pt = NULL;
        }

        meta_data* result = (meta_data*) realloc(pt, request_size+sizeof(meta_data));

        //add the variables
        if (init == 0)
        {
            meta_data* block = (meta_data*) payload - 1;

        
            if (request_size > block -> request_size)
            {
                total_memory_requested = total_memory_requested + request_size - block -> request_size;
            }
            else if (request_size < block -> request_size)
            {
                total_memory_freed = total_memory_freed - request_size + block -> request_size;
            }
        }
        else if (init == 1)
        {
            total_memory_requested = total_memory_requested + request_size;
            result -> next = NULL;

        }


        
        result -> request_size = request_size;
        result -> filename = filename;
        result -> instruction = instruction;

        if(!head)
        {
            head = result;
        }
        else if(next)
        {
            next -> next = result;
        }
        else
        {
            meta_data* ptr = head;
            while (ptr->next)
            {
                ptr = ptr->next;
            }

            ptr->next = result;

        }
        
        
        return (void*) (result+1);
    }

    return NULL;
}

void mini_free(void *payload) {
    // your code here
    if (payload == NULL)
    {
        return;
    }

    meta_data* tmp = head;
    int valid = 0;
    int begin = 0;

    if (tmp+1 == payload)
    {
        valid = 1;
        begin = 1;
    }
    else if(tmp && tmp -> next)
    {
        while (tmp->next != NULL)
        {
	        if((tmp->next)+1 == (meta_data*)payload)
            {
	            valid = 1;
                break;
    	    }

	        tmp = tmp->next;
        }
    }
    
    if(valid == 0)
    {
	    invalid_addresses++;
        return;
    }
    meta_data* block = (meta_data*) payload - 1;

    total_memory_freed = total_memory_freed + block -> request_size;

    //connect the pointers
    if(begin == 0)
    {
        meta_data* next_data = tmp->next;
        tmp->next = next_data->next;
        free(next_data);
        return;
    }

    if(tmp -> next)
    {
        head = tmp -> next;
    }
    else
    {
        head = NULL;
    }

    free(tmp);
     
}
