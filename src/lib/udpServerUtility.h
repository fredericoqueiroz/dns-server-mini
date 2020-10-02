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
#define MAX_HOSTNAME_LEN 254
#define MAX_IP_LEN 46

typedef struct Host
{
    char hostname[MAX_HOSTNAME_LEN];
    char ip_adrr[MAX_IP_LEN];
} Host;

//Handle erro with user message
void dieWithMessage(const char * file_name,int line_number, const char * format, ...);

//For debugging
void printSocketAddress(const struct sockaddr *address, FILE *stream);

int setupServerSocket(const char *service);

void *handleConnection(void *service);

int getcmd(char *buf, int nbuf);

void parsecmd(char *buf, char **tokens);

#endif