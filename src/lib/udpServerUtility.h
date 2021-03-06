#ifndef SERVER_UTILITY_H_
#define SERVER_UTILITY_H_

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdarg.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h> // addrinfo
#include <errno.h>
#include <arpa/inet.h>

#define MAX_HOSTS 1000
#define MAX_SERVERS 10

#define MAX_HOSTNAME_LEN 254
#define MAX_IP_LEN 46
#define MAX_SERVICE_LEN 6

#define BUFFER_SIZE MAX_HOSTNAME_LEN + MAX_IP_LEN + 10

typedef struct Host{
    char hostname[MAX_HOSTNAME_LEN];
    char ip_adrr[MAX_IP_LEN];
} Host;

typedef struct Server{
    char ip_adrr[MAX_IP_LEN];
    char service[MAX_SERVICE_LEN];
} Server;

typedef  struct ConnectionArgs{
    char *service;
    Host *dns_list;
    int *dns_list_size;
    Server *server_list;
    int *server_list_size;
} Args;

//Handle erro with user message
void dieWithMessage(const char * file_name,int line_number, const char * format, ...);

//For debugging
void printSocketAddress(const struct sockaddr *address, FILE *stream);

int setupServerSocket(const char *service);

void *handleConnection(void *arguments);

int getcmd(char *buf, int nbuf);

void parsecmd(char *buf, char **tokens);

void runcmd(Host *dns_list, int *dns_list_size, Server *server_list, int *server_list_size, char **cmd);

int find_ip(Host *dns_list, int *dns_list_size, Host host);

void add_host(Host *dns_list, int *dns_list_size, Host host);

void print_hostlist(Host *dns_list, int dns_list_size);

void link_server(Server *server_list, int *server_list_size, Server server);

void print_serverlist(Server *server_list, int server_list_size);

void search(Host *host, Host *dns_list, int *dns_list_size, Server *server_list, int *server_list_size);

#endif