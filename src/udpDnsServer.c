#include <stdio.h>
#include <unistd.h>
#include "lib/udpServerUtility.h"

#include <pthread.h>

int main(int argc, char **argv){

    // Check parameter
    if(argc < 2 || argc > 3)
        dieWithMessage(__FILE__, __LINE__, "Sintaxe de Uso: %s <Server Port> [startup_file]", argv[0]);

    Host dns_list[MAX_HOSTS];
    memset(dns_list, 0, sizeof(dns_list));
    int dns_list_size = 0;

    Server server_list[MAX_SERVERS];
    memset(server_list, 0, sizeof(server_list));
    int server_list_size = 0;
    
    char buffer[BUFFER_SIZE];
    //memset(buffer, 0, sizeof(buffer));
    char *cmd[3];

    char *service = argv[1];
    char *startup_file;


    // Load startup_file
    if(argc == 3){
        startup_file = argv[2];
        FILE *startup_fd;
        startup_fd = fopen(startup_file, "r");
        if(startup_file == NULL)
            fprintf(stdout, "Can not open file %s\n", startup_file);
        else{
            fprintf(stdout, "Loading file [%s] ...\n", startup_file);
            while(!feof(startup_fd)){
                memset(buffer, 0, sizeof(buffer));
                fgets(buffer, sizeof(buffer), startup_fd);
                parsecmd(buffer, cmd);
                //check if the command is add
                if(strcmp(cmd[0], "add") == 0 || strcmp(cmd[0], "link") == 0)
                    runcmd(dns_list, &dns_list_size, server_list, &server_list_size, cmd);
                else
                    continue;
            }
        }
    }

    Args connection_args;
    connection_args.service = service;
    connection_args.dns_list = dns_list;
    connection_args.dns_list_size = &dns_list_size;
    connection_args.server_list = server_list;
    connection_args.server_list_size = &server_list_size;

    // Create thread for UDP socket
    pthread_t thread_id;
    if (pthread_create(&thread_id, NULL, handleConnection, (void*) &connection_args) < 0)
        dieWithMessage(__FILE__, __LINE__, "error: pthread_create()");

    //fprintf(stdout, "$ ");
    while(getcmd(buffer, sizeof(buffer)) >= 0){
        parsecmd(buffer, cmd);
        runcmd(dns_list, &dns_list_size, server_list, &server_list_size, cmd);
    }
    
    return 0;
}