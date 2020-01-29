/**
* Utilities Unleashed Lab
* CS 241 - Fall 2018
*/

#include <time.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include "format.h"

typedef struct timespec Time;

int main(int argc, char *argv[]){
    if(argc <= 1)
    {
	print_time_usage();
    }
    
    Time* result = (Time*) malloc(sizeof(Time));
    clock_gettime(CLOCK_MONOTONIC, result);
    pid_t child = fork();

    if (child == -1)
    {
        print_fork_failed();
    }
    else if (child)
    {
        int status;
	waitpid(child, &status, 0);
	double nano = (double)result -> tv_nsec/(double) 1000000000;
	double before = (double) result -> tv_sec + nano;
	clock_gettime(CLOCK_MONOTONIC, result);
	nano = (double)result -> tv_nsec/(double) 1000000000;
	double after = (double) result -> tv_sec + nano;
	display_results(argv, after - before);

	free(result);
	return 0;
    }
    else
    {
        char** run = argv+1;
	execvp(*run, run);

	print_exec_failed();
    }
    return 0;
}
