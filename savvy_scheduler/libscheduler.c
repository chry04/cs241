/**
* Savvy_scheduler Lab
* CS 241 - Fall 2018
*/

#include "libpriqueue/libpriqueue.h"
#include "libscheduler.h"

#include <assert.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include "print_functions.h"
typedef struct _job_info {
    int id;
    int order;
    double arrival;
    double start_time;
    double total_time;
    double priority;
    double remain;
    double resume;

    bool begin;

    /* Add whatever other bookkeeping you need into this struct. */
} job_info;

priqueue_t pqueue;
scheme_t pqueue_scheme;
comparer_t comparision_func;

double total_turnaround = 0;
double total_response = 0;
double total_wait_time = 0;
int turns = 0;
int job_num = 0;

void scheduler_start_up(scheme_t s) {
    switch (s) {
    case FCFS:
        comparision_func = comparer_fcfs;
        break;
    case PRI:
        comparision_func = comparer_pri;
        break;
    case PPRI:
        comparision_func = comparer_ppri;
        break;
    case PSRTF:
        comparision_func = comparer_psrtf;
        break;
    case RR:
        comparision_func = comparer_rr;
        break;
    case SJF:
        comparision_func = comparer_sjf;
        break;
    default:
        printf("Did not recognize scheme\n");
        exit(1);
    }
    priqueue_init(&pqueue, comparision_func);
    pqueue_scheme = s;
    // Put any set up code you may need here
}

static int break_tie(const void *a, const void *b) {
    return comparer_fcfs(a, b);
}

int comparer_fcfs(const void *a, const void *b) {
    job_info* a_i = (job_info*) (((job*) a) -> metadata);
    job_info* b_i = (job_info*) (((job*) b) -> metadata);

    if(a_i->arrival < b_i->arrival)
        return -1;
    else if (a_i->arrival > b_i->arrival)
        return 1;
    else
        return 0;
}

int comparer_ppri(const void *a, const void *b) {
    // Complete as is
    return comparer_pri(a, b);
}

int comparer_pri(const void *a, const void *b) {
    job_info* a_i = (job_info*) (((job*) a) -> metadata);
    job_info* b_i = (job_info*) (((job*) b) -> metadata);

    if(a_i -> priority < b_i -> priority)
        return -1;
    else if (a_i -> priority > b_i -> priority)
        return 1;
    else
        return break_tie(a, b);
}

int comparer_psrtf(const void *a, const void *b) {
    job_info* a_i = (job_info*) (((job*) a) -> metadata);
    job_info* b_i = (job_info*) (((job*) b) -> metadata);

    if(a_i->remain < b_i->remain)
        return -1;
    else if (a_i->remain > b_i->remain)
        return 1;
    else
        return break_tie(a, b);
}

int comparer_rr(const void *a, const void *b) {
    job_info* a_i = (job_info*) (((job*) a) -> metadata);
    job_info* b_i = (job_info*) (((job*) b) -> metadata);

    if(a_i->order < a_i->order)
        return -1;
    else if (a_i->order > b_i->order)
        return 1;
    else
        return break_tie(a, b);
}

int comparer_sjf(const void *a, const void *b) {
    job_info* a_i = (job_info*) (((job*) a) -> metadata);
    job_info* b_i = (job_info*) (((job*) b) -> metadata);

    if(a_i->total_time < b_i->total_time)
        return -1;
    else if (a_i->total_time > b_i->total_time)
        return 1;
    else
        return break_tie(a, b);
}

// Do not allocate stack space or initialize ctx. These will be overwritten by
// gtgo
void scheduler_new_job(job *newjob, int job_number, double time,
                       scheduler_info *sched_data) {
    // TODO complete me!
    newjob -> metadata = malloc(sizeof(job_info));
    job_info* data = newjob -> metadata;

    data -> id = job_number;
    data -> order = turns;
    data -> arrival = time;
    data -> total_time = sched_data -> running_time;
    data -> priority = sched_data -> priority;
    data -> remain = sched_data -> running_time;

    data -> begin = false;

    priqueue_offer(&pqueue, newjob);
    job_num++;
    turns++;
    
}

job *scheduler_quantum_expired(job *job_evicted, double time) {
    // TODO complete me!
    if(priqueue_size(&pqueue) == 0 && !job_evicted)
        return NULL;
    if(!job_evicted || (job_evicted && (pqueue_scheme == PPRI || 
                    pqueue_scheme == PSRTF || pqueue_scheme == RR)))
    {
       
        if(job_evicted)
        {
            job_info* evicted_info = (job_info*) job_evicted->metadata;


            if(pqueue_scheme == PSRTF)
                (evicted_info -> remain) -= (time - evicted_info -> resume);
            else if(pqueue_scheme == RR)
            {
                evicted_info -> order = turns;
                turns++;
            }
            
            priqueue_offer(&pqueue, job_evicted);

        }

        job* result = priqueue_poll(&pqueue);
        job_info* result_info = (job_info*) (result->metadata);
        if(!(result_info -> begin))
        {
            result_info -> begin = true;
            result_info -> start_time = time;
        }

        //printf("switching to job: %d, turn: %d\n", result_info->id, result_info -> order);

        result_info -> resume = time;

        return result;
    }
    else
    {
        return job_evicted;
    }
    return NULL;
}

void scheduler_job_finished(job *job_done, double time) {
    // TODO complete me!
    job_info* info = (job_info*)(job_done->metadata);
    
    total_turnaround += (time - info -> arrival);
    total_response += (info -> start_time - info -> arrival);
    total_wait_time += (time - info -> arrival - info -> total_time);
    free(job_done->metadata);

}

static void print_stats() {
    fprintf(stderr, "turnaround     %f\n", scheduler_average_turnaround_time());
    fprintf(stderr, "total_waiting  %f\n", scheduler_average_waiting_time());
    fprintf(stderr, "total_response %f\n", scheduler_average_response_time());
}

double scheduler_average_waiting_time() {
    // TODO complete me!
    return total_wait_time/job_num;
}

double scheduler_average_turnaround_time() {
    // TODO complete me!
    return total_turnaround/job_num;
}

double scheduler_average_response_time() {
    // TODO complete me!
    return total_response/job_num;
}

void scheduler_show_queue() {
    // Implement this if you need it!
}

void scheduler_clean_up() {
    priqueue_destroy(&pqueue);
    print_stats();
}
