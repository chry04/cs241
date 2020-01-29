/**
* Chatroom Lab
* CS 241 - Fall 2018
*/

#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>

#include "utils.h"
static const size_t MESSAGE_SIZE_DIGITS = 4;

char *create_message(char *name, char *message) {
    int name_len = strlen(name);
    int msg_len = strlen(message);
    char *msg = calloc(1, msg_len + name_len + 4);
    sprintf(msg, "%s: %s", name, message);

    return msg;
}

ssize_t get_message_size(int socket) {
    int32_t size;
    ssize_t read_bytes =
        read_all_from_socket(socket, (char *)&size, MESSAGE_SIZE_DIGITS);
    if (read_bytes == 0 || read_bytes == -1)
        return read_bytes;

    return (ssize_t)ntohl(size);
}


ssize_t read_all_from_socket(int socket, char *buffer, size_t count) {
    // Your Code Here
    /*
    size_t remain = count;
    char* tmp = buffer;
    while(1)
    {
        ssize_t n = read(socket, tmp, remain);
        remain -= n;
        tmp += n;

        if(n == 0)
        {
            return 0;
        }
        else if(n > 0 && remain == 0)
            return count;
        else if(n == -1 && errno == EINTR)
        {
            continue;
        }
        else if(n == -1)
        {
            return -1;
        }

    }
    return -1;
    */
    size_t counter = 0;

    while(counter < count)
    {
        ssize_t n = read(socket, buffer+counter, count-counter);

        if(n == 0)
        {
            return 0;
        }
        else if(n > 0)
        {
            counter += n;
        }
        else if(n == -1 && errno == EINTR)
        {
            continue;
        }
        else
        {
            return -1;
        }

    }

    return count;
}

ssize_t write_all_to_socket(int socket, const char *buffer, size_t count) {
    // Your Code Here
    /*
    size_t remain = count;
    const char* tmp = buffer;
    while(1)
    {
        ssize_t n = write(socket, tmp, remain);
        remain -= n;
        tmp += n;

        if(n == 0)
        {
            return 0;
        }
        else if(n > 0 && remain == 0)
            return count;
        else if(n == -1 && errno == EINTR)
        {
            continue;
        }
        else if(n == -1)
        {
            return -1;
        }

    }

    return -1;
    */

    size_t counter = 0;

    while(counter < count)
    {
        ssize_t n = write(socket, buffer+counter, count-counter);

        if(n == 0)
        {
            return 0;
        }
        else if(n > 0)
        {
            counter += n;
        }
        else if(n == -1 && errno == EINTR)
        {
            continue;
        }
        else
        {
            return -1;
        }

    }

    return count;

}

// You may assume size won't be larger than a 4 byte integer
ssize_t write_message_size(size_t size, int socket) {
    // Your code here
    int32_t messages = htonl(size);
    return write_all_to_socket(socket, (char*) &messages, 4);
}

