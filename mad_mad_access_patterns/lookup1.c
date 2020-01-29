/**
* Mad Mad Access Patterns Lab
* CS 241 - Fall 2018
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "tree.h"
#include "utils.h"
/*
  Look up a few nodes in the tree and print the info they contain.
  This version uses fseek() and fread() to access the data.

  ./lookup1 <data_file> <word> [<word> ...]
*/

char* get_word(FILE* f)
{
    char c;
    char* word = (char*) malloc(20);
    int capacity = 20;
    int size = 0;

    c = fgetc(f);

    while(c != '\0')
    {
        if(size >= capacity)
        {
            capacity *= 2;
            word = (char*) realloc(word, capacity);
        }
        word[size] = c;
        size++;
        c = fgetc(f);
    }

    word[size] = '\0';
    word = (char*) realloc(word, size);
    

    return word;
}


int main(int argc, char **argv) {
    if (argc < 3)
    {
        printArgumentUsage();
        exit(1);
    }

    FILE* file = fopen(argv[1], "r");

    if(!file)
    {
         openFail(argv[1]);
         exit(0);
    }

    /*
    int size = 0;
    fseek(file, 0, SEEK_END);
    size = ftell(file);
    fseek(file, 0, SEEK_SET);
    */
    
    char** list = argv+2;
    char start[5];
    fgets(start, 5, file);

    if(strcmp(start, "BTRE") != 0)
    {
        formatFail(argv[1]);
        exit(2);
    }

    BinaryTreeNode* node = (BinaryTreeNode*) malloc(sizeof(BinaryTreeNode)+20);
    
    
    for(int i = 0; i < argc-2; i++)
    {
        off_t offset = 4;

        fseek((void*) file, offset, SEEK_SET);
        fread(node, sizeof(BinaryTreeNode), 1, file);
        char* node_word = get_word(file);

        int compare = strcmp(node_word, list[i]);
        free(node_word);

        while(compare != 0)
        {
                        
            if(compare > 0)
            {
                if(!(node -> left_child))
                {
                    break;
                }
                offset = node -> left_child;
            }
            else if(compare < 0)
            {
                if(!(node -> right_child))
                {
                    break;
                }

                offset = node -> right_child;
            }

            fseek(file, offset, SEEK_SET);
            fread((void*)node, sizeof(BinaryTreeNode), 1, file);
            
            node_word = get_word(file);
            compare = strcmp(node_word, list[i]);
            free(node_word);
        }

        if(compare == 0)
        {
            printFound(list[i], node -> count, node -> price);
        }
        else
        {
            printNotFound(list[i]);
        }
    }

    fclose(file);
    free(node);
    
    return 0;
}
