/**
* Password Cracker Lab
* CS 241 - Fall 2018
*/

#define _GUN_SOURCE

#include <string.h>
#include <pthread.h>
#include <crypt.h>
#include <stdlib.h>
#include <stdio.h>
#include "cracker1.h"
#include "format.h"
#include "queue.h"
#include "utils.h"

static int size = 0;
static size_t success = 0;
static size_t fail = 0;
pthread_mutex_t m = PTHREAD_MUTEX_INITIALIZER;

typedef struct threads_t
{
    pthread_t id;
    int num;
    queue* parts;    
}threads;

void* creck_password(void* info)
{
    threads* data = (threads*) info;

    while(1)
    {
        pthread_mutex_lock(&m);
        size--;
        if(size < 0)
        {
            //fprintf(stderr, "break\n");
            pthread_mutex_unlock(&m);
            break;
        }
        pthread_mutex_unlock(&m);

        double start_time = getTime();
        char* line = queue_pull(data->parts);
        
        char* saveptr = NULL;

        char* username = strtok_r(line, " ", &saveptr);
        char* hash_value = strtok_r(NULL, " ", &saveptr);
        char* partial_string = strtok_r(NULL, " ", &saveptr);

        
        v1_print_thread_start(data -> num, username);

        int known = getPrefixLength(partial_string);
        int increase = 1;
        int hashes = 0;
        char* crecked = NULL;
        int found = 1;
        int length = (int) strlen(partial_string);
        
        char* try = (char*) calloc(length+1, sizeof(char));
        try[length] = '\0';
        
        strncpy(try, partial_string, known);
        
        char* unknown = (char*) malloc(length-known);
        unknown[length-1-known] = '\0';
        memset(unknown, 'a', length-1-known);

        //fprintf(stderr, "begin find, %s thread: %d\n", username, data->num);
        struct crypt_data cdata = {0};
        
        while(increase)
        {
            strncpy(try+known, unknown, length-1-known);

            crecked = crypt_r(try, "xx", &cdata);
            hashes++;

            if(strcmp(crecked, hash_value) == 0)
            {
                //fprintf(stderr, "found, %s, thread: %d\n", try, data->num);

                found = 0;
                pthread_mutex_lock(&m);
                success++;
                pthread_mutex_unlock(&m);
                break;
            }
            
            increase = incrementString(unknown);

        }

        free(unknown);

        double timeElapsed = getTime() - start_time;

        if(found == 1)
        {
            pthread_mutex_lock(&m);
            fail++;
            pthread_mutex_unlock(&m);
        }

        //fprintf(stderr, "cannot end, thread: %d\n", data->num);

        v1_print_thread_result(data->num, username, try, hashes, timeElapsed, found);
        free(try);
        free(line);

        
    }

    return NULL;
}

int start(size_t thread_count) {
    // TODO your code here, make sure to use thread_count!
    // Remember to ONLY crack passwords in other threads

    queue* parts = queue_create(-1);

    char* buffer = NULL;
    size_t buffer_size = 0;
    ssize_t check = 0;

    check = getline(&buffer, &buffer_size, stdin);

    while(check != -1 && !feof(stdin))
    {
        queue_push(parts, strdup(buffer));

        //fprintf(stderr, "size: %zu, string: %s\n", size, buffer);
        size++;
        check = getline(&buffer, &buffer_size, stdin);
    }

    threads* thread_list = (threads*) malloc(thread_count*sizeof(threads));
    
    
    for(size_t i = 0; i < thread_count; i++)
    {
        thread_list[i].num = i+1;
        thread_list[i].parts = parts;
        
        pthread_create(&(thread_list[i].id), NULL, creck_password, &thread_list[i]);
    }

    void* status;

    for(size_t i = 0; i < thread_count; i++)
    {
        pthread_join(thread_list[i].id, &status);
    }

    
    //char* line = queue_pull(parts);
    //free(line);

    free(thread_list);
    free(buffer);
    queue_destroy(parts);

    //free(queue_pull(parts));

    v1_print_summary(success, fail);

    pthread_mutex_destroy(&m);
    
    return 0; // DO NOT change the return code since AG uses it to check if your
              // program exited normally
}
