/**
* Critical Concurrency Lab
* CS 241 - Fall 2018
*/

#include "queue.h"
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>

/**
 * This queue is implemented with a linked list of queue_nodes.
 */
typedef struct queue_node {
    void *data;
    struct queue_node *next;
} queue_node;

struct queue {
    /* queue_node pointers to the head and tail of the queue */
    queue_node *head, *tail;

    /* The number of elements in the queue */
    ssize_t size;

    /**
     * The maximum number of elements the queue can hold.
     * max_size is non-positive if the queue does not have a max size.
     */
    ssize_t max_size;

    /* Mutex and Condition Variable for thread-safety */
    pthread_cond_t cv;
    pthread_mutex_t m;
};

queue *queue_create(ssize_t max_size) {
    /* Your code here */
    queue* result = (queue*) malloc(sizeof(queue));
    result -> size = 0;
    result -> max_size = max_size;

    result -> head = NULL;
    result -> tail = NULL;
    
    
    pthread_mutex_t m = PTHREAD_MUTEX_INITIALIZER;
    pthread_cond_t cv = PTHREAD_COND_INITIALIZER;

    result -> m = m;
    result -> cv = cv;

    return result;
}

void queue_destroy(queue *this) {
    /* Your code here */
    queue_node* tmp = this->head;
    
    while(tmp && tmp -> next)
    {
        queue_node* next_node = tmp -> next;
        free(tmp -> next);
        tmp -> next = next_node;
        
    }

    pthread_mutex_destroy(&(this->m));
    pthread_cond_destroy(&(this->cv));

    free(this);

}

void queue_push(queue *this, void *data) {
    /* Your code here */

        pthread_mutex_lock(&(this->m));

        if(this -> max_size > 0)
        {
            while (this -> size >= this -> max_size)
            {
                pthread_cond_wait(&(this->cv), &(this->m));
            }
        }

        queue_node* new_node = (queue_node*) malloc(sizeof(queue_node));
        new_node -> data = data;
        new_node -> next = NULL;

        if(!this-> head)
        {
            this -> head = new_node;
            this -> tail = new_node;
        }
        else
        {
            this -> tail -> next = new_node;
            this -> tail = new_node;
        }

        this -> size++;
        pthread_cond_broadcast(&this->cv);

        //fprintf(stderr,"%zu\n" ,this -> size);



        pthread_mutex_unlock(&(this->m));
        return;
}

void *queue_pull(queue *this) {
    /* Your code here */
    void* result = NULL;
        pthread_mutex_lock(&(this->m));
        while (this -> size <= 0 || !(this -> head))
        {
            pthread_cond_wait(&(this->cv), &(this->m));
        }

        if(this -> size > 0  && this -> head == this -> tail)
        {
            queue_node* tmp = this -> head;
            result = tmp -> data;
            this -> head = NULL;
            this -> tail = NULL;
            this -> size--;
            pthread_cond_broadcast(&this->cv);



            free(tmp);
        }
        else
        {
            queue_node* tmp = this -> head;
            result = tmp -> data;
            this -> head = this -> head -> next;
            this -> size--;
            pthread_cond_broadcast(&this->cv);



            free(tmp);
        }

        
        pthread_mutex_unlock(&(this->m));
        return result;
}
