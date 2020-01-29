/**
* Mad Mad Access Patterns Lab
* CS 241 - Fall 2018
*/

#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <string.h>
#include <unistd.h>
#include "tree.h"
#include "utils.h"

/*
  Look up a few nodes in the tree and print the info they contain.
  This version uses mmap to access the data.

  ./lookup2 <data_file> <word> [<word> ...]
*/

int main(int argc, char **argv) {
    if (argc < 3)
    {
        printArgumentUsage();
        exit(1);
    }

    int fd = open(argv[1], O_RDONLY);

    if(fd == -1)
    {
        openFail(argv[1]);
        exit(0);
    }

    struct stat info;
    stat(argv[1], &info);

    size_t file_size = (size_t) info.st_size;

    char** list = argv+2;
    void* file = mmap(NULL, file_size, PROT_READ, MAP_PRIVATE, fd, 0);

    char buffer[5];

    strncpy(buffer, file, 4);
    buffer[4] = '\0';

    if(strcmp(buffer, "BTRE") != 0)
    {
        formatFail(argv[1]);
        exit(2);
    }
    
    //char* node_word =((BinaryTreeNode*) (file+4)) -> word;
    //int compare = strcmp(node_word, list[0]);

    for(int i = 0; i < argc-2; i++)
    {
        off_t offset = 4;
        char* node_word =((BinaryTreeNode*) (file+offset)) -> word;
        int compare = strcmp(node_word, list[i]);

        //printf("node word: %s, compare: %d\n", node_word, compare);

        while(compare != 0)
        {                     
            //printf("node word: %s, offset: %ld, compare: %d\n", node_word, offset, compare);

            if(compare > 0)
            {
                if(!(((BinaryTreeNode*) (file+offset)) -> left_child))
                {
                    //printf("cannot find left\n");
                    break;
                }
                offset = ((BinaryTreeNode*) (file+offset)) -> left_child;
            }
            else if(compare < 0)
            {
                if(!(((BinaryTreeNode*) (file+offset))  -> right_child))
                {
                    //printf("cannot find right\n");

                    break;
                }

                offset = ((BinaryTreeNode*) (file+offset))  -> right_child;
            }
            
            node_word = ((BinaryTreeNode*) (file+offset)) -> word;
            compare = strcmp(node_word, list[i]);
        }

        if(compare == 0)
        {
            printFound(list[i], ((BinaryTreeNode*) (file+offset)) -> count, 
                    ((BinaryTreeNode*) (file+offset)) -> price);
        }
        else
        {
            printNotFound(list[i]);
        }


    }


    munmap(file, file_size);
    close(fd);
    
    
    return 0;
}
