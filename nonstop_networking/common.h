/**
* Networking Lab
* CS 241 - Fall 2018
*/

#pragma once
#include <stddef.h>
#include <sys/types.h>

#define LOG(...)                      \
    do {                              \
        fprintf(stderr, __VA_ARGS__); \
        fprintf(stderr, "\n");        \
    } while (0);

typedef enum { GET, PUT, DELETE, LIST, V_UNKNOWN } verb;

ssize_t read_all(int socket, char *buffer, size_t count);
ssize_t write_all(int socket, const char *buffer, size_t count);
ssize_t get_size(size_t* size, int socket);
ssize_t write_size(size_t size, int socket);
