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

void *handleConnection(void *service){

  int serverSocket = setupServerSocket(service);
  if(serverSocket < 0)
      dieWithMessage(__FILE__, __LINE__, "error: setupServerSocket(): %s",strerror(errno));
  
  while(1){
    
  }

}

int getcmd(char *buf, int nbuf){
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