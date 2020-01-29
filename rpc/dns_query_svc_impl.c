/**
* Resplendent RPCs Lab
* CS 241 - Fall 2018
*/

#include <arpa/inet.h>
#include <errno.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <unistd.h>

#include "common.h"
#include "dns_query.h"
#include "dns_query_svc_impl.h"

#define CACHE_FILE "cache_files/rpc_server_cache"

char *contact_nameserver(query *argp, char *host, int port) {
    // Your code here
    // Look in the header file for a few more comments
    int sock = socket(AF_INET, SOCK_DGRAM, 0);
    if(sock < 0)
        perror("socket");

    int optval = 1;
    setsockopt(sock, SOL_SOCKET, SO_REUSEPORT, &optval, sizeof(optval)); 
   
    uint32_t binary_host;
    if(!inet_pton(AF_INET, host, &binary_host))
        perror("inet_pton:");

    struct sockaddr_in addr_info;
    addr_info.sin_family = AF_INET;
    addr_info.sin_port = htons((uint16_t)port);
    addr_info.sin_addr.s_addr = binary_host;

    char* buffer = (char*) malloc(16);

    ssize_t retval = sendto(sock, argp->host, strlen(argp->host), 0, 
            (struct sockaddr*)&addr_info, sizeof(addr_info));
    
    retval = recvfrom(sock, buffer, 15, 0, NULL, 0);

    buffer[retval] = '\0';

    if(!strcmp(buffer, "-1.-1.-1.-1"))
        return NULL;
    else
        return buffer;
}

void create_response(query *argp, char *ipv4_address, response *res) {
    // Your code here
    // As before there are comments in the header file
    struct host_address* addr = (struct host_address*) malloc(sizeof(struct host_address));
    unsigned long host_len = strlen(argp->host);
    addr->host = (char*) malloc(host_len+1);
    addr->host[host_len] = '\0';
    memcpy(addr->host, argp->host, host_len);    
    
    if(!ipv4_address)
    {
        char* buffer = (char*) malloc(16);
        addr->host_ipv4_address = buffer;
        res->success = 0;
    }
    else
    {
        unsigned long ipv4_len = strlen(ipv4_address);
        addr->host_ipv4_address = (char*) malloc(ipv4_len+1);
        addr->host_ipv4_address[ipv4_len] = '\0';
        memcpy(addr->host_ipv4_address, ipv4_address, ipv4_len);
        
        res->success = 1;
    }

    res->address = addr;

    
}

// Stub code

response *dns_query_1_svc(query *argp, struct svc_req *rqstp) {
    printf("Resolving query...\n");
    // check its cache, 'rpc_server_cache'
    // if it's in cache, return with response object with the ip address
    char *ipv4_address = check_cache_for_address(argp->host, CACHE_FILE);
    if (ipv4_address == NULL) {
        // not in the cache. contact authoritative servers like a recursive dns
        // server
        printf("Domain not found in server's cache. Contacting authoritative "
               "servers...\n");
        char *host = getenv("NAMESERVER_HOST");
        int port = strtol(getenv("NAMESERVER_PORT"), NULL, 10);
        ipv4_address = contact_nameserver(argp, host, port);
    } else {
        // it is in the server's cache; no need to ask the authoritative
        // servers.
        printf("Domain found in server's cache!\n");
    }

    static response res;
    xdr_free(xdr_response, &res); // Frees old memory in the response struct
    create_response(argp, ipv4_address, &res);

    free(ipv4_address);

    return &res;
}
