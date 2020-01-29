/**
* Shell Lab
* CS 241 - Fall 2018
*/

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/wait.h>
#include "format.h"
#include "shell.h"
#include "vector.h"
#include "sstring.h"
#include "string.h"

typedef struct process {
    char *command;
    char *status;
    pid_t pid;
} process;


static vector* current_p;//global vector to track process
static int foreground = 0;

int run_command(const char* commands, vector* line, vector* history, FILE* input, FILE* history_file);
int logic_expression(char* buffer, char* logic, int logic_type, FILE* input, vector* history, FILE* history_file);

void destroy_process(process* p)
{
    free(p->command);
    free(p);
}

void destroy_pid(pid_t pid)
{


    for(size_t i = 0; i < vector_size(current_p); i++)
    {
        process* p = (process*) vector_get(current_p, i);
        
        if(p->pid == pid)
        {
            destroy_process(p);
            vector_erase(current_p, i);

            return;
        }

    }

}

void handle_sigint(int signal)
{
    return;

}

void cleanup(int signal)
{
    pid_t child = 0;

    while ((child = waitpid((pid_t) (-1), 0, WNOHANG)) > 0) 
    {
        destroy_pid(child);
    }
}

int command_cd(vector* line)
{
    if(vector_size(line) != 2)
    {
        print_no_directory("");
        return 1;
    }
    else
    {
        int success = chdir(vector_get(line, 1));

        if(success == -1)
        {
            print_no_directory(vector_get(line, 1));
            return 1;
        }
        else
        {
            return 0;
        }        
    }

    return 1;
}

void command_history(vector* history)
{
    for (size_t i = 0; i < vector_size(history); i++)
    {
        char* command = vector_get(history, i);
        print_history_line(i, command);
    }
}

void command_ps(vector* current_p)
{
    print_process_info("Running", getpid(), "shell");


    for(size_t i = 0; i < vector_size(current_p); i++)
    {
        process* p = (process*) vector_get(current_p, i);
        print_process_info(p->status, p->pid, p->command);
    }
}

int run_past_command(int n, vector* history, FILE* input, FILE* history_file)
{
    char* past = vector_get(history, n);
    print_command(past);
    sstring* p_line = cstr_to_sstring(past);
    
    vector* line = sstring_split(p_line, ' ');
    sstring_destroy(p_line);
    
    char* logic = NULL;
        int logic_type = 0;
        int result = 1;

        if((logic = strstr(past, "&&")))
        {
            logic_type = 1;
            result = logic_expression(past, logic, logic_type, input, history, history_file);

        }
        else if((logic = strstr(past, "||")))
        {
            logic_type = 2;
            result = logic_expression(past, logic, logic_type, input, history, history_file);
        }
        else if((logic = strstr(past, ";")))
        {
            logic_type = 3;
            result = logic_expression(past, logic, logic_type, input, history, history_file);
        }
        else
        {

            vector_push_back(history, (char*) past);


    result = run_command(past, line, history, input, history_file); 
  }   

    vector_destroy(line);

    return result;

}

int valid_signals(const char* commands, vector* line, pid_t pid, size_t* index, process** p)
{
    if(vector_size(line) != 2)
    {
        print_invalid_command(commands);
        return 1;
    }

    int found = 0;
        
    for(size_t i = 0; i < vector_size(current_p); i++)
    {
        *p = (process*) vector_get(current_p, i);

        if((*p)->pid == pid)
        {
            found = 1;
            *index = i;
        }
    }

    if(found == 0)
    {
        print_no_process_found(pid);
        return 1;
    }

    return 0;
}

void command_exit(vector* history, FILE* input, FILE* history_file)
{
    if(history_file)
    {
        for (size_t i = 0; i < vector_size(history); i++)
        {
            char* command = (char*) vector_get(history, i);
            fprintf(history_file, "%s\n", command);
        }
        
        fclose(history_file);
    }
    
    vector_destroy(history);

    for(size_t i = 0; i < vector_size(current_p); i++)
    {
        process* p = (process*) vector_get(current_p, i);
        kill(p->pid, SIGTERM);

        destroy_process(p);
    }

    vector_destroy(current_p);

    if(input != stdin)
        fclose(input);

    exit(0);
}

int external_commands(const char* commands, vector* line, vector* history)
{
    char* c = (char*) vector_get(line, 0);
    
    int background = 0;

    char* word = (char*) vector_get(line, vector_size(line)-1);

    if (strcmp(word, "&") == 0)
    {
        background = 1;
        vector_pop_back(line);
    }
    else if(word[strlen(word)-1] == '&')
    {
        word[strlen(word)-1] = '\0';
        background = 1;

    }

    char** command_array = (char**) malloc((vector_size(line)+1)*sizeof(char*));
    command_array[vector_size(line)] = NULL;

    for(size_t i = 0; i < vector_size(line); i++)
    {
        command_array[i] = strdup(vector_get(line, i));
    }

    pid_t child = fork();

        
    if(child == -1)
    {
        print_fork_failed();
        exit(1);
    }
    else if (child)
    {
        print_command_executed(child);
        
        foreground = 1;
        int status;

        
        for(size_t i = 0; i < vector_size(line)+1; i++)
        {
            free(command_array[i]);
        }
        free(command_array);

        if (background == 0)
        {

            int wait_r = waitpid(child, &status, 0);


            if(wait_r == -1)
            {
                print_wait_failed();
                exit(1);
            }


            int exit_value = WEXITSTATUS(status);
            foreground = 0;

            if((exit_value) == 1)
            {
                return 1;
            }
            return 0;
        }
        else
        {
            
            process* p = (process*) malloc(sizeof(process));
            p->command = strdup(commands);
            p->status = "Running";
            p->pid = child;

            vector_push_back(current_p, p);

            if (setpgid(child, child) == -1) 
            {
                print_setpgid_failed();
                exit(1);
            }
        }

        return 1;

    }
    else
    {     
        execvp(c, command_array);

        print_exec_failed(commands);
        exit(1);
        
    }

}

//if success, return 0; else return 1
int run_command(const char* commands, vector* line, vector* history, FILE* input, FILE* history_file)
{
        
    char* c = (char*) vector_get(line, 0);
        
    if(strcmp(c, "exit") == 0)
    {
        vector_destroy(line);
        command_exit(history, input, history_file);
    }
    else if(strcmp(c, "cd") == 0)
    {
        vector_push_back(history, (char*) commands);
        return command_cd(line);
    }
    else if(strcmp(c, "!history") == 0)
    {
        command_history(history);
        return 0;
    }
    else if(c[0] == '#')
    {
        char* number = (char*) malloc(strlen(c));
        strncpy(number, c+1, strlen(c));
        int n = atoi(number);
        free(number);

        if(n < 0 || (size_t)n > vector_size(history))
        {
            print_invalid_index();
            return 1;
        }

        return run_past_command(n, history, input, history_file);

    }
    else if(c[0] == '!')
    {
        int find = 0;
        int n = -1;
         
        for(size_t i = vector_size(history)-1; i > 0; i--)
        {
            if(strstr(vector_get(history, i), c+1))
            {
                find = 1;
                n = i;
                break;
            }
        }

        if(find == 0 && strstr(vector_get(history, 0), c+1))
        {
            find = 1;
            n = 0;
        }


        if(n == -1)
        {
            print_no_history_match();
            return 1;
        }

        return run_past_command(n, history, input, history_file);

    }
    else if(strcmp(c, "ps") == 0)
    {
        if(vector_size(line) != 1)
        {
            print_invalid_command(commands);
        }
        command_ps(current_p);
        return 0;
    }
    else if(strcmp(c, "kill") == 0)
    {
        size_t index = -1;
        pid_t pid = (pid_t) atoi((char*)vector_get(line, 1));
        process* p = NULL;

        int valid = valid_signals(commands, line, pid, &index, &p);

        if(valid == 1)
        {
            return 1;
        }

        int result = kill(pid, SIGTERM);
        
        if(result == 0)
        {
            print_killed_process(pid, c);
            return 0;
        }

        return 1;
    }
    else if(strcmp(c, "stop") == 0)
    {
        size_t index = -1;
        pid_t pid = (pid_t) atoi((char*)vector_get(line, 1));
        process* p = NULL;

        int valid = valid_signals(commands, line, pid, &index, &p);

        if(valid == 1)
        {
            return 1;
        }

        int result = kill(pid, SIGTSTP);
        
        if(result == 0)
        {
            print_stopped_process(pid, c);
            p->status = "Stopped";
            return 0;
        }

        return 1;

    }
    else if(strcmp(c, "cont") == 0)
    {
        //vector_push_back(history, (char*) commands);

        size_t index = -1;
        pid_t pid = (pid_t) atoi((char*)vector_get(line, 1));
        process* p = NULL;

        int valid = valid_signals(commands, line, pid, &index, &p);

        if(valid == 1)
        {
            return 1;
        }

        int result = kill(pid, SIGCONT);
        
        if(result == 0)
        {
            p -> status = "Running";
            return 0;
        }

        return 1;

    }
    else             
    {
        return external_commands(commands, line, history);
    }

    return 1;
}

int logic_expression(char* buffer, char* logic, int logic_type, FILE* input, vector* history, FILE* history_file)
{

    vector_push_back(history, (void*) buffer);

    char* left = NULL;
    if(logic_type == 1 || logic_type == 2)
    {
        left = (char*) malloc(logic - buffer);
        strncpy(left, buffer, logic - buffer-1);
        left[logic - buffer-1] = '\0';
    }
    else
    {
        left = (char*) malloc(logic - buffer+1);
        strncpy(left, buffer, logic - buffer);
        left[logic - buffer] = '\0';
    }
    sstring* s_left = cstr_to_sstring(left);

    vector* line = sstring_split(s_left, ' ');
    int result = run_command(left, line, history, input, history_file);

    free(left);
    sstring_destroy(s_left);
    vector_destroy(line);

    int decrease = 2;
    int run = 0;
    int start = 0;

    if(logic_type == 1 )
    {
        decrease = 2;
        start = 3;
        if(result == 0)
        {
            run = 1;
        }
        else
        {
            return 1;
        }
    }
    else if(logic_type == 2)
    {
        decrease = 2;
        start = 3;
        if(result == 1)
        {
            run = 1;
        }
        else
        {
            return 0;
        }
    }
    else if(logic_type == 3)
    {
        decrease = 1;
        start = 2;
        run = 1;
    }



    if(run == 1)
    {
        char* right = (char*) malloc(buffer + (int)strlen(buffer) - logic - decrease);
        strncpy(right, logic+start, buffer + (int)strlen(buffer) - logic - decrease - 1);
        right[buffer + (int)strlen(buffer) - logic - decrease - 1] = '\0';
        sstring* s_right = cstr_to_sstring(right);

        line = sstring_split(s_right, ' ');
        int result = run_command(right, line, history, input, history_file);
        free(right);
        sstring_destroy(s_right);
        vector_destroy(line);

        return result;
    }

    return 1;
}

int shell(int argc, char *argv[]) {
    // TODO: This is the entry point for your shell.

    signal(SIGINT, handle_sigint);
    signal(SIGCHLD, cleanup);


    FILE* history_file = NULL;
    FILE* input = stdin;
    
    int h_counter = 0;
    int f_counter = 0;

    for (int i = 0; i < argc; i++)
    {
        if(!strcmp(argv[i],"-h"))
        {
            h_counter++;
            if (i+1 < argc)
            {
                history_file = fopen(argv[i+1], "a");

                if(!history_file)
                {
                    print_history_file_error();
                }
            }
            else
            {
                print_history_file_error();
            }
        }

        if(!strcmp(argv[i], "-f"))
        {
            f_counter++;
            if (i+1 < argc)
            {
                input = fopen(argv[i+1], "r");

                if(!input)
                {
                    print_script_file_error();
                }
            }
            else
            {
                print_script_file_error();
            }
        }
    }

    if(h_counter > 1)
    {
        print_history_file_error();
        history_file = NULL;
    }

    if(f_counter > 1)
    {
        print_script_file_error();
        input = stdin;
    }

    char* buffer = NULL;
    size_t n = 0;
    vector* history = string_vector_create();
    current_p = shallow_vector_create();

    while(1)
    {
        char* directory = get_current_dir_name();
        print_prompt(directory, getpid());
        free(directory);
        
        int num = getline(&buffer, &n, input);

        if(feof(input))
        {
            command_exit(history, input, history_file);
            exit(1);
        }

        buffer[num-1] = '\0';

        if(input != stdin)
        {
            print_command(buffer);
        }

        if (strcmp(buffer, "\n") == 0)
        {
            continue;
        }


       
        char* logic = NULL;
        int logic_type = 0;

        if((logic = strstr(buffer, "&&")))
        {
            logic_type = 1;
            logic_expression(buffer, logic, logic_type, input, history, history_file);

        }
        else if((logic = strstr(buffer, "||")))
        {
            logic_type = 2;
            logic_expression(buffer, logic, logic_type, input, history, history_file);
        }
        else if((logic = strstr(buffer, ";")))
        {
            logic_type = 3;
            logic_expression(buffer, logic, logic_type, input, history, history_file);
        }
        else
        {
            sstring* oldline = cstr_to_sstring(buffer);
            vector* line = sstring_split(oldline, ' ');
            sstring_destroy(oldline);

            char* type = (char*) vector_get(line, 0);

            if(!(type[0] == '!') && !(type[0] == '#') && strcmp(type, "exit"))
            {
                vector_push_back(history, (void*) buffer);
            }
            
            run_command(buffer, line, history, input, history_file);
            vector_destroy(line);
        }

    }
    
    return 0;
}
