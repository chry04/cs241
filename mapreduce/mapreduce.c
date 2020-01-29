/**
*  Lab
* CS 241 - Fall 2018
*/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include "utils.h"

void waiting(char* program, pid_t child)
{
    int status = 0;
    waitpid(child, &status,0);
    if(WEXITSTATUS(status))
    {
        print_nonzero_exit_status(program, WEXITSTATUS(status));
    }
}

int main(int argc, char **argv) {
    if(argc != 6)
    {
        print_usage();
    }
    // Create an input pipe for each mapper.
    int maps = atoi(argv[argc-1]);
    int** mapper_fd = (int**) malloc(maps*sizeof(int*));

    for(int i = 0; i < maps; i++)
    {
        int* tmp = (int*) malloc(sizeof(int) * 2);
        pipe2(tmp, O_CLOEXEC);
        mapper_fd[i] = tmp;


        descriptors_add(mapper_fd[i][0]);
        descriptors_add(mapper_fd[i][1]);
    }

    // Create one input pipe for the reducer.
    int reducer_fd[2];
    pipe2(reducer_fd, O_CLOEXEC);
    descriptors_add(reducer_fd[0]);
    descriptors_add(reducer_fd[1]);

    // Open the output file.
    int output = open(argv[2], O_CREAT | O_WRONLY | O_TRUNC, S_IWUSR | S_IRUSR);

    pid_t* splitter_child = (pid_t*) malloc(sizeof(pid_t)*maps);
    pid_t* map_child = (pid_t*) malloc(sizeof(pid_t)*maps);
    pid_t reducer_child = 0;

    // Start a splitter process for each mapper.

    char id[2];
    for(int i = 0; i < maps; i++)
    {
        pid_t child = fork();

        if(child)
        {
            splitter_child[i] = child;            
        }
        else
        {
            dup2(mapper_fd[i][1], STDOUT_FILENO);

            sprintf(id, "%d", i);
            id[1] = '\0';

            execlp("./splitter", "./splitter", argv[1], argv[5], id, (char*)NULL);

            exit(3);
        }
    }

    // Start all the mapper processes.
    
    for(int i = 0; i < maps; i++)
    {
        pid_t child = fork();

        if(child)
        {
            map_child[i] = child;

        }
        else
        {
            dup2(mapper_fd[i][0], STDIN_FILENO);
            dup2(reducer_fd[1], STDOUT_FILENO);

            execlp(argv[3], argv[3], NULL);
            exit(3);
        }
    }

    // Start the reducer process.
    pid_t child = fork();

    // Wait for the reducer to finish.
    if(child)
    {
        reducer_child = child;
    }
    else
    {
        dup2(reducer_fd[0], STDIN_FILENO);
        dup2(output, STDOUT_FILENO);

        execlp(argv[4], argv[4], NULL);
        exit(3);
    }

    descriptors_closeall();
    descriptors_destroy();

    for(int i = 0; i < maps; i++)
    {
        waiting("./splitter", splitter_child[i]);
        waiting(argv[3], map_child[i]);
    }

    waiting(argv[4], reducer_child);

    // Count the number of lines in the output file.
    //close(reducer_fd[1]);
    
    close(output);

    
    //print_num_lines(argv[2]);

    for(int i = 0; i < maps; i++)
    {
        free(mapper_fd[i]);
    }
    free(mapper_fd);

    free(splitter_child);
    free(map_child);

    
    return 0;
}
