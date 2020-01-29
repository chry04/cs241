/**
* Parallel Make Lab
* CS 241 - Fall 2018
*/

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include "format.h"
#include "graph.h"
#include "vector.h"
#include "set.h"
#include "dictionary.h"
#include "queue.h"
#include "parmake.h"
#include "parser.h"

static int size = 0;
static int current_size = 0;
pthread_mutex_t m = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t cv = PTHREAD_COND_INITIALIZER;

//int cycle_detection(graph* g, char* target, set* visited, set* leaf);
int cycle_detection(graph* g, char* target, set* visited, vector* leaf_order, set* leaf, set* node);
vector* order_comments(graph* g, vector* targets, set* node);
void* run_commends(void* thread_info);


typedef struct thread_data
{
    pthread_t id;
    graph* g;
    dictionary* state;
    vector* rules;
}data_t;

int cycle_detection(graph* g, char* target, set* visited, vector* leaf_order, set* leaf, set* node)
{
    int result = 0;
    set_add(visited, target);
    vector* dependency = graph_neighbors(g, target);
    if(vector_size(dependency) == 0)
    {
        //fprintf(stderr, "visited: %s\n", (char*) target);
        set_add(leaf, target);
        vector_push_back(leaf_order, target);
        vector_destroy(dependency);
        set_add(node, target);
        return 0;
    }
    else
    {
        for(size_t i = 0; i < vector_size(dependency); i++)
        {
            //fprintf(stderr, "depend: %s\n", (char*) vector_get(dependency, i));

            void* child = vector_get(dependency, i);
            if(set_contains(visited, child) && !set_contains(node, child))
            {
                //fprintf(stderr, "%s, cycle\n", (char*) vector_get(dependency, i));
                vector_destroy(dependency);
                return 1;
            }
            else if(set_contains(visited, child))
            {
                continue;
            }
            else             
            {
                //set_add(*visited, vector_get(dependency, i));
                result = result + cycle_detection(g, vector_get(dependency, i), visited, 
                        leaf_order, leaf, node);
            }
        }
    }

    //fprintf(stderr, "visited: %s\n", (char*) target);
    set_add(node, target);
    vector_destroy(dependency);

    return result;
}

vector* order_comments(graph* g, vector* leaf_order, set* node)
{
    //fprintf(stderr, "ordering comments\n");
    vector* result = string_vector_create();

    set* in_result = string_set_create();//contian all the nodes in result

    //push all the leaf nodes in the result
    //vector* leaf_nodes = set_elements(leaf);

    for(size_t i = 0; i < vector_size(leaf_order); i++)
    {
        vector_push_back(result, vector_get(leaf_order, i));

        set_remove(node, vector_get(leaf_order, i));
        set_add(in_result, vector_get(leaf_order, i));
        size++;
    }

    //fprintf(stderr, "pushed all the leaf nodes\n");

    //vector_destroy(leaf_nodes);

    /*
    for(size_t i = 0; i < vector_size(result); i++)
    {
        fprintf(stderr, "current result: %s\n", (char*)vector_get(result, i));
    }
    */


    size_t counter = 0;
    
    while(set_cardinality(node) > 0)
    {
        void* current = vector_get(result, counter);
        vector* parents = graph_antineighbors(g, current);
        counter++;

        for(size_t i = 0; i < vector_size(parents); i++)
        {
            void* current_parent = vector_get(parents, i);
            
            if(set_contains(node, current_parent))
            {
                int all_in_result = 1;

                vector* children = graph_neighbors(g, current_parent);

                for(size_t j = 0; j < vector_size(children); j++)
                {
                    void* child = vector_get(children, j);
                    if(!set_contains(in_result, child))
                    {
                        all_in_result = 0;
                    }
                }

                if(all_in_result == 1)
                {
                    //fprintf(stderr, "pushed node: %s\n", (char*) current_parent);
                    vector_push_back(result, current_parent);
                    set_remove(node, current_parent);
                    set_add(in_result, current_parent);
                    size++;
                }

                vector_destroy(children);
            }
        }

        vector_destroy(parents);
    }

    set_destroy(in_result);


    return result;

}

void* run_commends(void* thread_info)
{
    data_t* data = (data_t*) thread_info;

    if(vector_size(data->rules) == 0)
    {
        return NULL;
    }

    while(1)
    {
        pthread_mutex_lock(&m);
        current_size++;
        if(current_size > size)
        {
            //fprintf(stderr, "in thread\n");

            pthread_mutex_unlock(&m);
            break;
        }
        void* target = vector_get(data->rules, current_size-1);
        //fprintf(stderr, "this is thread %zu, at current size %d, running %s\n", data->id, current_size, (char*)target);
        pthread_mutex_unlock(&m);

        //void* target = vector_get(data->rules, current_size--);
        vector* dependencies = graph_neighbors(data->g, target);
        int dependency_check = 1;       


        for(size_t i = 0; i < vector_size(dependencies); i++)
        {
            //fprintf(stderr, "dependency_check: %d\n", *((int*) dictionary_get(data->state, target)));

            pthread_mutex_lock(&m);
            //fprintf(stderr, "sleep, value = %d\n", *((int*) dictionary_get(data->state, target)));
            while(*((int*) dictionary_get(data->state, vector_get(dependencies, i))) == -1)
            {
                pthread_cond_wait(&cv, &m);
            }
            pthread_mutex_unlock(&m);


            if(*((int*) dictionary_get(data->state, vector_get(dependencies, i))) == 0)
            {
                //fprintf(stderr, "dependency_failed\n");
                int value = 0;
                dictionary_set(data->state, target, &value);
                dependency_check = 0;
                break;
            }
        }

        if(dependency_check == 0)
        {
            vector_destroy(dependencies);
            //fprintf(stderr, "stop running %s due to fail \n", (char*)target);
            continue;
        }
        
        rule_t* rule = (rule_t*) graph_get_vertex_value(data->g, target);
        
        vector* commands = rule->commands;
        struct stat comm_info;
        int comm = stat(rule->target, &comm_info);

        //FILE* comm_file = fopen(rule->target, "r");

        if(comm == 0)
        {
            //fclose(comm_file);
            
            
            int non_file_comm = 0;
            int newer_file = 0;
            vector* deps = graph_neighbors(data->g, rule->target);
            
            for(size_t i = 0; i < vector_size(deps); i++)
            {
                char* name = (char*)vector_get(deps, i);
                //FILE* dep_file = fopen(name, "r");
                struct stat dep_info;
                int dep = stat(name, &dep_info);


                if(dep == 0)
                {
                    //fclose(dep_file);
                    
                    time_t comd_time = comm_info.st_mtim.tv_sec;
                    time_t dep_time = dep_info.st_mtim.tv_sec;

                    double diff = difftime(comd_time, dep_time);

                    if(diff < 0)
                    {
                        //fprintf(stderr, "target:%s, comm:%s, dependency is newer\n", rule->target, name);
                        newer_file = 1;
                    }

                    
                }
                else
                {
                    //fprintf(stderr, "not a file\n");
                    non_file_comm = 1;
                }
            }

            if(!non_file_comm && !newer_file)
            {
                //if(*((int*) dictionary_get(data->state, target)) == -1)
                //{
                    int value = 1;
                    dictionary_set(data -> state, target, &value);
                //}
                
                vector_destroy(dependencies);
                vector_destroy(deps);
                pthread_cond_broadcast(&cv);
                //fprintf(stderr, "stop running %s due to newer file \n", (char*)target);

                continue;

            }

            vector_destroy(deps);

        }
        /*
        else
        {
            fprintf(stderr, "comm is not a file\n");
        }
        */

        
        for(size_t i = 0; i < vector_size(commands); i++)
        {
            int partial_result = system((char*) vector_get(commands, i));

            if(partial_result != 0)
            {
                int value = 0;
                dictionary_set(data -> state, target, &value);
                break;
            }
            
        }
        if(*((int*) dictionary_get(data->state, target)) == -1)
        {
            int value = 1;
            dictionary_set(data -> state, target, &value);
        }

        pthread_cond_broadcast(&cv);
        vector_destroy(dependencies);

        //fprintf(stderr, "finish running %s\n", (char*)target);


    }
    return NULL;
}

int parmake(char *makefile, size_t num_threads, char **targets) {
    // good luck!
    //fprintf(stderr, "start\n");
    graph* parsed = parser_parse_makefile(makefile, targets);

    vector* all_targets = graph_vertices(parsed);

    set* visited = string_set_create();
    //set* left = string_set_create();
    set* goal = string_set_create();
    set* all_goal = string_set_create();
    set* leaf = string_set_create();
    set* all_leaf = string_set_create();
    vector* leaf_order = string_vector_create();
    vector* tmp_order = string_vector_create();

    vector* original_targets = graph_neighbors(parsed, "");

    /*
    for(size_t i = 0; i < vector_size(all_targets); i++)
    {
        set_add(left, vector_get(all_targets, i));
    }
    */

    for(size_t i = 0; i < vector_size(original_targets); i++)
    {
        set_clear(visited);
        set_clear(goal);
        vector_clear(tmp_order);
        
        char* tmp_targets = vector_get(original_targets, i);
        if(cycle_detection(parsed, tmp_targets, visited, tmp_order, leaf, goal))
        {
            print_cycle_failure(tmp_targets);
            graph_remove_edge(parsed, "", tmp_targets);
            continue;
        }

        for(size_t j = 0; j < vector_size(tmp_order); j++)
        {
            void* tmp = vector_get(tmp_order, j);
            
            if(!set_contains(all_leaf, tmp))
            {
                vector_push_back(leaf_order, tmp);
            }
        }

        set* union_set = set_union(all_goal, goal);
        set_destroy(all_goal);
        all_goal = union_set;

        set* leaf_union = set_union(all_leaf, leaf);
        set_destroy(all_leaf);
        all_leaf = leaf_union;
    }

    //fprintf(stderr, "finishe circle_detection\n");

    /*
    vector* leaf_debug = set_elements(all_leaf);
    for(size_t i = 0; i < vector_size(leaf_debug); i++)
    {
        fprintf(stderr, "leaf_node: %s\n", (char*)vector_get(leaf_debug, i));
    }
    vector_destroy(leaf_debug);

    vector* goal_debug = set_elements(all_goal);
    for(size_t i = 0; i < vector_size(goal_debug); i++)
    {
        fprintf(stderr, "goal_node: %s\n", (char*)vector_get(goal_debug, i));
    }
    vector_destroy(goal_debug);
    */



    set_destroy(visited);
    set_destroy(goal);
    set_destroy(leaf);
    vector_destroy(tmp_order);

    vector* modified_targets = graph_neighbors(parsed, "");

    /*
    for(size_t i = 0; i < vector_size(modified_targets); i++)
    {
        fprintf(stderr, "targets: %s\n", (char*)vector_get(modified_targets, i));
    }
    */

    //reverse leaf order
    for(size_t i = 0; i < vector_size(leaf_order)/2; i++)
    {
        void* tmp = strdup(vector_get(leaf_order, i));

        size_t reverse_index = vector_size(leaf_order)-1-i;

        vector_set(leaf_order, i, vector_get(leaf_order, reverse_index));
        vector_set(leaf_order, reverse_index, tmp);

        free(tmp);
    }

    /*
    for(size_t i = 0; i < vector_size(leaf_order); i++)
    {
        fprintf(stderr, "leaf_node: %s\n", (char*)vector_get(leaf_order, i));
    }
    */


    vector* rules = string_vector_create();

    if(vector_size(modified_targets) > 0)
    {
        vector_destroy(rules);
        rules = order_comments(parsed, leaf_order, all_goal);
        //fprintf(stderr, "ordered comments\n");
        size = vector_size(rules);
    }

    /*
    for(size_t i = 0; i < vector_size(rules); i++)
    {
        fprintf(stderr, "rule: %s\n", (char*)vector_get(rules, i));
    }
    */
    

    data_t* thread_list = (data_t*) malloc(num_threads*sizeof(data_t));

    dictionary* state = string_to_int_dictionary_create();

    for(size_t i = 0; i < vector_size(all_targets); i++)
    {
        int value = -1;
        dictionary_set(state, vector_get(all_targets, i), &value);
    }

    for(size_t i = 0; i < num_threads; i++)
    {
        thread_list[i].rules = rules;
        thread_list[i].g = parsed;
        thread_list[i].state = state;

        pthread_create(&(thread_list[i].id), NULL, run_commends, &thread_list[i]);
    }

    void* status = NULL;

    for(size_t i = 0; i < num_threads; i++)
    {
        pthread_join(thread_list[i].id, status);
    }

    free(thread_list);
    graph_destroy(parsed);
    vector_destroy(modified_targets);
    vector_destroy(rules);
    vector_destroy(all_targets);
    vector_destroy(original_targets);
    vector_destroy(leaf_order);
    dictionary_destroy(state);
    set_destroy(all_leaf);
    set_destroy(all_goal);
    
    pthread_mutex_destroy(&m);
    pthread_cond_destroy(&cv);
    
    return 0;
}
