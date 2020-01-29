/**
* Extreme Edge Cases Lab
* CS 241 - Fall 2018
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "camelCaser.h"
#include "camelCaser_tests.h"

/*
 * Testing function for various implementations of camelCaser.
 *
 * @param  camelCaser   A pointer to the target camelCaser function.
 * @param  destroy      A pointer to the function that destroys camelCaser
 * output.
 * @return              Correctness of the program (0 for wrong, 1 for correct).
 */
int test_camelCaser(char **(*camelCaser)(const char *),
                    void (*destroy)(char **)) {
    // TODO: Return 1 if the passed in function works properly; 0 if it doesn't.

    //sample test
    char* sample = "The Heisenbug is an incredible creature. Facenovel servers get their power from its indeterminism. Code smell can be ignored with INCREDIBLE use of air freshener. God objects are the new religion.";
    char** sampleTest = camelCaser(sample);
    //expected outcome
    char* sample0 = "theHeisenbugIsAnIncredibleCreature";
    char* sample1 = "facenovelServersGetTheirPowerFromItsIndeterminism";
    char* sample2 = "codeSmellCanBeIgnoredWithIncredibleUseOfAirFreshener";
    char* sample3 = "godObjectsAreTheNewReligion";
    
    int len_sample_test = 1;

    while(sampleTest[len_sample_test-1])
    {
        len_sample_test++;
    }

    if(len_sample_test != 5)
    {
	return 0;
    }



    if(strcmp(sampleTest[0], sample0) || strcmp(sampleTest[1], sample1)
           || strcmp(sampleTest[2], sample2) || strcmp(sampleTest[3], sample3)
	   || sampleTest[len_sample_test-1])
    {
	return 0;
    }

    destroy(sampleTest);

    //printf("pass sample\n");
    
    //test contain multiple punctuation and ascii in middle
    char* test1 = "The Heisenbug is an incredible creature. ,, Facenovel, servers get their power from its indeterminism. Code smel}l can be ignored with \bINCRED!!IBLE use of air freshener. God objects are the new religion.";
    char** result1 = camelCaser(test1);
    char* result10 = "theHeisenbugIsAnIncredibleCreature";
    char* result11 = "facenovel";
    char* result12 = "serversGetTheirPowerFromItsIndeterminism";
    char* result13 = "codeSmel";
    char* result14 = "lCanBeIgnoredWith\bIncred";
    char* result15 = "ibleUseOfAirFreshener";
    char* result16 = "godObjectsAreTheNewReligion";

    len_sample_test = 1;

    while(result1[len_sample_test-1])
    {
        len_sample_test++;
   }

    if(len_sample_test != 11)
    {
	return 0;
    }

    
    //printf("%s\n", result1[0]);
    //printf("%s\n", result1[3]);
    //printf("%s\n", result1[4]);
    //printf("%s\n", result1[5]);
    //printf("%s\n", result1[6]);
    //printf("%s\n", result1[8]);
    //printf("%s\n", result1[9]);


    if(strcmp(result1[0], result10) || strcmp(result1[3], result11)
	   || strcmp(result1[1], "")  || strcmp(result1[2], "")
           || strcmp(result1[4], result12) || strcmp(result1[5], result13)
	   || strcmp(result1[6], result14) || strcmp(result1[8], result15)
	   || strcmp(result1[9], result16) || strcmp(result1[7], "")
           || result1[len_sample_test-1])
    {
	return 0;
    }

    destroy(result1);

    //printf("pass test1\n");

    //test null
    char* null_test = NULL;
    char** result_null = camelCaser(null_test);

    if (result_null)
        return 0;

    destroy(result_null);
    
    //test blank
    char* blank = "";
    char** b_result = camelCaser(blank);

    len_sample_test = 1;

    while(b_result[len_sample_test-1])
    {
        len_sample_test++;
   }

    if(len_sample_test != 1)
    {
	return 0;
    }


    if(b_result[0])
	return 0;

    destroy(b_result);

    //test blank2
    char* blank2 = "    gq6ifyhug9w448gbsr43     43t9w37ghdsu     ";
    char** b2_result = camelCaser(blank2);

    len_sample_test = 1;

    while(b2_result[len_sample_test-1])
    {
        len_sample_test++;
    }

    if(len_sample_test != 1)
    {
	return 0;
    }

    if(b2_result[0])
	return 0;

    destroy(b2_result);

    //space test and ascii in begin and end and not complete sentence
    char* spaces = "The Heisenbug is an incr   \nedible creature. Facenovel             servers get their power from its indeterminism. Code smell can be ig\t   nored with INCRED\bIBLE\b use of air freshener            .            God objects are the new religion. sdtfvgbh";
    char** space_t = camelCaser(spaces);
    char* s0 = "theHeisenbugIsAnIncrEdibleCreature";
    char* s1 = "facenovelServersGetTheirPowerFromItsIndeterminism";
    char* s2 = "codeSmellCanBeIgNoredWithIncred\bible\bUseOfAirFreshener";
    char* s3 = "godObjectsAreTheNewReligion";

    len_sample_test = 1;

    while(space_t[len_sample_test-1])
    {
	len_sample_test++;
    }

    if(len_sample_test != 5)
    {
	return 0;
    }

    //match content
    if(strcmp(space_t[0], s0) || strcmp(space_t[1], s1)
           || strcmp(space_t[2], s2) || strcmp(space_t[3], s3)|| space_t[len_sample_test-1])
    {
	//printf("not match sample");
	return 0;
    }

    destroy(space_t);

    //printf("pass\n");

    //test numbers
    char* test2 = "234576898765   . 12345678asd2345678\n\t\ba   . The Hei56senbug is an incredible creature. Facenovel servers get their power from its indetermi45555nism. Code smell can be ignor6576ed 567o89with46668674 INCREDIBLE use of air freshener.\" 4546789 dxcfgvhb";
    char** result2 = camelCaser(test2);
    char* result20 = "234576898765";
    char* result21 = "12345678asd2345678\bA";
    char* result22 = "theHei56senbugIsAnIncredibleCreature";
    char* result23 = "facenovelServersGetTheirPowerFromItsIndetermi45555nism";
    char* result24 = "codeSmellCanBeIgnor6576ed567O89with46668674IncredibleUseOfAirFreshener";
    char* result25 = "";

    len_sample_test = 1;

    while(result2[len_sample_test-1])
    {
        len_sample_test++;
    }

    if(len_sample_test != 7)
    {
        //printf("length match\n");
	return 0;
    }

    //printf("%s\n", result2[0]);
    //printf("%s\n", result2[1]);
    //printf("%s\n", result2[2]);
    //printf("%s\n", result2[3]);
    //printf("%s\n", result2[4]);
    //printf("%s\n", result2[5]);


    if(strcmp(result2[0], result20) || strcmp(result2[1], result21)
           || strcmp(result2[2], result22) || strcmp(result2[3], result23)
	   || strcmp(result2[4], result24) || strcmp(result2[5], result25)
	   || result2[len_sample_test-1])
    {
	return 0;
    }

    destroy(result2);

    //printf("pass number\n");

    //mixed words
    char* mix = "632748ysbhcn\b. 67BIBBBbbb32tr3 657bsGVGb32\b  \b\b\bFYTGVyv\b567. \x1gh \x5uf. \x2; \x3\x4;6578   34  23456 \b\b\b\b435\b\b\b >";
    char** mix_r = camelCaser(mix);
    char* mix0 = "632748ysbhcn\b";
    char* mix1 = "67bibbbbbb32tr3657Bsgvgb32\b\b\b\bFytgvyv\b567";
    char* mix2 = "\x01gh\x05Uf";
    char* mix3 = "\x02";
    char* mix4 = "\x03\x04";
    char* mix5 = "65783423456\b\b\b\b435\b\b\b";

    len_sample_test = 1;

    while(mix_r[len_sample_test-1])
    {
        len_sample_test++;
    }

    if(len_sample_test != 7)
    {
        //printf("length match\n");
	return 0;
    }

    //printf("%s\n", mix_r[0]);
    //printf("%s\n", mix_r[1]);
    //printf("%s\n", mix_r[2]);
    //printf("%s\n", mix_r[3]);
    //printf("%s\n", mix_r[4]);


        if(strcmp(mix_r[0], mix0) || strcmp(mix_r[1], mix1)
           || strcmp(mix_r[2], mix2) || strcmp(mix_r[3], mix3)
	   || strcmp(mix_r[4], mix4) || strcmp(mix_r[5], mix5) || mix_r[len_sample_test-1])
    {
	return 0;
    }

    destroy(mix_r);


    return 1;
}
