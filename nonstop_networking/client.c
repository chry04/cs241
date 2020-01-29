/**
* Networking Lab
* CS 241 - Fall 2018
*/

#include "common.h"
#include "format.h"
#include <ctype.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <netdb.h> 
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/socket.h>

char **parse_args(int argc, char **argv);
verb check_args(char **args);
void handle_get(int sock, char** args);
void handle_put(int sock, char** args);
void handle_delete(int sock, char** args);
void handle_list(int sock);

int main(int argc, char **argv) {
    // Good luck!

    char** arguments = parse_args(argc, argv);
    
    char* host = arguments[0];
    char* port = arguments[1];

    //printf("%s, %s\n", host, port);

    verb commd = check_args(arguments);
    
    
    int sock = socket(AF_INET, SOCK_STREAM, 0);

    struct addrinfo hints, *infoptr;
    memset(&hints, 0, sizeof(struct addrinfo));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;

    int s = getaddrinfo(host, port, &hints, &infoptr);
    if (s != 0) {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(s));
        exit(1);
    }


    if (connect(sock, infoptr->ai_addr, infoptr->ai_addrlen) == -1) {
        perror("connect");
        exit(2);
    }

    freeaddrinfo(infoptr);

    //printf("conneted\n");

    if(commd == GET)
    {
        handle_get(sock, arguments);
    }
    else if(commd == PUT)
    {
        handle_put(sock, arguments);
    }
    else if(commd == DELETE)
    {
        handle_delete(sock, arguments);
    }
    else if(commd == LIST)
    {
                //printf("commend list\n");
        handle_list(sock);
    }

    
    free(arguments);

        
    return 0;
}

void handle_get(int sock, char** args)
{
    size_t total_size = 5+strlen(args[3]);
    char* buffer = malloc(total_size);

    sprintf(buffer, "GET %s", args[3]);
    buffer[total_size-1] = '\n';

    ssize_t retval = write_all(sock, (void*)buffer, total_size);

    shutdown(sock, SHUT_WR);
    free(buffer);

    
    char response[255];
    response[3] = '\0';

    retval = read_all(sock, response, 3);

    if(!strcmp("ERR", response))
    {
        retval = read_all(sock, response+3, 252);

        print_error_message(response);

        return;
    }
    
    
    if(strcmp("OK\n", response))
    {
        print_invalid_response();
        return;
    }

    size_t size;
    retval = read_all(sock, (char*) &size, 8);

    //printf("file size: %zu\n", size);

    //int file_size = *((int*) size);
    
    FILE* file = fopen(args[4], "w");

    unsigned long counter = 0;

    if(size > 8192)
    {
        for(counter = 0; counter < (size/8192); counter++)
        {
            
            void* file_buffer = malloc(8192);
            
            retval = read_all(sock, file_buffer, 8192);

            if ((size_t) retval < 8192)
            {
                print_too_little_data();
                free(file_buffer);
                return;
            }

            fwrite(file_buffer, 1, 8192, file);
            fseek(file, 8192*(counter+1), SEEK_SET);

            free(file_buffer);
        }
    }

    void* file_buffer = malloc(size%8192+1);

    retval = read_all(sock, file_buffer, size%8192);

    if ((size_t) retval < size%8192)
    {
        print_too_little_data();
        free(file_buffer);
        return;
    }

    retval = read_all(sock, file_buffer+retval, 1);

    
    if(retval != 0)
    {
        print_received_too_much_data();
        free(file_buffer);
        return;
    }

    
    fwrite(file_buffer, 1, size%8192, file);

    shutdown(sock, SHUT_RD);

    free(file_buffer);
    fclose(file);

    
}

void handle_put(int sock, char** args)
{
    struct stat info;
    memset(&info, 0, sizeof(struct stat));
    stat(args[4], &info);
    size_t size = (size_t) info.st_size;
    int fd = open(args[4], O_RDONLY);
    size_t total_size = 5+strlen(args[3]);
    
    char* buffer = malloc(total_size);
    

    sprintf(buffer, "PUT %s", args[3]);
    buffer[total_size-1] = '\n';
    ssize_t retval = write_all(sock, (void*) buffer, total_size);
    //printf("commend size: %lu\n", retval);

    retval = write_size(size, sock);
    //printf("size byte: %lu, %zu\n", retval,size);

    
    //void* file = mmap(NULL, size, PROT_READ, MAP_SHARED, fd, 0);
    //retval = write_all(sock, file, size);

    unsigned long counter = 0;

    if(size > 8192)
    {
        for(counter = 0; counter < (size/8192); counter++)
        {
            void* file_ptr = mmap(NULL, 8192, PROT_READ, MAP_SHARED, fd, counter*8192);
            write_all(sock, file_ptr, 8192);
            munmap(file_ptr, 8192);
        }
    }

    void* file_ptr = mmap(NULL, size%8192, PROT_READ, MAP_SHARED, fd, counter*8192);
    write_all(sock, file_ptr, size%8192);
    munmap(file_ptr, size%8192);



    shutdown(sock, SHUT_WR);
    
    char response[255];
    response[3] = '\0';

    retval = read_all(sock, response, 3);

    //printf("response: %s\n",response);

    if(!strcmp("ERR", response))
    {
        retval = read_all(sock, response+3, 252);
        //printf("error message: %s\n", response);

        print_error_message(response);
        free(buffer);

        return;
    }


    if(!strcmp("OK\n", response))
    {
        print_success();
    }
    else
    {
        print_invalid_response();
    }

    shutdown(sock, SHUT_RD);

    close(fd);
    free(buffer);


    
}

void handle_delete(int sock, char** args)
{
    size_t total_size = 8+strlen(args[3]);
    char* buffer = malloc(total_size);

    sprintf(buffer, "DELETE %s", args[3]);
    buffer[total_size-1] = '\n';

    ssize_t retval = write_all(sock, (void*)buffer, total_size);

    shutdown(sock, SHUT_WR);
    free(buffer);

    
    char response[255];
    response[3] = '\0';

    retval = read_all(sock, response, 3);

    if(!strcmp("ERR", response))
    {
        retval = read_all(sock, response+3, 252);

        print_error_message(response);
        return;
    }
    
    if(!strcmp("OK\n", response))
    {
        print_success();
    }
    else
    {
        print_invalid_response();
    }

    
    shutdown(sock, SHUT_RD);


}

void handle_list(int sock)
{
    ssize_t retval = write_all(sock, "LIST\n", 5);

    shutdown(sock, SHUT_WR);
    
    char response[255];
    response[3] = '\0';

    retval = read_all(sock, response, 3);

    if(!strcmp("ERR", response))
    {
        retval = read_all(sock, response+3, 255);

        print_error_message(response);

        return;
    }
    
    if(strcmp("OK\n", response))
    {
        print_invalid_response();
        return;
    }

    //printf("%s\n", response);


    size_t size;
    retval = get_size(&size, sock);
    
    char* list_buffer = malloc(size+1);

    retval = read_all(sock, list_buffer, size);


    if ((size_t) retval < size)
    {
        //printf("read_size: %zu\n", retval);
        print_too_little_data();
        free(list_buffer);
        return;
    }
    
    list_buffer[size] = '\0';
    retval = read_all(sock, list_buffer+size, 1);

    if(retval != 0)
    {
        print_received_too_much_data();
        free(list_buffer);
        return;
    }


    printf("%s\n", list_buffer);

    

    shutdown(sock, SHUT_RD);

    free(list_buffer);

}

/**
 * Given commandline argc and argv, parses argv.
 *
 * argc argc from main()
 * argv argv from main()
 *
 * Returns char* array in form of {host, port, method, remote, local, NULL}
 * where `method` is ALL CAPS
 */
char **parse_args(int argc, char **argv) {
    if (argc < 3) {
        return NULL;
    }

    char *host = strtok(argv[1], ":");
    char *port = strtok(NULL, ":");
    if (port == NULL) {
        return NULL;
    }

    char **args = calloc(1, 6 * sizeof(char *));
    args[0] = host;
    args[1] = port;
    args[2] = argv[2];
    char *temp = args[2];
    while (*temp) {
        *temp = toupper((unsigned char)*temp);
        temp++;
    }
    if (argc > 3) {
        args[3] = argv[3];
    }
    if (argc > 4) {
        args[4] = argv[4];
    }

    return args;
}

/**
 * Validates args to program.  If `args` are not valid, help information for the
 * program is printed.
 *
 * args     arguments to parse
 *
 * Returns a verb which corresponds to the request method
 */
verb check_args(char **args) {
    if (args == NULL) {
        print_client_usage();
        exit(1);
    }

    char *command = args[2];

    if (strcmp(command, "LIST") == 0) {
        return LIST;
    }

    if (strcmp(command, "GET") == 0) {
        if (args[3] != NULL && args[4] != NULL) {
            return GET;
        }
        print_client_help();
        exit(1);
    }

    if (strcmp(command, "DELETE") == 0) {
        if (args[3] != NULL) {
            return DELETE;
        }
        print_client_help();
        exit(1);
    }

    if (strcmp(command, "PUT") == 0) {
        if (args[3] == NULL || args[4] == NULL) {
            print_client_help();
            exit(1);
        }
        return PUT;
    }

    // Not a valid Method
    print_client_help();
    exit(1);
}
