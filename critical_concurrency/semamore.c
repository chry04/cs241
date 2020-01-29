/**
* Critical Concurrency Lab
* CS 241 - Fall 2018
*/

#include <stdlib.h>
#include "semamore.h"

/**
 * Initializes the Semamore. Important: the struct is assumed to have been
 * allocated by the user.
 * Example:
 * 	Semamore *s = malloc(sizeof(Semamore));
 * 	semm_init(s, 5, 10);
 *
 */
void semm_init(Semamore *s, int value, int max_val) {
    /* Your code here */
    s -> value = value;
    s -> max_val = max_val;

    pthread_mutex_t m = PTHREAD_MUTEX_INITIALIZER;
    pthread_cond_t cv = PTHREAD_COND_INITIALIZER;

    s -> m = m;
    s -> cv = cv;
}

/**
 *  Should block when the value in the Semamore struct (See semamore.h) is at 0.
 *  Otherwise, should decrement the value.
 */
void semm_wait(Semamore *s) {
    /* Your code here */
    if(s -> value > 0)
    {
        s -> value--;
        pthread_cond_broadcast(&(s->cv));
        return;
    }
    else
    {
        pthread_mutex_lock(&(s->m));
        while (s -> value == 0)
        {
            pthread_cond_wait(&(s->cv), &(s->m));
        }

        s -> value--;

        pthread_mutex_unlock(&(s->m));
        return;
    }
}

/**
 *  Should block when the value in the Semamore struct (See semamore.h) is at
 * max_value.
 *  Otherwise, should increment the value.
 */
void semm_post(Semamore *s) {
    /* Your code here */
    if(s -> value < s -> max_val)
    {
        s -> value++;
        pthread_cond_broadcast(&(s->cv));

        return;
    }
    else
    {
        pthread_mutex_lock(&(s->m));
        while (s -> value == s -> max_val)
        {
            pthread_cond_wait(&(s->cv), &(s->m));
        }

        s -> value++;

        pthread_mutex_unlock(&(s->m));
        return;
    }

}

/**
 * Takes a pointer to a Semamore struct to help cleanup some members of the
 * struct.
 * The actual Semamore struct must be freed by the user.
 */
void semm_destroy(Semamore *s) {
    /* Your code here */
    pthread_mutex_destroy(&(s->m));
    pthread_cond_destroy(&(s->cv));
}
