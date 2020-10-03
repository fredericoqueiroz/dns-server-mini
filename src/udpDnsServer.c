#include <stdio.h>
#include <unistd.h>
#include "lib/udpServerUtility.h"

#include <pthread.h>

int main(int argc, char **argv){

    // Check parameter
    if(argc < 2 || argc > 3)
        dieWithMessage(__FILE__, __LINE__, "Sintaxe de Uso: %s <Server Port> [startup_file]", argv[0]);

    
    Host dns_list[MAX_HOSTS];
    memset(dns_list, '\0', sizeof(dns_list));
    int dns_list_size = 0;

    //fprintf(stdout, "sizeof(dns_list): %ld\n", sizeof(dns_list));

    pthread_t thread_id;

    char buffer[254];

    char *service = argv[1];
/* 
    char *startup_file;
    if(argc == 3){
        startup_file = argv[2];
        FILE *fd;
        fd = fopen(startup_file, "r");
        if(startup_file == NULL)
            fprintf(stdout, "Can not open file %s", startup_file);
        else{
            while(!feof(startup_file)){
                fscanf(startup_file, "%s %s", dns_list[dns_list_size].hostname,);
                dns_list_size++;
            }
        }
    }
 */
    
    // Create thread for UDP socket
    if (pthread_create(&thread_id, NULL, handleConnection, (void*) service) < 0)
        dieWithMessage(__FILE__, __LINE__, "error: pthread_create(): %s",strerror(errno));

    //const char delim[2] = ""
    char *cmd[3];
    
    while(getcmd(buffer, sizeof(buffer)) >= 0){
        parsecmd(buffer, cmd);
        /* fprintf(stdout, "cmd[0]: %s\n", cmd[0]);
        fprintf(stdout, "cmd[1]: %s\n", cmd[1]);
        fprintf(stdout, "cmd[2]: %s\n", cmd[2]); */
        runcmd(dns_list, &dns_list_size, cmd);
        
    }
    
    return 0;
}