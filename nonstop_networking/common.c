/**
* Networking Lab
* CS 241 - Fall 2018
*/

#include <unistd.h>
#include <errno.h>
#include <arpa/inet.h>
#include "common.h"

ssize_t read_all(int socket, char *buffer, size_t count) {
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
        else if(n == -1 && (errno == EINTR || errno == EAGAIN || errno == EWOULDBLOCK))
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

ssize_t write_all(int socket, const char *buffer, size_t count) {
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
        else if(n == -1 && (errno == EINTR || errno == EAGAIN || errno == EWOULDBLOCK))
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

ssize_t get_size(size_t* size, int socket) {
    return read_all(socket, (char *)size, 8);
}

ssize_t write_size(size_t size, int socket) {
    //int32_t messages = htonl(size);
    return write_all(socket, (char *)&size, 8);
}


