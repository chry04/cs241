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
#include "queue.h"
#include "cracker2.h"
#include "format.h"
#include "utils.h"

static size_t size = 0;
static size_t success = 0;
static size_t fail = 0;
static size_t total_hash = 0;
static size_t current = 0;
static int known = 0;
static int length = 0;

static int start_creck = 0;
static int end = 0;
static int found = 0;

static double start_time = 0;
static double start_cpu = 0;

static char* password = NULL;
char** parts = NULL;
static char* username = NULL;
static char* hash_value = NULL;
static char* partial_string = NULL;
pthread_mutex_t m = PTHREAD_MUTEX_INITIALIZER;
pthread_barrier_t barrier;

typedef struct threads_t
{
    pthread_t id;
    int num;
    size_t thread_count;   
}threads;

void* creck_password(void* info)
{
    threads* data = (threads*) info;

    while(1)
    {        
        pthread_mutex_lock(&m);
        if(current >= size-1)
        {
            pthread_mutex_unlock(&m);
            break;
        }
        if(start_creck == 0)
        {
            start_time = getTime();
            start_cpu = getCPUTime();
            
            //fprintf(stderr, "current: %zu\n", current);
            char* line = parts[current];

            char* saveptr = NULL;
            username = strtok_r(line, " ", &saveptr);
            hash_value = strtok_r(NULL, " ", &saveptr);
            partial_string = strtok_r(NULL, " ", &saveptr);
            v2_print_start_user(username);

            known = getPrefixLength(partial_string);
            length = (int) strlen(partial_string);
            password = (char*) malloc(length+1);
            strncpy(password, partial_string, known);
            memset(password+known, 'z', length-1-known);
            password[length-1] = '\0';

            start_creck = 1;
            found = 0;
        }
        pthread_mutex_unlock(&m);

        pthread_barrier_wait(&barrier);

        pthread_mutex_lock(&m);
        end = 0;
        start_creck = 0;
        pthread_mutex_unlock(&m);

        int hashes = 0;
        char* crecked = NULL;
        
        char* try = (char*) calloc(length+1, sizeof(char));
        try[length] = '\0';
        
        strncpy(try, partial_string, known);
        
        char* unknown = (char*) malloc(length-known);
        unknown[length-1-known] = '\0';
        memset(unknown, 'a', length-1-known);

        //fprintf(stderr, "unknown: %s, known: %d, length: %d\n", unknown, known, length);
        
        long start_index = 0;
        long count = 0;

        getSubrange(length-1-known, data->thread_count, data->num, &start_index, &count);
        setStringPosition(unknown, start_index);
        //fprintf(stderr, "%lu\n", count);

        long increase = 0;
        strncpy(try+known, unknown, length-1-known);

        v2_print_thread_start(data -> num, username, start_index, try);
        pthread_barrier_wait(&barrier);

        struct crypt_data cdata = {0};
       
        while(increase < count)
        {
            pthread_mutex_lock(&m);
            if(found == 1)
            {
                v2_print_thread_result(data->num, hashes, 1);
                pthread_mutex_unlock(&m);

                //fprintf(stderr, "cancelled\n");
                break;
            }
            pthread_mutex_unlock(&m);

            strncpy(try+known, unknown, length-1-known);
            try[length] = '\0';
            
            crecked = crypt_r(try, "xx", &cdata);
            hashes++;

            if(strcmp(crecked, hash_value) == 0)
            {
                //fprintf(stderr, "found\n");
                //found = 0;

                pthread_mutex_lock(&m);
                v2_print_thread_result(data->num, hashes, 0);
                success++;
                found = 1;
                strncpy(password, try, length-1);
                pthread_mutex_unlock(&m);
                
                //fprintf(stderr, "found\n");
                break;
            }
            
            incrementString(unknown);
            increase++;
            //fprintf(stderr, "unknown: %s, try: %s, count: %lu, increase: %lu, thread_id: %d\n", unknown, try, count, increase, data->num);

        }


        free(unknown);

        
        if(found == 0)
        {
            pthread_mutex_lock(&m);
            v2_print_thread_result(data->num, hashes, 2);
            fail++;
            pthread_mutex_unlock(&m);
            
        }

        
        pthread_mutex_lock(&m);
        total_hash += hashes;
        pthread_mutex_unlock(&m);

        free(try);
        
        pthread_barrier_wait(&barrier);

        pthread_mutex_lock(&m);
        if(end == 0)
        {
            current++;
            double timeElapsed = getTime() - start_time;
            double totalCPUTime = getCPUTime() - start_cpu;
            int result = 0;
            if(found == 0)
            {
                result = 1;
            }
            v2_print_summary(username, password, total_hash, timeElapsed, totalCPUTime, result);
            end = 1;
            found = 0;
            free(password);
        }
        pthread_mutex_unlock(&m);      
    }

    return NULL;
}


int start(size_t thread_count) {
    // TODO your code here, make sure to use thread_count!
    // Remember to ONLY crack passwords in other threads
    parts = (char**) malloc(8*sizeof(char*));
    size_t capacity = 8;

    char* buffer = NULL;
    size_t buffer_size = 0;
    ssize_t check = 0;
    pthread_barrier_init(&barrier, NULL, thread_count);

    while(check != -1 && !feof(stdin))
    {
        check = getline(&buffer, &buffer_size, stdin);
        
        if(size > capacity)
        {
            capacity *= 2;
            parts = (char**) realloc(parts, capacity*sizeof(char*));
        }

        parts[size] = strdup(buffer);

        //fprintf(stderr, "parts[%zu]%s\n", size, parts[size]);
        
        size++;
    }

    parts = (char**) realloc(parts, size*sizeof(char*));
    

    threads* thread_list = (threads*) malloc(thread_count*sizeof(threads));
    
    
    for(size_t i = 0; i < thread_count; i++)
    {
        thread_list[i].num = i+1;
        thread_list[i].thread_count = thread_count;
        
        pthread_create(&(thread_list[i].id), NULL, creck_password, &thread_list[i]);
    }

    void* status;

    for(size_t i = 0; i < thread_count; i++)
    {
        pthread_join(thread_list[i].id, &status);
    }

    for(size_t i = 0; i < size; i++)
    {
        free(parts[i]);
    }

    free(parts);

    free(buffer);

    free(thread_list);

    pthread_mutex_destroy(&m);
    pthread_barrier_destroy(&barrier);
    
    return 0; // DO NOT change the return code since AG uses it to check if your
              // program exited normally
}
