/**
* Extreme Edge Cases Lab
* CS 241 - Fall 2018
*/

#include "camelCaser.h"
#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include <stdbool.h>

char **camel_caser(const char *input_str) {
    if (!input_str)
    {
        return NULL;
    }

    int sentence = 0;
    char* copy = (char*) malloc(strlen(input_str)+1);
    strcpy(copy, input_str);
   
    //find the size of result
    for (unsigned long i = 0; i < strlen(input_str); i++)
    {
	if(ispunct(input_str[i]))
        {
	    sentence++;
	}
    }

    char** result = (char**) calloc(sentence+1, sizeof(char*));

    result[sentence] = NULL;
    int counter = 0;
    
    for(int i = 0; i < sentence; i++)
    {
	int word = 0;//length of the sentence without space
	int letter = 0;//length of words
	int capacity = 256;
	int alpha = 0;//number of characters
	
	char* line = (char*) calloc(256, sizeof(char));

	//skip the extra space
	while(isspace(copy[counter]))
	{
	    counter++;
	}

	copy[counter] = (char)tolower(copy[counter]);

	//set the first character to lowercase
	int init = 0;

	while(counter >= 0 && (unsigned long)(counter + init) <= strlen(copy) && !isalpha(copy[counter + init]))
	{
	    init++;
	}

	if(counter >= 0 && (unsigned long)(counter + init) <= strlen(copy) && isalpha(copy[counter+init]))
	{
	    copy[counter + init] = (char)tolower(copy[counter + init]);
	}
	
	//camelcase the sentence
        while((unsigned long)counter < strlen(copy) && !ispunct(copy[counter]))
	{
	    //make sure the capacity is enough
	    if(word+letter+1 > capacity) 
	    {
	        capacity *= 2;
	        line = (char*) realloc(line, capacity);
	    }

	    char tmp = copy[counter];

	    //new word
	    if (isspace(tmp))
	    {
		char* new_word = (char*) malloc (letter+1);
		strncpy(new_word, copy+counter-letter, letter);
		new_word[letter] = '\0';

		strcat(line, new_word);

		//printf("%s\n", new_word);

		free(new_word);
		new_word = NULL;

		int back = 1;

		while((unsigned long)(counter + back) <= strlen(copy) && !isalpha(copy[counter + back]))
		{
		    back++;
		}
                
		if((unsigned long)(counter + back) <= strlen(copy) )
		{
		    copy[counter+back] = (char) toupper(copy[counter+back]);
		}
		//printf("%d, %c\n", back, copy[counter+back]);

		alpha = 0;
		
		letter = 0;
	    }
	    else if (isalpha(tmp))//letters
	    {
	        if (alpha > 0)
		{
		    copy[counter] = (char) tolower(copy[counter]);
		}
		word++;
		letter++;
		alpha++;
	    }
	    else//ascii
	    {
	       //ascii in the front
	       if(counter >= 1 && isspace(copy[counter-1]) && isalpha(copy[counter+1])&& (unsigned long) counter <= strlen(copy))
	       {
	           line[word] = copy[counter];
	       }
	       else//ascii in the middle and end
	       {
		   letter++;
	       }

	       word++;

	    }
		
	    counter++;
	}

	//concate last word
	char* new_word = (char*) malloc (letter+1);
        strncpy(new_word, copy+counter-letter, letter);
	new_word[letter] = '\0';

	strcat(line, new_word);

	free(new_word);
	new_word = NULL;
	
	result[i] = (char*) realloc(line, word+1);
	result[i][word] = '\0';

	counter++;
	letter = 0;
	word = 0;
	alpha = 0;
	  
    }


    free(copy);
    
    return result;
}

void destroy(char **result) {
    if (!result)
	return;

    int i = 0;
    while (result[i])
    {
        free(result[i]);
	i++;
    }

    free(result);
    return;
}
