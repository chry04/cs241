/**
* Pointers Gone Wild Lab
* CS 241 - Spring 2018
*/

#include "part2-functions.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/**
 * (Edit this function to print out the "Illinois" lines in
 * part2-functions.c in order.)
 */
int main() {
    // your code here
    first_step(81);
    
    int second = 132;
    second_step(&second);

    int num = 8942;
    int* third = &num;
    double_step(&third);
    
    char strange[9];
    int* tmp = (int*) (strange+5);
    *tmp = 15;
    strange_step(strange);
    
    char value[4];
    value[3] = 0;
    empty_step(value);
    
    char* s = "aeiu";
    two_step(s, s);
    
    char* first_step = "a";
    char* second_step = first_step + 2;
    char* third_step = second_step + 2;
    three_step(first_step, second_step, third_step);

    char first_step_s[3];
    char second_step_s[4];
    char third_step_s[5];
    first_step_s[1] = 0;
    second_step_s[2] = 8;
    third_step_s[3] = 16;
    step_step_step(first_step_s, second_step_s, third_step_s);

    char a[1];
    *a = 10;
    int b = 10;
    it_may_be_odd(a, b);

    char str[12] = "cc,CS241,AB";
    tok_step(str);

    char orange[5];
    orange[0] = 1;
    orange[1] = 0;
    orange[2] = 0;
    orange[3] = 2;
    the_end(orange, orange);

    return 0;
}
