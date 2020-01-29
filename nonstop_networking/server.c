/**
* Networking Lab
* CS 241 - Fall 2018
*/

#include <stdio.h>
#include <stdlib.h>
#include <netdb.h>
#include <fcntl.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <stdbool.h>
#include <sys/epoll.h>
#include <sys/types.h> 
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <errno.h>
#include "dictionary.h"
#include "vector.h"
#include "common.h"
#include "format.h"

static int endSession = 0;
static int epfd = 0;

typedef enum { connection, commends, size, file, done } state;

static state connection_p = connection;
static state commends_p = commends;
static state size_p = size;
static state file_p = file;
static state done_p = done;

static char* dir;
static char buffer[8193];

static size_t zero = 0;
static char* empty_string = "";

verb string_to_verb(char *command) {
    if (strcmp(command, "LIST") == 0) {
        return LIST;
    }
    if (strcmp(command, "GET") == 0) {
        return GET;
    }
    if (strcmp(command, "PUT") == 0) {
        return PUT;
    }
    if (strcmp(command, "DELETE") == 0) {
        return DELETE;
    }
    return V_UNKNOWN;
}

void handle_error(int fd, const char* error_type, vector* received, vector* status)
{
    char* tmp_error = (char*) malloc(strlen(error_type) + 7);
    strncpy(tmp_error, "ERROR\n", 6);
    strncpy(tmp_error+6, error_type, strlen(error_type));
    tmp_error[strlen(error_type) + 6] = 0;

    vector_set(status, fd, &done_p);

    write_all(fd, tmp_error, strlen(tmp_error)+1);
    shutdown(fd, SHUT_WR);
    epoll_ctl(epfd, EPOLL_CTL_DEL, fd, NULL);
    vector_set(received, fd, empty_string);

    free(tmp_error);
}

int build_up_server(char* port)
{
    int serverSocket = socket(AF_INET, SOCK_STREAM | SOCK_NONBLOCK, 0);
    if (serverSocket == -1) {
        perror("socket()");
        exit(1);
    }

    struct addrinfo hints, *result;
    memset(&hints, 0, sizeof (struct addrinfo));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;

    int s = getaddrinfo(NULL, port, &hints, &result);
    if (s != 0) {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(s));
        exit(1);
    }

    int optval = 1;
    setsockopt(serverSocket, SOL_SOCKET, SO_REUSEPORT, &optval, sizeof (optval));

    if (bind(serverSocket, result->ai_addr, result->ai_addrlen) != 0) {
        perror("bind()");
        exit(1);
    }

    if (listen(serverSocket, 10) != 0) {
        perror("listen()");
        exit(1);
    }

    freeaddrinfo(result);

    return serverSocket;

}


void handle_crtl_c()
{
    endSession = 1;
}

void add_fd_to_epoll(struct epoll_event* event, int fd, uint32_t mode)
{
    memset(event, 0, sizeof(struct epoll_event));
    event->events = mode;
    event->data.fd = fd;
    epoll_ctl(epfd, EPOLL_CTL_ADD, fd, event);
}

verb parse_commend(char* command, char* file_name, int* size)
{
    if(strcmp(command, "LIST") == 0)
    {
        return LIST;
    }
    
    char* position = strchr(command, ' ');

    if(!position)
        return V_UNKNOWN;

    char* v = (char*) malloc(position - command+1);
    memcpy(v, command, (position-command));
    v[position-command] = '\0';

    *size = strlen(command) - (position-command) -1;

    memcpy(file_name, dir, 6);
    file_name[6] = '/';
    memcpy(file_name+7, (position+1), *size);
    file_name[(*size)+7] = '\0';

    verb result_verb = string_to_verb(v);
    free(v);

    return result_verb;
}

char* run_commend(char* line, vector* status, int fd, dictionary* file_size, vector* received)
{
     //printf("running commends\n");
     char* file_name = (char*) calloc(226, 1);
     int name_size = 0;
     verb request = parse_commend(line, file_name, &name_size);

     //printf("name size: %d\n", name_size);

     if(name_size > 255 || request == V_UNKNOWN)
     {
         handle_error(fd, err_bad_request, received, status);
         free(file_name);
         return NULL;
     }

     if(request != PUT)
     {
         shutdown(fd, SHUT_RD);

         vector_set(status, fd, &done_p);

         if(request == GET)
         {
             //printf("file_name: %s\n", file_name);
             if(dictionary_contains(file_size, file_name) == false)
             {
                 handle_error(fd, err_no_such_file, received, status);
                 free(file_name);
                 return NULL;
             }

             write_all(fd, "OK\n", 3);

             struct stat info;
             memset(&info, 0, sizeof(struct stat));
             int r = stat(file_name, &info);
             if(r == -1)
             {
                 perror("stat:");
                 return NULL;
             }
             
             size_t f_size = (size_t) info.st_size;


             write_size(f_size, fd);

             int file_fd = open(file_name, O_RDONLY);
             if(file_fd == -1)
             {
                 perror("open:");
                 return NULL;
             }

             unsigned long counter = 0;

             if(f_size > 8192)
             {
                 for(counter = 0; counter < (f_size/8192); counter++)
                 {
                     void* file_ptr = mmap(NULL, 8192, PROT_READ, MAP_SHARED, file_fd, counter*8192);
                     
                     write_all(fd, file_ptr, 8192);
                     munmap(file_ptr, 8192);
                 }
             }

             void* file_ptr = mmap(NULL, f_size%8192, PROT_READ, MAP_SHARED, file_fd, counter*8192);
             write_all(fd, file_ptr, f_size%8192);
             munmap(file_ptr, f_size%8192);




             
             close(file_fd);
         }
         else if(request == DELETE)
         {
             if(dictionary_contains(file_size, file_name) == true)
             {
                 void* tmp = dictionary_get(file_size, file_name);
                 free(tmp);
                 unlink(file_name);
                 dictionary_remove(file_size, file_name);
                 write_all(fd, "OK\n", 3);
             }
             else
             {
                 handle_error(fd, err_no_such_file, received, status);
                 free(file_name);
                 return NULL;

             }
         }
         else if(request == LIST)
         {
             //printf("commend list\n");
             vector* keys = dictionary_keys(file_size);
             size_t length = 0;



             for(size_t i = 0; i < vector_size(keys); i++)
             {
                 char* name = (char*) vector_get(keys, i);
                 length += (strlen(name)-6);
             }

             if(length == 0)
             {
                 write_all(fd, "OK\n", 3);
                 write_size(length, fd);
                 write_all(fd, " ", 0);
                 shutdown(fd, SHUT_WR);
                 epoll_ctl(epfd, EPOLL_CTL_DEL, fd, NULL);
                 vector_set(received, fd, empty_string);
                 free(keys);
                 return file_name;
             }


             length--;
             write_all(fd, "OK\n", 3);
             write_size(length, fd);

             
             char* list = (char*) malloc(length);
             size_t counter = 0;

             char* tmp = (char*) vector_get(keys, 0);

             memcpy(list+counter, tmp+7, (strlen(tmp)-7));
             counter += (strlen(tmp)-7);

             for(size_t i = 1; i < vector_size(keys); i++)
             {
                 list[counter] = '\n';
                 counter++;
                 
                 char* name = (char*) vector_get(keys, i);

                 memcpy(list+counter, name+7, (strlen(name)-7));
                 counter += (strlen(name)-7);
             }


             write_all(fd, list, length);

             vector_destroy(keys);
             free(list);
         }

         epoll_ctl(epfd, EPOLL_CTL_DEL, fd, NULL);
         vector_set(received, fd, empty_string);
         shutdown(fd, SHUT_WR);
         

     }
     else
     {
         vector_set(status, fd, &size_p);
     }


     return file_name;

}

int main(int argc, char **argv) {
    // good luck!
    signal(SIGINT, handle_crtl_c);

    if(argc < 2)
    {
        return 1;
    }
    
    int sock = build_up_server(argv[1]);


    epfd = epoll_create(1);
    struct epoll_event event;
    add_fd_to_epoll(&event, sock, EPOLLIN);

    vector* received = shallow_vector_create();
    vector* status = shallow_vector_create();
    vector* size_len = shallow_vector_create();
    vector* names = string_vector_create();
    dictionary* file_size = string_to_shallow_dictionary_create();

    for(int i = 0; i < 13; i++)
    {
        vector_push_back(received, empty_string);
        vector_push_back(status, &connection_p);
        vector_push_back(size_len, &zero);
        vector_push_back(names, empty_string);
    }

    char tmpdir[] = "XXXXXX";
    dir = mkdtemp(tmpdir);
    print_temp_directory(dir);
    
    struct epoll_event* events = calloc(10, sizeof(struct epoll_event));


    while (endSession == 0)
    {
        int n = epoll_wait(epfd, events, 10, -1);

        for(int i = 0; i < n; i++)
        {
            if(events[i].data.fd == sock)
            {
                int client_fd = accept(sock, NULL, NULL);

                if((size_t)client_fd >= vector_size(received))             
                {
                    int diff = client_fd - vector_size(received)+1;

                    for(int i = 0; i < diff; i++)
                    {
                        vector_push_back(received, empty_string);
                        vector_push_back(status, &connection_p);
                        vector_push_back(size_len, &zero);
                        vector_push_back(names, empty_string);
                    }

                }

                vector_set(status, client_fd, &connection_p);

                struct epoll_event client_event;
                add_fd_to_epoll(&client_event, client_fd, EPOLLIN);
            }
            else
            {
                //printf("process comments\n");
                //char* buffer = (char*) malloc(1025);
                int pos = 0;
                
                int curr_fd = events[i].data.fd;
                //printf("curr_fd: %d, vector size: %zu\n", curr_fd, sizeof(status));
                state curr_status = *((state*) vector_get(status, curr_fd));

                if(curr_status == connection)
                {
                    vector_set(status, curr_fd, &commends_p);
                    curr_status = commends;
                }
                else if(curr_status == commends)
                {
                    char* exist = vector_get(received, curr_fd);
                    memcpy(buffer, exist, strlen(exist));
                    pos = strlen(exist);

                    free(exist);
                }

                //printf("before reading, pos: %d\n", pos);

                int ret = read(curr_fd, buffer+pos, 8192);

                if(ret == -1 && (errno == EINTR || errno == EAGAIN || errno == EWOULDBLOCK))
                {
                    char* partial = (char*) malloc(pos);
                    memcpy(partial, buffer, pos);
                    vector_set(received, curr_fd, partial);

                    continue;
                }
                else if(ret == -1)
                {
                    if(curr_status == commends)
                    {
                        handle_error(curr_fd, err_bad_request, received, status);
                    }
                    else
                    {
                        handle_error(curr_fd, err_bad_file_size, received, status);
                    }
                }

                buffer[ret] = '\0';
                int change_size = 1;

                //printf("after reading %d bits, %s\n", ret, buffer);

                char* arg_end = strchr(buffer, '\n');
                char* file_name = NULL;

                if(curr_status == done && ret > 0)
                {
                    handle_error(curr_fd, err_bad_file_size, received, status);
                    continue;
                }
                
                if(curr_status == commends)
                {     
                    //printf("state commends\n");
                    if(arg_end)
                    {
                        char* commend_line = (char*) malloc(arg_end-buffer+1);
                        commend_line[arg_end - buffer] = '\0';
                        memcpy(commend_line, buffer, arg_end-buffer);

                        //printf("command line: %s, size: %ld, str: %ld\n", commend_line, (arg_end-buffer), strlen(commend_line));
                        file_name = run_commend(commend_line, status, curr_fd, file_size, received);
                        vector_set(names, curr_fd, file_name);
                        free(file_name);


                        free(commend_line);

                        //writing the rest file
                        size_t arg_len = arg_end-buffer;
                        if((arg_len+1) < (size_t)ret)
                        {
                            char* tmp = (char*) malloc(ret-arg_len-1);
                            memcpy(tmp, buffer+arg_len+1, (ret-arg_len-1));

                            size_t* curr_len = (size_t*) malloc(sizeof(size_t));
                            *curr_len = ret-arg_len-1;
                            
                            vector_set(size_len, curr_fd, (void*)curr_len);
                            vector_set(status, curr_fd, &size_p);
                            vector_set(received, curr_fd, tmp);
                        }

                        change_size = 0;                        
                    }
                    else
                    {
                        char* partial = (char*) malloc(ret);
                        memcpy(partial, buffer, ret);
                        vector_set(received, curr_fd, partial);
                    }
                }

                //printf("sizeof size_len after commends: %zu\n", vector_size(size_len));

                curr_status = *((state*) vector_get(status, curr_fd));

                int read_size = 0;

                if(curr_status == size)
                {
                    read_size = 1;
                    size_t* size_length = (size_t*)vector_get(size_len, curr_fd);
                    size_t total_len = *size_length;

                    void* tmp = vector_get(received, curr_fd);


                    if(change_size)
                    {
                       total_len = *size_length + ret;
                    }

                    if(total_len >= 8)
                    {         
                        size_t* rest_file = (size_t*) malloc(sizeof(size_t));
                        size_t* file_len = (size_t*) malloc(8);

                        if(*size_length > 8)
                        {
                            *size_length = 8;
                        }

                        memcpy(file_len, tmp, *size_length);

                        
                        if(change_size)
                        {
                            memcpy(file_len+*size_length, buffer, 8-*size_length);
                        }
                        file_name = (char*) vector_get(names,curr_fd);


                        
                        *rest_file = total_len-8;

                        //printf("file_name: %s, file length: %zu, rest_len: %zu\n", file_name, 
                               //*file_len, *rest_file);
                        vector_set(status, curr_fd, &file_p);
                        vector_set(size_len, curr_fd, (void*) rest_file);
                        dictionary_set(file_size, (void*) file_name, (void*)file_len);

                        if(size_length != &zero)
                            free(size_length);
                    }
                    else if(change_size)
                    {
                        char* new_tmp = (char*) malloc(total_len);
                        memcpy(new_tmp, tmp, *size_length);
                        memcpy(new_tmp+*size_length, buffer,ret);
                        
                        *size_length = total_len;
                        vector_set(received, curr_fd, new_tmp);
                        free(new_tmp);
                    }

                    if(tmp != empty_string)
                        free(tmp);
                }

                curr_status = *((state*)vector_get(status, curr_fd));

                if(curr_status == file)
                {
                    size_t* received_size = (size_t*) vector_get(size_len, curr_fd);
                    file_name = (char*) vector_get(names, curr_fd);
                    size_t expected = *((size_t*) dictionary_get(file_size, file_name));
                    //printf("state file, file_name: %s\n", file_name);



                    size_t file_s = ret;

                    if(read_size != 0)
                    {
                        file_s = 0;
                    }

                    //printf("received_size: %zu, expected size: %zu, file_s: %zu, ret: %d\n",
                            //*received_size, expected, file_s, ret);


                    int all_file = 0;

                    if((*received_size + file_s) > expected || 
                            ((*received_size+file_s) < expected && ret == 0))
                    {

                        if(*received_size+file_s < expected && ret == 0)
                        {
                            print_too_little_data();
                        }
                        else
                        {
                            print_received_too_much_data();
                        }

                        shutdown(curr_fd, SHUT_RD);

                        handle_error(curr_fd, err_bad_file_size, received, status);

                        //free(buffer);
                        continue;                        
                    }
                    else if(*received_size+file_s == expected)
                    {
                        write_all(curr_fd, "OK\n", 3);
                        vector_set(status,curr_fd,&done_p);
                        shutdown(curr_fd, SHUT_RD);
                        all_file = 1;
                    }


                    
                    //*new_len = *received_size + file_s;

                    //printf("new file length: %zu\n", *new_len);

                    //vector_set(size_len, curr_fd, new_len);

                        FILE* put_fd = fopen(file_name, "a");

                        if(file_s == 0)
                        {
                            fwrite(buffer+ret-*received_size, 1, *received_size, put_fd);
                        }
                        else
                        {
                            fseek(put_fd, *received_size, SEEK_SET);
                            fwrite(buffer, 1, ret, put_fd);
                        }

                        fclose(put_fd);

                    *received_size = *received_size + file_s;


                    if(all_file)
                    {
                        shutdown(curr_fd, SHUT_WR);
                        vector_set(received, curr_fd, empty_string);
                        epoll_ctl(epfd, EPOLL_CTL_DEL, curr_fd, NULL);
                        free(received_size);
                    }

                    
                    //free(received_size);

                }
                //free(buffer);
            }
        }
    }

    vector* keys = dictionary_keys(file_size);

    for(size_t i = 0; i < vector_size(keys); i++)
    {
        char* name = vector_get(keys, i);
        unlink(name);
    }

    rmdir(tmpdir);

    /*
    for(size_t i = 0; i < vector_size(size_len); i++)
    {
        void* tmp = vector_get(size_len, i);
        if(tmp != &zero)
            free(tmp);
    }
    */

    
    vector_destroy(keys);
    vector_destroy(received);
    vector_destroy(status);
    vector_destroy(size_len);
    vector_destroy(names);

    free(events);

    vector* dic_v = dictionary_values(file_size);
    for(size_t i = 0; i < vector_size(dic_v); i++)
    {
        void* tmp2 = vector_get(dic_v, i);
        free(tmp2);
    }

    vector_destroy(dic_v);
    dictionary_destroy(file_size);

    return 0;

}
