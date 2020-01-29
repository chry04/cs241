/**
* Mini Valgrind Lab
* CS 241 - Fall 2018
*/

#include <stdio.h>
#include <stdlib.h>

int main() {
    // Your tests here using malloc and free
    void *p1 = realloc(NULL, 1);
    fprintf(stderr, "finish1");
    void *p2 = realloc(NULL, 1);
    fprintf(stderr, "finish2");
    void *p3 = realloc(NULL, 1);
    void *p4 = realloc(NULL, 1);
    void *p5 = realloc(NULL, 1);
    void *p6 = realloc(NULL, 1);
    void *p7 = realloc(NULL, 1);
    free(p7);
    free(p5);
    free(p6);
    free(p2);
    free(p1);
    free(p3);
    free(p4);
    return 0;
}
