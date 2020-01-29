/**
* Critical Concurrency Lab
* CS 241 - Fall 2018
*/

#include <stdlib.h>
#include <stdio.h>
#include "barrier.h"

// The returns are just for errors if you want to check for them.
int barrier_destroy(barrier_t *barrier) {
    int error = 0;
    //free(barrieri;
    pthread_mutex_destroy(&(barrier->mtx));
    pthread_cond_destroy(&(barrier -> cv));
    return error;
}

int barrier_init(barrier_t *barrier, unsigned int num_threads) {
    int error = 0;
    //barrier = (barrier_t*) malloc(sizeof(barrier_t));
    
    pthread_mutex_t m = PTHREAD_MUTEX_INITIALIZER;
    pthread_cond_t cv = PTHREAD_COND_INITIALIZER;

    barrier -> mtx = m;
    barrier -> cv = cv;

    barrier -> n_threads = num_threads;
    barrier -> count = num_threads;
    barrier -> times_used = 0;

    return error;
}

int barrier_wait(barrier_t *barrier) {

    pthread_mutex_lock(&(barrier -> mtx));    
    if (barrier -> count == 0 && barrier -> times_used == 0)
    {
        barrier -> count = barrier -> n_threads;

    }

    barrier -> count--; 

    if (barrier -> count == 0) 
    {
        pthread_cond_broadcast(&(barrier -> cv));
        barrier -> times_used++;

    }
    else {
        while (barrier -> count != 0) 
        { 
            pthread_cond_wait(&(barrier -> cv), &(barrier -> mtx)); 
            barrier -> times_used++;

        }
    }

    if(barrier -> times_used == barrier->n_threads)
    {
        barrier -> times_used = 0;
    }
    pthread_mutex_unlock(&barrier -> mtx);

    pthread_mutex_lock(&(barrier -> mtx));

    if (barrier -> times_used == 0 && barrier -> count == 0) 
    {
        pthread_cond_broadcast(&(barrier -> cv));
    }
    else
    {
        while(barrier->times_used != 0) 
        {
            pthread_cond_wait(&(barrier -> cv), &(barrier -> mtx)); 
        }
    }

    pthread_mutex_unlock(&(barrier -> mtx));



        
    return 0;
}
