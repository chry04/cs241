/**
* Teaching Threads Lab
* CS 241 - Fall 2018
*/

#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "reduce.h"
#include "reducers.h"

/* You might need a struct for each task ... */
typedef struct reduces
{
    pthread_t id;
    
    int* list;
    size_t length;
    reducer reduce_func;
    int base_case;
    size_t num_threads;
    size_t i;
    int result;
}reduce_t;

/* You should create a start routine for your threads. */
void* run_reduce(void* threads)
{
    reduce_t* r = (reduce_t*) threads;
    int n = r->length/r->num_threads;
    size_t end = (r->i+1)*n;

    if(r->i == r->num_threads-1)
    {
        end = r->length;
    }

    size_t tmp_len = end-(r->i)*n;
    int* part = (int*) malloc((int)tmp_len*sizeof(int));

    for (size_t j = 0; j < tmp_len; j++)
    {
        part[j] = r->list[(r->i)*n+j];
        //printf("list: %zu, number: %zu = %d\n", r->i, j, part[j]);
    }

    int result = reduce(part, tmp_len, r->reduce_func ,r->base_case);
    r->result = result;
    //int* return_value = &result;

    free(part);

    return NULL;
}

int par_reduce(int *list, size_t list_len, reducer reduce_func, int base_case,
               size_t num_threads) {
    /* Your implementation goes here */    

    //void** tmp_list = (void**) malloc(num_threads*sizeof(void*));
    if(num_threads > list_len)
    {
        num_threads = list_len;
    }

    reduce_t** thread_list = (reduce_t**) malloc(num_threads*sizeof(reduce_t*));
    int* convert = (int*) malloc(num_threads*sizeof(int));
    void* status;
    
    for (size_t i = 0; i < num_threads; i++)
    {
        pthread_t thread_id = 0;
        reduce_t* reduce_info = (reduce_t*)malloc(sizeof(reduce_t));
    
        reduce_info->id = thread_id;
        reduce_info->list = list;
        reduce_info->length = list_len;
        reduce_info->reduce_func = reduce_func;
        reduce_info->base_case = base_case;
        reduce_info->num_threads = num_threads;
        reduce_info->i = i;

        thread_list[i] = reduce_info;

        pthread_create(&(thread_list[i]->id), NULL, run_reduce, thread_list[i]);
    }

    for(size_t i = 0; i < num_threads; i++)
    {
        pthread_join(thread_list[i]->id, &status);
    }
    
    
    for(size_t i = 0; i < num_threads; i++)
    {
        convert[i] = thread_list[i] -> result;
    }

    int result = reduce(convert, num_threads, reduce_func, base_case);

    for(size_t i = 0; i < num_threads; i++)
    {
        free(thread_list[i]);
    }

    free(thread_list);
    free(convert);

    return result;
}
