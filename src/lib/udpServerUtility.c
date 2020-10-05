#include "udpServerUtility.h"

void dieWithMessage(const char * file_name,int line_number, const char * format, ...){
  va_list vargs;
  va_start(vargs, format);
  fprintf(stderr, "%s:%d: ",file_name, line_number);
  vfprintf(stderr, format, vargs);
  fprintf (stderr, ".\n");
  va_end (vargs);
  exit (EXIT_FAILURE);
}

void printSocketAddress(const struct sockaddr *address, FILE *stream) {
  // Test for address and stream
  if (address == NULL || stream == NULL)
    return;

  void *numericAddress; // Pointer to binary address
  // Buffer to contain result (IPv6 sufficient to hold IPv4)
  char addrBuffer[INET6_ADDRSTRLEN];
  in_port_t port; // Port to print
  // Set pointer to address based on address family
  switch (address->sa_family) {
  case AF_INET:
    numericAddress = &((struct sockaddr_in *) address)->sin_addr;
    port = ntohs(((struct sockaddr_in *) address)->sin_port);
    break;
  case AF_INET6:
    numericAddress = &((struct sockaddr_in6 *) address)->sin6_addr;
    port = ntohs(((struct sockaddr_in6 *) address)->sin6_port);
    break;
  default:
    fputs("[unknown type]", stream);    // Unhandled type
    return;
  }
  // Convert binary to printable address
  if (inet_ntop(address->sa_family, numericAddress, addrBuffer,
      sizeof(addrBuffer)) == NULL)
    fputs("[invalid address]", stream); // Unable to convert
  else {
    fprintf(stream, "%s", addrBuffer);
    if (port != 0)                // Zero not valid in any socket addr
      fprintf(stream, "-%u", port);
  }
  
}

int setupServerSocket(const char *service){

  // Construct the server address structure
  struct addrinfo addrCriteria;                   // criteria for address match
  memset(&addrCriteria, 0, sizeof(addrCriteria)); // empty struct
  addrCriteria.ai_family = AF_UNSPEC;             // IPv4 or IPv6 (any address family)
  addrCriteria.ai_flags = AI_PASSIVE;             // any address/port
  addrCriteria.ai_socktype = SOCK_DGRAM;          // only datagram socket
  addrCriteria.ai_protocol = IPPROTO_UDP;         // onde UPD protocol socket

  // Get address
  struct addrinfo *servAddr;
  int rtnAddrInfo = getaddrinfo(NULL, service, &addrCriteria, &servAddr);
  if(rtnAddrInfo != 0)
      dieWithMessage(__FILE__, __LINE__, "error: getaddrinfo(): %s", gai_strerror(rtnAddrInfo));

  int serverSocket = -1;
  for(struct addrinfo *addr = servAddr; addr != NULL; addr = addr->ai_next){
      // Create UDP stream socket
      serverSocket = socket(servAddr->ai_family, servAddr->ai_socktype, servAddr->ai_protocol);
      if(serverSocket < 0)
          continue; // try next address

      // Bind to the local address
      if((bind(serverSocket, servAddr->ai_addr, servAddr->ai_addrlen) != 0)){
          dieWithMessage(__FILE__, __LINE__, "error: bind(): %s",strerror(errno));
      }
      else {
          // Print socket local address
          struct sockaddr_storage localAddr;
          socklen_t addrSize = sizeof(localAddr);
          if(getsockname(serverSocket, (struct sockaddr *) &localAddr, &addrSize) < 0)
              dieWithMessage(__FILE__, __LINE__, "error: getsockname(): %s",strerror(errno));
          
          fprintf(stdout, "Binding to ");
          printSocketAddress((struct sockaddr *) &localAddr, stdout);
          fprintf(stdout,"\n");
          break;
      }

      close(serverSocket); // close and try again
      serverSocket = -1;
  }

  freeaddrinfo(servAddr);
  return serverSocket;
}

void *handleConnection(void *arguments){

  Args *args = (Args *) arguments;

  int serverSocket = setupServerSocket(args->service);
  if(serverSocket < 0)
      dieWithMessage(__FILE__, __LINE__, "error: setupServerSocket(): %s",strerror(errno));
  
  while(1){
    struct sockaddr_storage clientAddr;
    socklen_t clientAddrLen = sizeof(clientAddr);

    // Block until receive message from a client
    char buffer[BUFFER_SIZE]; // I/O buffer
    memset(buffer, 0, sizeof(buffer));

    // Size of received message
    ssize_t numBytesRcvd = recvfrom(serverSocket, buffer, BUFFER_SIZE, 0,
     (struct sockaddr *) &clientAddr, &clientAddrLen);
    
    if(numBytesRcvd < 0)
      dieWithMessage(__FILE__, __LINE__, "error: recvfrom(): %s",strerror(errno));

    fprintf(stdout, "Handling request from server ");
    printSocketAddress((struct sockaddr *) &clientAddr, stdout);
    fprintf(stdout, "\n");

    // SEARCH FUNCTION
    Host host;
    host.ip_adrr[0] = '\0';
    if(buffer[0] == '1'){ // Check message type
      char * ip_adrr = buffer + 1;
      strcpy(host.hostname, ip_adrr);
      fprintf(stdout, "Searching for %s IP Address...\n", host.hostname);
      search(&host, args->dns_list, args->dns_list_size, args->server_list, args->server_list_size);

      memset(buffer, 0, sizeof(buffer));

      if(host.ip_adrr[0] == '\0'){
        strcpy(buffer, "2-1");
        fprintf(stdout, "IP not found\n");
      }
      else{
        buffer[0] = '2';
        strcat(buffer, host.ip_adrr);
        fprintf(stdout, "IP found: %s\n", host.ip_adrr);
      }

      fprintf(stdout, "Sending response to server ");
      printSocketAddress((struct sockaddr *) &clientAddr, stdout);
      fprintf(stdout, "\n");

      ssize_t numBytesSent = sendto(serverSocket, buffer, sizeof(buffer), 0,
        (struct sockaddr *) &clientAddr, sizeof(clientAddr));
      
      if(numBytesSent < 0)
        dieWithMessage(__FILE__, __LINE__, "error: sendto(): %s",strerror(errno));
    }
  }
  // Not Reached
}

int getcmd(char *buf, int nbuf){
  fprintf(stdout, "\n");
  if(isatty(fileno(stdin)))
      fprintf(stdout, "$ ");
  memset(buf, 0, nbuf);
  fgets(buf, nbuf, stdin);
  if(buf[0] == 0)
      return -1;
  return 0;
}

void parsecmd(char *buf, char **tokens){
  int idx = 0;
  char *token;
  token = strtok(buf, " \t\n\r\f");
  while(token != NULL){
    tokens[idx] = token;
    idx++;
    token = strtok(NULL, " \t\n\r\f");
  }
  tokens[idx] = NULL;
}

void runcmd(Host *dns_list, int *dns_list_size, Server *server_list, int *server_list_size, char **cmd){
  Host host;
  Server server;
  if(strcmp(cmd[0], "add") == 0){
    strcpy(host.hostname, cmd[1]);
    strcpy(host.ip_adrr, cmd[2]);
    add_host(dns_list, dns_list_size, host);
  }
  else if(strcmp(cmd[0], "search") == 0){
    strcpy(host.hostname, cmd[1]);
    search(&host, dns_list, dns_list_size, server_list, server_list_size);
    if(host.ip_adrr[0] == '\0'){
      fprintf(stdout, "Hostname %s not found", host.hostname);
    }
    else{
      fprintf(stdout, "IP: %s\n", host.ip_adrr);
    }
  }
  else if(strcmp(cmd[0], "link") == 0){
    strcpy(server.ip_adrr, cmd[1]);
    strcpy(server.service, cmd[2]);
    link_server(server_list, server_list_size, server);
  }
  else if(strcmp(cmd[0], "hostlist") == 0){
    print_hostlist(dns_list, *dns_list_size);
  }
  else if(strcmp(cmd[0], "serverlist") == 0){
    print_serverlist(server_list, *server_list_size);
  }
  else{
    fprintf(stdout, "Unknown command: %s\n", cmd[0]);
  }
}

int find_ip(Host *dns_list, int *dns_list_size, Host host){
  int i;
  for(i=0; i<*dns_list_size; i++){
    if(strcmp(dns_list[i].hostname, host.hostname) == 0){
      return i; // Return the host index found
    }
  }
  return -1; // Host not found on dns_list
}

void add_host(Host *dns_list, int *dns_list_size, Host host){
  int i;
  int host_idx = -1;
  host_idx = find_ip(dns_list, dns_list_size, host);
  if(host_idx == -1){
    stpcpy(dns_list[*dns_list_size].hostname, host.hostname);
    strcpy(dns_list[*dns_list_size].ip_adrr, host.ip_adrr);
    fprintf(stdout,"New Host added: Hostname [%s]\tIP [%s]\n", dns_list[*dns_list_size].hostname, dns_list[*dns_list_size].ip_adrr);
    *dns_list_size += 1;
  }
  else{
    strcpy(dns_list[host_idx].ip_adrr, host.ip_adrr);
    fprintf(stdout,"Host address updated: Hostname [%s]\tIP [%s]\n", 
    dns_list[host_idx].hostname, dns_list[host_idx].ip_adrr);
  }
}

void print_hostlist(Host *dns_list, int dns_list_size){
  int i;
  fprintf(stdout, "[%d] known host(s) -------------------------------------------------\n", dns_list_size);
  for(i=0; i<dns_list_size; i++){
    fprintf(stdout, "%d - Hostname: %s \t IP: %s\n", i+1, dns_list[i].hostname, dns_list[i].ip_adrr);
  }
  fprintf(stdout, "--------------------------------------------------------------------\n");
}

void link_server(Server *server_list, int *server_list_size, Server server){
  int i;
  strcpy(server_list[*server_list_size].ip_adrr, server.ip_adrr);
  strcpy(server_list[*server_list_size].service, server.service);
  fprintf(stdout, "New server link: IP [%s]\tPort [%s]\n", server_list[*server_list_size].ip_adrr, server_list[*server_list_size].service);
  *server_list_size += 1;
}

void print_serverlist(Server *server_list, int server_list_size){
  int i;
  fprintf(stdout, "[%d] known server(s) -----------------------------------------------\n", server_list_size);
  for(i=0; i<server_list_size; i++){
    fprintf(stdout, "%d - IP: %s \t Port: %s\n", i+1, server_list[i].ip_adrr, server_list[i].service);
  }
  fprintf(stdout, "--------------------------------------------------------------------\n");
}

void search(Host *host, Host *dns_list, int *dns_list_size, Server *server_list, int *server_list_size){
  int i;
  int host_idx = -1;
  host_idx = find_ip(dns_list, dns_list_size, *host);
  if(host_idx != -1){ // IP found on hostlist
    //fprintf(stdout, "%s IP: %s\n", dns_list[host_idx].hostname, dns_list[host_idx].ip_adrr);
    strcpy(host->ip_adrr, dns_list[host_idx].ip_adrr);
  }
  else{ // Send to linked servers
    for(i=0; i<*server_list_size; i++){
      struct addrinfo addrCriteria;                   // criteria for address match
      memset(&addrCriteria, 0, sizeof(addrCriteria)); // empty struct
      addrCriteria.ai_family = AF_UNSPEC;             // IPv4 or IPv6 (any address family)
      addrCriteria.ai_socktype = SOCK_DGRAM;          // only datagram socket
      addrCriteria.ai_protocol = IPPROTO_UDP;         // onde UPD protocol socket
      
      // Get address
      struct addrinfo *servAddr;
      int rtnAddrInfo = getaddrinfo(server_list[i].ip_adrr, server_list[i].service, &addrCriteria, &servAddr);
      if(rtnAddrInfo != 0)
        dieWithMessage(__FILE__, __LINE__, "error: getaddrinfo(): %s", gai_strerror(rtnAddrInfo));
      
      // Create a datagram/UDP socket
      int serverSocket = socket(servAddr->ai_family, servAddr->ai_socktype, servAddr->ai_protocol);
      if(serverSocket < 0)
        dieWithMessage(__FILE__, __LINE__, "error: socket(): %s",strerror(errno));
      
      // Send request to the server
      char request[MAX_HOSTNAME_LEN + 1];
      strcpy(request, "1");
      strncat(request, host->hostname, MAX_HOSTNAME_LEN);
      //fprintf(stdout, "Request message: %s\n", request);
      ssize_t numBytes = sendto(serverSocket, request, sizeof(request), 0, servAddr->ai_addr, servAddr->ai_addrlen);
      if(numBytes < 0)
        dieWithMessage(__FILE__, __LINE__, "error: sendto(): %s",strerror(errno));
      else if(numBytes != sizeof(request))
        dieWithMessage(__FILE__, __LINE__, "error: sendto(): unexpected number of bytes");
      
      // Receive a response
      char response[MAX_IP_LEN + 1];
      struct sockaddr_storage fromAddr; // Source address of server
      socklen_t fromAddrLen = sizeof(fromAddr);
      numBytes = recvfrom(serverSocket, response, sizeof(response), 0,
        (struct sockaddr *) &fromAddr, &fromAddrLen);
      if(numBytes < 0)
        dieWithMessage(__FILE__, __LINE__, "error: recvfrom(): %s",strerror(errno));
    
      if(strcmp(response, "2-1") == 0){
        //fprintf(stdout, "Hostname %s not found", host.hostname);
        strcpy(host->ip_adrr, "\0");
      }
      else{
        char *ip_adrr = response + 1;
        strcpy(host->ip_adrr, ip_adrr);
        //fprintf(stdout, "%s IP: %s\n", host.hostname, host.ip_adrr);
      }

      freeaddrinfo(servAddr);

      //close(serverSocket);
    }
  }
}