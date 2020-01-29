/**
* Critical Concurrency Lab
* CS 241 - Fall 2018
*/

#include <assert.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "queue.h"

typedef struct information
{
    pthread_t id;
    //int num;   
    pthread_mutex_t m;
    queue* test;
    
}info;

void* pushes(void* threads)
{
    info* use = (info*) threads;
    
    pthread_mutex_lock(&(use->m));

    for(int i = 0; i < 100; i++)
    {

        int* tmp = (int*) malloc(sizeof(int));
        *tmp = i;
        queue_push(use->test, tmp);
        //free(tmp);
    }
    pthread_mutex_unlock(&(use->m));

    
    return NULL;
}


void* pulles(void* threads)
{
    info* use = (info*) threads;

    pthread_mutex_lock(&(use->m));

    for(int i = 0; i < 100; i++)
    {
        void* result = queue_pull(use->test);

        fprintf(stderr, "i:%d, result:%d\n", i, *((int*) result));

        free(result);
    }

    pthread_mutex_unlock(&(use->m));

    
    return NULL;
}


int main(int argc, char **argv) {
    if (argc != 2) {
        printf("usage: %s test_number\n", argv[0]);
        exit(1);
    }

    pthread_mutex_t m = PTHREAD_MUTEX_INITIALIZER;


    queue* test = queue_create(5);

   
    int n = atoi(argv[1]);

    info** thread_list = (info**) malloc(n*2*sizeof(info*));
    void* status;

    for(int i = 0; i< n; i++)
    {
        pthread_t thread_id = 0;
        info* queue_info = (info*)malloc(sizeof(info));

        queue_info -> test = test;
        queue_info -> id = thread_id;
        queue_info -> m = m;

        thread_list[i] = queue_info;

        pthread_create(&(queue_info->id), NULL, pushes, queue_info);
    }

    for(int i = 0; i< n; i++)
    {
        pthread_t thread_id = 0;
        info* queue_info = (info*)malloc(sizeof(info));

        queue_info -> test = test;
        queue_info -> id = thread_id;

        thread_list[i+n] = queue_info;

        pthread_create(&(queue_info->id), NULL, pulles, queue_info);
    }


    for(int i = 0; i < n*2; i++)
    {
        pthread_join(thread_list[i]->id, &status);
    }

    queue_destroy(test);

    for(int i = 0; i < n*2; i++)
    {
        free(thread_list[i]);
    }

    free(thread_list);


    fprintf(stderr, "end_tests\n");

    
    return 0;
}
