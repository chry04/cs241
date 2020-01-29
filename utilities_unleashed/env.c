/**
* Utilities Unleashed Lab
* CS 241 - Fall 2018
*/

#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <stdio.h>
#include "format.h"

extern char** environ;

int main(int argc, char *argv[]) {
    int cmd_p = 0;//position of command
    int n = 0;
    int start = 0;
    
    for(int i = 0; i < argc; i++)
    {
	if(strcmp(argv[i], "--") == 0)
	{
	    cmd_p = i;
	    break;
	}

    }

    if(cmd_p == 0)//if there is no "--" sign
    {
	print_env_usage();
    }

    if(argc-cmd_p == 1)//if no program
    {
	print_env_usage();
    }

    if(strcmp(argv[1], "-n") == 0)//if n flag is on
    {
	char* end;
	n = (int) strtol(argv[2], &end, 0);//number of parameters

	if(*end)
	{
	    print_env_usage();
	}

	for(int i = 3; i < cmd_p; i++)
        {
	    int tmp = 0;
	    for(unsigned long j = 0; j < strlen(argv[i]); j++)
	    {
		if(argv[i][(int)j] == ',')
		{
		    tmp++;
	        }
	    }

	    if ((!strchr(argv[i], '%')) && tmp+1 != n)
	    {
		print_env_usage();
	    }
        }

	start = 3;

    }
    else //if -n flag is not on
    {
	for(int i = 1; i < cmd_p; i++)
        {
	    int tmp = 0;
	    for(unsigned long j = 0; j < strlen(argv[i]); j++)
	    {
		if(argv[i][(int)j] == ',')
		{
		    tmp++;
	        }
	    }

	    //check the number of arguments
	    if (n == 0||n==1)
	    {
		if (n == 0||n==1)
	        {
		    n = tmp+1;
		}
		else
		{
		    print_env_usage();
		}
	    }
	    else if(n>1)
	    {
		if ((tmp+1)!=1 && (tmp+1) != n)
		{
		    print_env_usage();
		}
	    }
        }

	start = 1;

    }



    char*** parameters = (char***) malloc((cmd_p-start)*sizeof(char**));
    char** vir_name = (char**) malloc((cmd_p-start)*sizeof(char*));

    //append the parameters into the dynamic array
    int decrease = 0;

    for (int i = start; i < cmd_p; i++)
    {
	char** tmp = (char**) malloc(n*sizeof(char*));//the line of argument
    	char* saveptr;
	vir_name[i-decrease-start] = strdup(strtok_r(argv[i], "=", &saveptr));

	char* rest =  strdup(strtok_r(NULL, "=", &saveptr));
	char* name = vir_name[i-decrease-start];
	char* token;

	//chenge variable name
	if(strchr(rest, '%'))
	{
    	    token = strtok_r(rest, "%", &saveptr);

	    for(int j = 1; j < i-start+1-decrease; j++)
	    {
		if(strcmp(vir_name[i-decrease-j-start], token) == 0)
		{
		    free(vir_name[i-decrease-start-j]);
		    vir_name[i-decrease-j-start] = strdup(name);
		}
	    }

	    decrease++;
	    free(name);
	    free(rest);
	    free(tmp);
	    continue;

	}

        
	//append the values
	int counter = 0;
	for(token = strtok_r(rest, ",", &saveptr); token != NULL;
			token = strtok_r(NULL, ",", &saveptr))
	{
	    if(counter >= n)
    	    {
	        print_env_usage();
	    }

	    tmp[counter] = strdup(token);

	    counter++;
	}

	//append the variable that only have one value
	if(counter == 1)
	{
	    for (int j = 1; j < n; j++)
	    {
		tmp[j] = strdup(tmp[j-1]);
	    }
	}

	free(rest);

	parameters[i-decrease-start] = tmp;

    }

    //if there is no argument
    if(cmd_p-decrease-start == 0)
    {
	pid_t child = fork();

	if (child == -1)
	{
	    print_fork_failed();
	}
	else if (child)
	{
	    int status;
	    waitpid(child, &status, 0);
	}
	else
	{
	    execvp(*(argv+cmd_p+1), (argv+cmd_p+1));

	    print_exec_failed();
	}
	
    }
    else//for the rest of program
    {  
        for(int i = 0; i < n; i++)
        {
	    pid_t child = fork();

	    if (child == -1)
	    {
	        print_fork_failed();
	    }
	    else if (child)
	    {
	        int status;
	        waitpid(child, &status, 0);
	    }
	    else
	    {
	        for(int j = 0; j < (cmd_p-decrease-start); j++)
	        {
		    int env = setenv(vir_name[j], parameters[j][i], 1);
		    if(env == -1)
		    {
		        print_environment_change_failed();
		    }

	        }

	        execvp(*(argv+cmd_p+1), (argv+cmd_p+1));

	        print_exec_failed();
	    }

        }
    }


    //free the parameter and vir_name    
    for (int i = 0; i < (cmd_p-decrease-start); i++)
    {
	for (int j = 0; j < n; j++)
	{
	    free(parameters[i][j]);
	}

	free(parameters[i]);
	free(vir_name[i]);
    }


    free(parameters);
    free(vir_name);


    return 0;
}
